#include "uv_tls_alloc.h"

#include <stdio.h>

/************************************************************************/
/* Handle allocator                                                    */
/************************************************************************/


DECLARE_TLS handle_list_t* client_handle_freelist = NULL;

uv_handle_t* 
tls_handle_alloc(void)
{
	handle_list_t* list_item;

	list_item = client_handle_freelist;
	if (NULL != list_item)
	{
		client_handle_freelist = list_item->next;
		return (uv_handle_t*) list_item;
	}

	list_item = (handle_list_t*) malloc(sizeof *list_item);
	return (uv_handle_t*) list_item;
}

void 
tls_handle_free(uv_handle_t* uvhandle)
{
	handle_list_t* list_item = (handle_list_t*) uvhandle;

	list_item->next = client_handle_freelist;
	client_handle_freelist = list_item;
}

void 
tls_clear_handle_freelist(void)
{
	handle_list_t* list_item = NULL;
	while(client_handle_freelist)
	{
		list_item = client_handle_freelist;
		client_handle_freelist = client_handle_freelist->next;
		free(list_item);
	}
}

/************************************************************************/
/* Request allocator                                                    */
/************************************************************************/


DECLARE_TLS req_list_t* req_freelist = NULL;

uv_req_t* 
tls_req_alloc(void)
{
	req_list_t* list_item;

	list_item = req_freelist;
	if (NULL != list_item)
	{
		req_freelist = list_item->next;
		return (uv_req_t*) list_item;
	}

	list_item = (req_list_t*) malloc(sizeof *list_item);
	return (uv_req_t*) list_item;
}

void 
tls_req_free(uv_req_t* uvreq)
{
	req_list_t* list_item = (req_list_t*) uvreq;
	
	list_item->next = req_freelist;
	req_freelist = list_item;
}

void 
tls_clear_req_freelist(void)
{
	req_list_t* list_item = NULL;
	while(req_freelist)
	{
		list_item = req_freelist;
		req_freelist = req_freelist->next;
		free(list_item);
	}
}

/************************************************************************/
/* Buffer allocator                                                     */
/************************************************************************/


DECLARE_TLS buf_list_t* buf_freelist = NULL;

uv_buf_t 
tls_buf_alloc_simple(size_t suggested_size)
{
	buf_list_t* list_item;

	// debug
	//suggested_size = 4096;

	list_item = buf_freelist;
	if (NULL != list_item)
	{
		buf_freelist = list_item->next;
		return list_item->uvbuf;
	}

	list_item = (buf_list_t*) malloc(suggested_size);
	list_item->uvbuf.len = (unsigned int)suggested_size - sizeof *list_item;
	list_item->uvbuf.base = ((char*) list_item) + sizeof *list_item;

	return list_item->uvbuf;
}

uv_buf_t 
tls_buf_alloc(uv_handle_t *handle, size_t suggested_size)
{
	return tls_buf_alloc_simple(suggested_size);
}

void 
tls_buf_free(uv_buf_t uvbuf)
{
	if (NULL != uvbuf.base)
	{
		buf_list_t* list_item = (buf_list_t*) (uvbuf.base - sizeof *list_item);

		list_item->next = buf_freelist;
		buf_freelist = list_item;
	}
}

void 
tls_clear_buf_freelist(void)
{
	void* buf = NULL;
	while(buf_freelist)
	{
		buf = buf_freelist;
		buf_freelist = buf_freelist->next;
		free(buf);
	}
}

buf_list_t **
tls_get_buf_freelist_ptrptr(void)
{
	return &buf_freelist;
}

uv_buf_t 
buf_alloc_2(uv_handle_t* handle, size_t suggested_size)
{
	buf_list_t* list_item = (buf_list_t*) malloc(suggested_size);
	list_item->uvbuf.len = (unsigned int)suggested_size - sizeof *list_item;
	list_item->uvbuf.base = ((char*) list_item) + sizeof *list_item;

	return list_item->uvbuf;
}

void 
buf_free_2(uv_buf_t uvbuf)
{
	buf_list_t* list_item = (buf_list_t*) (uvbuf.base - sizeof *list_item);
	free( list_item);
}

/** -- EOF -- **/