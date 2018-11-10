//------------------------------------------------------------------------------
//  TcpConnManager.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "TcpConnManager.h"

#include <time.h>
#include <locale.h>

#include "tcp_packet_def.h"
#include "TcpInnerPacket.h"
#include "TcpOuterPacket.h"

#include "ITcpEventManager.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

size_t CTcpConnManager::s_send = 0;
size_t CTcpConnManager::s_recv = 0;

//------------------------------------------------------------------------------
/**

*/
CTcpConnManager::CTcpConnManager(ITcpConnFactory *pFactory)
	: _refConnFactory(pFactory) {

	CTcpInnerPacket::InitPacketBuffer();
	CTcpOuterPacket::InitPacketBuffer();
}

//------------------------------------------------------------------------------
/**

*/
CTcpConnManager::~CTcpConnManager() {

	CTcpInnerPacket::DestroyPacketBuffer();
	CTcpOuterPacket::DestroyPacketBuffer();
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpConnManager::OnAddClient(ITcpServer *pServer, ITcpClient *pClient) {

	uintptr_t serverptr = (uintptr_t)pServer;
	uint64_t uConnId = pClient->GetConnId();
	assert(serverptr > 0 && uConnId > 0);

	//
	_mapServerEntry[serverptr]._mapClients[uConnId] = pClient;
	++_szClientCount;

#ifdef _DEBUG
	int nSum = 0;
	for (auto& iter : _mapServerEntry) {
		nSum += iter.second._mapClients.size();
	}

	assert(_szClientCount == nSum);
#endif
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpConnManager::OnRemoveClient(ITcpServer *pServer, ITcpClient *pClient) {

	uintptr_t serverptr = (uintptr_t)pServer;
	uint64_t uConnId = pClient->GetConnId();
	assert(serverptr > 0 && uConnId > 0);

	auto& it1 = _mapServerEntry.find(serverptr);
	if (it1 != _mapServerEntry.end()) {
		//
		auto& entry = it1->second;
		MAP_TCP_CONN_MANAGER_CLIENTS::iterator it2 = entry._mapClients.find(uConnId);
		if (it2 != entry._mapClients.end()) {

			ITcpClient *pClient2 = it2->second;
			assert(pClient2 == pClient);

			// dispose to backend through factory
			pServer->GetConnFactory().DisposeConnection(pClient2);

			entry._mapClients.erase(it2);

			// recycle connection
			RecyclerPush(pClient2);

			//
			--_szClientCount;

#ifdef _DEBUG
			int nSum = 0;
			for (auto& iter : _mapServerEntry) {
				nSum += iter.second._mapClients.size();
			}

			assert(_szClientCount == nSum);
#endif

		}
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpConnManager::OnRemoveAllClients(ITcpServer *pServer) {

	uintptr_t serverptr = (uintptr_t)pServer;
	assert(serverptr > 0);

	// must clear because handler may be released already
	pServer->GetEventManager().ClearAllEventHandlers();

	// walk
	auto& it1 = _mapServerEntry.find(serverptr);
	if (it1 != _mapServerEntry.end()) {
		//
		auto& entry = it1->second;
		for (auto& it2 : entry._mapClients) {
			//
			uint64_t uConnId = it2.first;
			ITcpClient *pClient = it2.second;

			assert(uConnId == pClient->GetConnId());

			// dispose to backend through factory
			_refConnFactory->DisposeConnection(pClient);

			// recycle connection
			RecyclerPush(pClient);

			//
			--_szClientCount;
		}
		entry._mapClients.clear();
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpConnManager::OnDisposeAllClients(ITcpServer *pServer) {

	uintptr_t serverptr = (uintptr_t)pServer;
	assert(serverptr > 0);

	auto& it1 = _mapServerEntry.find(serverptr);
	if (it1 != _mapServerEntry.end()) {
		//
		auto& entry = it1->second;
		for (auto& it2 : entry._mapClients) {
			//
			uint64_t uConnId = it2.first;
			ITcpClient *pClient = it2.second;
			if (pClient
				&& pClient->GetConnId() == uConnId) {
				// dispose
				pClient->DisposeConnection();

				// disconnect cb to front end
				// Note: Client may be [ false == IsConnected() ], but we always need main thread to [ RemoveClient() ]
				_refConnFactory->AddClientDisconnectCb(pServer, pClient, &_refConnFactory->GetConnManager());
			}
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpConnManager::OnAddIsolated(ITcpIsolated *pIsolated) {
	
	uint64_t uConnId = pIsolated->GetConnId();
	assert(uConnId > 0);

	//
	_mapIsolated[uConnId] = pIsolated;
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpConnManager::OnRemoveAllIsolateds() {

	for (auto& iter : _mapIsolated) {
		ITcpIsolated *pIsolated = iter.second;

		// must clear because handler may be released already
		pIsolated->GetEventManager().ClearAllEventHandlers();

		// stop running
		pIsolated->Disconnect();

		// dispose to backend through factory
		_refConnFactory->DisposeConnection(pIsolated);

		// recycle connection
		RecyclerPush(pIsolated);
	}
	_mapIsolated.clear();
}

//------------------------------------------------------------------------------
/**

*/
ITcpClient *
CTcpConnManager::LookupClientByConnId(uint64_t uConnId) {

	for (auto& it1 : _mapServerEntry) {
		auto& entry = it1.second;
		MAP_TCP_CONN_MANAGER_CLIENTS::iterator it2 = entry._mapClients.find(uConnId);
		if (it2 != entry._mapClients.end()) {
			return it2->second;
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
/**

*/
bool
CTcpConnManager::Recycle() {

	RecyclerPop();
	SafeRelease();
	return (_vRecycler.size() > 0);
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpConnManager::RecyclerPush(ITcpConn *pConn) {
	// add to recycle
	_vRecycler.push_back(pConn);
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpConnManager::RecyclerPop() {
	//
	ITcpConn *pConn;
	CONN_RECYCLER::iterator it = _vRecycler.begin(),
		itEnd = _vRecycler.end();
	while (it != itEnd) {

		pConn = (*it);

		// (0 == reference count) means thread payload queue is cleaned
		// disposed means thread is released ok
		if (!pConn->IsConnected()
			&& pConn->IsFrontEndClean() // front end reference count = 0
			&& pConn->IsBackEndClean() // back end reference count = 0
			&& pConn->IsDisposed()
			&& pConn->IsFlushed()) {
			//
			_vSafeToRelease.push_back(pConn);

			it = _vRecycler.erase(it);
			itEnd = _vRecycler.end();
		}
		else {
			++it;
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpConnManager::SafeRelease() {
	//
	for (auto& pConn : _vSafeToRelease) {
		//
		SAFE_DELETE(pConn);
	}
	_vSafeToRelease.clear();
}

/* -- EOF -- */