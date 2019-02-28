#ifndef __SIMPLE_ZONE_MANAGER_H__
#define __SIMPLE_ZONE_MANAGER_H__
//------------------------------------------------------------------------------
/**
@class CSimpleZoneManager

(C) 2016 n.lee
*/
#include <map>
#include <vector>

#include "../UsingBase.h"

#include "IZoneManager.h"

//------------------------------------------------------------------------------
/**
@brief CSimpleZoneManager
*/
class CSimpleZoneManager : public IZoneManager {
public:
	CSimpleZoneManager();
	virtual ~CSimpleZoneManager();

	/** **/
	virtual MAP_ZONES *			GetZones() {
		return &m_mapZoneInfos;
	}

	virtual MyZoneInfo *		GetZone(int nZoneId) {
		MAP_ZONES::iterator it = m_mapZoneInfos.find(nZoneId);
		if (it != m_mapZoneInfos.end()) {
			return &(it->second);
		}
		return nullptr;
	}

	virtual MyZoneInfo *		GetZoneWithCreate(int nZoneId) {
		return &(m_mapZoneInfos[nZoneId]);
	}

	virtual void				Register(int nZoneId, uint64_t uConnId, const char *sDomain, const char *sIp, uint16_t nPort);
	virtual void				Unregister(int nZoneId);
	virtual void				UnregisterByConnId(uint64_t uConnId);
	virtual MyZoneInfo *		GetZoneByConnId(uint64_t uConnId);

	/** **/
	virtual void				SetWhiteName(const char *sIp, const char *sMask);

	/** **/
	virtual void				RegisterChannel(MyChannelInfo *pChannelInfo);

	virtual MAP_CHANNELS *		GetChannels() {
		return &m_mapChannels;
	}

	virtual MyChannelInfo *		GetChannel(int nChannelId) {
		MAP_CHANNELS::iterator it = m_mapChannels.find(nChannelId);
		if (it != m_mapChannels.end()) {
			return &(it->second);
		}
		return nullptr;
	}

	virtual MyZoneInfo *		GetZoneByChannel(int nChannelId) {
		if (nChannelId > 0) {
			MyChannelInfo *pChannelInfo = GetChannel(nChannelId);
			if (pChannelInfo) {
				MyZoneInfo *pZoneInfo = GetZone(pChannelInfo->nZoneId);
				return pZoneInfo;
			}
		}
		return nullptr;
	}

	/** **/
	virtual MyChannelPeer *		AddChannelPeer(int nChannelId, int nPeerId);
	virtual void				RemoveChannelPeer(int nChannelId, int nPeerId);

	virtual CHANNEL_PEER_LIST *	GetChannelPeers(int nChannelId) {
		MAP_CHANNEL_PEERS::iterator it = m_mapChannelPeers.find(nChannelId);
		if (it != m_mapChannelPeers.end()) {
			return &(it->second);
		}
		return nullptr;
	}

private:
	void						RemoveDirtyChannels(int nZoneId);
	void						RemoveDirtyPeers(int nZoneId);

	bool						CheckIpInWhiteList(const char *sIp);

public:
	MAP_ZONES			m_mapZoneInfos;
	MAP_CHANNELS		m_mapChannels;
	MAP_CHANNEL_PEERS	m_mapChannelPeers;

	std::vector<ip_pair_t>	m_vWhiteList;
};

#endif //