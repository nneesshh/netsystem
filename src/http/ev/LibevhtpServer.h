#ifndef LIBEVHTPSERVER_H
#define LIBEVHTPSERVER_H

//------------------------------------------------------------------------------
/**
@class CLibevhtpServer

(C) 2016 n.lee
*/
#include "../IHttpServer.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libevhtpd.h"

#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////
class CEvConnFactory;

//------------------------------------------------------------------------------
/**
@brief CLibevhtpServer
*/
class CLibevhtpServer : public IHttpServer {
public:
	CLibevhtpServer(unsigned short nPort, StdLog *pLog = nullptr);
	virtual ~CLibevhtpServer();

	/** **/
	virtual int					OnInit();

	/** Server exit callback **/
	virtual void				OnDelete();

	/** **/
	virtual void				OnUpdate();

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

	virtual void				PauseRequest(void *req_handle);
	virtual void				ResumeRequest(void *req_handle);

	virtual void				ForEach(void *query_handle, HTTP_QUERY_ITERATOR kvscb);

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
	CLibevhtpServer(const CLibevhtpServer& s) {}

private:
	CLibevhtpServer& operator=(const CLibevhtpServer&) { return *this; }

protected:
	libevhtp_server_t		_srvr;
	bool					_closed;

public:
	//////////////////////////////////////////////////////////////////////////
	StdLog					*_refLog;
	HTTP_INIT_HANDLER		_initHandler;
	HTTP_REQUEST_HANDLER	_requsetHandler;
	

};

#endif