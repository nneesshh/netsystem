#include "uv_client.h"
#include <malloc.h>

//------------------------------------------------------------------------------
/**

*/
static void
on_timer_close(uv_handle_t* handle) {
	free(handle);
}

/**------------------------------------------------------------------------------
*
*/
uv_loop_t *
uv_loop_new_() {
	uv_loop_t *loop = malloc(uv_loop_size());
	loop->data = NULL;
	uv_loop_init(loop);
	return loop;
}

/**------------------------------------------------------------------------------
*
*/
void
uv_close_(uv_handle_t *sockimpl, uv_close_cb on_sock_close) {

	if (sockimpl && !uv_is_closing(sockimpl)) {
		uv_close(sockimpl, on_sock_close);
	}
}

/**------------------------------------------------------------------------------
*
*/
client_handle_t *
uv_connect_(uv_loop_t *loop, const char *ip, unsigned short port, uv_connect_cb on_error) {
	
	client_handle_t *clnt = NULL;
	int r;

	/* create client handle */
	clnt = malloc(sizeof(client_handle_t));
	clnt->_fd = -1;

	/* parse ip. */
	r = uv_ip4_addr(ip, port, &clnt->_sin);
	if (0 != r) {
		fprintf(stderr, "Invalid IP(%s)!\n", ip);
		free(clnt);
		return NULL;
	}

	/* start connect */
	printf("Client start to connect -- IP=(%s), port=(%d)...\n", ip, port);
	r = uv_real_connect_(clnt, loop, on_error);
	if (0 != r) {
		free(clnt);
		fprintf(stderr, "Connect failed -- IP=(%s), port=(%d)!\n", ip, port);
		return NULL;
	}

	/* */
	clnt->_connimpl = NULL;

	/* buffer */
	clnt->_bb = bip_buf_create(TCP_STREAM_READ_SIZE);

	/* add any custom code anywhere from here to the end of this function
	* to initialize your application-specific attributes in the client struct. */

	return clnt;
}

/**------------------------------------------------------------------------------
*
*/
int
uv_real_connect_(client_handle_t *clnt, uv_loop_t *loop, uv_connect_cb on_error) {
	int r;
	uv_tcp_t *sockimpl;
	uv_connect_t *connect_req;

	/* create the tcp handle. */
	sockimpl = malloc(sizeof(uv_tcp_t));
	r = uv_tcp_init(loop, sockimpl);
	if (0 == r) {

		connect_req = malloc(sizeof(uv_connect_t));
		connect_req->data = (void*)clnt;
		
		r = uv_tcp_connect(connect_req, sockimpl, (const struct sockaddr*)&clnt->_sin, on_error);
		if (0 == r) {
			sockimpl->data = clnt;
			clnt->_sockimpl = sockimpl;
			return 0;
		}
		else {
			free(sockimpl);
			free(connect_req);
			return -1;
		}
	}

	free(sockimpl);
	return -2;
}

/**------------------------------------------------------------------------------
*
*/
void
uv_loop_delete_(uv_loop_t *loop) {
	fprintf(stderr, "loop delete -- loop(0x%08Ix)\n",
		(uintptr_t)loop);

	uv_loop_close(loop);
	free(loop);
}

/**------------------------------------------------------------------------------
*
*/
int
uv_loop_run_(uv_loop_t *loop) {
	return uv_run(loop, UV_RUN_DEFAULT);
}

/**------------------------------------------------------------------------------
*
*/
int
uv_loop_run_once_(uv_loop_t *loop) {
	return uv_run(loop, UV_RUN_NOWAIT);
}

/**------------------------------------------------------------------------------
*
*/
uv_timer_t *
uv_create_timer_(uv_loop_t *loop, void *arg) {
	uv_timer_t *timer_handle = malloc(sizeof(uv_timer_t));
	int r = uv_timer_init(loop, timer_handle);
	if (r<0) {
		fprintf(stderr, "Error create timer!\n");
		return NULL;
	}

	//
	timer_handle->data = arg;
	return timer_handle;
}

/**------------------------------------------------------------------------------
*
*/
int
uv_start_timer_(uv_timer_t *timer_handle
	, uv_timer_cb cb
	, uint64_t timeout
	, uint64_t repeat) {

	int r = uv_timer_start(timer_handle, cb, timeout, repeat);
	if (r < 0) {
		fprintf(stderr, "Error start timer!\n");
	}
	return r;
}

/**------------------------------------------------------------------------------
*
*/
void
uv_stop_and_free_timer_(uv_timer_t *timer_handle) {
	uv_timer_stop(timer_handle);
	uv_close((uv_handle_t*)timer_handle, on_timer_close);
}

/** -- EOF -- **/