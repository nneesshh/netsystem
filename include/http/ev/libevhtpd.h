#ifndef LIBEVHTPD_H
#define LIBEVHTPD_H

#ifdef __cplusplus 
extern "C" { 
#endif 

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN 1
# include <winsock2.h>
# include <ws2tcpip.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <inttypes.h>
#include <event2/event.h>

#include "evhtp.h"
#include "../http_thr_aux.h"

/* */
struct libevhtp_server;
typedef struct libevhtp_server libevhtp_server_t;
typedef void(*evhtp_thread_initcb)(libevhtp_server_t *, http_thr_aux_t *);
struct libevhtp_server {
	int _fd;
	unsigned short _port;
	evbase_t *_base;
	evhtp_t *_htp;
	void *_userdata;

	evhtp_thread_initcb _initcb;
};

/** **/
extern int						libevhtpd_init(libevhtp_server_t *srvr, int max_keepalives, int use_threads, int num_threads);
extern void						libevhtpd_exit(libevhtp_server_t *srvr);

extern void						libevhtpd_set_cb(libevhtp_server_t *srvr, const char *path, evhtp_callback_cb cb, void *arg);
extern void						libevhtpd_set_gencb(libevhtp_server_t *srvr, evhtp_callback_cb cb, void *arg);

#ifdef __cplusplus 
} 
#endif 

#endif