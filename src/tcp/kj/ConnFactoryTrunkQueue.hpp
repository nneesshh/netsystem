#pragma once
//------------------------------------------------------------------------------
/**
	@class CConnFactoryTrunkQueue

	(C) 2016 n.lee
*/
#include <string>

#include "base/CamelReaderWriterQueueWL.h"

#include "KjConnFactoryWorkQueue.hpp"

#include "../ITcpServer.h"
#include "../ITcpClient.h"
#include "../ITcpIsolated.h"
#include "../ITcpConnManager.h"

class CKjConnFactoryBase;

//------------------------------------------------------------------------------
/**
	@brief CConnFactoryTrunkQueue
	
	(C) 2016 n.lee
*/
class CConnFactoryTrunkQueue {
public:
	CConnFactoryTrunkQueue(CKjConnFactoryBase *pFactory);
	~CConnFactoryTrunkQueue();

	void RunOnce() {
		_callbacks->RunOnce();
	}

	void Close() {
		_callbacks->Close();
	}

	void Add(std::function<void()>&& workCb);

	void AddAcceptClientCb(ITcpServer *pServer, uintptr_t streamptr, std::string&& sPeerIp);
	void AddClientDisconnectCb(ITcpServer *pServer, ITcpClient *pClient, ITcpConnManager *pConnMgr);
	void AddClientPacketCb(ITcpClient *pClient, uint64_t uConnId, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody);
	void AddClientInnerPacketCb(ITcpClient *pClient, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody);

	void AddIsolatedConnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr);
	void AddIsolatedDisconnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr);
	void AddIsolatedPacketCb(ITcpIsolated *pIsolated, uint64_t uConnId, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody);
	void AddIsolatedInnerPacketCb(ITcpIsolated *pIsolated, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody);

private:
	CCamelReaderWriterQueueWLPtr _callbacks;

public:
	CKjConnFactoryBase *_refConnFactory;

};
using CConnFactoryTrunkQueuePtr = std::shared_ptr<CConnFactoryTrunkQueue>;

/*EOF*/