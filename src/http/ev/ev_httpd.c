#include "ev_httpd.h"

/**------------------------------------------------------------------------------
 *
 */
int
ev_httpd_init(evhttp_server_t *srvr) {
	struct event_base *evbase = srvr->_base;
	struct evhttp_bound_socket *handle;
	struct evhttp *http = evhttp_new(evbase);

	if (!http) {
		fprintf(stderr, "ev_httpd_init -- couldn't create ev_httpd. Exiting.\n");
		return 1;
	}

	srvr->_fd = 0;
	srvr->_http = http;

	/* Now we tell the evhttp what port to listen on */
	handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", srvr->_port);
	if (!handle) {
		fprintf(stderr, "ev_httpd_init -- couldn't bind to port %d. Exiting.\n",
		    (int)srvr->_port);
		return 1;
	}

	if (srvr->_initcb)
		srvr->_initcb(srvr);

	{
		/* Extract and display the address we're listening on. */
		struct sockaddr_storage ss;
		evutil_socket_t fd;
		ev_socklen_t socklen = sizeof(ss);
		char addrbuf[128];
		void *inaddr;
		const char *addr;
		int got_port = -1;
		fd = evhttp_bound_socket_get_fd(handle);
		memset(&ss, 0, sizeof(ss));
		if (getsockname(fd, (struct sockaddr *)&ss, &socklen)) {
			perror("getsockname() failed");
			return 1;
		}
		if (ss.ss_family == AF_INET) {
			got_port = ntohs(((struct sockaddr_in*)&ss)->sin_port);
			inaddr = &((struct sockaddr_in*)&ss)->sin_addr;
		} else if (ss.ss_family == AF_INET6) {
			got_port = ntohs(((struct sockaddr_in6*)&ss)->sin6_port);
			inaddr = &((struct sockaddr_in6*)&ss)->sin6_addr;
		} else {
			fprintf(stderr, "ev_httpd_init -- Weird address family %d\n",
			    ss.ss_family);
			return 1;
		}
		addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf,
		    sizeof(addrbuf));
		if (addr) {
			printf("ev_httpd_init -- Listening on %s:%d\n", addr, got_port);
		} else {
			fprintf(stderr, "ev_httpd_init -- evutil_inet_ntop failed\n");
			return 1;
		}
	}
	return 0;
}

/**------------------------------------------------------------------------------
 *
 */
void
ev_httpd_exit(evhttp_server_t *srvr) {
	if (srvr) {
		evhttp_free(srvr->_http);
	}
}

/**------------------------------------------------------------------------------
 *
 */
void
ev_httpd_set_cb(evhttp_server_t *srvr, const char *path, ev_httpd_cb cb, void *arg) {
	struct evhttp *http = srvr->_http;
	evhttp_set_cb(http, path, cb, arg);
}

/**------------------------------------------------------------------------------
 *
 */
void
ev_httpd_set_gencb(evhttp_server_t *srvr, ev_httpd_cb cb, void *arg) {
	struct evhttp *http = srvr->_http;
	evhttp_set_gencb(http, cb, arg);
}

/** -- EOF -- **/