#pragma once

//------------------------------------------------------------------------------
/**
    @class CKjGateLobby
    
    (C) 2016 n.lee
*/
#include "../tcp/tcp_packet_def.h"
#include "../tcp/TcpConnManager.h"

#include "ILobby.h"

//------------------------------------------------------------------------------
/** 
	@brief CKjGateLobby
*/
class CKjGateLobby : public ILobby {
public:
	CKjGateLobby(unsigned short nPort, StdLog *pLog);
	virtual ~CKjGateLobby();

public:
	virtual int					OnInit() override;
	virtual void				OnDelete() override;

	virtual void				OnUpdate() override {
		_tcpConnFactory->OnUpdate();
	}

	/** **/
	virtual void *				GetLoopBase() override {
		return _tcpConnFactory->GetLoopBase();
	}

	virtual ITcpConn *			LookupByConnId(uint64_t uConnId) override {
		//
		return _tcpConnFactory->GetConnManager().LookupClientByConnId(uConnId);
	}

	virtual ITcpIsolated *		OpenTcpIsolated(std::string& sIp_or_Hostname, unsigned short nPort) override;
	virtual ITcpIsolated *		OpenTcpIsolated2(std::string& sIp_or_Hostname, unsigned short nPort) override;

	virtual ITcpConnFactory&	GetTcpConnFactory() override {
		return *_tcpConnFactory;
	}

	virtual IZoneManager&		GetZoneManager() override {
		return *_zoneManager;
	}

	virtual IAccountManager&	GetAccountManager() override {
		return *_accountManager;
	}

	virtual ITcpServer&			GetMainServer() override {
		return *_mainServer;
	}

private:
	unsigned short				_port;

	ITcpConnFactory				*_tcpConnFactory;
	IZoneManager				*_zoneManager;
	IAccountManager				*_accountManager;

	ITcpServer					*_mainServer;
};

/*EOF*/