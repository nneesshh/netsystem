#pragma once

#include "../tcp_def.h"

#ifdef __cplusplus 
extern "C" {
#endif

#include "../../base/platform_types.h"

#include "client_handle.h"
#include "server_handle.h"

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN 1
# include <winsock2.h>
# include <WS2tcpip.h>
#endif

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>

#ifndef WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/types.h>  
#include <sys/socket.h>
#endif

#include <event2/thread.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>
#include <event2/bufferevent_ssl.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/http.h>

/** **/
MY_EXTERN struct event_base *	ev_loop_new_(int flag, int use_threads);
MY_EXTERN void					ev_loop_delete_(struct event_base *base, unsigned int seconds);
MY_EXTERN int					ev_loop_run_(struct event_base *base);
MY_EXTERN int					ev_loop_run_once_(struct event_base *base, int flags);

MY_EXTERN void					ev_close_(struct bufferevent *sockimpl);

MY_EXTERN struct evutil_addrinfo *	ev_getaddrinfoforhost_(const char *hostname, ev_uint16_t port);

MY_EXTERN client_handle_t *		ev_connect_(struct event_base *base, const char *ip, unsigned short port,
	bufferevent_data_cb on_recv,
	bufferevent_data_cb on_send,
	bufferevent_event_cb on_error);

MY_EXTERN int					ev_real_connect_(
	client_handle_t *clnt,
	struct event_base *base,
	bufferevent_data_cb on_recv,
	bufferevent_data_cb on_send,
	bufferevent_event_cb on_error);

MY_EXTERN struct event *		ev_create_timer_(struct event_base *base,
	event_callback_fn cb,
	void *arg,
	int one_time);

MY_EXTERN int					ev_start_timer_(struct event *timer_handle, struct timeval *timeout);
MY_EXTERN void					ev_stop_and_free_timer_(struct event *timer_handle);

#ifdef __cplusplus 
}
#endif 

/*EOF*/