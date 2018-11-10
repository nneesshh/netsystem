#ifndef __UV_TLS_ALLOC_H__
#define __UV_TLS_ALLOC_H__

#include "uv.h"
#include "toolkit/tp/tputils.h"

///
typedef struct handle_list_s
{
	union uv_any_handle		uvhandle;
	struct handle_list_s	*next;
} handle_list_t;

///
typedef struct req_list_s
{
	union uv_any_req	uvreq;
	struct req_list_s	*next;
} req_list_t;

///
typedef struct buf_list_s
{
	uv_buf_t			uvbuf;
	struct buf_list_s	*next;
} buf_list_t;

/************************************************************************/
/* Handle allocator                                                    */
/************************************************************************/
extern DECLARE_TLS handle_list_t* handle_freelist;

extern uv_handle_t* tls_handle_alloc(void);
extern void tls_handle_free(uv_handle_t* uvhandle);
extern void tls_clear_handle_freelist(void);

/************************************************************************/
/* Request allocator                                                    */
/************************************************************************/

extern DECLARE_TLS req_list_t* req_freelist;

extern uv_req_t * tls_req_alloc(void);
extern void tls_req_free(uv_req_t* uvreq);
extern void tls_clear_req_freelist(void);

/************************************************************************/
/* Buffer allocator                                                     */
/************************************************************************/
extern DECLARE_TLS buf_list_t* buf_freelist;

extern uv_buf_t tls_buf_alloc_simple(size_t suggested_size);
extern uv_buf_t tls_buf_alloc(uv_handle_t *handle, size_t suggested_size);
extern void tls_buf_free(uv_buf_t uvbuf);
extern void tls_clear_buf_freelist(void);
extern buf_list_t ** tls_get_buf_freelist_ptrptr( void );

#endif // __UV_TLS_ALLOC_H__
