#pragma once
//------------------------------------------------------------------------------
/**
@class CEvConnFactory

(C) 2016 n.lee
*/
#include "../ITcpConnFactory.h"
#include "../ITcpEventManager.h"

#include "../TcpConnManager.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "ev_client.h"
#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------------------
/**

*/
class CEvConnFactory : public ITcpConnFactory {
public:
	CEvConnFactory(StdLog *pLog);
	virtual ~CEvConnFactory();

	using CLOSED_SERVER_LIST = std::vector<ITcpServer *>;

	/** **/
	virtual void				OnInit() override;
	virtual void				OnDelete() override;

	virtual void				OnUpdate() override {
		ev_loop_run_once_((struct event_base *)_loopBase, EVLOOP_NONBLOCK);
		_connManager.OnCheckConnection();
	}

	/** **/
	virtual void *				GetLoopBase() override {
		return _loopBase;
	}

	virtual ITcpConnManager&	GetConnManager() override {
		return _connManager;
	}

	/************************************************************************/
	/* Request send to backend (thread)                                     */
	/************************************************************************/
	virtual int					OpenTcpServer(ITcpServer *pServer, unsigned short nPort) override { return pServer->Open(_loopBase, nPort); }
	virtual void				CloseTcpServer(ITcpServer *pServer) override { pServer->Close(); _vClosedServer.push_back(pServer); }
	virtual void				DisposeConnection(ITcpConn *pConn) override { pConn->DisposeConnection(); }

	virtual void				PostPacket(ITcpConn *pConn, uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) override {
		pConn->PostPacket(uInnerUuid, uSerialNo, sTypeName, sBody);
	}

	virtual void				ConfirmClientIsReady(ITcpClient *pClient, uintptr_t streamptr) override { pClient->ConfirmClientIsReady(_loopBase, streamptr); }

	virtual void				PostBroadcastPacket(ITcpServer *pServer, std::vector<uint64_t>& vTarget, std::string& sTypeName, std::string& sBody) override {
		//pServer->PostBroadcastPacket(vTarget, sTypeName, sBody);
	}

	virtual int					IsolatedConnConnect(ITcpIsolated *pIsolated, std::string sIp_or_Hostname, unsigned short nPort) override {
		return pIsolated->Connect(_loopBase, sIp_or_Hostname, nPort);
	}

	virtual void				IsolatedConnFlush(ITcpIsolated *pIsolated) override {
		pIsolated->FlushConnection();
	}

	virtual void				IsolatedConnDelayReconnect(ITcpIsolated *pIsolated, int nDelaySeconds) override {
		pIsolated->DelayReconnect(nDelaySeconds);
	}

	/************************************************************************/
	/* Callback recv from backend (thread)                                  */
	/************************************************************************/
	virtual void				AddAcceptClientCb(ITcpServer *pServer, uintptr_t streamptr, std::string& sPeerIp) override {

		pServer->OnAcceptClient(streamptr, sPeerIp);
	}
	
	virtual void				AddClientDisconnectCb(ITcpServer *pServer, ITcpClient *pClient, ITcpConnManager *pConnMgr) override {

		pClient->OnDisconnect();

		// remove and dispose conn
		pConnMgr->OnRemoveClient(pServer, pClient);
	}

	virtual void				AddClientPacketCb(ITcpClient *pClient, uint64_t uConnId, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) override {
		
		assert(pClient->GetConnId() == uConnId);

		//
		pClient->GetEventManager().OnPacket(pClient, uSerialNo, sTypeName, sBody);
	}

	virtual void				AddClientInnerPacketCb(ITcpClient *pClient, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) override {
		
		assert(pClient->GetConnId() == uConnId);

		pClient->GetEventManager().OnInnerPacket(pClient, uInnerUuid, uSerialNo, sTypeName, sBody);
	}

	virtual void				AddIsolatedConnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr) override {
		
		fprintf(stderr, "Connect ok -- connid(%08llu)connptr(0x%08Ix).\n",
			pIsolated->GetConnId(), (uintptr_t)pIsolated);
		pIsolated->OnConnect();
	}

	virtual void				AddIsolatedDisconnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr) override {

		pIsolated->OnDisconnect();

		// note: must not remove this connection because it will reconnect automatically
	}

	virtual void				AddIsolatedPacketCb(ITcpIsolated *pIsolated, uint64_t uConnId, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) override {
		
		assert(pIsolated->GetConnId() == uConnId);

		//
		pIsolated->GetEventManager().OnPacket(pIsolated, uSerialNo, sTypeName, sBody);
	}

	virtual void				AddIsolatedInnerPacketCb(ITcpIsolated *pIsolated, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) override {
		
		assert(pIsolated->GetConnId() == uConnId);

		pIsolated->GetEventManager().OnInnerPacket(pIsolated, uInnerUuid, uSerialNo, sTypeName, sBody);
	}

	/** **/
	virtual ITcpServer *		CreateTcpServer() override;
	virtual ITcpClient *		CreateTcpClientOnServer(const std::string& sPeerIp, ITcpServer *pServer) override;

	/** **/
	virtual ITcpIsolated *		CreateTcpIsolated() override;
	virtual ITcpIsolated *		CreateTcpIsolated2() override;

protected:
	void *_loopBase;
	StdLog *_refLog;
	CTcpConnManager _connManager;

	std::unordered_map<uintptr_t, ITcpServer *> _mapServer; // pointer 2 server
	CLOSED_SERVER_LIST _vClosedServer;

};

/*EOF*/