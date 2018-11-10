#ifndef EVHTTPSERVER_H
#define EVHTTPSERVER_H

//------------------------------------------------------------------------------
/**
    @class CEvHttpServer
    
    (C) 2016 n.lee
*/
#include "../IHttpServer.h"

#ifdef __cplusplus
extern "C" {
#endif
	#include "ev_httpd.h"

#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////
class CEvConnFactory;

//------------------------------------------------------------------------------
/** 
	@brief CEvHttpServer
*/
class CEvHttpServer : public IHttpServer {
public:
	CEvHttpServer(unsigned short nPort, StdLog *pLog = nullptr);
	virtual ~CEvHttpServer();

	/** **/
	virtual int					OnInit();

	/** Server exit callback **/
	virtual void				OnDelete();

	/** **/
	virtual void				OnUpdate();

public:
	virtual void				OnRequest(const char *decoded_path, void *req, char *buff, size_t nLen) {
		if (_requsetHandler)
			_requsetHandler(this, decoded_path, req, buff, nLen);
	}

public:
	/** **/
	virtual void				RegisterInitHandler(HTTP_INIT_HANDLER d) {
		_initHandler = d;
	}

	virtual void				RegisterRequestHandler(HTTP_REQUEST_HANDLER d) {
		_requsetHandler = d;
	}

	virtual void				SendReply(void *req_handle, int code, const char *reason, const char *str);
	virtual void				SendReply(void *req_handle, int code, const char *reason, const char *data, size_t len);

	virtual void				PauseRequest(void *req_handle) { }
	virtual void				ResumeRequest(void *req_handle) { }

	virtual void				ForEach(void *query_handle, HTTP_QUERY_ITERATOR kvscb) { }

	/** send close signal. **/
	virtual void				Close();
	virtual bool				IsClosed();

	/** */
	virtual void *				GetBase() {
		return _srvr._base;
	}

	virtual unsigned short		GetPort() {
		return _srvr._port;
	}

protected:
	CEvHttpServer(const CEvHttpServer& s) {}

private:
	CEvHttpServer& operator=(const CEvHttpServer& ) { return *this; }

protected:
	struct evhttp_server	_srvr;
	bool					_closed;

public:
	//////////////////////////////////////////////////////////////////////////
	StdLog					*_refLog;
	HTTP_INIT_HANDLER		_initHandler;
	HTTP_REQUEST_HANDLER	_requsetHandler;
};

#endif
