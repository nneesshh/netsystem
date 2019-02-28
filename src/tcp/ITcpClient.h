#pragma once
//------------------------------------------------------------------------------
/**
    @class ITcpClient
    
    (C) 2016 n.lee
*/
#include "ITcpConn.h"

//------------------------------------------------------------------------------
/**

*/
class ITcpClient : public ITcpConn {
public:
	virtual ~ITcpClient() noexcept { }

public:
	/** **/
	virtual void				ConfirmClientIsReady(void *base, uintptr_t streamptr) = 0;

};

/*EOF*/