#pragma once
//------------------------------------------------------------------------------
/**
	@class ITcpConn

	(C) 2016 n.lee
*/
#include "../UsingProtobuf.h"
#include "../base/bip_buf.h"

#include "tcp_def.h"

class ITcpEventManager;

//------------------------------------------------------------------------------
/**

*/
class ITcpConn {
public:
	virtual ~ITcpConn() noexcept { }

	/** **/
	virtual void				OnConnect() = 0;
	virtual void				OnDisconnect() = 0;

	virtual void				OnRawData(bip_buf_t& bb) = 0;
	virtual void				OnGotPacket(const uint8_t *buf_in, size_t len) = 0;

public:
	/** **/
	virtual void				DisposeConnection() = 0;
	virtual void				FlushStream() = 0;

	virtual void				PostPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) = 0;
	virtual size_t				SendRaw(const uint8_t *buf, size_t len) = 0;

	/** Send data */
	virtual size_t				SendPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) = 0;

	/** Don't use "SendMessage" as func name, windows already defined it to "SendMessageA" */
	virtual size_t				SendPBMessage(google::protobuf::MessageLite *pMessage, uint64_t uInnerUuid, uint8_t uSerialNo) = 0;

	virtual void				Disconnect() = 0;

	virtual uint64_t			GetConnId() = 0;
	
	virtual void				SetCustomId(unsigned int) = 0;
	virtual unsigned int		GetCustomId() = 0;

	virtual const std::string&	GetPeerIp() = 0;

	virtual ITcpEventManager& 	GetEventManager() = 0;

	/** Check socket.
	\returns true when socket file descriptor is valid,
	socket connection is established, and socket is not about to
	be closed. **/
	virtual bool				IsReady() = 0;

	/** Set connected status. **/
	virtual	void				SetConnected(bool = true) = 0;

	/** Check connected status.
	\return true if connected **/
	virtual	bool				IsConnected() = 0;
	/** Set close and delete to terminate the connection. **/

	virtual void				SetDisposed(bool = true) = 0;

	/** Check close and delete flag.
		\ it is an enable connection and disable connection flag.
		\ if the _bCloseAndDelete is true, it means that connection will be killed in future time.
		\ (return true if this socket should be closed and the instance removed) **/
	virtual	bool				IsDisposed() = 0;

	/** **/
	virtual	bool				IsFlushed() = 0;
	virtual	void				SetFlushed(bool x = true) = 0;

	/** **/
	virtual void				IncrFrontEndProduceNum() = 0;
	virtual void				IncrBackEndConsumeNum() = 0;
	virtual bool				IsFrontEndClean() = 0;

	virtual void				IncrBackEndProduceNum() = 0;
	virtual void				IncrFrontEndConsumeNum() = 0;
	virtual bool				IsBackEndClean() = 0;

};

/*EOF*/