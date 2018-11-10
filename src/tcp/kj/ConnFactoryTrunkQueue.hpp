#pragma once
//------------------------------------------------------------------------------
/**
@class CConnFactoryTrunkQueue

(C) 2016 n.lee
*/
#include <string>
#include "base/CamelReaderWriterQueueWL.h"

#include "../ITcpServer.h"
#include "../ITcpClient.h"
#include "../ITcpIsolated.h"
#include "../ITcpConnManager.h"

//------------------------------------------------------------------------------
/**
@brief CConnFactoryTrunkQueue
*/
class CConnFactoryTrunkQueue {
public:
	CConnFactoryTrunkQueue();
	~CConnFactoryTrunkQueue();

	void RunOnce() {
		_workQueue->RunOnce();
	}

	void Close() {
		_workQueue->Close();
	}

	void Add(std::function<void()>&& workCb) {
		_workQueue->Add(std::move(workCb));
	}

	void AddAcceptClientCb(ITcpServer *pServer, uintptr_t streamptr, std::string& sPeerIp);
	void AddClientDisconnectCb(ITcpServer *pServer, ITcpClient *pClient, ITcpConnManager *pConnMgr);
	void AddClientPacketCb(ITcpClient *pClient, uint64_t uConnId, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody);
	void AddClientInnerPacketCb(ITcpClient *pClient, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody);

	void AddIsolatedConnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr);
	void AddIsolatedDisconnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr);
	void AddIsolatedPacketCb(ITcpIsolated *pIsolated, uint64_t uConnId, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody);
	void AddIsolatedInnerPacketCb(ITcpIsolated *pIsolated, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody);

private:
	CCamelReaderWriterQueueWLPtr _workQueue;
};
using CConnFactoryTrunkQueuePtr = std::shared_ptr<CConnFactoryTrunkQueue>;

/*EOF*/