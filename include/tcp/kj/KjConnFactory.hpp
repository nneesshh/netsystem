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
	CKjConnFactory();
	virtual ~CKjConnFactory();

	/** **/
	virtual ITcpServer *		CreateTcpServer() override;
	virtual ITcpClient *		CreateTcpClientOnServer(std::string&& sPeerIp, ITcpServer *pServer) override;

	/** **/
	virtual ITcpIsolated *		CreateTcpIsolated() override;
	virtual ITcpIsolated *		CreateTcpIsolated2() override;

};

/*EOF*/