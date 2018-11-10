//------------------------------------------------------------------------------
//  ConnFactoryTrunkQueue.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "ConnFactoryTrunkQueue.hpp"

#include "../ITcpEventManager.h"
#include "../ITcpConnFactory.h"

//------------------------------------------------------------------------------
/**

*/
CConnFactoryTrunkQueue::CConnFactoryTrunkQueue()
	: _workQueue(std::make_shared<CCamelReaderWriterQueueWL>()) {

}

//------------------------------------------------------------------------------
/**

*/
CConnFactoryTrunkQueue::~CConnFactoryTrunkQueue() {

}

//------------------------------------------------------------------------------
/**

*/
void
CConnFactoryTrunkQueue::AddAcceptClientCb(ITcpServer *pServer, uintptr_t streamptr, std::string& sPeerIp) {

	auto workCb = std::bind([](ITcpServer *pServer, uintptr_t streamptr, const std::string& sPeerIp) {

		pServer->OnAcceptClient(streamptr, sPeerIp);
  	}, pServer, streamptr, std::move(sPeerIp));

	//
 	Add(std::move(workCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CConnFactoryTrunkQueue::AddClientDisconnectCb(ITcpServer *pServer, ITcpClient *pClient, ITcpConnManager *pConnMgr) {

	auto workCb = std::bind([](ITcpServer *pServer, ITcpClient *pClient, ITcpConnManager *pConnMgr) {

		pClient->IncrFrontEndConsumeNum();

		//
		pClient->OnDisconnect();

		// remove and dispose conn
		pServer->GetConnFactory().GetConnManager().OnRemoveClient(pServer, pClient);

	}, pServer, pClient, pConnMgr);
	pClient->IncrBackEndProduceNum();

	//
	Add(std::move(workCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CConnFactoryTrunkQueue::AddClientPacketCb(ITcpClient *pClient, uint64_t uConnId, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) {

	auto workCb = std::bind([](ITcpClient *pClient, uint64_t uConnId, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

		pClient->IncrFrontEndConsumeNum();

		assert(pClient->GetConnId() == uConnId);

		//
		pClient->GetEventManager().OnPacket(pClient, uSerialNo, sTypeName, sBody);

	}, pClient, uConnId, uSerialNo, std::move(sTypeName), std::move(sBody));
	pClient->IncrBackEndProduceNum();

	//
  	Add(std::move(workCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CConnFactoryTrunkQueue::AddClientInnerPacketCb(ITcpClient *pClient, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) {

	auto workCb = std::bind([](ITcpClient *pClient, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

		pClient->IncrFrontEndConsumeNum();

		assert(pClient->GetConnId() == uConnId);

		//
		pClient->GetEventManager().OnInnerPacket(pClient, uInnerUuid, uSerialNo, sTypeName, sBody);

	}, pClient, uConnId, uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
	pClient->IncrBackEndProduceNum();

	//
	Add(std::move(workCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CConnFactoryTrunkQueue::AddIsolatedConnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr) {

	assert(!pIsolated->IsConnected());

	auto workCb = std::bind([](ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr) {

		pIsolated->IncrFrontEndConsumeNum();

		fprintf(stderr, "Connect ok -- connid(%08llu)connptr(0x%08Ix).\n",
			pIsolated->GetConnId(), (uintptr_t)pIsolated);
		pIsolated->OnConnect();

	}, pIsolated, pConnMgr);
	pIsolated->IncrBackEndProduceNum();

	//
	Add(std::move(workCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CConnFactoryTrunkQueue::AddIsolatedDisconnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr) {

	assert(pIsolated->IsConnected());

	auto workCb = std::bind([](ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr) {

		pIsolated->IncrFrontEndConsumeNum();

		//
		pIsolated->OnDisconnect();

		// note: must not remove this connection because it will reconnect automatically

	}, pIsolated, pConnMgr);
	pIsolated->IncrBackEndProduceNum();

	//
	Add(std::move(workCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CConnFactoryTrunkQueue::AddIsolatedPacketCb(ITcpIsolated *pIsolated, uint64_t uConnId, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) {

	auto workCb = std::bind([](ITcpIsolated *pIsolated, uint64_t uConnId, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

		pIsolated->IncrFrontEndConsumeNum();

		assert(pIsolated->GetConnId() == uConnId);

		//
		pIsolated->GetEventManager().OnPacket(pIsolated, uSerialNo, sTypeName, sBody);

	}, pIsolated, uConnId, uSerialNo, std::move(sTypeName), std::move(sBody));
	pIsolated->IncrBackEndProduceNum();

	//
	Add(std::move(workCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CConnFactoryTrunkQueue::AddIsolatedInnerPacketCb(ITcpIsolated *pIsolated, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) {

	auto workCb = std::bind([](ITcpIsolated *pIsolated, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

		pIsolated->IncrFrontEndConsumeNum();

		assert(pIsolated->GetConnId() == uConnId);

		pIsolated->GetEventManager().OnInnerPacket(pIsolated, uInnerUuid, uSerialNo, sTypeName, sBody);

	}, pIsolated, uConnId, uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
	pIsolated->IncrBackEndProduceNum();

	//
	Add(std::move(workCb));
}

/* -- EOF -- */