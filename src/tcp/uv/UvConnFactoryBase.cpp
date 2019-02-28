//------------------------------------------------------------------------------
//  UvConnFactoryBase.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "UvConnFactoryBase.h"

#include "../../base/MyMacros.h"
#include "../../base/platform_utilities.h"

#include "../../netsystem/RootContextDef.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CUvConnFactoryBase::CUvConnFactoryBase()
	: _loopBase(uv_loop_new_())
	, _connManager(this) {
	
}

//------------------------------------------------------------------------------
/**

*/
CUvConnFactoryBase::~CUvConnFactoryBase() {
	if (_loopBase) {
		uv_loop_delete_((uv_loop_t *)_loopBase);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CUvConnFactoryBase::OnInit() {
	
}

//------------------------------------------------------------------------------
/**

*/
void
CUvConnFactoryBase::OnDelete() {
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
void
CUvConnFactoryBase::AddIsolatedConnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr) {

	StdLog *pLog = netsystem_get_log();
	if (pLog)
		pLog->logprint(LOG_LEVEL_NOTICE, "[CUvConnFactoryBase::AddIsolatedConnectCb()] Connect ok -- connid(%08llu)connptr(0x%08Ix).\n",
			pIsolated->GetConnId(), (uintptr_t)pIsolated);

// 	fprintf(stderr, "[CUvConnFactoryBase::AddIsolatedConnectCb()] Connect ok -- connid(%08llu)connptr(0x%08Ix).\n",
// 		pIsolated->GetConnId(), (uintptr_t)pIsolated);

	pIsolated->OnConnect();
}

/* -- EOF -- */