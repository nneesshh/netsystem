#pragma once

#ifdef __cplusplus 
extern "C" {
#endif

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN 1
# include <winsock2.h>
# include <WS2tcpip.h>
#else
# include <sys/types.h>  
# include <sys/socket.h>
#endif

#include "../../base/bip_buf.h"

/************************************************************************/
/* client handle                                                        */
/************************************************************************/
typedef struct client_handle_s {
	int					_fd;
	struct sockaddr_in	_sin;

	void				*_sockimpl;
	void				*_connimpl;
	bip_buf_t			*_bb;

} client_handle_t;

#ifdef __cplusplus 
}
#endif 

/*EOF*/