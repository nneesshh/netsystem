#ifndef __UV_SERVER_H__
#define __UV_SERVER_H__

#ifdef __cplusplus 
extern "C" { 
#endif 

#include "../../base/netsystem_extern.h"

#include "../ev/server_handle.h"

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

#include "uv_client.h"

#define UV_SERVER_ONLINES_MAX 8192

/* */
struct uvsrvr_impl_t {
	uv_loop_t					*_loop;
	uv_handle_t					*_server;
	int							_server_closed;

 	server_accept_client_cb		_acceptcb;

	uv_close_cb					_closecb;
 	uv_read_cb					_readcb;
	uv_write_cb					_writecb;
};

/** **/
MY_NETSYSTEM_EXTERN uv_loop_t *			uv_server_get_loop_(server_handle_t *srvr);

MY_NETSYSTEM_EXTERN int					uv_server_init_(server_handle_t *srvr, struct uvsrvr_impl_t *srvr_impl);
MY_NETSYSTEM_EXTERN void				uv_server_dispose_(server_handle_t *srvr);

#ifdef __cplusplus 
} 
#endif 

#endif //