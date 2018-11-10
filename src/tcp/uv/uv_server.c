#include "uv_server.h"

/**------------------------------------------------------------------------------
*
*/
static void
alloc_cb(uv_handle_t *stream1, size_t suggested_size, uv_buf_t *buf) {
	client_handle_t *clnt = (client_handle_t *)(stream1->data);
	buf->len = suggested_size;
	buf->base = bip_buf_reserve(clnt->_bb, &buf->len);
}

//------------------------------------------------------------------------------
/**

*/
static void
on_sock_close(uv_handle_t* handle) {
	free(handle);
}

/**------------------------------------------------------------------------------
*
*/
static void
on_connection(uv_stream_t *stream0, int status) {
	server_handle_t *srvr = (server_handle_t *)stream0->data;
	struct uvsrvr_impl_t *srvr_impl = (struct uvsrvr_impl_t *)srvr->_impl;
	uv_loop_t *loop = srvr_impl->_loop;

	uv_stream_t *stream1;
	client_handle_t *clnt = NULL;

	struct sockaddr_in peername;
	int peernamelen;
	char peerip[36];
	int r;

	if (status != 0) {
		fprintf(stderr, "Connect error: %s - %s\n", 
			uv_err_name(status), uv_strerror(status));
		return;
	}

	stream1 = (uv_stream_t *)malloc(sizeof(uv_tcp_t));
	r = uv_tcp_init(loop, (uv_tcp_t*)stream1);
	if (r != 0) {
		fprintf(stderr, "Tcp init error: %s - %s\n",
			uv_err_name(status), uv_strerror(status));
		free(stream1);
		return;
	}

	r = uv_accept(stream0, stream1);
	if (r != 0) {
		fprintf(stderr, "Accept error: %s - %s\n", 
			uv_err_name(status), uv_strerror(status));
		free(stream1);
		return;
	}

	r = uv_read_start(stream1, alloc_cb, srvr_impl->_readcb);
	if (r != 0) {
		fprintf(stderr, "Read start error: %s - %s\n", 
			uv_err_name(status), uv_strerror(status));
		free(stream1);
		return;
	}

	/* create client handle */
	clnt = (client_handle_t *)malloc(sizeof(client_handle_t));
	clnt->_fd = stream1->u.fd;

	/* associate client with stream */
	stream1->data = clnt;

	/* add any custom code anywhere from here to the end of this function
	* to initialize your application-specific attributes in the client struct. */
	clnt->_sockimpl = stream1;
	clnt->_connimpl = 0;

	/* peer ip */
	peernamelen = sizeof(peername);
	memset(&peername, -1, sizeof(peername));
	r = uv_tcp_getpeername((uv_tcp_t*)clnt->_sockimpl, (struct sockaddr *)&peername, &peernamelen);
	if (0 == r) {
		uv_ip4_name(&peername, peerip, sizeof(peerip));
	}
	else {
		peerip[0] = '?';
		peerip[1] = '\0';
	}

	r = 0;
	if (srvr_impl->_acceptcb) {
		r = srvr_impl->_acceptcb(srvr, clnt, peerip);
	}

	/* server add client */
	if (0 != r) {
		uv_close_((uv_handle_t *)clnt->_sockimpl, on_sock_close);
		bip_buf_destroy(clnt->_bb);
		free(clnt);
	}

	/* */
	clnt->_bb = bip_buf_create(TCP_STREAM_READ_SIZE);
}

/**------------------------------------------------------------------------------
 *
 */
uv_loop_t *
uv_server_get_loop_(server_handle_t *srvr) {
	struct uvsrvr_impl_t *srvr_impl = (struct uvsrvr_impl_t *)srvr->_impl;
	return srvr_impl->_loop;
}

/**------------------------------------------------------------------------------
 *
 */
int
uv_server_init_(server_handle_t *srvr, struct uvsrvr_impl_t *srvr_impl) {
	struct sockaddr_in addr;
	uv_tcp_t *sockimpl;
	int r;

	if (!srvr_impl->_loop) {
		fprintf(stderr, "Could not initialize libuv\n");
		free(srvr_impl);
		return 1;
	}

	r = uv_ip4_addr("0.0.0.0", srvr->_port, &addr);
	if (r) {
		fprintf(stderr, "Bad ip addr\n");
		free(srvr_impl);
		return 1;
	}

	sockimpl = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
	r = uv_tcp_init(srvr_impl->_loop, sockimpl);
	if (r) {
		/* TODO: Error codes */
		fprintf(stderr, "Socket creation error\n");
		free(sockimpl);
		free(srvr_impl);
		return 1;
	}

	r = uv_tcp_bind(sockimpl, (const struct sockaddr*) &addr, 0);
	if (r) {
		/* TODO: Error codes */
		fprintf(stderr, "Bind error\n");
		free(sockimpl);
		free(srvr_impl);
		return 1;
	}

	r = uv_listen((uv_stream_t*)sockimpl, SOMAXCONN, on_connection);
	if (r) {
		/* TODO: Error codes */
		fprintf(stderr, "Listen error: %s - %s\n", 
			uv_err_name(r), uv_strerror(r));
		free(srvr_impl);
		return 1;
	}

	sockimpl->data = srvr;

	srvr_impl->_server = (uv_handle_t *)sockimpl;
	srvr_impl->_server_closed = 0;
	srvr->_impl = srvr_impl;

	srvr->_fd = sockimpl->u.fd;
	fprintf(stderr, "server init on port(%d) fd(%d)...\n",
		srvr->_port, srvr->_fd);
	return 0;
}

/**------------------------------------------------------------------------------
 *
 */
void
uv_server_dispose_(server_handle_t *srvr) {
	struct uvsrvr_impl_t *srvr_impl = (struct uvsrvr_impl_t *)srvr->_impl;
	if (srvr_impl->_server) {
		//
		uv_close(srvr_impl->_server, srvr_impl->_closecb);

		//
		while (0 == srvr_impl->_server_closed) {
			uv_loop_run_once_(srvr_impl->_loop);
		}
	}

	//
	free(srvr_impl);
	srvr->_impl = NULL;
}

/** -- EOF -- **/
