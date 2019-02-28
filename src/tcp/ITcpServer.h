#pragma once
//------------------------------------------------------------------------------
/**
    @class ITCPServer
    
    (C) 2016 n.lee
*/
#include <stdint.h>
#include <string>

class ITcpConnFactory;
class ITcpEventManager;
class ITcpClient;

//------------------------------------------------------------------------------
/** 
	@brief ITCPServer
*/
class ITcpServer {
public:
	virtual ~ITcpServer() noexcept { }

	/** Server accept and dispose client */
	virtual ITcpClient *		OnAcceptClient(uintptr_t streamptr, std::string&& sPeerIp) = 0;
	virtual void				OnDisposeClient(ITcpClient *pClient) = 0;

public:
	/** Open and close **/
	virtual int					Open(void *base, unsigned short port) = 0;
	virtual void				Close() = 0;
	virtual void				FlushDownStream(uintptr_t streamptr) = 0;

	virtual bool				IsClosed() = 0;
	virtual bool				IsReady() = 0;

	/** **/
	virtual void *				GetBase() = 0;
	virtual unsigned short		GetPort() = 0;
	virtual ITcpConnFactory& 	GetConnFactory() = 0;
	virtual ITcpEventManager& 	GetEventManager() = 0;

};

/*EOF*/