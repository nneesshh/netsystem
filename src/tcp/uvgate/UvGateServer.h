#pragma once
//------------------------------------------------------------------------------
/**
@class CUvGateServer

(C) 2016 n.lee
*/
#include <unordered_map>

#include "../../common/UsingMyToolkitMini.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../uv/uv_server.h"
#ifdef __cplusplus
}
#endif

#include "../ITcpClient.h"
#include "../ITcpServer.h"
#include "../ITcpConnFactory.h"
#include "../ITcpEventManager.h"

//------------------------------------------------------------------------------
/**
@brief CUvGateServer
*/
class CUvGateServer : public ITcpServer {
public:
	CUvGateServer(ITcpConnFactory *pFactory);
	virtual ~CUvGateServer();

	/** Server accept and dispose client */
	virtual ITcpClient *		OnAcceptClient(uintptr_t streamptr, const std::string& sPeerIp) override;
	virtual void				OnDisposeClient(ITcpClient *pClient) override;

public:
	/** Open and close **/
	virtual int					Open(void *base, unsigned short port) override;
	virtual void				Close() override;

	virtual bool				IsClosed() override {
		return _closed;
	}

	/** **/
	virtual void *				GetBase() override {
		return uv_server_get_loop_(&_srvr);
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
	CUvGateServer& operator=(const CUvGateServer&) = delete;

public:
	server_handle_t _srvr;
	std::unordered_map<uintptr_t, client_handle_t *> _tcpStreamDict;  // stream ptr 2 client handle

private:
	ITcpConnFactory				*_refConnFactory;
	ITcpEventManager			*_eventManager;

	bool						_closed = false;

};

/*EOF*/