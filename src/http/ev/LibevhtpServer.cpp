//------------------------------------------------------------------------------
//  LibevhtpServer.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "LibevhtpServer.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../../tcp/ev/ev_client.h"

#include "../../netsystem/RootContextDef.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

/** **/
static evthr_t *
get_request_thr(evhtp_request_t *request) {
	evhtp_connection_t * htpconn;
	evthr_t            * thread;

	htpconn = evhtp_request_get_connection(request);
	thread = htpconn->thread;

	return thread;
}

/************************************************************************/
/* callback                                                             */
/************************************************************************/

static void
initcb(libevhtp_server_t *srvr, http_thr_aux_t *aux) {
 	CLibevhtpServer *htp_server = (CLibevhtpServer *)(srvr->_userdata);
 	if (htp_server && htp_server->_initHandler) {
 		htp_server->_initHandler(htp_server, aux);
 	}
}

static void
gencb(evhtp_request_t *request, void *arg) {
	evthr_t *thread;
	http_thr_aux_t *thraux;
	evhtp_connection_t *conn;
	evhtp_uri_t *uri;
	struct sockaddr_in *sin;
	char address[1024];

	evbuf_t *ibuf;
	char cbuf[4096];
	size_t n, drain_size;

	CLibevhtpServer *htp_server;
	int r = 0;

	//printf("process_request(%p)\n", request);

	thread = get_request_thr(request);
	if (thread) {
		thraux = (http_thr_aux_t *)evthr_get_aux(thread);
	}

	conn = evhtp_request_get_connection(request);
	uri = request->uri;

	sin = (struct sockaddr_in *)conn->saddr;
	evutil_inet_ntop(AF_INET, &sin->sin_addr, address, sizeof(address));

	/* drain buffer */
	ibuf = request->buffer_in;
	drain_size = evbuffer_get_length(ibuf);
	if (drain_size < sizeof(cbuf) - 1) {
		n = evbuffer_remove(ibuf, cbuf, sizeof(cbuf) - 1);
	}

	/* process request */
	htp_server = (CLibevhtpServer *)arg;
	if (htp_server && htp_server->_requsetHandler) {
		r = htp_server->_requsetHandler(htp_server, uri->path->full, request, cbuf, n);
		if (0 != r) {
			StdLog *pLog = netsystem_get_log();
			if (pLog) {
				pLog->logprint(LOG_LEVEL_WARNING, "[LibevhtpServer->gencb()]!!! process request error -- result(%d)",
					r);
			}
		}
	}
}

struct kvs_userdata {
	IHttpServer *_httpServer;
	std::function<int(void *kvobj)> _kvscb;
};

static int
kvs_iterator(evhtp_kv_t *kvobj, void *arg) {
	struct kvs_userdata *ku = (struct kvs_userdata *)arg;
	return ku->_kvscb(kvobj);
}

//------------------------------------------------------------------------------
/**

*/
CLibevhtpServer::CLibevhtpServer(unsigned short nPort)
	: _closed(false) {

	memset(&_srvr, 0, sizeof(_srvr));

	_srvr._fd = 0;
	_srvr._port = nPort;
	_srvr._base = (struct event_base *)ev_loop_new_(0, 0);
	_srvr._initcb = initcb;
	_srvr._htp = nullptr;
	_srvr._userdata = this;
}

//------------------------------------------------------------------------------
/**

*/
CLibevhtpServer::~CLibevhtpServer() {
	ev_loop_delete_((struct event_base *)_srvr._base, 3);
}

//------------------------------------------------------------------------------
/**

*/
int
CLibevhtpServer::OnInit() {
	//
	libevhtpd_init(&_srvr, 0, 1, 0);
	libevhtpd_set_gencb(&_srvr, gencb, this);
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
void
CLibevhtpServer::OnDelete() {
	if (!_closed)	{
		_closed = true;
		Close();
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CLibevhtpServer::OnUpdate() {
	ev_loop_run_once_(_srvr._base, EVLOOP_NONBLOCK);
}

//------------------------------------------------------------------------------
/**

*/
void
CLibevhtpServer::SendReply(void *req_handle, int code, const char *reason, const char *str) {
	evhtp_request_t *request = (evhtp_request_t *)req_handle;

	if (str) {
		evbuffer_add_printf(request->buffer_out, "%s", str);
	}

	/*  Response the client  */
	evhtp_send_reply(request, (evhtp_res)code);
}

//------------------------------------------------------------------------------
/**

*/
void
CLibevhtpServer::SendReply(void *req_handle, int code, const char *reason, const char *data, size_t len) {
	if (req_handle) {
		evhtp_request_t *request = (evhtp_request_t *)req_handle;

		if (data) {
			evbuffer_add(request->buffer_out, data, len);
		}

		/*  Response the client  */
		evhtp_send_reply(request, (evhtp_res)code);
	}
	else {
		//assert(false);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CLibevhtpServer::PauseRequest(void *req_handle) {
	evhtp_request_t *request = (evhtp_request_t *)req_handle;
	evhtp_request_pause(request);
}

//------------------------------------------------------------------------------
/**

*/
void
CLibevhtpServer::ResumeRequest(void *req_handle) {
	evhtp_request_t *request = (evhtp_request_t *)req_handle;
	evhtp_request_resume(request);
}

//------------------------------------------------------------------------------
/**

*/
void
CLibevhtpServer::Close() {
	libevhtpd_exit(&_srvr);
}

//------------------------------------------------------------------------------
/**

*/
bool
CLibevhtpServer::IsClosed() {
	return _closed;
}

//------------------------------------------------------------------------------
/**

*/
bool
CLibevhtpServer::ParseQueryString(void *req_handle, std::string& outQueyString) {
	evhtp_request_t *request = (evhtp_request_t *)req_handle;
	evhtp_uri_t *uri = request->uri;

	if (uri && uri->query) {
		struct kvs_userdata ku;
		ku._httpServer = this;
		ku._kvscb = [&outQueyString](void *kv)->int {

			evhtp_kv_t *kvobj = (evhtp_kv_t *)kv;
			outQueyString.append(kvobj->key).append("=").append(kvobj->val).append("&");
			return 0;
		};

		//
		evhtp_kvs_for_each(uri->query, kvs_iterator, &ku);
		return true;
	}
	return false;
}

/** -- EOF -- **/