#pragma once
//------------------------------------------------------------------------------
/**
    @class IHttpServer
    
    (C) 2016 n.lee
*/
#ifdef __cplusplus 
extern "C" {
#endif 
	struct http_thr_aux;
	typedef struct http_thr_aux http_thr_aux_t;
	struct http_thr_aux {
		int _thr_fd;
		void *_thr_base;
		void *_thr_userdata;
	};


#ifdef __cplusplus 
}
#endif 

/*EOF*/