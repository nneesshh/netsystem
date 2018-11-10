//------------------------------------------------------------------------------
//  KjConnFactoryBase.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjConnFactoryBase.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CKjConnFactoryBase::CKjConnFactoryBase(StdLog *pLog)
	: _refLog(pLog)
	, _connManager(this) {

}

//------------------------------------------------------------------------------
/**

*/
CKjConnFactoryBase::~CKjConnFactoryBase() {

}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryBase::OnInit() {
	_trunkQueue = std::make_shared<CConnFactoryTrunkQueue>();
	_workQueue = std::make_shared<CKjConnFactoryWorkQueue>(this, kj::addRef(*g_rootContext));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryBase::OnDelete() {
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

	//
	_workQueue->Finish();
	_trunkQueue->Close();

	_workQueue = nullptr;
	_trunkQueue = nullptr;

	// real delete closed server
	for (auto& pServer : _vClosedServer) {
		//
		SAFE_DELETE(pServer);
	}
	_vClosedServer.clear();
}

/* -- EOF -- */