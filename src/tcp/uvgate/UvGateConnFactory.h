#pragma once
//------------------------------------------------------------------------------
/**
@class CUvGateConnFactory

(C) 2016 n.lee
*/
#include "../uv/UvConnFactoryBase.h"

//------------------------------------------------------------------------------
/**

*/
class CUvGateConnFactory : public CUvConnFactoryBase {
public:
	CUvGateConnFactory();
	virtual ~CUvGateConnFactory();

	/** **/
	virtual ITcpServer *		CreateTcpServer() override;
	virtual ITcpClient *		CreateTcpClientOnServer(std::string&& sPeerIp, ITcpServer *pServer) override;

	/** **/
	virtual ITcpIsolated *		CreateTcpIsolated() override;
	virtual ITcpIsolated *		CreateTcpIsolated2() override;

};

/*EOF*/