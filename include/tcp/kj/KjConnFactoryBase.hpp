#pragma once
//------------------------------------------------------------------------------
/**
@class CKjConnFactoryBase

(C) 2016 n.lee
*/
#include "../kj/KjRootContextDef.hpp"
#include "../kj/ConnFactoryTrunkQueue.hpp"
#include "../kj/KjConnFactoryWorkQueue.hpp"

#include "../TcpConnManager.h"

//------------------------------------------------------------------------------
/**

*/
class CKjConnFactoryBase : public ITcpConnFactory {
public:
	CKjConnFactoryBase(StdLog *pLog);
	virtual ~CKjConnFactoryBase();

	using CLOSED_SERVER_LIST = std::vector<ITcpServer *>;

	/** **/
	virtual void				OnInit() override;
	virtual void				OnDelete() override;

	virtual void				OnUpdate() override {
		_trunkQueue->RunOnce();
		_connManager.OnCheckConnection();
	}

	/** **/
	virtual void *				GetLoopBase() override {
		return g_rootContext.get();
	}

	virtual StdLog *			GetLogHandler() override {
		return _refLog;
	}

	virtual ITcpConnManager&	GetConnManager() override {
		return _connManager;
	}

	/************************************************************************/
	/* Request send to backend (thread)                                     */
	/************************************************************************/
	virtual int					OpenTcpServer(ITcpServer *pServer, unsigned short nPort) override { return _workQueue->OpenTcpServer(pServer, nPort); }
	virtual void				CloseTcpServer(ITcpServer *pServer) override { _workQueue->CloseTcpServer(pServer); _vClosedServer.push_back(pServer); }
	virtual void				DisposeConnection(ITcpConn *pConn) override { _workQueue->DisposeConnection(pConn); }

	virtual void				PostPacket(ITcpConn *pConn, uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) override {
		_workQueue->PostPacket(pConn, uInnerUuid, uSerialNo, sTypeName, sBody);
	}

	virtual void				ConfirmClientIsReady(ITcpClient *pClient, uintptr_t streamptr) override { _workQueue->ConfirmClientIsReady(pClient, streamptr); }

	virtual void				PostBroadcastPacket(ITcpServer *pServer, std::vector<uint64_t>& vTarget, std::string& sTypeName, std::string& sBody) override {
		_workQueue->PostBroadcastPacket(pServer, vTarget, sTypeName, sBody);
	}

	virtual int					IsolatedConnConnect(ITcpIsolated *pIsolated, std::string sIp_or_Hostname, unsigned short nPort) override {
		return _workQueue->IsolatedConnConnect(pIsolated, sIp_or_Hostname, nPort);
	}

	virtual void				IsolatedConnFlush(ITcpIsolated *pIsolated) override {
		_workQueue->IsolatedConnFlush(pIsolated);
	}

	virtual void				IsolatedConnDelayReconnect(ITcpIsolated *pIsolated, int nDelaySeconds) override {
		_workQueue->IsolatedConnDelayReconnect(pIsolated, nDelaySeconds);
	}

	/************************************************************************/
	/* Callback recv from backend (thread)                                  */
	/************************************************************************/
	virtual void				AddAcceptClientCb(ITcpServer *pServer, uintptr_t streamptr, std::string& sPeerIp) override {
		_trunkQueue->AddAcceptClientCb(pServer, streamptr, sPeerIp);
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

protected:
	StdLog *_refLog;

	CConnFactoryTrunkQueuePtr _trunkQueue;
	CKjConnFactoryWorkQueuePtr _workQueue;

	CTcpConnManager _connManager;

	std::unordered_map<uintptr_t, ITcpServer *> _mapServer; // pointer 2 server
	CLOSED_SERVER_LIST _vClosedServer;

};

/*EOF*/