#pragma once
//------------------------------------------------------------------------------
/**
@class CKjConnFactory

(C) 2016 n.lee
*/
#include "KjConnFactoryBase.hpp"

//------------------------------------------------------------------------------
/**

*/
class CKjConnFactory : public CKjConnFactoryBase {
public:
	CKjConnFactory(StdLog *pLog);
	virtual ~CKjConnFactory();

	/** **/
	virtual ITcpServer *		CreateTcpServer() override;
	virtual ITcpClient *		CreateTcpClientOnServer(const std::string& sPeerIp, ITcpServer *pServer) override;

	/** **/
	virtual ITcpIsolated *		CreateTcpIsolated() override;
	virtual ITcpIsolated *		CreateTcpIsolated2() override;

};

/*EOF*/