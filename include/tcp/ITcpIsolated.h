#pragma once
//------------------------------------------------------------------------------
/**
@class ITcpIsolated

(C) 2016 n.lee
*/
#include "ITcpConn.h"

//------------------------------------------------------------------------------
/**

*/
class ITcpIsolated : public ITcpConn {
public:
	virtual ~ITcpIsolated() noexcept { }

	/** **/
	virtual void				OnIsolatedError() = 0;

	/** **/
	virtual void				SetConnectedEventPosted(bool b) = 0;

	/** Connect and reconnect **/
	virtual int					Connect(void *base, std::string& sIp_or_Hostname, unsigned short nPort) = 0;
	virtual void				Reconnect() = 0;
	virtual void				DelayReconnect(int nDelaySeconds) = 0;

	/** **/
	virtual void				Reopen(int nDelaySeconds) = 0;

};

/*EOF*/