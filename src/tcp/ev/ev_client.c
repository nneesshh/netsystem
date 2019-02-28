#include "ev_client.h"
#include <malloc.h>

/**------------------------------------------------------------------------------
*
*/
struct event_base *
	ev_loop_new_(int flag, int use_threads) {
	struct event_base *base = NULL;
	struct event_config *cfg = NULL;

#ifdef WIN32
	WSADATA WSAData;
	DWORD Ret;
	if ((Ret = WSAStartup(MAKEWORD(2, 2), &WSAData)) != 0) {
		fprintf(stderr, "WSAStartup failed with error %d!\n", Ret);
		return (0);
	}
#else
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return (0);
#endif

	if (use_threads > 0) {
		// thread code
#ifdef _WIN32
		evthread_use_windows_threads();
#else
		evthread_use_pthreads();
#endif
	}

	// custom event_base_new()
	cfg = event_config_new();
	if (cfg) {
		//event_config_require_features(cfg, EV_FEATURE_O1);
		event_config_set_flag(cfg, flag);

		SYSTEM_INFO si;
		GetSystemInfo(&si);
		event_config_set_num_cpus_hint(cfg, si.dwNumberOfProcessors);
		base = event_base_new_with_config(cfg);

		event_config_free(cfg);
	}
	return base;
}

/**------------------------------------------------------------------------------
*
*/
void
ev_loop_delete_(struct event_base *base, unsigned int seconds) {
	struct timeval delay = { seconds, 0 };
	printf("exiting cleanly in %d seconds -- base(0x%08Ix)\n",
		delay.tv_sec, (uintptr_t)base);

	/* exit */
	event_base_loopexit(base, &delay);
	event_base_dispatch(base);
	event_base_free(base);
}

/**------------------------------------------------------------------------------
*
*/
int
ev_loop_run_(struct event_base *base) {
	return event_base_dispatch(base);
}

/**------------------------------------------------------------------------------
*
*/
int
ev_loop_run_once_(struct event_base *base, int flags) {
	return event_base_loop(base, flags);
}

/**------------------------------------------------------------------------------
*
*/
void
ev_close_(struct bufferevent *sockimpl) {
	if (sockimpl) {
		/* don't call "evutil_closesocket(client->fd);" directly, because fd is managed by buffered event */
		bufferevent_free(sockimpl);
	}
}

/**------------------------------------------------------------------------------
*
*/
struct evutil_addrinfo *
	ev_getaddrinfoforhost_(const char *hostname, ev_uint16_t port) {
	struct evutil_addrinfo *answer = NULL;

	char port_buf[6];
	struct evutil_addrinfo hints;
	int err;

	/* Convert the port to decimal. */
	evutil_snprintf(port_buf, sizeof(port_buf), "%d", (int)port);

	/* Build the hints to tell getaddrinfo how to act. */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; /* v4 or v6 is fine. */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP; /* we want a TCP socket */

									 /* Only return addresses we can use. */
	hints.ai_flags = EVUTIL_AI_ADDRCONFIG;

	/* Look up the hostname. */
	err = evutil_getaddrinfo(hostname, port_buf, &hints, &answer);
	if (0 != err) {
		fprintf(stderr, "Error while resolving '%s': '%s'!\n",
			hostname, evutil_gai_strerror(err));
		return NULL;
	}

	/* If there was no error, we should have at least one answer. */
	/* Just use the first answer. */
	return answer;
}

/**------------------------------------------------------------------------------
*
*/
client_handle_t *
ev_connect_(struct event_base *base, const char *ip, unsigned short port,
	bufferevent_data_cb on_recv,
	bufferevent_data_cb on_send,
	bufferevent_event_cb on_error) {
	
	client_handle_t *clnt = NULL;
	int r;

	/* create client handle */
	clnt = malloc(sizeof(client_handle_t));
	clnt->_fd = -1;

	memset(&clnt->_sin, 0, sizeof(clnt->_sin));
	clnt->_sin.sin_family = AF_INET;
	clnt->_sin.sin_port = htons(port);

	/* parse ip. */
	if (evutil_inet_pton(AF_INET, ip, &clnt->_sin.sin_addr) <= 0) {
		fprintf(stderr, "Invalid IP(%s)!\n", ip);
		event_base_loopbreak(base);
		free(clnt);
		return NULL;
	}

	/* start connect */
	printf("Client start to connect -- IP=(%s), port=(%d)...\n", ip, port);
	r = ev_real_connect_(clnt, base, on_recv, on_send, on_error);
	if (0 != r) {
		fprintf(stderr, "Connect failed -- IP=(%s), port=(%d)!\n", ip, port);
		free(clnt);
		return NULL;
	}

	/* */
	clnt->_connimpl = NULL;

	/* buffer */
	clnt->_bb = bip_buf_create(TCP_STREAM_READ_SIZE);

	/* add any custom code anywhere from here to the end of this function
	* to initialize your application-specific attributes in the client struct. */

	return clnt;
}

/**------------------------------------------------------------------------------
*
*/
int
ev_real_connect_(client_handle_t *clnt,
	struct event_base *base,
	bufferevent_data_cb on_recv,
	bufferevent_data_cb on_send,
	bufferevent_event_cb on_error) {

	/* create the buffered event. */
	struct bufferevent *sockimpl = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
	if (sockimpl) {
		int r = bufferevent_socket_connect(sockimpl, (struct sockaddr *)&clnt->_sin, sizeof(clnt->_sin));
		if (0 == r) {
			/* callback */
			bufferevent_setcb(sockimpl, on_recv, on_send, on_error, clnt);

			/* enable the buffered event. */
			bufferevent_enable(sockimpl, EV_READ | EV_WRITE);

			clnt->_sockimpl = sockimpl;
			return 0;
		}
		else {
			bufferevent_free(sockimpl);
			return -1;
		}
	}

	return -2;
}

/**------------------------------------------------------------------------------
*
*/
struct event *
	ev_create_timer_(struct event_base *base
		, event_callback_fn cb
		, void *arg
		, int one_time) {
	struct event *timer_handle = NULL;

	//
	if (0 == one_time) {
		// persist timer
		timer_handle = event_new(base, -1, EV_PERSIST, cb, arg);
	}
	else {
		timer_handle = event_new(base, -1, 0, cb, arg);
	}
	return timer_handle;
}

/**------------------------------------------------------------------------------
*
*/
int
ev_start_timer_(struct event *timer_handle, struct timeval *timeout) {
	int r = evtimer_add(timer_handle, timeout);
	if (r<0) {
		fprintf(stderr, "Error start timer!\n");
	}
	return r;
}

/**------------------------------------------------------------------------------
*
*/
void
ev_stop_and_free_timer_(struct event *timer_handle) {
	evtimer_del(timer_handle);
	free(timer_handle);
}

/** -- EOF -- **/