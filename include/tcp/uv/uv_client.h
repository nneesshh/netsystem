#pragma once

#include "../tcp_def.h"

#ifdef __cplusplus 
extern "C" { 
#endif

#include "../../base/netsystem_extern.h"

#include "../ev/client_handle.h"
#include "../ev/server_handle.h"

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

#include "uv.h"

/** **/
typedef struct {
	uv_write_t req;
	uv_buf_t buf;
} write_req_t;

/** **/
MY_NETSYSTEM_EXTERN uv_loop_t *			uv_loop_new_();
MY_NETSYSTEM_EXTERN void				uv_loop_delete_(uv_loop_t *loop);
MY_NETSYSTEM_EXTERN int					uv_loop_run_(uv_loop_t *loop);
MY_NETSYSTEM_EXTERN int					uv_loop_run_once_(uv_loop_t *loop);

MY_NETSYSTEM_EXTERN void				uv_close_(uv_handle_t *sockimpl, uv_close_cb on_sock_close);

MY_NETSYSTEM_EXTERN client_handle_t *	uv_connect_(uv_loop_t *loop, const char *ip, unsigned short port, uv_connect_cb on_error);
MY_NETSYSTEM_EXTERN int					uv_real_connect_(client_handle_t *clnt, uv_loop_t *loop, uv_connect_cb on_error);

MY_NETSYSTEM_EXTERN uv_timer_t *		uv_create_timer_(uv_loop_t *loop, void *arg);
 
MY_NETSYSTEM_EXTERN int					uv_start_timer_(uv_timer_t *timer_handle
														, uv_timer_cb cb
														, uint64_t timeout
														, uint64_t repeat);

MY_NETSYSTEM_EXTERN void				uv_stop_and_free_timer_(uv_timer_t *timer_handle);

#ifdef __cplusplus 
} 
#endif 

/*EOF*/
