#pragma once
//------------------------------------------------------------------------------
/**
@class CTcpConnManager

(C) 2016 n.lee
*/
#include <unordered_map>

#include "ITcpConnFactory.h"
#include "ITcpConnManager.h"

#define TCP_CONNID_BASE	1000000000
#define TCP_CONNID_MAX	1000000000000

typedef std::unordered_map<uint64_t, ITcpClient *> MAP_TCP_CONN_MANAGER_CLIENTS;

typedef struct tcp_conn_manager_server_entry_s {
	uintptr_t _entryptr;
	MAP_TCP_CONN_MANAGER_CLIENTS _mapClients; // connid 2 client
} tcp_conn_manager_server_entry_t;

//------------------------------------------------------------------------------
/**
@brief CTcpConnManager
*/
class CTcpConnManager : public ITcpConnManager {
public:
	CTcpConnManager(ITcpConnFactory *pFactory);
	virtual ~CTcpConnManager();

	using CONN_RECYCLER = std::vector<ITcpConn *>;

	/** **/
	virtual void				OnCheckConnection() override {
		// do some delay
		if (++_nDelayCount >= 65536) {
			Recycle();
			_nDelayCount = 0;
		}
	}

	/** **/
	virtual void				OnAddClient(ITcpServer *pServer, ITcpClient *pClient) override;
	virtual void				OnRemoveClient(ITcpServer *pServer, ITcpClient *pClient) override;
	virtual void				OnRemoveAllClients(ITcpServer *pServer) override;
	virtual void				OnDisposeAllClients(ITcpServer *pServer) override;

	/** **/
	virtual void				OnAddIsolated(ITcpIsolated *pIsolated) override;
	virtual void				OnRemoveAllIsolateds() override;

	/** **/
	virtual ITcpClient *		LookupClientByConnId(uint64_t uConnId) override;

	virtual size_t				GetClientCount() override {
		return _szClientCount;
	}

	virtual uint64_t			GetNextConnectionId() override {
		++_uNextId;

		// skip TCP_CONNID_MAX
		if (TCP_CONNID_MAX <= _uNextId) {
			_uNextId = TCP_CONNID_BASE + 1;
		}
		return _uNextId;
	}

	virtual bool				Recycle() override;

private:
	void						RecyclerPush(ITcpConn *pConn);
	void						RecyclerPop();
	void						SafeRelease();

public:
	static size_t s_send;
	static size_t s_recv;

private:
	ITcpConnFactory *_refConnFactory;

	//!
	uint64_t _uNextId = TCP_CONNID_BASE;
	int _nDelayCount = 0;

	//!
	std::unordered_map<uintptr_t, tcp_conn_manager_server_entry_t > _mapServerEntry; // pointer 2 server_entry
	std::unordered_map<uint64_t, ITcpIsolated *> _mapIsolated; // connid 2 isolated
	size_t _szClientCount = 0;

	//!
	CONN_RECYCLER _vRecycler;
	CONN_RECYCLER _vSafeToRelease;
};

/*EOF*/