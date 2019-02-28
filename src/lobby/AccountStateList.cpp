//------------------------------------------------------------------------------
//  accStateList.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "AccountStateList.h"

#include <assert.h>
#include <algorithm>

#include "../base/cityhash/city.h"

//------------------------------------------------------------------------------
/**

*/
CAccountStateList::CAccountStateList(int nReserve)
	: _accountStatePool() {
	//
	_accountStatePool.SafeInit(600000, nReserve);
}

//------------------------------------------------------------------------------
/**

*/
CAccountStateList::~CAccountStateList() {

}

//------------------------------------------------------------------------------
/**

*/
ACCOUNT_STATE *
CAccountStateList::Alloc() {
	ACCOUNT_STATE *accState = nullptr;
	unsigned int objid = _accountStatePool.SafeAlloc();
	if (objid > 0) {
		accState = _accountStatePool.Get(objid);

		// initialize
		memset(accState, 0, sizeof(*accState));
		accState->keyid = objid;
		accState->route_table.onoff = ACC_CONNECTION_ONOFF_INIT;
	}
	return accState;
}

//------------------------------------------------------------------------------
/**

*/
void
CAccountStateList::Online(ACCOUNT_STATE& accState, uint64_t uConnId, uint64_t uInnerUuid, void *pConnImpl, time_t tmNow) {
	//
	accState.route_table.connid = uConnId;
	accState.route_table.inner_uuid = uInnerUuid;
	accState.route_table.conn_impl = pConnImpl;

	accState.route_table.onoff = ACC_CONNECTION_ONOFF_ONLINE;
	accState.route_table.onoff_time = tmNow;

	//
	_szOnlineCount = CalcOnlineCount();
}

//------------------------------------------------------------------------------
/**

*/
void
CAccountStateList::Offline(ACCOUNT_STATE& accState, time_t tmNow) {
	//
	if (ACC_CONNECTION_ONOFF_ONLINE == accState.route_table.onoff) {
		// fast calc online count
		--_szOnlineCount;
	}

	accState.route_table.onoff = ACC_CONNECTION_ONOFF_OFFLINE;
	accState.route_table.onoff_time = tmNow;
}

//------------------------------------------------------------------------------
/**

*/
ACCOUNT_STATE *
CAccountStateList::OfflineByUid(const char *sUid, time_t tmNow) {
	// walk
	ACCOUNT_STATE *accState;
	ACCOUNT_STATE_POOL *_pool = &_accountStatePool;
	OBJECT_POOL_ITERATE_USED_OBJECT_BEGIN(_pool, accState) {
		if (TestUserId(*accState, sUid)) {
			Offline(*accState, tmNow);
			return accState;
		}
	}
	OBJECT_POOL_ITERATE_USED_OBJECT_END();
	return nullptr;
}

//------------------------------------------------------------------------------
/**

*/
ACCOUNT_STATE *
CAccountStateList::KeepAliveByUid(const char *sUid, time_t tmNow) {
	ACCOUNT_STATE *accState = GetByUid(sUid);
	if (accState) {
		KeepAlive(*accState, tmNow);
	}
	return accState;
}

//------------------------------------------------------------------------------
/**

*/
void
CAccountStateList::CheckKeepAlive(int nLostContactToleranceSeconds, time_t tmNow) {
	// walk
	ACCOUNT_STATE *accState;
	ACCOUNT_STATE_POOL *pool = &_accountStatePool;
	OBJECT_POOL_ITERATE_USED_OBJECT_BEGIN(pool, accState) {
		//
		if (ACC_CONNECTION_ONOFF_ONLINE == accState->route_table.onoff
			&& tmNow - accState->route_table.onoff_time >= nLostContactToleranceSeconds) {
			//
			Offline(*accState, tmNow);
		}
	}
	OBJECT_POOL_ITERATE_USED_OBJECT_END();
}

//------------------------------------------------------------------------------
/**

*/
ACCOUNT_STATE *
CAccountStateList::GetByUid(const char *sUid) {
	size_t szUidLen = strlen(sUid);
	uint64_t uUidHash = CityHash64(sUid, szUidLen);
	UID_HASH_MAP::iterator it = _mapUidHash2ConnState.find(uUidHash);
	if (it != _mapUidHash2ConnState.end()) {
		assert(it->second->is_ready);
		return it->second;
	}
	else {
		// walk
		ACCOUNT_STATE *accState;
		ACCOUNT_STATE_POOL *pool = &_accountStatePool;
		OBJECT_POOL_ITERATE_USED_OBJECT_BEGIN(pool, accState) {
			if (0==strcmp(sUid, accState->uid)) {
				// store it in map for speed
				_mapUidHash2ConnState[uUidHash] = accState;
				assert(accState->is_ready);
				return accState;
			}
		}
		OBJECT_POOL_ITERATE_USED_OBJECT_END();
	}
	return nullptr;
}

//------------------------------------------------------------------------------
/**

*/
ACCOUNT_STATE *
CAccountStateList::GetByConnId(uint64_t uConnId) {
	CONN_ID_MAP::iterator it = _mapConnId2ConnState.find(uConnId);
	if (it != _mapConnId2ConnState.end()) {
		assert(it->second->is_ready);
		return it->second;
	}
	else {
		// walk
		ACCOUNT_STATE *accState;
		ACCOUNT_STATE_POOL *pool = &_accountStatePool;
		OBJECT_POOL_ITERATE_USED_OBJECT_BEGIN(pool, accState) {
			if (uConnId == accState->route_table.connid) {
				// store it in map for speed
				_mapConnId2ConnState[uConnId] = accState;
				assert(accState->is_ready);
				return accState;
			}
		}
		OBJECT_POOL_ITERATE_USED_OBJECT_END();
	}
	return nullptr;
}

