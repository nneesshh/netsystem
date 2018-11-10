//------------------------------------------------------------------------------
//  TcpEventManager.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "TcpEventManager.h"

#ifdef _MSC_VER
	#ifdef _DEBUG
		#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
	#endif
#endif

static TCP_EVENT_HANDLER s_defaultEventHandler = [](ITcpConn *) {};
static TCP_PACKET_HANDLER s_defaultPacketHandler = [](ITcpConn *, uint8_t, std::string&, std::string&) {};
static TCP_INNER_PACKET_HANDLER s_defaultInnerPacketHandler = [](ITcpConn *, uint64_t, uint8_t, std::string&, std::string&) {};

//------------------------------------------------------------------------------
/**

*/
CTcpEventManager::CTcpEventManager()
	: _bReady(false) {
	//
	ClearAllEventHandlers();
}

//------------------------------------------------------------------------------
/**

*/
CTcpEventManager::~CTcpEventManager() {
	
}

//------------------------------------------------------------------------------
/**

*/
void
CTcpEventManager::ClearAllEventHandlers() {

	for (auto& h : _arrEvtHandlers) {
		h = s_defaultEventHandler;
	}

	_packetHandler = s_defaultPacketHandler;
	_innerPacketHandler = s_defaultInnerPacketHandler;
}

/* -- EOF -- */