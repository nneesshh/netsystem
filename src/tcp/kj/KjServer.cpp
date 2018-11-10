//------------------------------------------------------------------------------
//  KjServer.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjServer.hpp"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../TcpConnManager.h"
#include "../TcpEventManager.h"

#include "KjConnFactory.hpp"
#include "KjClientConn.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

#define SERVER_SELF_CHECK_TIME				5000L

//------------------------------------------------------------------------------
/**

*/
CKjServer::CKjServer(ITcpConnFactory *pFactory)
	: _refConnFactory(pFactory)
	, _eventManager(new CTcpEventManager()) {

}

//------------------------------------------------------------------------------
/**

*/
CKjServer::~CKjServer() {

	SAFE_DELETE(_eventManager);
	_refConnFactory = nullptr;
}

//------------------------------------------------------------------------------
/**

*/
ITcpClient *
CKjServer::OnAcceptClient(uintptr_t streamptr, const std::string& sPeerIp) {

	if (IsClosed())
		return nullptr;

	if (!_eventManager->IsReady())
		return nullptr;

	//
	ITcpClient *pClient = _refConnFactory->CreateTcpClientOnServer(sPeerIp, this);
	if (pClient) {

		pClient->OnConnect();

		// connection is ready
		_refConnFactory->ConfirmClientIsReady(pClient, streamptr);
	}
	return pClient;
}

//------------------------------------------------------------------------------
/**

*/
void
CKjServer::OnDisposeClient(ITcpClient *pClient) {

	if (pClient) {
		// dispose
		pClient->DisposeConnection();
		
		// disconnect cb to front end
		// Note: Client may be [ false == IsConnected() ], but we always need main thread to [ RemoveClient() ]
		_refConnFactory->AddClientDisconnectCb(this, pClient, &_refConnFactory->GetConnManager());
	}
}

//------------------------------------------------------------------------------
/**

*/
int
CKjServer::Open(void *base, unsigned short port) {
	// init base
	_thr_tioContext = kj::addRef(*(KjSimpleThreadIoContext *)base);
	_thr_tasks = _thr_tioContext->CreateTaskSet(*this);

	// init shutdown watcher
	auto paf = kj::newPromiseAndFulfiller<void>();
	_thr_shutdownPromise = paf.promise.fork();
	_thr_shutdownFulfiller = kj::mv(paf.fulfiller);

	// init port
	_serverPort = port;

	// init listener
	return InitListener();
}

//------------------------------------------------------------------------------
/**

*/
void
CKjServer::Close() {

	if (!_closed) {
		//
		for (auto& iter : _thr_tcpStreamDict) {
			auto& tcpStream = iter.second;

			// flush stream
			tcpStream->Disconnect();
		}
		_thr_tcpStreamDict.clear();

		// dispose shutdown watcher
		_thr_shutdownFulfiller->fulfill();
		_thr_shutdownPromise = nullptr;
		_thr_shutdownFulfiller = nullptr;

		// dispose base
		_thr_tioContext = nullptr;
		_thr_tasks = nullptr;

		//
		_refConnFactory->GetConnManager().OnDisposeAllClients(this);

		//
		_closed = true;
	}
}

//------------------------------------------------------------------------------
/**

*/
int
CKjServer::InitListener() {

	int nResult = 0;

	// init listener
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(_serverPort);

	kj::Own<kj::NetworkAddress>&& addr = _thr_tioContext->GetNetwork().getSockaddr((const sockaddr *)&sin, sizeof(sin));
	kj::Own<kj::ConnectionReceiver> listener = addr->listen();

	_serverFd = listener->getFd();
	if (-1 == _serverFd) {
		nResult = -1;
	}
	else {
		fprintf(stderr, "server init on port(%d) fd(%d)...\n",
			_serverPort, (int)_serverFd);

		// server start accept loop
		_thr_tasks->add(StartAccepLoop(kj::mv(listener)));
	}
	return nResult;
}

//------------------------------------------------------------------------------
/**

*/
kj::Promise<void>
CKjServer::StartAccepLoop(kj::Own<kj::ConnectionReceiver> listener) {
	return AcceptLoop(kj::mv(listener))
		.exclusiveJoin(ShutdownWatcher());
}

//------------------------------------------------------------------------------
/**

*/
kj::Promise<void>
CKjServer::AcceptLoop(kj::Own<kj::ConnectionReceiver>&& listener) {

	auto ptr = listener.get();
	return ptr->accept().then(kj::mvCapture(kj::mv(listener),
		[this](kj::Own<kj::ConnectionReceiver>&& listener,
			kj::Own<kj::AsyncIoStream>&& stream) {
		//
		struct sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));
		kj::uint len = sizeof(sin);
		stream->getpeername((sockaddr *)&sin, &len);
		std::string sPeerIp = inet_ntoa(sin.sin_addr);

		// accept client cb
		uintptr_t streamptr = AddStream(kj::mv(stream));
		_refConnFactory->AddAcceptClientCb(this, streamptr, sPeerIp);

		// loop
		return AcceptLoop(kj::mv(listener));
	}));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjServer::taskFailed(kj::Exception&& exception) {
	char chDesc[1024];
#if defined(__CYGWIN__) || defined( _WIN32)
	_snprintf_s(chDesc, sizeof(chDesc), "\n\n\n[CKjServer::taskFailed()] exception_desc(%s)!!! port(%d)fd(%d)closed(%d)\n\n\n",
		exception.getDescription().cStr(), _serverPort, (int)_serverFd, _closed);
#else
	snprintf(chDesc, sizeof(chDesc), "\n\n\n[CKjServer::taskFailed()] exception_desc(%s)!!! port(%d)fd(%d)closed(%d)\n\n\n",
		exception.getDescription().cStr(), _serverPort, (int)_serverFd, _closed);
#endif

	// alert
	StdLog *pLog = _refConnFactory->GetLogHandler();
	if (pLog)
		pLog->logprint(LOG_LEVEL_ALERT, "\n\n\n[CKjServer::taskFailed()] exception_desc(%s)!!! port(%d)fd(%d)closed(%d)\n\n\n",
			exception.getDescription().cStr(), _serverPort, (int)_serverFd, _closed);

	fprintf(stderr, "%s", chDesc);

	/*  AcceptEx() failed: The specified network name is no longer available.
	    : AcceptEx() is a Microsoft WinSock v2 API that provides some performance improvements over the use of the BSD style accept() API
	      in certain circumstances. Some popular Windows products, typically virus scanning or virtual private network packages, have bugs
	      that interfere with the proper operation of AcceptEx().
	*/
	// reset listener
	InitListener();
}

/** -- EOF -- **/