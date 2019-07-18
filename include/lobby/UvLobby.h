#pragma once
//------------------------------------------------------------------------------
/**
    @class CUvLobby
    
    (C) 2016 n.lee
*/
#include "servercore/log/StdLog.h"

#include "../tcp/tcp_packet_def.h"
#include "../tcp/TcpConnManager.h"

#include "ILobby.h"

//------------------------------------------------------------------------------
/** 
	@brief CUvLobby
*/
class CUvLobby : public ILobby {
public:
	CUvLobby(unsigned short nPort);
	virtual ~CUvLobby();

public:
	virtual int					OnInit();
	virtual void				OnDelete();

	virtual void				OnUpdate() {
		_tcpConnFactory->OnUpdate();
	}

	/** **/
	virtual void *				GetLoopBase() {
		return _tcpConnFactory->GetLoopBase();
	}

	virtual ITcpConn *			LookupByConnId(uint64_t uConnId) {
		//
		return _tcpConnFactory->GetConnManager().LookupClientByConnId(uConnId);
	}

	virtual ITcpIsolated *		OpenTcpIsolated(std::string& sIp_or_Hostname, unsigned short nPort);
	virtual ITcpIsolated *		OpenTcpIsolated2(std::string& sIp_or_Hostname, unsigned short nPort);

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