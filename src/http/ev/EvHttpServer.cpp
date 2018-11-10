//------------------------------------------------------------------------------
//  EvHttpServer.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "EvHttpServer.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../../tcp/ev/ev_client.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

/************************************************************************/
/* callback                                                             */
/************************************************************************/

static void
initcb(struct evhttp_server *srvr) {
	CEvHttpServer *http_server = (CEvHttpServer *)srvr->_userdata;
	if (http_server && http_server->_initHandler) {
		http_server->_initHandler(http_server, nullptr);
	}
}

static void
gencb(struct evhttp_request *request, void *arg) {
	const char *cmdtype;
#ifdef _DEBUG
	struct evkeyvalq *headers;
	struct evkeyval *header;
#endif

	const char *uri = evhttp_request_get_uri(request);
	struct evhttp_uri *decoded = nullptr;
	const char *path;
	char *decoded_path;
	int fd = -1;

	struct evbuffer *input;
	char cbuf[4096];
	unsigned int n, drain_size;
	CEvHttpServer *http_server;

	switch (evhttp_request_get_command(request)) {
	case EVHTTP_REQ_GET: cmdtype = "GET"; break;
	case EVHTTP_REQ_POST: cmdtype = "POST"; break;
	case EVHTTP_REQ_HEAD: cmdtype = "HEAD"; break;
	case EVHTTP_REQ_PUT: cmdtype = "PUT"; break;
	case EVHTTP_REQ_DELETE: cmdtype = "DELETE"; break;
	case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
	case EVHTTP_REQ_TRACE: cmdtype = "TRACE"; break;
	case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
	case EVHTTP_REQ_PATCH: cmdtype = "PATCH"; break;
	default: cmdtype = "unknown"; break;
	}

#ifdef _DEBUG
	printf("Received a %s request for %s\nHeaders:\n", cmdtype, uri);

	headers = evhttp_request_get_input_headers(request);
	for (header = headers->tqh_first; header;
		header = header->next.tqe_next) {
		printf("  %s: %s\n", header->key, header->value);
	}
#endif

	/* Decode the URI */
	decoded = evhttp_uri_parse(uri);
	if (!decoded) {
		printf("It's not a good URI. Sending BADREQUEST\n");
		evhttp_send_error(request, HTTP_BADREQUEST, 0);
		return;
	}

	/* Let's see what path the user asked for. */
	path = evhttp_uri_get_path(decoded);
	if (!path) path = "/";

	/* We need to decode it, to see what path the user really wanted. */
	decoded_path = evhttp_uridecode(path, 0, nullptr);
	if (decoded_path == nullptr)
		goto err;

	/* drain buffer */
	input = evhttp_request_get_input_buffer(request);
	n = 0;

	drain_size = evbuffer_get_length(input);
	if (drain_size < sizeof(cbuf) - 1) {
		n = evbuffer_remove(input, cbuf, sizeof(cbuf) - 1);
	}

#ifdef _DEBUG
	printf("Size: (%d/%d)\n", n, drain_size);
#endif

	/* process request */
	http_server = (CEvHttpServer *)arg;
	if (http_server && http_server->_requsetHandler) {
		http_server->_requsetHandler(http_server, decoded_path, request, cbuf, n);
	}

	goto done;

err:
	evhttp_send_error(request, 404, "Document was not found");
	if (fd >= 0)
		close(fd);

done:
	evhttp_uri_free(decoded);
	free(decoded_path);
	return;
}

//------------------------------------------------------------------------------
/**

*/
CEvHttpServer::CEvHttpServer(unsigned short nPort, StdLog *pLog)
	: _closed(false)
	, _refLog(pLog) {
	memset(&_srvr, 0, sizeof(struct evhttp_server));

	_srvr._fd = 0;
	_srvr._port = nPort;
	//_srvr._base = (struct event_base *)ev_loop_new_(EVENT_BASE_FLAG_STARTUP_IOCP);
	_srvr._base = (struct event_base *)ev_loop_new_(0, 0);
	_srvr._initcb = initcb;
	_srvr._http = nullptr;
	_srvr._userdata = this;
}

//------------------------------------------------------------------------------
/**

*/
CEvHttpServer::~CEvHttpServer() {
	ev_loop_delete_((struct event_base *)_srvr._base, 3);
}

//------------------------------------------------------------------------------
/**

*/
int
CEvHttpServer::OnInit() {
	//
	ev_httpd_init(&_srvr);
	ev_httpd_set_gencb(&_srvr, gencb, this);
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
void
CEvHttpServer::OnDelete() {
	if (!_closed)	{
		_closed = true;
		Close();
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CEvHttpServer::OnUpdate() {
	ev_loop_run_once_(_srvr._base, EVLOOP_NONBLOCK);
}

//------------------------------------------------------------------------------
/**

*/
void
CEvHttpServer::SendReply(void *req_handle, int code, const char *reason, const char *str) {
	SendReply(req_handle, code, reason, str, strlen(str));
}

//------------------------------------------------------------------------------
/**

*/
void
CEvHttpServer::SendReply(void *req_handle, int code, const char *reason, const char *data, size_t len) {
	struct evhttp_request *request = (struct evhttp_request *)req_handle;

	//
	if (nullptr == data) {
		evhttp_send_reply(request, code, reason, nullptr);
	}
	else {
		struct evbuffer *buf;
		buf = evbuffer_new();

		/*  Response the client  */
		evbuffer_add(buf, data, len);
		evhttp_send_reply(request, code, reason, buf);

		/*  Release the memory  */
		evbuffer_free(buf);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CEvHttpServer::Close() {
	ev_httpd_exit(&_srvr);
}

//------------------------------------------------------------------------------
/**

*/
bool
CEvHttpServer::IsClosed() {
	return _closed;
}

/** -- EOF -- **/