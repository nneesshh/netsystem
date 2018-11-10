#pragma once
//------------------------------------------------------------------------------
/**
    @class CUvNullLobby
    
    (C) 2016 n.lee
*/
#include "../tcp/tcp_packet_def.h"
#include "../tcp/TcpConnManager.h"

#include "ILobby.h"

//------------------------------------------------------------------------------
/** 
	@brief CUvNullLobby
*/
class CUvNullLobby : public ILobby {
public:
	CUvNullLobby();
	virtual ~CUvNullLobby();

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
		throw std::exception("not implemented!!!");
	}

private:
	ITcpConnFactory				*_tcpConnFactory;
	IZoneManager				*_zoneManager;
	IAccountManager				*_accountManager;

};

/*EOF*/