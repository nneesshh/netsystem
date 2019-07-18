#pragma once
//------------------------------------------------------------------------------
/**
@class ITcpEventManager

(C) 2016 n.lee
*/
#include <functional>

//
enum TCP_EVENT_ID {
	CONNECTION_CONNECTED = 1,
	CONNECTION_DISCONNECTED,
	SERVICE_READY,

	//
	TCP_EVENT_ID_MAX
};

//
class ITcpConn;
using TCP_EVENT_HANDLER = std::function<void(ITcpConn *)>;
using TCP_PACKET_HANDLER = std::function<void(ITcpConn *, uint8_t, std::string&, std::string&)>;
using TCP_INNER_PACKET_HANDLER = std::function<void(ITcpConn *, uint64_t, uint8_t, std::string&, std::string&)>;

//------------------------------------------------------------------------------
/**

*/
class ITcpEventManager {
public:
	virtual ~ITcpEventManager() noexcept { }

	/** **/
	virtual void				OnEvent(TCP_EVENT_ID nEventId, ITcpConn *pConn) = 0;

	/** **/
	virtual void				OnPacket(ITcpConn *pConn, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) = 0;
	virtual void				OnInnerPacket(ITcpConn *pConn, uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) = 0;

	/** **/
	virtual bool				IsReady() = 0;
	virtual void				SetReady(bool bReady) = 0;

	/** **/
	virtual void				RegisterEventHandler(uint32_t uEventId, TCP_EVENT_HANDLER d) = 0;
	virtual void				ClearAllEventHandlers() = 0;

	/**  **/
	virtual void				RegisterPacketHandler(TCP_PACKET_HANDLER d) = 0;
	virtual void				RegisterInnerPacketHandler(TCP_INNER_PACKET_HANDLER d) = 0;

};

/*EOF*/