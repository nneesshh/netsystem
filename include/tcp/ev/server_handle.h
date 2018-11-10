#pragma once

#ifdef __cplusplus 
extern "C" {
#endif

#include "client_handle.h"

/************************************************************************/
/* server handle                                                        */
/************************************************************************/

/* */
typedef struct server_handle_t {
	int					_fd;
	unsigned short		_port;
	void				*_impl;

	void				*_userdata;
} server_handle_t;

/* */
typedef int(*server_accept_client_cb)(server_handle_t *srvr, client_handle_t *clnt, const char *peerip);

#ifdef __cplusplus 
}
#endif 

/*EOF*/