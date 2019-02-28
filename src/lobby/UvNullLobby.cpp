//------------------------------------------------------------------------------
//  UvNullLobby.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "UvNullLobby.h"

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
CUvNullLobby::CUvNullLobby()
	: _tcpConnFactory(new CUvConnFactory())
	, _zoneManager(new CSimpleZoneManager())
	, _accountManager(new CSimpleAccountManager()) {

}

//------------------------------------------------------------------------------
/**

*/
CUvNullLobby::~CUvNullLobby() {

	SAFE_DELETE(_accountManager);
	SAFE_DELETE(_zoneManager);
	SAFE_DELETE(_tcpConnFactory);
}

//------------------------------------------------------------------------------
/**

*/
int
CUvNullLobby::OnInit() {
	// prepare for connections
	_tcpConnFactory->OnInit();
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
void
CUvNullLobby::OnDelete() {
	// clear all connections
	_tcpConnFactory->OnDelete();
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CUvNullLobby::OpenTcpIsolated(std::string& sIp_or_Hostname, unsigned short nPort) {
	ITcpIsolated *pIsolated = _tcpConnFactory->CreateTcpIsolated();
	if (pIsolated) {

		fprintf(stderr, "[CUvNullLobby::OpenTcpIsolated()] connect to ip(%s)port(%d)...connptr(0x%08Ix)\n",
			sIp_or_Hostname.c_str(), nPort, (uintptr_t)pIsolated);

		_tcpConnFactory->IsolatedConnConnect(pIsolated, sIp_or_Hostname, nPort);
	}
	return pIsolated;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CUvNullLobby::OpenTcpIsolated2(std::string& sIp_or_Hostname, unsigned short nPort) {
	ITcpIsolated *pIsolated2 = _tcpConnFactory->CreateTcpIsolated2();
	if (pIsolated2) {

		fprintf(stderr, "[CUvNullLobby::OpenTcpIsolated2()] connect to ip(%s)port(%d)...connptr(0x%08Ix)\n",
			sIp_or_Hostname.c_str(), nPort, (uintptr_t)pIsolated2);

		_tcpConnFactory->IsolatedConnConnect(pIsolated2, sIp_or_Hostname, nPort);
	}
	return pIsolated2;
}

/** -- EOF -- **/