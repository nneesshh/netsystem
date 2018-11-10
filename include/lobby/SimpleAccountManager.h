#pragma  once

//------------------------------------------------------------------------------
/**
@class CSimpleAccountManager

(C) 2016 n.lee
*/
#include "IAccountManager.h"

//------------------------------------------------------------------------------
/**
@brief CSimpleAccountManager
*/
class CSimpleAccountManager : public IAccountManager
{
public:
	CSimpleAccountManager();
	virtual ~CSimpleAccountManager();

	/** **/
	virtual ACC_REG_RESULT		Register(
		const char *sUid,
		const char *sNick,
		const char *sPwd,
		uint64_t uConnId,
		uint64_t uInnerUuid,
		void *pConnImpl,
		time_t tmNow,
		ONLINE_HANDLE **ppOutAccount);

	virtual void				Unregister(ONLINE_HANDLE& account);
	virtual void				UnregisterByConnId(uint64_t uConnId);

	/** Return online items. */
	virtual ONLINE_LIST& 		GetOnlineList() {
		return *_onlineList;
	}

	/** **/
	virtual ONLINE_HANDLE *		LookupAccountByInnerUuid(uint64_t uInnerUuid);

public:
	ONLINE_LIST *_onlineList;
};

/*EOF*/