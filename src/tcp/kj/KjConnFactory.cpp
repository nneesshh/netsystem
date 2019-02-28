//------------------------------------------------------------------------------
//  KjConnFactory.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjConnFactory.hpp"

#include "../TcpConnManager.h"
#include "../TcpEventManager.h"

#include "KjServer.hpp"
#include "KjClientConn.hpp"

#include "KjIsolatedConn.hpp"
#include "KjIsolatedConn2.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CKjConnFactory::CKjConnFactory()
	: CKjConnFactoryBase() {

}

//------------------------------------------------------------------------------
/**

*/
CKjConnFactory::~CKjConnFactory() {
	
}

//------------------------------------------------------------------------------
/**

*/
ITcpServer *
CKjConnFactory::CreateTcpServer() {
	ITcpServer *pServer = new CKjServer(this);
	_mapServer[(uintptr_t)pServer] = pServer;
	return pServer;
}

//------------------------------------------------------------------------------
/**

*/
ITcpClient *
CKjConnFactory::CreateTcpClientOnServer(std::string&& sPeerIp, ITcpServer *pServer) {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpClient *pClient = new CKjClientConn(uConnId, std::move(sPeerIp), pServer);
	_connManager.OnAddClient(pServer, pClient);
	return pClient;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CKjConnFactory::CreateTcpIsolated() {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpIsolated *pIsolated = new CKjIsolatedConn(uConnId, this);
	_connManager.OnAddIsolated(pIsolated);
	return pIsolated;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CKjConnFactory::CreateTcpIsolated2() {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpIsolated *pIsolated2 = new CKjIsolatedConn2(uConnId, this);
	_connManager.OnAddIsolated(pIsolated2);
	return pIsolated2;
}

/* -- EOF -- */