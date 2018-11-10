#pragma once
//------------------------------------------------------------------------------
/**
@class CTcpEventManager

(C) 2016 n.lee
*/
#include <vector>
#include <map>

#include "ITcpEventManager.h"

//------------------------------------------------------------------------------
/**

*/
class CTcpEventManager : public ITcpEventManager {
public:
	CTcpEventManager();
	virtual ~CTcpEventManager();

	/** **/
	virtual void				OnEvent(TCP_EVENT_ID nEventId, ITcpConn *pConn) {
		_arrEvtHandlers[nEventId](pConn);
	}

	/** **/
	virtual void				OnPacket(ITcpConn *pConn, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {
		_packetHandler(pConn, uSerialNo, sTypeName, sBody);
	}

	/** **/
	virtual void				OnInnerPacket(ITcpConn *pConn, uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {
		_innerPacketHandler(pConn, uInnerUuid, uSerialNo, sTypeName, sBody);
	}

	/** **/
	virtual bool				IsReady() {
		return _bReady;
	}

	virtual void				SetReady(bool bReady) {
		_bReady = bReady;
	}

	/** **/
	virtual void				RegisterEventHandler(uint32_t uEventId, TCP_EVENT_HANDLER d) {
		_arrEvtHandlers[uEventId] = d;
	}

	virtual void				ClearAllEventHandlers();

	/** **/
	virtual void				RegisterPacketHandler(TCP_PACKET_HANDLER d) {
		_packetHandler = d;
	}

	/** **/
	virtual void				RegisterInnerPacketHandler(TCP_INNER_PACKET_HANDLER d) {
		_innerPacketHandler = d;
	}

private:
	TCP_EVENT_HANDLER								_arrEvtHandlers[TCP_EVENT_ID_MAX];

	TCP_PACKET_HANDLER								_packetHandler;
	TCP_INNER_PACKET_HANDLER						_innerPacketHandler;

	bool											_bReady;
};

/*EOF*/