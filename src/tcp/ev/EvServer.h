#pragma once
//------------------------------------------------------------------------------
/**
@class CEvServer

(C) 2016 n.lee
*/
#include "../../common/UsingMyToolkitMini.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "ev_server.h"
#ifdef __cplusplus
}
#endif

#include "../ITcpClient.h"
#include "../ITcpServer.h"
#include "../ITcpConnFactory.h"
#include "../ITcpEventManager.h"

//------------------------------------------------------------------------------
/**
@brief CEvServer
*/
class CEvServer : public ITcpServer {
public:
	CEvServer(ITcpConnFactory *pFactory);
	virtual ~CEvServer();

	/** Server accept and dispose client */
	virtual ITcpClient *		OnAcceptClient(uintptr_t streamptr, const std::string& sPeerIp);
	virtual void				OnDisposeClient(ITcpClient *pClient);

public:
	/** Open and close **/
	virtual int					Open(void *base, unsigned short port);
	virtual void				Close();

	virtual bool				IsClosed() {
		return _closed;
	}

	/** **/
	virtual void *				GetBase() {
		return ev_server_get_base_(&_srvr);
	}

	virtual unsigned short		GetPort() {
		return _srvr._port;
	}

	virtual ITcpConnFactory&	GetConnFactory() {
		return *_refConnFactory;
	}

	virtual ITcpEventManager& 	GetEventManager() {
		return *_eventManager;
	}

public:
	uintptr_t					AddStream(client_handle_t *clnt) {
		uintptr_t streamptr = (uintptr_t)clnt;
		_tcpStreamDict.insert(std::make_pair(streamptr, clnt));
		return streamptr;
	}

	client_handle_t *			RemoveStream(uintptr_t streamptr) {
		client_handle_t *clnt = nullptr;
		auto iter = _tcpStreamDict.find(streamptr);
		if (iter != _tcpStreamDict.end()) {

			clnt = iter->second;
			_tcpStreamDict.erase(iter);
		}
		return clnt;
	}

private:
	CEvServer& operator=(const CEvServer&) = delete;

public:
	server_handle_t _srvr;
	std::unordered_map<uintptr_t, client_handle_t *> _tcpStreamDict;  // stream ptr 2 client handle

private:
	ITcpConnFactory				*_refConnFactory;
	ITcpEventManager			*_eventManager;

	bool						_closed = false;
};

/*EOF*/