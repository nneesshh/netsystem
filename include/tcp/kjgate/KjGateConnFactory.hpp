#pragma once
//------------------------------------------------------------------------------
/**
@class CKjGateConnFactory

(C) 2016 n.lee
*/
#include "../kj/KjConnFactoryBase.hpp"

//------------------------------------------------------------------------------
/**

*/
class CKjGateConnFactory : public CKjConnFactoryBase {
public:
	CKjGateConnFactory();
	virtual ~CKjGateConnFactory();

	/** **/
	virtual ITcpServer *		CreateTcpServer() override;
	virtual ITcpClient *		CreateTcpClientOnServer(std::string&& sPeerIp, ITcpServer *pServer) override;

	/** **/
	virtual ITcpIsolated *		CreateTcpIsolated() override;
	virtual ITcpIsolated *		CreateTcpIsolated2() override;

};

/*EOF*/