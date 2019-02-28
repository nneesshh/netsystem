//------------------------------------------------------------------------------
//  KjGateServer.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjGateServer.hpp"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#endif

#include "../../base/MyMacros.h"

#include "../TcpConnManager.h"
#include "../TcpEventManager.h"

#include "KjGateConnFactory.hpp"
#include "KjGateClientConn.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

#define SERVER_SELF_CHECK_TIME_IN_MS	5000L

//------------------------------------------------------------------------------
/**

*/
CKjGateServer::CKjGateServer(ITcpConnFactory *pFactory)
	: _refConnFactory(pFactory)
	, _eventManager(new CTcpEventManager()) {

}

//------------------------------------------------------------------------------
/**

*/
CKjGateServer::~CKjGateServer() noexcept {

	SAFE_DELETE(_eventManager);
	_refConnFactory = nullptr;
}

//------------------------------------------------------------------------------
/**

*/
ITcpClient *
CKjGateServer::OnAcceptClient(uintptr_t streamptr, std::string&& sPeerIp) {
	//
	ITcpClient *pClient = _refConnFactory->CreateTcpClientOnServer(std::move(sPeerIp), this);
	if (pClient) {
		// connection is ready
		_refConnFactory->ConfirmClientIsReady(pClient, streamptr);

		// event
		pClient->OnConnect();
	}
	return pClient;
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateServer::OnDisposeClient(ITcpClient *pClient) {

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
CKjGateServer::Open(void *base, unsigned short port) {
	// init base
	_thr_endpointContext = kj::addRef(*(KjPipeEndpointIoContext *)base);
	_thr_tasks = netsystem_get_servercore()->NewTaskSet(*this);

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
CKjGateServer::Close() {

	if (!_closed) {
		//
		for (auto& iter : _thr_tcpStreamDict) {
			auto& tcpStream = iter.second;

			// flush stream
			tcpStream->FlushStream();
		}
		_thr_tcpStreamDict.clear();

		// dispose shutdown watcher
		_thr_shutdownFulfiller->fulfill();
		_thr_shutdownPromise = nullptr;
		_thr_shutdownFulfiller = nullptr;

		// dispose base
		_thr_endpointContext = nullptr;
		_thr_tasks = nullptr;
	
		//
		_refConnFactory->GetConnManager().DisposeDownStreams(this);

		//
		_closed = true;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateServer::FlushDownStream(uintptr_t streamptr) {
	kj::Own<KjTcpDownStream> tcpStream;
	auto iter = _thr_tcpStreamDict.find(streamptr);
	if (iter != _thr_tcpStreamDict.end()) {

		tcpStream = kj::mv(iter->second);
		_thr_tcpStreamDict.erase(iter);
	}
	tcpStream = nullptr;
}

//------------------------------------------------------------------------------
/**

*/
int
CKjGateServer::InitListener() {

	int nResult = 0;

	// init listener
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(_serverPort);

	kj::Own<kj::NetworkAddress>&& addr = _thr_endpointContext->GetNetwork().getSockaddr((const sockaddr *)&sin, sizeof(sin));
	kj::Own<kj::ConnectionReceiver> listener = addr->listen();

	_serverFd = listener->getFd();
	if (-1 == _serverFd) {
		nResult = -1;
	}
	else {
		fprintf(stderr, "server init on port(%d) fd(%d)...\n",
			_serverPort, (int)_serverFd);

		// gate server start accept loop
		_thr_tasks->add(StartAccepLoop(kj::mv(listener)));
	}
	return nResult;
}

//------------------------------------------------------------------------------
/**

*/
kj::Promise<void>
CKjGateServer::StartAccepLoop(kj::Own<kj::ConnectionReceiver> listener) {
	return AcceptLoop(kj::mv(listener))
		.exclusiveJoin(ShutdownWatcher());
}

//------------------------------------------------------------------------------
/**

*/
static kj::Promise<void> kj_ready_now() { return kj::READY_NOW; }

kj::Promise<void>
CKjGateServer::AcceptLoop(kj::Own<kj::ConnectionReceiver>&& listener) {

	return listener->accept().then(kj::mvCapture(listener,
		[this](kj::Own<kj::ConnectionReceiver>&& listener,
			kj::Own<kj::AsyncIoStream>&& stream) {

		if (!this->IsReady()) {
			// flush stream at once
			/*try {
				_stream->abortRead();
				_stream->shutdownWrite();
			}
			catch(std::exception& e) {
				fprintf(stderr, "[CKjGateServer::AcceptLoop()] abortRead or shutdownWrite exception -- what(%s)!!!"
					, e.what());
			}*/
			stream = nullptr;

			// loop or stop
			if (IsClosed()) {
				return kj_ready_now();
			}
			return AcceptLoop(kj::mv(listener));
		}
		else {
			//
			struct sockaddr_in sin;
			memset(&sin, 0, sizeof(sin));
			kj::uint len = sizeof(sin);
			stream->getpeername((sockaddr *)&sin, &len);
			std::string sPeerIp = inet_ntoa(sin.sin_addr);

			// accept client cb
			uintptr_t streamptr = AddStream(kj::mv(stream));
			_refConnFactory->AddAcceptClientCb(this, streamptr, kj::mv(sPeerIp));

			// loop
			return AcceptLoop(kj::mv(listener));
		}
	}));
}


//------------------------------------------------------------------------------
/**

*/
void
CKjGateServer::taskFailed(kj::Exception&& exception) {
	char chDesc[1024];
#if defined(__CYGWIN__) || defined( _WIN32)
	_snprintf_s(chDesc, sizeof(chDesc), "\n[CKjGateServer::taskFailed()] exception_desc(%s)!!! port(%d)fd(%d)closed(%d)\n",
		exception.getDescription().cStr(), _serverPort, (int)_serverFd, _closed);
#else
	snprintf(chDesc, sizeof(chDesc), "\n[CKjGateServer::taskFailed()] exception_desc(%s)!!! port(%d)fd(%d)closed(%d)\n",
		exception.getDescription().cStr(), _serverPort, (int)_serverFd, _closed);
#endif

	// alert
	StdLog *pLog = netsystem_get_log();
	if (pLog)
		pLog->logprint(LOG_LEVEL_ALERT, "\n\n\n[CKjGateServer::taskFailed()] exception_desc(%s)!!! port(%d)fd(%d)closed(%d)\n\n\n",
			exception.getDescription().cStr(), _serverPort, (int)_serverFd, _closed);

	fprintf(stderr, "%s", chDesc);

	// reset listener
	InitListener();
}

/** -- EOF -- **/