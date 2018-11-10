//------------------------------------------------------------------------------
//  SimpleZoneManager.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "SimpleZoneManager.h"

#include <time.h>
#include <locale.h>

#include "../common/UsingMyToolkitMini.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CSimpleZoneManager::CSimpleZoneManager() {

}

//------------------------------------------------------------------------------
/**

*/
CSimpleZoneManager::~CSimpleZoneManager() {

}

//------------------------------------------------------------------------------
/**

*/
void
CSimpleZoneManager::Register(int nZoneId, uint64_t uConnId, const char *sDomain, const char *sIp, uint16_t nPort) {

	bool bInWhiteList = CheckIpInWhiteList(sIp);
	if (!bInWhiteList)
		return;

	MyZoneInfo *pZoneInfo = GetZoneWithCreate(nZoneId);

	pZoneInfo->nZoneId = nZoneId;
	pZoneInfo->nType = 0;
	o_snprintf(pZoneInfo->chName, sizeof(pZoneInfo->chName), "Server%d", nZoneId);
	o_snprintf(pZoneInfo->chIp, sizeof(pZoneInfo->chIp), "%s", sIp);
	o_snprintf(pZoneInfo->chDomain, sizeof(pZoneInfo->chDomain), "%s", sDomain);
	pZoneInfo->nPort = nPort;
	pZoneInfo->uConnId = uConnId;

	// remove invalid peers of this server
	RemoveDirtyPeers(nZoneId);

	// remove invalid channels of this server
	RemoveDirtyChannels(nZoneId);
}

//------------------------------------------------------------------------------
/**

*/
void
CSimpleZoneManager::Unregister(int nZoneId) {

	MAP_ZONES::iterator it = m_mapZoneInfos.find(nZoneId);
	if (it != m_mapZoneInfos.end()) {

		m_mapZoneInfos.erase(it);

		// remove invalid peers of this server
		RemoveDirtyPeers(nZoneId);

		// remove invalid channels of this server
		RemoveDirtyChannels(nZoneId);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CSimpleZoneManager::UnregisterByConnId(uint64_t uConnId) {

	MyZoneInfo *pZoneInfo;
	MAP_ZONES::iterator it = m_mapZoneInfos.begin(), itEnd = m_mapZoneInfos.end();
	while (it != itEnd) {

		pZoneInfo = &(it->second);

		if (pZoneInfo->uConnId == uConnId) {

			it = m_mapZoneInfos.erase(it);
			itEnd = m_mapZoneInfos.end();
		}
		else {
			++it;
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
MyZoneInfo *
CSimpleZoneManager::GetZoneByConnId(uint64_t uConnId) {

	MyZoneInfo *pZoneInfo;
	MAP_ZONES::iterator it = m_mapZoneInfos.begin(), itEnd = m_mapZoneInfos.end();
	while (it != itEnd) {

		pZoneInfo = &(it->second);

		if (pZoneInfo->uConnId == uConnId) {
			return pZoneInfo;
		}

		//
		++it;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
/**

*/
void
CSimpleZoneManager::SetWhiteName(const char *sIp, const char *sMask) {

	ip_pair_t ip_pair;
	split_ip(sIp, strlen(sIp), &ip_pair._ip);
	split_ip(sMask, strlen(sMask), &ip_pair._mask);
	m_vWhiteList.push_back(ip_pair);
}

//------------------------------------------------------------------------------
/**

*/
void
CSimpleZoneManager::RegisterChannel(MyChannelInfo *pChannelInfo) {

	int nChannelId = pChannelInfo->nChannelId;
	if (nChannelId >= 0) {
		// add or update
		m_mapChannels[nChannelId] = *pChannelInfo;
	}
}

//------------------------------------------------------------------------------
/**

*/
MyChannelPeer *
CSimpleZoneManager::AddChannelPeer(int nChannelId, int nPeerId) {

	// check exist
	CHANNEL_PEER_LIST *pList = &(m_mapChannelPeers[nChannelId]);
	CHANNEL_PEER_LIST::iterator it = pList->begin(), itEnd = pList->end();
	while (it != itEnd) {

		if (nPeerId == it->nPeerId) {
			return &(*it);
		}

		//	
		++it;
	}

	// add one
	pList->resize(pList->size() + 1);

	//
	MyChannelPeer *pPeer = &(*pList->rbegin());
	pPeer->nPeerId = nPeerId;
	pPeer->chNick[0] = '\0';
	pPeer->nSex = -1;
	pPeer->nFlag = 0;
	return pPeer;
}

//------------------------------------------------------------------------------
/**

*/
void
CSimpleZoneManager::RemoveChannelPeer(int nChannelId, int nPeerId) {

	MAP_CHANNEL_PEERS::iterator it = m_mapChannelPeers.find(nChannelId);
	if (it != m_mapChannelPeers.end()) {

		CHANNEL_PEER_LIST *pList = &(it->second);
		CHANNEL_PEER_LIST::iterator it2 = pList->begin(), itEnd2 = pList->end();
		while (it2 != itEnd2) {

			if (nPeerId == it2->nPeerId) {
				pList->erase(it2);
				break;
			}

			//
			++it2;
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CSimpleZoneManager::RemoveDirtyChannels(int nZoneId) {

	MyChannelInfo *pChannelInfo;
	MAP_CHANNELS::iterator it = m_mapChannels.begin(), itEnd = m_mapChannels.end();
	while (it != itEnd) {

		pChannelInfo = &(it->second);

		//
		if (nullptr == pChannelInfo || pChannelInfo->nZoneId == nZoneId) {

			it = m_mapChannels.erase(it);
			itEnd = m_mapChannels.end();
		}
		else {
			++it;
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CSimpleZoneManager::RemoveDirtyPeers(int nZoneId) {

	int nChannelId;
	MyZoneInfo *pZoneInfo;
	MAP_CHANNEL_PEERS::iterator it = m_mapChannelPeers.begin(), itEnd = m_mapChannelPeers.end();
	while (it != itEnd) {

		nChannelId = it->first;
		pZoneInfo = GetZoneByChannel(nChannelId);
		if (nullptr == pZoneInfo || pZoneInfo->nZoneId == nZoneId) {

			it = m_mapChannelPeers.erase(it);
			itEnd = m_mapChannelPeers.end();
		}
		else {
			++it;
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
bool
CSimpleZoneManager::CheckIpInWhiteList(const char *sIp) {
	if (m_vWhiteList.size() <= 0)
		return true;

	bool bInWhiteList = false;

	ip_num_array_t ip;
	char chIp[32] = { 0 };

	o_snprintf(chIp, sizeof(chIp), "%s", sIp);
	split_ip(chIp, strlen(chIp), &ip);

	std::vector<ip_pair_t>::iterator it = m_vWhiteList.begin(), itEnd = m_vWhiteList.end();
	while (it != itEnd) {

		bool bAllMatched = true;
		int i;
		for (i = 0; i < sizeof(ip_num_array_t); ++i) {

			int n1 = ip._num[i];
			int n2 = (*it)._ip._num[i];
			int nMask = (*it)._mask._num[i];
			if ((n1&nMask) != (n2&nMask)) {
				bAllMatched = false;
				break;
			}
		}

		if (bAllMatched) {
			bInWhiteList = true;
			break;
		}

		//
		++it;
	}
	return bInWhiteList;
}

/* -- EOF -- */