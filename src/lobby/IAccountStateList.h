#pragma once

//------------------------------------------------------------------------------
/**
@class IConnStateList

(C) 2016 n.lee
*/
#include <time.h>
#include <vector>

#include "base/ObjectPool.hpp"

enum ACC_CONNECTION_ONOFF {
	ACC_CONNECTION_ONOFF_INIT = 0,
	ACC_CONNECTION_ONOFF_ONLINE = 1,
	ACC_CONNECTION_ONOFF_OFFLINE = 2,
};

/* ACC -- LOGIN STATE */
enum ACC_LOGIN_STATE {
	ACC_LOGIN_STATE_IDLE = 0,
	ACC_LOGIN_STATE_NEWLOGIN = 1,
	ACC_LOGIN_STATE_RELOGIN = 2,
	ACC_LOGIN_STATE_READY = 3,
	ACC_LOGIN_STATE_LOGOUT = 4,
	ACC_LOGIN_STATE_LOCKED = 5, // such as for activated
};

/************************************************************************/
/* ACCOUNT_STATE                                                       */
/************************************************************************/
typedef struct account_state_s {
	/* */
	unsigned int  keyid;
	bool          is_ready;                // ready for registered and not ready for unregistered

	/* */
	char          uid[36];
	char          nick[36];
	char          pwd[40];                 // user password, text or md5

	/* route table */
	struct {
		uint64_t  connid;      // conn id for client
		uint64_t  inner_uuid;  // inner uuid for client
		void     *conn_impl;   // ITcpConn

		ACC_CONNECTION_ONOFF  onoff;
		time_t    onoff_time;

	} route_table;

	/* */
	void         *ext_data;                // extension data

	/* */
	struct {
		ACC_LOGIN_STATE  login_state;
		unsigned int     conflict_keyid;    // conflict with another connection state key, 0 = no conflict
	} login_checker;

} account_state_t, ACCOUNT_STATE;

using ACCOUNT_STATE_POOL = CC_CONTAINER::CObjectPool<ACCOUNT_STATE>;

//------------------------------------------------------------------------------
/**
@brief IAccountStateList
*/
class IAccountStateList {
public:
	virtual ~IAccountStateList() { }

	virtual ACCOUNT_STATE *		Alloc() = 0;

	virtual void				Online(ACCOUNT_STATE& accState, uint64_t uConnId, uint64_t uInnerUuid, void *pConnImpl, time_t tmNow) = 0;
	virtual void				Offline(ACCOUNT_STATE& accState, time_t tmNow) = 0;
	virtual ACCOUNT_STATE *		OfflineByUid(const char *sUid, time_t tmNow) = 0;

	virtual void				KeepAlive(ACCOUNT_STATE& accState, time_t tmNow) = 0;
	virtual ACCOUNT_STATE *		KeepAliveByUid(const char *sUid, time_t tmNow) = 0;
	virtual void				CheckKeepAlive(int nLostContactToleranceSeconds) = 0;

	virtual ACCOUNT_STATE *		Get(unsigned int nIndexId) = 0;
	virtual ACCOUNT_STATE *		GetByUid(const char *sUid) = 0;
	virtual ACCOUNT_STATE *		GetByConnId(uint64_t uConnId) = 0;
	virtual ACCOUNT_STATE *		GetByInnerUuid(uint64_t uInnerUuid) = 0;

	virtual ACCOUNT_STATE_POOL&	GetPool() = 0;
	virtual unsigned int		GetCount() = 0;
	virtual size_t				GetOnlineCount() = 0;
	virtual size_t				GetOfflineCount() = 0;

	virtual void				Kick(unsigned int nIndexId) = 0;
	virtual void				KickByUid(const char *sUid) = 0;
	virtual void				KickOfflineMoreThan(int nOfflineToleranceSeconds, unsigned int nKickNumMax, std::vector<ACCOUNT_STATE *>& vOutKickedList) = 0;

	virtual void				ConfirmAccountIsReady(ACCOUNT_STATE& accState, uint64_t uConnId, uint64_t uInnerUuid, void *pConnImpl, time_t tmNow) = 0;
	virtual void				Release(ACCOUNT_STATE& accState) = 0;
};

/*EOF*/