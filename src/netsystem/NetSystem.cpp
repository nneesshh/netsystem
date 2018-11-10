//------------------------------------------------------------------------------
//  NetSystem.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "NetSystem.h"

#include "../http/ev/EvHttpServer.h"
#include "../http/ev/LibevhtpServer.h"

#include "../http/libcurl/CURLHttp.h"
#include "../http/libcurl/CURLSSL.h"

#if (NETSYSTEM_MT >= 0)
#include "../lobby/KjLobby.h"
#include "../lobby/KjGateLobby.h"
#include "../lobby/KjNullLobby.h"
#else
#include "../lobby/UvLobby.h"
#include "../lobby/UvGateLobby.h"
#include "../lobby/UvNullLobby.h"
#endif

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

static void
default_stats_cb(int, int, int, int) {
	// do nothing
}

//------------------------------------------------------------------------------
/**

*/
CNetSystem::CNetSystem()
	: _stats_cb(default_stats_cb)
	, _isLobbyReady(false)
	, _lobby(nullptr) {

#ifdef WIN32
	WSADATA WSAData;
	DWORD Ret;
	if ((Ret = WSAStartup(MAKEWORD(2, 2), &WSAData)) != 0) {
		fprintf(stderr, "WSAStartup failed with error %d!\n", Ret);
	}
#endif

	//
	curl_global_init(CURL_GLOBAL_ALL);
}

//------------------------------------------------------------------------------
/**

*/
CNetSystem::~CNetSystem() {
	SAFE_DELETE(_lobby);

	//
	curl_global_cleanup();

	//
	google::protobuf::ShutdownProtobufLibrary();

#ifdef WIN32
	DWORD Ret;
	if ((Ret = WSACleanup()) != 0) {
		fprintf(stderr, "WSACleanup failed with error %d!!!\n", Ret);
	}
#endif
}

//------------------------------------------------------------------------------
/**

*/
int
CNetSystem::Init(LOBBY_TYPE eType, unsigned int port, StdLog *pLog, const std::function<void(int, int, int, int)> stats_cb) {
	// notice
	if (pLog)
		pLog->logprint(LOG_LEVEL_NOTICE, "\n[CNetSystem::Init()] port(%d) type(%d)...\n",
		port, eType);

	if (stats_cb)
		_stats_cb = stats_cb;

	switch (eType) {
	case LOBBY_TYPE_NULL: {
		// dummy lobby
		return InitNullLobby();
	}

	case LOBBY_TYPE_TCP_SERVER: {
		// lobby with server
		return InitLobby(port, pLog);
	}

	case LOBBY_TYPE_GATE_TCP_SERVER: {
		// lobby with gate server
		return InitGateLobby(port, pLog);
	}

	default:
		break;
	}
	return -1;
}

//------------------------------------------------------------------------------
/**

*/
void
CNetSystem::Dispose() {
	//
	_stats_cb = default_stats_cb;

	// wait all message send.
	DisposeLobby();
}

//------------------------------------------------------------------------------
/**

*/
void
CNetSystem::Update() {
	//
	_lobby->OnUpdate();

	//
	_stats_cb(
		_lobby->GetTcpConnFactory().GetConnManager().GetClientCount(),
		_lobby->GetAccountManager().GetOnlineList().GetOnlineCount(),
		CTcpConnManager::s_recv,
		CTcpConnManager::s_send);
}

//------------------------------------------------------------------------------
/**

*/
IHttpServer *
CNetSystem::OpenHttpServer(unsigned short nPort) {
	IHttpServer *pHttpServer = new CEvHttpServer(nPort);
	return pHttpServer;
}

//------------------------------------------------------------------------------
/**

*/
IHttpServer *
CNetSystem::OpenLibevhtpServer(unsigned short nPort) {
	IHttpServer *pHttpServer = new CLibevhtpServer(nPort);
	return pHttpServer;
}

//------------------------------------------------------------------------------
/**

*/
IURLHttpRequest *
CNetSystem::OpenURLHttpRequest() {
	return new CCURLHttp();
}

//------------------------------------------------------------------------------
/**

*/
IURLSSLRequest *
CNetSystem::OpenURLSSLRequest() {
	return new CCURLSSL();
}

//------------------------------------------------------------------------------
/**

*/
int
CNetSystem::InitNullLobby() {
#if (NETSYSTEM_MT >= 0)
	_lobby = new CKjNullLobby();
#else
	_lobby = new CUvNullLobby();
#endif
	if (0 == _lobby->OnInit()) {
		_isLobbyReady = true;
		return 0;
	}
	return -1;
}

//------------------------------------------------------------------------------
/**

*/
int
CNetSystem::InitLobby(unsigned short nPort, StdLog *pLog) {
#if (NETSYSTEM_MT >= 0)
	_lobby = new CKjLobby(nPort, pLog);
#else
	_lobby = new CUvLobby(nPort, pLog);
#endif
	if (0 == _lobby->OnInit()) {
		_isLobbyReady = true;
		return 0;
	}
	return -1;
}

//------------------------------------------------------------------------------
/**

*/
int
CNetSystem::InitGateLobby(unsigned short nPort, StdLog *pLog) {
#if (NETSYSTEM_MT >= 0)
	_lobby = new CKjGateLobby(nPort, pLog);
#else
	_lobby = new CUvGateLobby(nPort, pLog);
#endif
	if (0 == _lobby->OnInit()) {
		_isLobbyReady = true;
		return 0;
	}
	return -1;
}

//------------------------------------------------------------------------------
/**

*/
void
CNetSystem::DisposeLobby() {
	if (_lobby) {
		_lobby->OnDelete();
	}
	_isLobbyReady = false;
}

/** -- EOF -- **/