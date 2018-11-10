//------------------------------------------------------------------------------
//  KjGateLobby.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjGateLobby.h"

#include "../tcp/kjgate/KjGateConnFactory.hpp"
#include "../tcp/kjgate/KjGateServer.hpp"

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
CKjGateLobby::CKjGateLobby(unsigned short nPort, StdLog *pLog)
	: _port(nPort)
	, _tcpConnFactory(new CKjGateConnFactory(pLog))
	, _zoneManager(new CSimpleZoneManager())
	, _accountManager(new CSimpleAccountManager()) {

}

//------------------------------------------------------------------------------
/**

*/
CKjGateLobby::~CKjGateLobby() {

	SAFE_DELETE(_accountManager);
	SAFE_DELETE(_zoneManager);
	SAFE_DELETE(_tcpConnFactory);
}

//------------------------------------------------------------------------------
/**

*/
int
CKjGateLobby::OnInit() {
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
CKjGateLobby::OnDelete() {
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
CKjGateLobby::OpenTcpIsolated(std::string& sIp_or_Hostname, unsigned short nPort) {
	ITcpIsolated *pIsolated = _tcpConnFactory->CreateTcpIsolated();
	if (pIsolated) {

		fprintf(stderr, "[CKjGateLobby::OpenTcpIsolated()] connect to ip(%s)port(%d)...connptr(0x%08Ix)\n",
			sIp_or_Hostname.c_str(), nPort, (uintptr_t)pIsolated);

		_tcpConnFactory->IsolatedConnConnect(pIsolated, sIp_or_Hostname, nPort);
	}
	return pIsolated;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CKjGateLobby::OpenTcpIsolated2(std::string& sIp_or_Hostname, unsigned short nPort) {
	ITcpIsolated *pIsolated2 = _tcpConnFactory->CreateTcpIsolated2();
	if (pIsolated2) {

		fprintf(stderr, "[CKjGateLobby::OpenTcpIsolated2()] connect to ip(%s)port(%d)...connptr(0x%08Ix)\n",
			sIp_or_Hostname.c_str(), nPort, (uintptr_t)pIsolated2);

		_tcpConnFactory->IsolatedConnConnect(pIsolated2, sIp_or_Hostname, nPort);
	}
	return pIsolated2;
}

/** -- EOF -- **/