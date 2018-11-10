//------------------------------------------------------------------------------
//  UvConnFactoryBase.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "UvConnFactoryBase.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CUvConnFactoryBase::CUvConnFactoryBase(StdLog *pLog)
	: _loopBase(uv_loop_new_())
	, _refLog(pLog)
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
	_connManager.OnRemoveAllIsolateds();

	// remove all clients
	for (auto& pServer : _vClosedServer) {
		_connManager.OnRemoveAllClients(pServer);
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

/* -- EOF -- */