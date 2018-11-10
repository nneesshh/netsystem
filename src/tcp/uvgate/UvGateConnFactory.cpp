//------------------------------------------------------------------------------
//  UvGateConnFactory.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "UvGateConnFactory.h"

#include "../TcpConnManager.h"
#include "../TcpEventManager.h"

#include "UvGateServer.h"
#include "UvGateClientConn.h"

#include "UvGateIsolatedConn.h"
#include "UvGateIsolatedConn2.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CUvGateConnFactory::CUvGateConnFactory(StdLog *pLog)
	: CUvConnFactoryBase(pLog) {

}

//------------------------------------------------------------------------------
/**

*/
CUvGateConnFactory::~CUvGateConnFactory() {

}

//------------------------------------------------------------------------------
/**

*/
ITcpServer *
CUvGateConnFactory::CreateTcpServer() {
	ITcpServer *pServer = new CUvGateServer(this);
	_mapServer[(uintptr_t)pServer] = pServer;
	return pServer;
}

//------------------------------------------------------------------------------
/**

*/
ITcpClient *
CUvGateConnFactory::CreateTcpClientOnServer(const std::string& sPeerIp, ITcpServer *pServer) {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpClient *pClient = new CUvGateClientConn(uConnId, sPeerIp, pServer);
	_connManager.OnAddClient(pServer, pClient);
	return pClient;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CUvGateConnFactory::CreateTcpIsolated() {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpIsolated *pIsolated = new CUvGateIsolatedConn(uConnId, this);
	_connManager.OnAddIsolated(pIsolated);
	return pIsolated;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CUvGateConnFactory::CreateTcpIsolated2() {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpIsolated *pIsolated2 = new CUvGateIsolatedConn2(uConnId, this);
	_connManager.OnAddIsolated(pIsolated2);
	return pIsolated2;
}

/* -- EOF -- */