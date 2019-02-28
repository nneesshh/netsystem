//------------------------------------------------------------------------------
//  UvConnFactory.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "UvConnFactory.h"

#include "../TcpConnManager.h"
#include "../TcpEventManager.h"

#include "UvServer.h"
#include "UvClientConn.h"

#include "UvIsolatedConn.h"
#include "UvIsolatedConn2.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CUvConnFactory::CUvConnFactory()
	: CUvConnFactoryBase() {

}

//------------------------------------------------------------------------------
/**

*/
CUvConnFactory::~CUvConnFactory() {

}

//------------------------------------------------------------------------------
/**

*/
ITcpServer *
CUvConnFactory::CreateTcpServer() {
	ITcpServer *pServer = new CUvServer(this);
	_mapServer[(uintptr_t)pServer] = pServer;
	return pServer;
}

//------------------------------------------------------------------------------
/**

*/
ITcpClient *
CUvConnFactory::CreateTcpClientOnServer(std::string&& sPeerIp, ITcpServer *pServer) {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpClient *pClient = new CUvClientConn(uConnId, std::move(sPeerIp), pServer);
	_connManager.OnAddClient(pServer, pClient);
	return pClient;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CUvConnFactory::CreateTcpIsolated() {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpIsolated *pIsolated = new CUvIsolatedConn(uConnId, this);
	_connManager.OnAddIsolated(pIsolated);
	return pIsolated;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CUvConnFactory::CreateTcpIsolated2() {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpIsolated *pIsolated2 = new CUvIsolatedConn2(uConnId, this);
	_connManager.OnAddIsolated(pIsolated2);
	return pIsolated2;
}

/* -- EOF -- */