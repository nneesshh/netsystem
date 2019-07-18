#pragma once
//------------------------------------------------------------------------------
/**
@class IAccountManager

(C) 2016 n.lee
*/
#include "IAccountStateList.h"

using ONLINE_HANDLE = ACCOUNT_STATE;
using ONLINE_LIST = IAccountStateList;

enum ACC_REG_RESULT {
	ACC_REG_RESULT_ERROR_ACCOUNT_LIST_IS_FULL = -5,
	ACC_REG_RESULT_ERROR_CONFLICT_MAIN_ACCOUNT_IS_NOT_READY = -4,
	ACC_REG_RESULT_ERROR_CONFLICT_TOO_MANY_TIMES = -3,
	ACC_REG_RESULT_ERROR_BIND_UID_OVERFLOW = -2,
	ACC_REG_RESULT_ERROR_INVALID_UID = -1,
	ACC_REG_RESULT_OK = 0,
	ACC_REG_RESULT_CONFLICT = 1, // different connection, and account is already registered
	ACC_REG_RESULT_DUPLICATE_REGISTRATION = 2, // same connection, and account is already registered
};

// walk used accounts
#define WALK_USED_ACCOUNTS_BEGIN(online_list_, online_)								\
	do {																			\
		ACCOUNT_STATE_POOL& pool_ = online_list_->GetPool();						\
		OBJECT_POOL_ITERATE_USED_OBJECT_BEGIN((&pool_), online_)					\

#define WALK_USED_ACCOUNTS_END()													\
		OBJECT_POOL_ITERATE_USED_OBJECT_END();										\
	} while(0)

// walk online accounts
#define WALK_ONLINE_ACCOUNTS_BEGIN(online_list_, online_)							\
		WALK_USED_ACCOUNTS_BEGIN(online_list_, online_) {							\
			if (ACC_CONNECTION_ONOFF_ONLINE == online_->route_table.onoff)


#define WALK_ONLINE_ACCOUNTS_END()													\
		}																			\
		WALK_USED_ACCOUNTS_END()

//------------------------------------------------------------------------------
/**
@brief IAccountManager
*/
class IAccountManager
{
public:
	virtual ~IAccountManager() { }

	/** **/
	virtual ACC_REG_RESULT		Register(
		const char *sUid,
		const char *sNick,
		const char *sPwd,
		uint64_t uConnId,
		uint64_t uInnerUuid,
		void *pConnImpl,
		time_t tmNow,
		ONLINE_HANDLE **ppOutAccount) = 0;

	virtual void				Unregister(ONLINE_HANDLE& account, time_t tmNow) = 0;
	virtual void				UnregisterByConnId(uint64_t uConnId, time_t tmNow) = 0;

	/** Return list of online items. */
	virtual ONLINE_LIST& 		GetOnlineList() = 0;

	/** **/
	virtual ONLINE_HANDLE *		LookupAccountByInnerUuid(uint64_t uInnerUuid) = 0;

};

/*EOF*/