#include "libevhtpd.h"

/**------------------------------------------------------------------------------
*
*/
void
init_thread(evhtp_t *htp, evthr_t *thread, void *arg) {
	http_thr_aux_t *aux;
	aux = calloc(sizeof(http_thr_aux_t), 1);
	aux->_thr_fd = 0;
	aux->_thr_base = evthr_get_base(thread);
	aux->_thr_userdata = NULL;
	evthr_set_aux(thread, aux);

	libevhtp_server_t *srvr = (libevhtp_server_t *)arg;
	if (srvr->_initcb)
		srvr->_initcb(srvr, aux);
}

/**------------------------------------------------------------------------------
 *
 */
int
libevhtpd_init(libevhtp_server_t *srvr, int max_keepalives, int use_threads, int num_threads) {
	struct event_base *evbase = srvr->_base;
	evhtp_t *htp = evhtp_new(evbase, NULL);
 
	if (!htp) {
 		fprintf(stderr, "libevhtpd_init -- couldn't create libevhtpd. Exiting.\n");
 		return 1;
 	}

	srvr->_fd = 0;
	srvr->_htp = htp;
 
	evhtp_set_parser_flags(htp, EVHTP_PARSE_QUERY_FLAG_LENIENT);
	evhtp_set_max_keepalive_requests(htp, max_keepalives);

#ifndef EVHTP_DISABLE_EVTHR
	if (use_threads && num_threads>0) {
		evhtp_use_threads(htp, init_thread, num_threads, srvr);
	}
	else if (srvr->_initcb)
		srvr->_initcb(srvr, NULL);
#else
	if (srvr->_initcb)
		srvr->_initcb(srvr, NULL);
#endif

	/* Now we tell the libevhtp what port to listen on */
	if (evhtp_bind_socket(htp, "0.0.0.0", srvr->_port, 1024) < 0) {
		fprintf(stderr, "Could not bind socket: %s\n", strerror(errno));
		exit(-1);
	}

	{
		printf("libevhtpd_init -- Listening on %d\n", srvr->_port);
	}
	return 0;
}

/**------------------------------------------------------------------------------
 *
 */
void
libevhtpd_exit(libevhtp_server_t *srvr) {
 	if (srvr) {
		evhtp_unbind_socket(srvr->_htp);
		evhtp_free(srvr->_htp);
 	}
}

/**------------------------------------------------------------------------------
 *
 */
void
libevhtpd_set_cb(libevhtp_server_t *srvr, const char *path, evhtp_callback_cb cb, void *arg) {
	evhtp_t *htp = srvr->_htp;
	evhtp_set_cb(htp, path, cb, arg);
}

/**------------------------------------------------------------------------------
 *
 */
void
libevhtpd_set_gencb(libevhtp_server_t *srvr, evhtp_callback_cb cb, void *arg) {
	evhtp_t *htp = srvr->_htp;
	evhtp_set_gencb(htp, cb, arg);
}

/** -- EOF -- **/