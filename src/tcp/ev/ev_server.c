#include "ev_server.h"

/************************************************************************/
/* callback                                                             */
/************************************************************************/
static void listener_accept_cb(struct evconnlistener *, evutil_socket_t
	, struct sockaddr *, int socklen, void *);
static void listener_accept_errorcb(struct evconnlistener *, void *);

/**------------------------------------------------------------------------------
*
*/
static void
listener_accept_cb(struct evconnlistener *listener, evutil_socket_t fd
	, struct sockaddr *sa, int socklen, void *arg) {

	server_handle_t *srvr = (server_handle_t *)arg;
	struct evsrvr_impl_t *srvr_impl = (struct evsrvr_impl_t *)srvr->_impl;
	struct event_base *base = srvr_impl->_base;

	struct bufferevent *bev;
	int client_fd;
	client_handle_t *clnt = NULL;
	int r;
	char yes = 1;
	char *peerip;

	/* create the buffered event. */
	//bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev) {
		fprintf(stderr, "Error constructing bufferevent!\n");
		event_base_loopbreak(base);
		return;
	}

	/* create client object. */
	client_fd = bufferevent_getfd(bev);
	if (-1 == setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes))) {
		fprintf(stderr, "[fd(%d)]setsockopt(TCP_NODELAY) failed, err=%s!\n", client_fd, strerror(errno));
		evutil_closesocket(client_fd);
		return;
	}

	/* create client handle */
	clnt = (client_handle_t *)malloc(sizeof(client_handle_t));
	clnt->_fd = client_fd;

	/* enable the buffered event. */
	bufferevent_setcb(bev, srvr_impl->_readcb, srvr_impl->_writecb, srvr_impl->_errorcb, clnt);
	bufferevent_enable(bev, EV_READ | EV_WRITE);
	bufferevent_setwatermark(bev, EV_READ, 8, 256 * 1024);

	/* add any custom code anywhere from here to the end of this function
	* to initialize your application-specific attributes in the client struct. */
	clnt->_sockimpl = bev;
	clnt->_connimpl = NULL;

	peerip = (char *)inet_ntoa(((struct sockaddr_in *)sa)->sin_addr);

	r = 0;
	if (srvr_impl->_acceptcb) {
		r = srvr_impl->_acceptcb(srvr, clnt, peerip);
	}

	/* server add client */
	if (0 != r) {
		evutil_closesocket(clnt->_fd);
		free(clnt);
	}

	/* */
	clnt->_bb = bip_buf_create(TCP_STREAM_READ_SIZE);
}

/**------------------------------------------------------------------------------
*
*/
static void
listener_accept_errorcb(struct evconnlistener *listener, void *arg) {
	struct event_base *base = evconnlistener_get_base(listener);
	int err = EVUTIL_SOCKET_ERROR();
	fprintf(stderr, "Got an erro %d(%s) on the listener, shutting down...\n", err, evutil_socket_error_to_string(err));
	event_base_loopexit(base, NULL);
}

/**------------------------------------------------------------------------------
*
*/
struct event_base *
	ev_server_get_base_(server_handle_t *srvr) {
	struct evsrvr_impl_t *srvr_impl = (struct evsrvr_impl_t *)srvr->_impl;
	return srvr_impl->_base;
}

/**------------------------------------------------------------------------------
*
*/
int
ev_server_init_(server_handle_t *srvr, struct evsrvr_impl_t *srvr_impl) {
	struct event_config *cfg = NULL;
	struct sockaddr_in sin;

	srvr->_impl = srvr_impl;
	if (!srvr_impl->_base) {
		fprintf(stderr, "Could not initialize libevent!\n");
		free(srvr_impl);
		return 1;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(srvr->_port);

	srvr_impl->_listener = evconnlistener_new_bind(
		srvr_impl->_base, listener_accept_cb, (void *)srvr,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
		(struct sockaddr*)&sin,
		sizeof(sin));

	if (!srvr_impl->_listener) {
		fprintf(stderr, "Error: Could not create a listener on port(%d), the socket may be in used!\n", srvr->_port);
		exit(-1);
	}

	evconnlistener_set_error_cb(srvr_impl->_listener, listener_accept_errorcb);

	srvr->_fd = evconnlistener_get_fd(srvr_impl->_listener);
	fprintf(stderr, "server init on port(%d) fd(%d)...\n", 
		srvr->_port, srvr->_fd);
	return 0;
}

/**------------------------------------------------------------------------------
*
*/
void
ev_server_dispose_(server_handle_t *srvr) {
	struct evsrvr_impl_t *srvr_impl = (struct evsrvr_impl_t *)srvr->_impl;
	if (srvr_impl->_listener) {
		evconnlistener_free(srvr_impl->_listener);	// free listener before loop exit to block accept event
	}
	free(srvr_impl);
	srvr->_impl = NULL;
}

/**------------------------------------------------------------------------------
*
*/
void
ev_server_enable_(server_handle_t *srvr) {
	struct evsrvr_impl_t *srvr_impl = (struct evsrvr_impl_t *)srvr->_impl;
	if (srvr_impl && srvr_impl->_listener)
		evconnlistener_enable(srvr_impl->_listener);
}

/**------------------------------------------------------------------------------
*
*/
void
ev_server_disable_(server_handle_t *srvr) {
	struct evsrvr_impl_t *srvr_impl = (struct evsrvr_impl_t *)srvr->_impl;
	if (srvr_impl && srvr_impl->_listener)
		evconnlistener_disable(srvr_impl->_listener);
}

/** -- EOF -- **/