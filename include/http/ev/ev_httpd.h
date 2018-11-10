#ifndef EVHTTPD_H
#define EVHTTPD_H

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

#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

#ifdef _EVENT_HAVE_NETINET_IN_H
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#endif

	/* Compatibility for possible missing IPv6 declarations */
#include "../util-internal.h"

#ifdef WIN32
#define stat _stat
#define fstat _fstat
#define open _open
#define close _close
#define O_RDONLY _O_RDONLY
#endif

/* */
struct evhttp_server;
typedef void(*ev_httpd_initcb)(struct evhttp_server *);
typedef void (*ev_httpd_cb)(struct evhttp_request *, void *);

/* */
struct evhttp_server {
	int _fd;
	unsigned short _port;
	struct event_base *_base;
	struct evhttp *_http;
	void *_userdata;

	ev_httpd_initcb _initcb;
};
typedef struct evhttp_server evhttp_server_t;

/** **/
extern int						ev_httpd_init(evhttp_server_t *srvr);
extern void						ev_httpd_exit(evhttp_server_t *srvr);

extern void						ev_httpd_set_cb(evhttp_server_t *srvr, const char *path, ev_httpd_cb cb, void *arg);
extern void						ev_httpd_set_gencb(evhttp_server_t *srvr, ev_httpd_cb cb, void *arg);

#ifdef __cplusplus 
} 
#endif 

#endif