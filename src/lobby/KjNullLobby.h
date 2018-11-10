#pragma once
//------------------------------------------------------------------------------
/**
    @class CKjNullLobby
    
    (C) 2016 n.lee
*/
#include "../tcp/tcp_packet_def.h"
#include "../tcp/TcpConnManager.h"

#include "ILobby.h"

//------------------------------------------------------------------------------
/** 
	@brief CKjNullLobby
*/
class CKjNullLobby : public ILobby {
public:
	CKjNullLobby();
	virtual ~CKjNullLobby();

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
		throw std::exception("not implemented!!!");
	}

private:
	ITcpConnFactory				*_tcpConnFactory;
	IZoneManager				*_zoneManager;
	IAccountManager				*_accountManager;

};

/*EOF*/