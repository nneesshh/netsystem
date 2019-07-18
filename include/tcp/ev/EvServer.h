#pragma once
//------------------------------------------------------------------------------
/**
@class CEvServer

(C) 2016 n.lee
*/
#include "../../UsingBase.h"

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
	virtual ITcpClient *		OnAcceptClient(uintptr_t streamptr, std::string&& sPeerIp) override;
	virtual void				OnDisposeClient(ITcpClient *pClient) override;

public:
	/** Open and close **/
	virtual int					Open(void *base, unsigned short port) override;
	virtual void				Close() override;
	virtual void				FlushDownStream(uintptr_t streamptr) override;

	virtual bool				IsClosed() override {
		return _closed;
	}

	virtual bool				IsReady() override {
		return !IsClosed() && GetEventManager().IsReady();
	}

	/** **/
	virtual void *				GetBase() override {
		return ev_server_get_base_(&_srvr);
	}

	virtual unsigned short		GetPort() override {
		return _srvr._port;
	}

	virtual ITcpConnFactory&	GetConnFactory() override {
		return *_refConnFactory;
	}

	virtual ITcpEventManager& 	GetEventManager() override {
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