//------------------------------------------------------------------------------
/**

*/
ACCOUNT_STATE *
CAccountStateList::GetByInnerUuid(uint64_t uInnerUuid) {
	INNER_UUID_MAP::iterator it = _mapInnerUuid2ConnState.find(uInnerUuid);
	if (it != _mapInnerUuid2ConnState.end()) {
		assert(it->second->is_ready);
		return it->second;
	}
	else {
		// walk
		ACCOUNT_STATE *accState;
		ACCOUNT_STATE_POOL *pool = &_accountStatePool;
		OBJECT_POOL_ITERATE_USED_OBJECT_BEGIN(pool, accState) {
			if (uInnerUuid == accState->route_table.inner_uuid) {
				// store it in map for speed
				_mapInnerUuid2ConnState[uInnerUuid] = accState;
				assert(accState->is_ready);
				return accState;
			}
		}
		OBJECT_POOL_ITERATE_USED_OBJECT_END();
	}
	return nullptr;
}

//------------------------------------------------------------------------------
/**

*/
void
CAccountStateList::Kick(unsigned int nIndexId, time_t tmNow) {
	ACCOUNT_STATE *accState = Get(nIndexId);
	if (accState) {
		ReleaseInternal(*accState, tmNow);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CAccountStateList::KickByUid(const char *sUid, time_t tmNow) {
	ACCOUNT_STATE *accState = GetByUid(sUid);
	if (accState) {
		ReleaseInternal(*accState, tmNow);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CAccountStateList::KickOfflineMoreThan(int nOfflineToleranceSeconds, unsigned int nKickNumMax, time_t tmNow, std::vector<ACCOUNT_STATE *>& vOutKickedList) {
	// walk
	ACCOUNT_STATE *accState;
	ACCOUNT_STATE_POOL *pool = &_accountStatePool;
	OBJECT_POOL_ITERATE_USED_OBJECT_BEGIN(pool, accState) {

		if (ACC_CONNECTION_ONOFF_OFFLINE == accState->route_table.onoff
			&& tmNow - accState->route_table.onoff_time >= nOfflineToleranceSeconds) {
			//
			ReleaseInternal(*accState, tmNow);

			//
			vOutKickedList.push_back(accState);
			if (vOutKickedList.size() >= nKickNumMax)
				return;
		}
	}
	OBJECT_POOL_ITERATE_USED_OBJECT_END();
}

//------------------------------------------------------------------------------
/**

*/
void
CAccountStateList::ReleaseInternal(ACCOUNT_STATE& accState, time_t tmNow) {
	uint64_t uUidHash, uConnId, uInnerUuid;

	if (accState.is_ready) {
		//
		accState.is_ready = false;

		// kick from uidhash map
		uUidHash = CityHash64(accState.uid, strlen(accState.uid));
		{
			UID_HASH_MAP::iterator it1 = _mapUidHash2ConnState.find(uUidHash);
			if (it1 != _mapUidHash2ConnState.end()) {
				_mapUidHash2ConnState.erase(it1);
			}
		}

		// kick from connid map
		uConnId = accState.route_table.connid;
		if (uConnId > 0) {
			CONN_ID_MAP::iterator it2 = _mapConnId2ConnState.find(uConnId);
			if (it2 != _mapConnId2ConnState.end()) {
				_mapConnId2ConnState.erase(it2);
			}
		}

		// kick from uuid map
		uInnerUuid = accState.route_table.inner_uuid;
		if (uInnerUuid > 0) {
			INNER_UUID_MAP::iterator it3 = _mapInnerUuid2ConnState.find(uInnerUuid);
			if (it3 != _mapInnerUuid2ConnState.end()) {
				_mapInnerUuid2ConnState.erase(it3);
			}
		}

		// offline
		Offline(accState, tmNow);

		// release accState by keyid(objid)
		_accountStatePool.ReleaseInternal(accState.keyid);
	}
}

//------------------------------------------------------------------------------
/**

*/
size_t
CAccountStateList::CalcOnlineCount() {
	size_t nCount = 0;

	// walk
	ACCOUNT_STATE *accState;
	ACCOUNT_STATE_POOL *pool = &_accountStatePool;
	OBJECT_POOL_ITERATE_USED_OBJECT_BEGIN(pool, accState) {
		nCount += (int)(ACC_CONNECTION_ONOFF_ONLINE == accState->route_table.onoff);
	}
	OBJECT_POOL_ITERATE_USED_OBJECT_END();
	return nCount;
}

//------------------------------------------------------------------------------
/**

*/
size_t
CAccountStateList::CalcOfflineCount() {
	size_t nCount = 0;

	// walk
	ACCOUNT_STATE *accState;
	ACCOUNT_STATE_POOL *pool = &_accountStatePool;
	OBJECT_POOL_ITERATE_USED_OBJECT_BEGIN(pool, accState) {
		nCount += (int)(ACC_CONNECTION_ONOFF_OFFLINE == accState->route_table.onoff);
	}
	OBJECT_POOL_ITERATE_USED_OBJECT_END();
	return nCount;
}

/** -- EOF -- **/