//------------------------------------------------------------------------------
//  KjConnFactoryBase.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjConnFactoryBase.hpp"

#include "../../base/MyMacros.h"
#include "../../base/platform_utilities.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CKjConnFactoryBase::CKjConnFactoryBase()
	: _connManager(this) {

}

//------------------------------------------------------------------------------
/**

*/
CKjConnFactoryBase::~CKjConnFactoryBase() noexcept {
	_refPipeWorker = nullptr;
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryBase::OnInit() {
	_workQueue = std::make_shared<CKjConnFactoryWorkQueue>(this);
	_trunkQueue = std::make_shared<CConnFactoryTrunkQueue>(this);

	//
	StartPipeWorker();
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryBase::OnDelete() {
	// remove all isolated
	_connManager.ReleaseAllIsolateds();

	// remove all clients
	for (auto& pServer : _vClosedServer) {
		_connManager.ReleaseAllClients(pServer);
	}

	// wait and recycle
	while (_connManager.Recycle()) {
		// pipe_trunk_read_loop can't execute now because main thread is blocked here
		// , so we must update trunk queue manually.
		_trunkQueue->RunOnce();

		//
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

	// join
	//if (_pipeThread.thread->joinable()) {
	//	_pipeThread.thread->join();
	//}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryBase::StartPipeWorker() {
	// create pipe thread
	_refPipeWorker = netsystem_get_servercore()->NewPipeWorker(
		"conn factory worker",
		_trunkOpCodeRecvBuf,
		sizeof(_trunkOpCodeRecvBuf),
		[this](size_t amount) { _trunkQueue->RunOnce();	},
		[this](svrcore_pipeworker_t *worker) {
		// work
		_workQueue->Run(worker);
	});
}

/* -- EOF -- */