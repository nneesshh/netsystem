#pragma once
//------------------------------------------------------------------------------
/**
@class CUvConnFactory

(C) 2016 n.lee
*/
#include "UvConnFactoryBase.h"

//------------------------------------------------------------------------------
/**

*/
class CUvConnFactory : public CUvConnFactoryBase {
public:
	CUvConnFactory(StdLog *pLog);
	virtual ~CUvConnFactory();

	/** **/
	virtual ITcpServer *		CreateTcpServer() override;
	virtual ITcpClient *		CreateTcpClientOnServer(const std::string& sPeerIp, ITcpServer *pServer) override;

	/** **/
	virtual ITcpIsolated *		CreateTcpIsolated() override;
	virtual ITcpIsolated *		CreateTcpIsolated2() override;

};

/*EOF*/