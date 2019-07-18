#pragma once

//------------------------------------------------------------------------------
/**
@class CAccountStateList

(C) 2016 n.lee
*/
#include <string>
#include <unordered_map>

#include "IAccountStateList.h"

//------------------------------------------------------------------------------
/**
@brief CAccountStateList
*/
class CAccountStateList : public IAccountStateList {
public:
	CAccountStateList(int nReserve = 1024);
	virtual ~CAccountStateList();

	using UID_HASH_MAP = std::unordered_map<uint64_t, ACCOUNT_STATE *>;
	using CONN_ID_MAP = std::unordered_map<uint64_t, ACCOUNT_STATE *>;
	using INNER_UUID_MAP = std::unordered_map<uint64_t, ACCOUNT_STATE *>;

	virtual ACCOUNT_STATE *		Alloc() override;

	virtual void				Online(ACCOUNT_STATE& accState, uint64_t uConnId, uint64_t uInnerUuid, void *pConnImpl, time_t tmNow) override;
	virtual void				Offline(ACCOUNT_STATE& accState, time_t tmNow) override;
	virtual ACCOUNT_STATE *		OfflineByUid(const char *sUid, time_t tmNow) override;

	virtual void				KeepAlive(ACCOUNT_STATE& accState, time_t tmNow) override {
		if (ACC_CONNECTION_ONOFF_ONLINE == accState.route_table.onoff) {
			accState.route_table.onoff_time = tmNow;
		}
	}

	virtual ACCOUNT_STATE *		KeepAliveByUid(const char *sUid, time_t tmNow) override;
	virtual void				CheckKeepAlive(int nLostContactToleranceSeconds, time_t tmNow) override;

	virtual ACCOUNT_STATE *		Get(unsigned int nIndexId) override {
#ifdef _DEBUG
		ACCOUNT_STATE *tmp = _accountStatePool.Get(nIndexId);
		assert(nullptr == tmp || nIndexId == tmp->keyid);
		return tmp;
#else
		return _accountStatePool.Get(nIndexId);
#endif
	}

	virtual ACCOUNT_STATE *		GetByUid(const char *sUid) override;
	virtual ACCOUNT_STATE *		GetByConnId(uint64_t uConnId) override;
	virtual ACCOUNT_STATE *		GetByInnerUuid(uint64_t uInnerUuid) override;

	virtual ACCOUNT_STATE_POOL&	GetPool() override { return _accountStatePool; }
	virtual unsigned int		GetCount() override { return _accountStatePool.GetObjectNum(); }

	virtual size_t				GetOnlineCount() override {
		return _szOnlineCount;
	}

	virtual size_t				GetOfflineCount() override {
		return CalcOfflineCount();
	}

	virtual void				Kick(unsigned int nIndexId, time_t tmNow) override;
	virtual void				KickByUid(const char *sUid, time_t tmNow) override;
	virtual void				KickOfflineMoreThan(int nOfflineToleranceSeconds, unsigned int nKickNumMax, time_t tmNow, std::vector<ACCOUNT_STATE *>& vOutKickedList) override;

	virtual void				ConfirmAccountIsReady(ACCOUNT_STATE& accState, uint64_t uConnId, uint64_t uInnerUuid, void *pConnImpl, time_t tmNow) override {
		Online(accState, uConnId, uInnerUuid, pConnImpl, tmNow);
		accState.is_ready = true;
	}


	virtual void				Release(ACCOUNT_STATE& accState, time_t tmNow) override {
		ReleaseInternal(accState, tmNow);
	}

private:
	bool						TestUserId(ACCOUNT_STATE& accState, const char *sUid) {
		return (0 == strcmp(accState.uid, sUid));
	}

	void						ReleaseInternal(ACCOUNT_STATE& accState, time_t tmNow);

	size_t						CalcOnlineCount();

	size_t						CalcOfflineCount();

private:
	ACCOUNT_STATE_POOL _accountStatePool;
	size_t _szOnlineCount = 0;

	UID_HASH_MAP _mapUidHash2ConnState; // uid hash 2 conn state
	CONN_ID_MAP _mapConnId2ConnState; // connid 2 conn state
	INNER_UUID_MAP _mapInnerUuid2ConnState; // inner uuid 2 conn state
};

/*EOF*/