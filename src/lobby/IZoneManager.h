#pragma once
//------------------------------------------------------------------------------
/**
@class IZoneManager

(C) 2016 n.lee
*/
#include "../common/UsingMyToolkitMini.h"

#include <vector>
#include <unordered_map>

//////////////////////////////////////////////////////////////////////////
typedef struct _MyZoneInfo_Tag
{
	int				nZoneId;
	int				nType;
	char			chName[32];

	char			chIp[32];
	char			chDomain[64];
	uint16_t		nPort;

	unsigned int	uConnId;		//

	struct
	{
		int		nExchangeRate;
		int		nBetRate;
		int		nTitleLimit;
		int		nPocketCashLimit;
		int		nChannelDefaultNum;
		int		nChannelBattleNum;
	} misc;
} MyZoneInfo;

typedef struct _MyChannelInfo_Tag
{
	int			nChannelId;
	int			nChannelType;
	char		chChannelDesc[32];
	bool		bHasLock;

	int			nZoneId;
} MyChannelInfo;

typedef struct _MyChannelPeer_Tag
{
	int			nPeerId;
	char		chNick[36];
	int			nSex;
	int			nTitleId;

	int			nFlag;
} MyChannelPeer;

//------------------------------------------------------------------------------
/**
@brief IZoneManager
*/
class IZoneManager
{
public:
	virtual ~IZoneManager() { }

	typedef std::unordered_map<int, MyZoneInfo>			MAP_ZONES;				// zone id -> zone info
	typedef std::unordered_map<int, MyChannelInfo>		MAP_CHANNELS;			// channel id -> channel info

	typedef std::vector<MyChannelPeer>			CHANNEL_PEER_LIST;
	typedef std::unordered_map<int, CHANNEL_PEER_LIST >	MAP_CHANNEL_PEERS;

	/** **/
	virtual MAP_ZONES *			GetZones() = 0;
	virtual MyZoneInfo *		GetZone(int nZoneId) = 0;
	virtual MyZoneInfo *		GetZoneWithCreate(int nZoneId) = 0;

	virtual void				Register(int nZoneId, uint64_t uConnId, const char *sDomain, const char *sIp, uint16_t nPort) = 0;
	virtual void				Unregister(int nZoneId) = 0;
	virtual void				UnregisterByConnId(uint64_t uConnId) = 0;
	virtual MyZoneInfo *		GetZoneByConnId(uint64_t uConnId) = 0;

	/** **/
	virtual void				SetWhiteName(const char *sIp, const char *sMask) = 0;

	/** **/
	virtual void				RegisterChannel(MyChannelInfo *pChannelInfo) = 0;

	virtual MAP_CHANNELS *		GetChannels() = 0;
	virtual MyChannelInfo *		GetChannel(int nChannelId) = 0;

	virtual MyZoneInfo *		GetZoneByChannel(int nChannelId) = 0;

	/** **/
	virtual MyChannelPeer *		AddChannelPeer(int nChannelId, int nPeerId) = 0;
	virtual void				RemoveChannelPeer(int nChannelId, int nPeerId) = 0;

	virtual CHANNEL_PEER_LIST *	GetChannelPeers(int nChannelId) = 0;
};

/*EOF*/