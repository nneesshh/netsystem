#pragma once
//------------------------------------------------------------------------------
/**
@class ILobby

(C) 2016 n.lee
*/
#include "../tcp/ITcpConnFactory.h"
#include "../tcp/ITcpConn.h"

#include "IAccountManager.h"
#include "IZoneManager.h"

//------------------------------------------------------------------------------
/**
@brief ILobby
*/
class ILobby
{
public:
	virtual ~ILobby() { }

	/** **/
	virtual int					OnInit() = 0;
	virtual void				OnDelete() = 0;
	virtual void				OnUpdate() = 0;

	/** **/
	virtual void *				GetLoopBase() = 0;
	virtual ITcpConn *			LookupByConnId(uint64_t uConnId) = 0;
	virtual ITcpIsolated *		OpenTcpIsolated(std::string& sIp_or_Hostname, unsigned short nPort) = 0;
	virtual ITcpIsolated *		OpenTcpIsolated2(std::string& sIp_or_Hostname, unsigned short nPort) = 0;

	/** **/
	virtual ITcpConnFactory&	GetTcpConnFactory() = 0;
	virtual IZoneManager& 		GetZoneManager() = 0;
	virtual IAccountManager& 	GetAccountManager() = 0;
	virtual ITcpServer&			GetMainServer() = 0;

};

/*EOF*/