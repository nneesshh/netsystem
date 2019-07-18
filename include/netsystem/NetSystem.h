#pragma once
//------------------------------------------------------------------------------
/**
	@class CNetSystem

	(C) 2016 n.lee
*/
#include "INetSystem.h"

//------------------------------------------------------------------------------
/**
@brief CNetSystem
*/
class MY_NETSYSTEM_EXTERN CNetSystem : public INetSystem {
	// params: void cb(int nConnectionCount, int nAccountCount, int nRecvBytes, int nSendBytes)
	using net_system_stats_cb = std::function<void(int, int, int, int)>;

public:
	CNetSystem(void *servercore);
	virtual ~CNetSystem();

public:
	/** **/
	virtual void				OnUpdate() override;

	/** **/
	virtual int					Init(LOBBY_TYPE eType, unsigned int port, const std::function<void(int, int, int, int)> stats_cb) override;

	/** **/
	virtual ILobby&				GetLobby() override {
		return *_lobby;
	}

	virtual bool				IsLobbyReady() override {
		return _isLobbyReady;
	}

	/** **/
	virtual void *				GetLobbyLoopBase() override {
		return GetLobby().GetLoopBase();
	}

	virtual ITcpEventManager& 	GetLobbyEventManager() override {
		return GetLobby().GetMainServer().GetEventManager();
	}

	virtual IAccountManager& 	GetLobbyAccountManager() override {
		return GetLobby().GetAccountManager();
	}

	/** **/
	virtual IHttpServer *		OpenHttpServer(unsigned short nPort) override;
	virtual IHttpServer *		OpenLibevhtpServer(unsigned short nPort) override;

	virtual IURLHttpRequest *	OpenURLHttpRequest() override;
	virtual IURLSSLRequest *	OpenURLSSLRequest() override;

	/** **/
	virtual void				Shutdown() override;

private:
	int							InitNullLobby();
	int							InitLobby(unsigned short nPort);
	int							InitGateLobby(unsigned short nPort);
	void						DisposeLobby();

private:
	net_system_stats_cb _stats_cb;

	bool    _isLobbyReady = false;
	ILobby *_lobby = nullptr;
};

/*EOF*/