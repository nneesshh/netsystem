//------------------------------------------------------------------------------
//  KjGateConnFactory.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjGateConnFactory.hpp"

#include "../TcpConnManager.h"
#include "../TcpEventManager.h"

#include "KjGateServer.hpp"
#include "KjGateClientConn.hpp"

#include "KjGateIsolatedConn.hpp"
#include "KjGateIsolatedConn2.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CKjGateConnFactory::CKjGateConnFactory()
	: CKjConnFactoryBase() {

}

//------------------------------------------------------------------------------
/**

*/
CKjGateConnFactory::~CKjGateConnFactory() {
	
}

//------------------------------------------------------------------------------
/**

*/
ITcpServer *
CKjGateConnFactory::CreateTcpServer() {
	ITcpServer *pServer = new CKjGateServer(this);
	_mapServer[(uintptr_t)pServer] = pServer;
	return pServer;
}

//------------------------------------------------------------------------------
/**

*/
ITcpClient *
CKjGateConnFactory::CreateTcpClientOnServer(std::string&& sPeerIp, ITcpServer *pServer) {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpClient *pClient = new CKjGateClientConn(uConnId, std::move(sPeerIp), pServer);
	_connManager.OnAddClient(pServer, pClient);
	return pClient;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CKjGateConnFactory::CreateTcpIsolated() {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpIsolated *pIsolated = new CKjGateIsolatedConn(uConnId, this);
	_connManager.OnAddIsolated(pIsolated);
	return pIsolated;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CKjGateConnFactory::CreateTcpIsolated2() {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpIsolated *pIsolated2 = new CKjGateIsolatedConn2(uConnId, this);
	_connManager.OnAddIsolated(pIsolated2);
	return pIsolated2;
}

/* -- EOF -- */