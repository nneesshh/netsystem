#pragma once
//------------------------------------------------------------------------------
/**
@class INetSystem

(C) 2016 n.lee
*/
#include "../common/UsingMyToolkitMini.h"

#include "../tcp/tcp_def.h"
#include "../tcp/tcp_packet_def.h"
#include "../tcp/ITcpConnFactory.h"
#include "../tcp/ITcpEventManager.h"

#include "../lobby/ILobby.h"
#include "../lobby/IZoneManager.h"
#include "../lobby/IAccountManager.h"
#include "../lobby/IAccountStateList.h"

#include "../http/IHttpServer.h"
#include "../http/IURLHttpRequest.h"
#include "../http/IURLSSLRequest.h"

//////////////////////////////////////////////////////////////////////////
enum LOBBY_TYPE {
	LOBBY_TYPE_NULL = 0,
	LOBBY_TYPE_TCP_SERVER,
	LOBBY_TYPE_GATE_TCP_SERVER,
};

//------------------------------------------------------------------------------
/**
@brief INetSystem
*/
class MY_EXTERN INetSystem {
public:
	virtual ~INetSystem() { }

	/** **/
	virtual int					Init(LOBBY_TYPE eType, unsigned int port, StdLog *pLog, const std::function<void(int, int, int, int)> stats_cb) = 0;
	virtual void				Dispose() = 0;
	virtual void				Update() = 0;

	/** **/
	virtual ILobby&				GetLobby() = 0;
	virtual bool				IsLobbyReady() = 0;

	/** **/
	virtual void *				GetLobbyLoopBase() = 0;
	virtual ITcpEventManager&	GetLobbyEventManager() = 0;
	virtual IAccountManager&	GetLobbyAccountManager() = 0;

	/** **/
	virtual IHttpServer *		OpenHttpServer(unsigned short nPort) = 0;
	virtual IHttpServer *		OpenLibevhtpServer(unsigned short nPort) = 0;

	virtual IURLHttpRequest *	OpenURLHttpRequest() = 0;
	virtual IURLSSLRequest *	OpenURLSSLRequest() = 0;

};

/*EOF*/