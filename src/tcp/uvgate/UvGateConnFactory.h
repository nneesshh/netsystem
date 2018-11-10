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
	CUvGateConnFactory(StdLog *pLog);
	virtual ~CUvGateConnFactory();

	/** **/
	virtual ITcpServer *		CreateTcpServer() override;
	virtual ITcpClient *		CreateTcpClientOnServer(const std::string& sPeerIp, ITcpServer *pServer) override;

	/** **/
	virtual ITcpIsolated *		CreateTcpIsolated() override;
	virtual ITcpIsolated *		CreateTcpIsolated2() override;

};

/*EOF*/