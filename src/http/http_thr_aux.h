#pragma once
//------------------------------------------------------------------------------
/**
    @class IHttpServer
    
    (C) 2016 n.lee
*/
#ifdef __cplusplus 
extern "C" {
#endif 
	typedef struct http_thr_aux_s {
		int _thr_fd;
		void *_thr_base;
		void *_thr_userdata;
	} http_thr_aux_t;


#ifdef __cplusplus 
}
#endif 

/*EOF*/