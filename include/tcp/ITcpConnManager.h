#pragma once
//------------------------------------------------------------------------------
/**
@class ITcpConnManager

(C) 2016 n.lee
*/
#include "ITcpServer.h"
#include "ITcpClient.h"
#include "ITcpIsolated.h"

//------------------------------------------------------------------------------
/**

*/
class ITcpConnManager {
public:
	virtual ~ITcpConnManager() noexcept { }

	/** **/
	virtual void				OnCheckConnection() = 0;

	/** **/
	virtual void				OnAddClient(ITcpServer *pServer, ITcpClient *pClient) = 0;
	virtual void				OnRemoveClient(ITcpServer *pServer, ITcpClient *pClient) = 0;

	/** **/
	virtual void				OnAddIsolated(ITcpIsolated *pIsolated) = 0;

	/** **/
	virtual void				ReleaseAllClients(ITcpServer *pServer) = 0;
	virtual void				ReleaseAllIsolateds() = 0;

	virtual void				DisposeDownStreams(ITcpServer *pServer) = 0;

	/** **/
	virtual ITcpClient *		LookupClientByConnId(uint64_t uConnId) = 0;
	virtual size_t				GetClientCount() = 0;
	virtual uint64_t			GetNextConnectionId() = 0;

	virtual bool				Recycle() = 0;

};

/*EOF*/