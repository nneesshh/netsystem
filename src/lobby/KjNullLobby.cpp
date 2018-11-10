//------------------------------------------------------------------------------
//  KjNullLobby.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjNullLobby.h"

#include "../tcp/kj/KjConnFactory.hpp"
#include "../tcp/kj/KjServer.hpp"

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
CKjNullLobby::CKjNullLobby()
	: _tcpConnFactory(new CKjConnFactory(nullptr))
	, _zoneManager(new CSimpleZoneManager())
	, _accountManager(new CSimpleAccountManager()) {

}

//------------------------------------------------------------------------------
/**

*/
CKjNullLobby::~CKjNullLobby() {

	SAFE_DELETE(_accountManager);
	SAFE_DELETE(_zoneManager);
	SAFE_DELETE(_tcpConnFactory);
}

//------------------------------------------------------------------------------
/**

*/
int
CKjNullLobby::OnInit() {
	// prepare for connections
	_tcpConnFactory->OnInit();
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
void
CKjNullLobby::OnDelete() {
	// clear all connections
	_tcpConnFactory->OnDelete();
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CKjNullLobby::OpenTcpIsolated(std::string& sIp_or_Hostname, unsigned short nPort) {
	ITcpIsolated *pIsolated = _tcpConnFactory->CreateTcpIsolated();
	if (pIsolated) {

		fprintf(stderr, "[CKjNullLobby::OpenTcpIsolated()] connect to ip(%s)port(%d)...connptr(0x%08Ix)\n",
			sIp_or_Hostname.c_str(), nPort, (uintptr_t)pIsolated);

		_tcpConnFactory->IsolatedConnConnect(pIsolated, sIp_or_Hostname, nPort);
	}
	return pIsolated;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CKjNullLobby::OpenTcpIsolated2(std::string& sIp_or_Hostname, unsigned short nPort) {
	ITcpIsolated *pIsolated2 = _tcpConnFactory->CreateTcpIsolated2();
	if (pIsolated2) {

		fprintf(stderr, "[CKjNullLobby::OpenTcpIsolated2()] connect to ip(%s)port(%d)...connptr(0x%08Ix)\n",
			sIp_or_Hostname.c_str(), nPort, (uintptr_t)pIsolated2);

		_tcpConnFactory->IsolatedConnConnect(pIsolated2, sIp_or_Hostname, nPort);
	}
	return pIsolated2;
}

/** -- EOF -- **/