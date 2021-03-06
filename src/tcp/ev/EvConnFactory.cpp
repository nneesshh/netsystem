//------------------------------------------------------------------------------
//  EvConnFactory.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "EvConnFactory.h"

#include "../TcpConnManager.h"
#include "../TcpEventManager.h"

#include "EvClientConn.h"
#include "EvServer.h"

#include "EvIsolatedConn.h"
#include "EvIsolatedConn2.h"

#include "../../netsystem/RootContextDef.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

#define RECYCLE_INTERVAL_SECONDS 60

//------------------------------------------------------------------------------
/**

*/
CEvConnFactory::CEvConnFactory()
	: _loopBase(ev_loop_new_(0, 0))
	, _connManager(this) {
	
}

//------------------------------------------------------------------------------
/**

*/
CEvConnFactory::~CEvConnFactory() {

	if (_loopBase) {
		ev_loop_delete_((struct event_base *)_loopBase, 3);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CEvConnFactory::OnInit() {
	
}

//------------------------------------------------------------------------------
/**

*/
void
CEvConnFactory::OnDelete() {
	// remove all isolated
	_connManager.ReleaseAllIsolateds();

	// remove all clients
	for (auto& pServer : _vClosedServer) {
		_connManager.ReleaseAllClients(pServer);
	}

	// wait and recycle
	while (_connManager.Recycle()) {
		OnUpdate();
		util_sleep(10);
	}

	// wait loop clear
	if (_loopBase) {
		int nCount = 10000;
		int i;
		for (i = 0; i < nCount; ++i) {
			OnUpdate();
		}
	}

	// real delete closed server
	for (auto& pServer : _vClosedServer) {
		SAFE_DELETE(pServer);
	}
	_vClosedServer.clear();
}

//------------------------------------------------------------------------------
/**

*/
ITcpServer *
CEvConnFactory::CreateTcpServer() {
	ITcpServer *pServer = new CEvServer(this);
	_mapServer[(uintptr_t)pServer] = pServer;
	return pServer;
}

//------------------------------------------------------------------------------
/**

*/
ITcpClient *
CEvConnFactory::CreateTcpClientOnServer(std::string&& sPeerIp, ITcpServer *pServer) {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpClient *pClient = new CEvClientConn(uConnId, std::move(sPeerIp), pServer);
	_connManager.OnAddClient(pServer, pClient);
	return pClient;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CEvConnFactory::CreateTcpIsolated() {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpIsolated *pIsolated = new CEvIsolatedConn(uConnId, this);
	_connManager.OnAddIsolated(pIsolated);
	return pIsolated;
}

//------------------------------------------------------------------------------
/**

*/
ITcpIsolated *
CEvConnFactory::CreateTcpIsolated2() {
	uint64_t uConnId = _connManager.GetNextConnectionId();
	ITcpIsolated *pIsolated2 = new CEvIsolatedConn2(uConnId, this);
	_connManager.OnAddIsolated(pIsolated2);
	return pIsolated2;
}

//------------------------------------------------------------------------------
/**

*/
void
CEvConnFactory::AddIsolatedConnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr) {

	StdLog *pLog = netsystem_get_log();
	if (pLog)
		pLog->logprint(LOG_LEVEL_NOTICE, "[CEvConnFactory::AddIsolatedConnectCb()] Connect ok -- connid(%08llu)connptr(0x%08Ix).\n",
			pIsolated->GetConnId(), (uintptr_t)pIsolated);

// 	fprintf(stderr, "[CEvConnFactory::AddIsolatedConnectCb()] Connect ok -- connid(%08llu)connptr(0x%08Ix).\n",
// 		pIsolated->GetConnId(), (uintptr_t)pIsolated);

	pIsolated->OnConnect();
}

/* -- EOF -- */