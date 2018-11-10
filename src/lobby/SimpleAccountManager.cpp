//------------------------------------------------------------------------------
//  SimpleAccountManager.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "SimpleAccountManager.h"

#include <time.h>
#include <locale.h>

#include "../common/UsingMyToolkitMini.h"
#include "AccountStateList.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CSimpleAccountManager::CSimpleAccountManager()
	: _onlineList(new CAccountStateList(16384)) {

}

//------------------------------------------------------------------------------
/**

*/
CSimpleAccountManager::~CSimpleAccountManager() {
	SAFE_DELETE(_onlineList);
}

//------------------------------------------------------------------------------
/**

*/
ACC_REG_RESULT
CSimpleAccountManager::Register(const char *sUid,
	const char *sNick,
	const char *sPwd,
	uint64_t uConnId,
	uint64_t uInnerUuid,
	void *pConnImpl,
	time_t tmNow,
	ONLINE_HANDLE **ppOutAccount) {

	ACC_REG_RESULT eResult = ACC_REG_RESULT_OK;
	ONLINE_HANDLE *account = nullptr;

	// -1
	if (strlen(sUid) <= 0) {
		// invalid uid
		eResult = ACC_REG_RESULT_ERROR_INVALID_UID;
	}
	else {
		size_t szUidLen = strlen(sUid);
		size_t szNickLen = strlen(sNick);
		size_t szPwdLen = strlen(sPwd);

		if (szUidLen >= 36
			|| szNickLen >= 36
			|| szPwdLen >= 40) {
			//
			eResult = ACC_REG_RESULT_ERROR_BIND_UID_OVERFLOW;
		}
		else {
			// check conflict
			ONLINE_HANDLE *conflictAccount = _onlineList->GetByUid(sUid);
			if (conflictAccount) {
				//
				if (conflictAccount->login_checker.conflict_keyid > 0) {
					// target is already in conflict state
					eResult = ACC_REG_RESULT_ERROR_CONFLICT_TOO_MANY_TIMES;
				}
				else if (NULL == conflictAccount->ext_data) {
					// target's ext_data is not ready, must wait
					eResult = ACC_REG_RESULT_ERROR_CONFLICT_MAIN_ACCOUNT_IS_NOT_READY;
				}
				else if (uConnId != conflictAccount->route_table.connid) {
					// conflict on different connection, alloc new account
					account = _onlineList->Alloc();
					
					if (nullptr == account) {
						// account list is full
						eResult = ACC_REG_RESULT_ERROR_ACCOUNT_LIST_IS_FULL;
					}
					else if (account == conflictAccount) {
						// FATAL ERROR
						fprintf(stderr, "MUST NOT HAPPEN: account conflict with self!!!");
						system("pause");
						throw std::exception("MUST NOT HAPPEN: account conflict with self!!!");
					}
					else {
						// success -- mark conflict keyid each other
						account->login_checker.conflict_keyid = conflictAccount->keyid;
						conflictAccount->login_checker.conflict_keyid = account->keyid;

						// 1 means conflict
						eResult = ACC_REG_RESULT_CONFLICT;
					}
				}
				else {
					// duplicate registration is ok, reuse directly
					account = conflictAccount;

					//
					eResult = ACC_REG_RESULT_DUPLICATE_REGISTRATION;
				}
			}
			else {
				// no conflict, alloc new account
				account = _onlineList->Alloc();

				if (nullptr == account) {
					// account list is full
					eResult = ACC_REG_RESULT_ERROR_ACCOUNT_LIST_IS_FULL;
				}
			}
		}
	}

	// check result
	if (eResult >= 0) {
		// bind account info
		o_snprintf(account->uid, sizeof(account->uid), "%s", sUid);
		o_snprintf(account->nick, sizeof(account->nick), "%s", sNick);
		o_snprintf(account->pwd, sizeof(account->pwd), "%s", sPwd);

		//
		_onlineList->ConfirmAccountIsReady(*account, uConnId, uInnerUuid, pConnImpl, tmNow);

		//
		(*ppOutAccount) = account;
	}

	//
	return eResult;
}

//------------------------------------------------------------------------------
/**

*/
void
CSimpleAccountManager::Unregister(ONLINE_HANDLE& account) {
	if (account.is_ready) {

		// clear conflict keyid before release
		if (account.login_checker.conflict_keyid > 0) {
			ONLINE_HANDLE *conflictAccount = _onlineList->Get(account.login_checker.conflict_keyid);
			// clear conflict keyid each other
			account.login_checker.conflict_keyid = 0;
			conflictAccount->login_checker.conflict_keyid = 0;
		}

		//
		_onlineList->Release(account);
	}
	else {
		fprintf(stderr, "\n\n\n!!![CSimpleAccountManager::Unregister()] unregister two times!!! -- keyid(%d),uid(%s)\n\n\n",
			account.keyid, account.uid);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CSimpleAccountManager::UnregisterByConnId(uint64_t uConnId) {
	// walk
	ONLINE_HANDLE *online_;
	ACCOUNT_STATE_POOL& pool_ = _onlineList->GetPool();
	OBJECT_POOL_ITERATE_USED_OBJECT_BEGIN((&pool_), online_) {
		if (uConnId == online_->route_table.connid) {
			Unregister(*online_);
		}
	}
	OBJECT_POOL_ITERATE_USED_OBJECT_END();
}

//------------------------------------------------------------------------------
/**

*/
ONLINE_HANDLE *
CSimpleAccountManager::LookupAccountByInnerUuid(uint64_t uInnerUuid) {

	return _onlineList->GetByInnerUuid(uInnerUuid);
}

/* -- EOF -- */