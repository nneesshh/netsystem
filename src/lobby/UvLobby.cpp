//------------------------------------------------------------------------------
//  UvLobby.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "UvLobby.h"

#include "../tcp/uv/UvConnFactory.h"
#include "../tcp/uv/UvServer.h"

#include "SimpleZoneManager.h"
#include "SimpleAccountManager.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CUvLobby::CUvLobby(unsigned short nPort)
	: _port(nPort)
	, _tcpConnFactory(new CUvConnFactory())
	, _zoneManager(new CSimpleZoneManager())
	, _accountManager(new CSimpleAccountManager()) {

}

//------------------------------------------------------------------------------
/**

*/
CUvLobby::~CUvLobby() {

	SAFE_DELETE(_accountManager);
	SAFE_DELETE(_zoneManager);
	SAFE_DELETE(_tcpConnFactory);
}

//------------------------------------------------------------------------------
/**

*/
int
CUvLobby::OnInit() {
	// prepare for connections
	_tcpConnFactory->OnInit();

	//
	_mainServer = _tcpConnFactory->CreateTcpServer();
	return _tcpConnFactory->OpenTcpServer(_mainServer, _port);
}

//------------------------------------------------------------------------------
/**

*/
void
CUvLobby::OnDelete() {
	// clear main server
	_tcpConnFactory->CloseTcpServer(_mainServer);
	_mainServer = nullptr;

	// clear all connections
	_tcpConnFactory->OnDelete();
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CUvLobby::OpenTcpIsolated(std::string& sIp_or_Hostname, unsigned short nPort) {
	ITcpIsolated *pIsolated = _tcpConnFactory->CreateTcpIsolated();
	if (pIsolated) {

		fprintf(stderr, "[CUvLobby::OpenTcpIsolated()] connect to ip(%s)port(%d)...connptr(0x%08Ix)\n",
			sIp_or_Hostname.c_str(), nPort, (uintptr_t)pIsolated);

		_tcpConnFactory->IsolatedConnConnect(pIsolated, sIp_or_Hostname, nPort);
	}
	return pIsolated;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CUvLobby::OpenTcpIsolated2(std::string& sIp_or_Hostname, unsigned short nPort) {
	ITcpIsolated *pIsolated2 = _tcpConnFactory->CreateTcpIsolated2();
	if (pIsolated2) {

		fprintf(stderr, "[CUvLobby::OpenTcpIsolated2()] connect to ip(%s)port(%d)...connptr(0x%08Ix)\n",
			sIp_or_Hostname.c_str(), nPort, (uintptr_t)pIsolated2);

		_tcpConnFactory->IsolatedConnConnect(pIsolated2, sIp_or_Hostname, nPort);
	}
	return pIsolated2;
}

/** -- EOF -- **/