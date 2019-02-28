#pragma once

#ifdef __cplusplus 
extern "C" {
#endif 

#include "../../base/netsystem_extern.h"

#include "server_handle.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN 1
# include <winsock2.h>
# include <WS2tcpip.h>
#endif

#ifndef WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#include <event2/thread.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

#include "ev_client.h"

#define EV_SERVER_ONLINES_MAX 8192

/* */
struct evsrvr_impl_t {
	struct event_base			*_base;
	struct evconnlistener		*_listener;
	int							_server_closed;

	server_accept_client_cb		_acceptcb;

	bufferevent_data_cb			_readcb;
	bufferevent_data_cb			_writecb;
	bufferevent_event_cb		_errorcb;
};

/** **/
MY_NETSYSTEM_EXTERN struct event_base *	ev_server_get_base_(server_handle_t *srvr);

MY_NETSYSTEM_EXTERN int					ev_server_init_(server_handle_t *srvr, struct evsrvr_impl_t *srvr_impl);
MY_NETSYSTEM_EXTERN void				ev_server_dispose_(server_handle_t *srvr);

MY_NETSYSTEM_EXTERN void				ev_server_enable_(server_handle_t *srvr);
MY_NETSYSTEM_EXTERN void				ev_server_disable_(server_handle_t *srvr);

#ifdef __cplusplus 
}
#endif 

/*EOF*/