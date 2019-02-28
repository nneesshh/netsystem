#pragma once
//------------------------------------------------------------------------------
/**
	@class CKjConnFactoryBase

	(C) 2016 n.lee
*/
#include "../../netsystem/RootContextDef.hpp"
#include "../TcpConnManager.h"

#include "ConnFactoryTrunkQueue.hpp"
#include "KjConnFactoryWorkQueue.hpp"

//------------------------------------------------------------------------------
/**

*/
class CKjConnFactoryBase : public ITcpConnFactory {
public:
	CKjConnFactoryBase();
	virtual ~CKjConnFactoryBase() noexcept;

	using CLOSED_SERVER_LIST = std::vector<ITcpServer *>;

	/** **/
	virtual void				OnInit() override;
	virtual void				OnDelete() override;

	virtual void				OnUpdate() override {
		_connManager.OnCheckConnection();
	}

	/** **/
	virtual void *				GetLoopBase() override {
		return netsystem_get_servercore()->GetCtx();
	}

	virtual ITcpConnManager&	GetConnManager() override {
		return _connManager;
	}

	/************************************************************************/
	/* Request send to backend (thread)                                     */
	/************************************************************************/
	virtual int					OpenTcpServer(ITcpServer *pServer, unsigned short nPort) override { return _workQueue->OpenTcpServer(pServer, nPort); }
	virtual void				CloseTcpServer(ITcpServer *pServer) override { _workQueue->CloseTcpServer(pServer); _vClosedServer.push_back(pServer); }
	virtual void				FlushTcpServerDownStream(ITcpServer *pServer, uintptr_t streamptr) override { _workQueue->FlushTcpServerDownStream(pServer, streamptr); }

	virtual void				ReleaseConnection(ITcpConn *pConn) override { _workQueue->DisposeConnection(pConn); }

	virtual void				PostPacket(ITcpConn *pConn, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) override {
		_workQueue->PostPacket(pConn, uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
	}

	virtual void				ConfirmClientIsReady(ITcpClient *pClient, uintptr_t streamptr) override { _workQueue->ConfirmClientIsReady(pClient, streamptr); }

	virtual void				PostBroadcastPacket(ITcpServer *pServer, std::vector<uint64_t>& vTarget, std::string&& sTypeName, std::string&& sBody) override {
		_workQueue->PostBroadcastPacket(pServer, vTarget, std::move(sTypeName), std::move(sBody));
	}

	virtual int					IsolatedConnConnect(ITcpIsolated *pIsolated, std::string sIp_or_Hostname, unsigned short nPort) override {
		return _workQueue->IsolatedConnConnect(pIsolated, sIp_or_Hostname, nPort);
	}

	virtual void				IsolatedConnDelayReconnect(ITcpIsolated *pIsolated, int nDelaySeconds) override {
		_workQueue->IsolatedConnDelayReconnect(pIsolated, nDelaySeconds);
	}

	/************************************************************************/
	/* Callback recv from backend (thread)                                  */
	/************************************************************************/
	virtual void				AddAcceptClientCb(ITcpServer *pServer, uintptr_t streamptr, std::string&& sPeerIp) override {
		_trunkQueue->AddAcceptClientCb(pServer, streamptr, std::move(sPeerIp));
	}

	virtual void				AddClientDisconnectCb(ITcpServer *pServer, ITcpClient *pClient, ITcpConnManager *pConnMgr) override {
		_trunkQueue->AddClientDisconnectCb(pServer, pClient, pConnMgr);
	}

	virtual void				AddClientPacketCb(ITcpClient *pClient, uint64_t uConnId, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) override {
		_trunkQueue->AddClientPacketCb(pClient, uConnId, uSerialNo, std::move(sTypeName), std::move(sBody));
	}

	virtual void				AddClientInnerPacketCb(ITcpClient *pClient, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) override {
		_trunkQueue->AddClientInnerPacketCb(pClient, uConnId, uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
	}

	virtual void				AddIsolatedConnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr) override {
		_trunkQueue->AddIsolatedConnectCb(pIsolated, pConnMgr);
	}

	virtual void				AddIsolatedDisconnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr) override {
		_trunkQueue->AddIsolatedDisconnectCb(pIsolated, pConnMgr);
	}

	virtual void				AddIsolatedPacketCb(ITcpIsolated *pIsolated, uint64_t uConnId, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) override {
		_trunkQueue->AddIsolatedPacketCb(pIsolated, uConnId, uSerialNo, std::move(sTypeName), std::move(sBody));
	}

	virtual void				AddIsolatedInnerPacketCb(ITcpIsolated *pIsolated, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) override {
		_trunkQueue->AddIsolatedInnerPacketCb(pIsolated, uConnId, uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
	}

private:
	void						StartPipeWorker();

public:
	CConnFactoryTrunkQueuePtr _trunkQueue;
	CKjConnFactoryWorkQueuePtr _workQueue;

	//! threads
	svrcore_pipeworker_t *_refPipeWorker = nullptr;
	char _trunkOpCodeSend = 0;
	char _trunkOpCodeRecvBuf[1024];

protected:
	CTcpConnManager _connManager;

	std::unordered_map<uintptr_t, ITcpServer *> _mapServer; // pointer 2 server
	CLOSED_SERVER_LIST _vClosedServer;

};

/*EOF*/