#pragma once
//------------------------------------------------------------------------------
/**
@class ITcpConnFactory

(C) 2016 n.lee
*/
#include "ITcpConnManager.h"

#include "ITcpServer.h"
#include "ITcpClient.h"
#include "ITcpIsolated.h"

//------------------------------------------------------------------------------
/**

*/
class ITcpConnFactory {
public:
	virtual ~ITcpConnFactory() noexcept { }

	/** **/
	virtual void				OnInit() = 0;
	virtual void				OnDelete() = 0;
	virtual void				OnUpdate() = 0;

	/** **/
	virtual void *				GetLoopBase() = 0;
	virtual ITcpConnManager& 	GetConnManager() = 0;

	/** **/
	virtual ITcpServer *		CreateTcpServer() = 0;
	virtual ITcpClient *		CreateTcpClientOnServer(std::string&& sPeerIp, ITcpServer *pServer) = 0;

	/** **/
	virtual ITcpIsolated *		CreateTcpIsolated() = 0;
	virtual ITcpIsolated *		CreateTcpIsolated2() = 0;

	/************************************************************************/
	/* Request send to backend (thread)                                     */
	/************************************************************************/
	virtual int					OpenTcpServer(ITcpServer *pServer, unsigned short nPort) = 0;
	virtual void				CloseTcpServer(ITcpServer *pServer) = 0;
	virtual void				FlushTcpServerDownStream(ITcpServer *pServer, uintptr_t streamptr) = 0;

	virtual void				ReleaseConnection(ITcpConn *pConn) = 0;

	virtual void				PostPacket(ITcpConn *pConn, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) = 0;

	virtual void				ConfirmClientIsReady(ITcpClient *pClient, uintptr_t streamptr) = 0;
	virtual void				PostBroadcastPacket(ITcpServer *pServer, std::vector<uint64_t>& vTarget, std::string&& sTypeName, std::string&& sBody) = 0;

	virtual int					IsolatedConnConnect(ITcpIsolated *pIsolated, std::string sIp_or_Hostname, unsigned short nPort) = 0;
	virtual void				IsolatedConnDelayReconnect(ITcpIsolated *pIsolated, int nDelaySeconds) = 0;

	/************************************************************************/
	/* Callback recv from backend (thread)                                  */
	/************************************************************************/
	virtual void				AddAcceptClientCb(ITcpServer *pServer, uintptr_t streamptr, std::string&& sPeerIp) = 0;
	virtual void				AddClientDisconnectCb(ITcpServer *pServer, ITcpClient *pClient, ITcpConnManager *pConnMgr) = 0;
	virtual void				AddClientPacketCb(ITcpClient *pClient, uint64_t uConnId, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) = 0;
	virtual void				AddClientInnerPacketCb(ITcpClient *pClient, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) = 0;

	virtual void				AddIsolatedConnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr) = 0;
	virtual void				AddIsolatedDisconnectCb(ITcpIsolated *pIsolated, ITcpConnManager *pConnMgr) = 0;
	virtual void				AddIsolatedPacketCb(ITcpIsolated *pIsolated, uint64_t uConnId, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) = 0;
	virtual void				AddIsolatedInnerPacketCb(ITcpIsolated *pIsolated, uint64_t uConnId, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) = 0;

};

/*EOF*/