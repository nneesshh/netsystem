#pragma once
//------------------------------------------------------------------------------
/**
@class CKjServer

(C) 2016 n.lee
*/
#include <unordered_map>

#include "../../common/UsingMyToolkitMini.h"

#include "../ITcpClient.h"
#include "../ITcpServer.h"
#include "../ITcpConnFactory.h"
#include "../ITcpEventManager.h"

#include "KjSimpleThreadIoContext.hpp"
#include "KjTcpIoStream.hpp"

//------------------------------------------------------------------------------
/**
@brief CKjServer
*/
class CKjServer : public ITcpServer, public kj::TaskSet::ErrorHandler {
public:
	CKjServer(ITcpConnFactory *pFactory);
	virtual ~CKjServer();

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
		return _thr_tioContext.get();
	}

	virtual unsigned short		GetPort() override {
		return _serverPort;
	}

	virtual ITcpConnFactory&	GetConnFactory() override {
		return *_refConnFactory;
	}

	virtual ITcpEventManager& 	GetEventManager() override {
		return *_eventManager;
	}

public:
	uintptr_t					AddStream(kj::Own<kj::AsyncIoStream>&& stream) {
		kj::Own<KjTcpIoStream> tcpStream = kj::heap<KjTcpIoStream>(kj::addRef(*_thr_tioContext), kj::mv(stream));
		uintptr_t streamptr = tcpStream->GetStreamPtr();
		_thr_tcpStreamDict.insert(std::make_pair(streamptr, kj::mv(tcpStream)));
		return streamptr;
	}

	kj::Own<KjTcpIoStream>		RemoveStream(uintptr_t streamptr) {
		kj::Own<KjTcpIoStream> tcpStream;
		auto iter = _thr_tcpStreamDict.find(streamptr);
		if (iter != _thr_tcpStreamDict.end()) {

			tcpStream = kj::mv(iter->second);
			_thr_tcpStreamDict.erase(iter);
		}
		return tcpStream;
	}

private:
	//////////////////////////////////////////////////////////////////////////
	int							InitListener();

	kj::Promise<void>			StartAccepLoop(kj::Own<kj::ConnectionReceiver> listener);
	kj::Promise<void>			AcceptLoop(kj::Own<kj::ConnectionReceiver>&& listener);

	kj::Promise<void>			ShutdownWatcher() {
		return _thr_shutdownPromise.addBranch();
	}

	//! 
	void taskFailed(kj::Exception&& exception) override;

private:
	CKjServer& operator=(const CKjServer&) = delete;

public:
	kj::Own<KjSimpleThreadIoContext> _thr_tioContext;
	kj::Own<kj::TaskSet> _thr_tasks;
	kj::ForkedPromise<void> _thr_shutdownPromise = nullptr;
	kj::Own<kj::PromiseFulfiller<void>> _thr_shutdownFulfiller;

	std::unordered_map<uintptr_t, kj::Own<KjTcpIoStream>> _thr_tcpStreamDict;  // stream ptr 2 stream

private:
	ITcpConnFactory				*_refConnFactory;
	ITcpEventManager			*_eventManager;

	unsigned short				_serverPort = 0;
	socket_t					_serverFd = -1;
	bool						_closed = false;

};

/*EOF*/