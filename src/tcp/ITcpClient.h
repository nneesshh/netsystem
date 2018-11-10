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
	virtual ~ITcpClient() noexcept(false) { }

public:
	/** **/
	virtual void				ConfirmClientIsReady(void *base, uintptr_t streamptr) = 0;

};

/*EOF*/