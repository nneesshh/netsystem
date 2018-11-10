#pragma once
//------------------------------------------------------------------------------
/**
    @class IHttpServer
    
    (C) 2016 n.lee
*/
#include "../common/UsingMyToolkitMini.h"

#include "log/StdLog.h"

#include <functional>

#include "http_thr_aux.h"

//
class IHttpServer;
typedef std::function<void (IHttpServer *, http_thr_aux_t *)> HTTP_INIT_HANDLER;
typedef std::function<int (IHttpServer *, const char *, void *, char *, size_t)> HTTP_REQUEST_HANDLER;
typedef std::function<int(void *kvobj)> HTTP_QUERY_ITERATOR;

//------------------------------------------------------------------------------
/** 
	@brief IHttpServer
*/
class IHttpServer {
public:
	virtual ~IHttpServer() { }

	/** **/
	virtual int					OnInit() = 0;

	/** Server exit callback **/
	virtual void				OnDelete() = 0;

	/** **/
	virtual void				OnUpdate() = 0;

	/** send close signal. */
	virtual void				Close() = 0;
	virtual bool				IsClosed() = 0;

	/** **/
	virtual void *				GetBase() = 0;
	virtual unsigned short		GetPort() = 0;

	virtual void				RegisterInitHandler(HTTP_INIT_HANDLER d) = 0;
	virtual void				RegisterRequestHandler(HTTP_REQUEST_HANDLER d) = 0;
	virtual void				SendReply(void *req_handle, int code, const char *reason, const char *str) = 0;
	virtual void				SendReply(void *req_handle, int code, const char *reason, const char *data, size_t len) = 0;
	virtual void				PauseRequest(void *req_handle) = 0;
	virtual void				ResumeRequest(void *req_handle) = 0;
	virtual void				ForEach(void *query_handle, HTTP_QUERY_ITERATOR kvscb) = 0;
};

/*EOF*/