#pragma once
//------------------------------------------------------------------------------
/**
    @class CEvIsolatedConn
    
    (C) 2016 n.lee
*/
#ifdef __cplusplus
extern "C" {
#endif
#include "ev_client.h"
#include "ev_server.h"
#ifdef __cplusplus
}
#endif

#include "../IPacket.h"
#include "../ITcpIsolated.h"
#include "../ITcpConnFactory.h"
#include "../ITcpEventManager.h"

#include "../TcpConnManager.h"

//------------------------------------------------------------------------------
/** 
	@brief CEvIsolatedConn
*/
class CEvIsolatedConn : public ITcpIsolated
{
public:
	CEvIsolatedConn(uint64_t uConnId, ITcpConnFactory *pFactory);
	virtual ~CEvIsolatedConn();

	/** **/
	virtual void				OnConnect() override;
	virtual void				OnDisconnect() override;

	virtual void				OnRawData(bip_buf_t& bb) override {

		int nConsumed = _packet->DecodeStream(bb);

		CTcpConnManager::s_recv += nConsumed;
	}

	virtual void				OnGotPacket(const uint8_t *buf_in, size_t len) override;

public:
	/** **/
	virtual void				DisposeConnection() override;
	virtual void				FlushStream() override;

	virtual void				PostPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) override;
	virtual size_t				SendRaw(const uint8_t *buf, size_t len) override;

	/** Send data **/
	virtual size_t				SendPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) override;
	virtual size_t				SendPBMessage(google::protobuf::MessageLite *pMessage, uint64_t uInnerUuid, uint8_t uSerialNo) override;

	virtual void				Disconnect() override;

	virtual uint64_t			GetConnId() override {
		return _connId;
	}

	virtual void				SetCustomId(unsigned int nCustomId) override {
		_customId = nCustomId;
	}

	virtual unsigned int		GetCustomId() override {
		return _customId;
	}

	virtual const std::string&	GetPeerIp() override { static std::string sEmpty; return sEmpty; }

	virtual ITcpEventManager&	GetEventManager() override {
		return *_eventManager;
	}

	/** Check socket.
	\returns true when socket file descriptor is valid,
	socket connection is established, and socket is not about to
	be closed. **/
	virtual bool				IsReady() override {
		return (_connId > 0 && !IsDisposed() && !IsFlushed());
	}

	/** **/
	virtual	void				SetConnected(bool x = true) override {
		_bConnected = x;
	}

	virtual	bool				IsConnected() override {
		return _bConnected;
	}

	/** Set close and delete to terminate the connection. **/
	virtual void				SetDisposed(bool x = true) override {
		_bDisposed = x;
	}

	/** Check close and delete flag.
	\return true if this socket should be closed and the instance removed **/
	virtual	bool				IsDisposed() override {
		return _bDisposed;
	}

	/** **/
	virtual	bool				IsFlushed() override {
		return _bFlushed;
	}

	virtual	void				SetFlushed(bool x = true) override {
		_bFlushed = x;
	}

	/** **/
	virtual void				IncrFrontEndProduceNum() override {
		++_frontEndProduceNum;
	}

	virtual void				IncrBackEndConsumeNum() override {
		++_backEndConsumeNum;
	}

	virtual bool				IsFrontEndClean() override {
		return (_frontEndProduceNum == _backEndConsumeNum);
	}

	virtual void				IncrBackEndProduceNum() override {
		++_backEndProduceNum;
	}

	virtual void				IncrFrontEndConsumeNum() override {
		++_frontEndConsumeNum;
	}

	virtual bool				IsBackEndClean() override {
		return (_backEndProduceNum == _frontEndConsumeNum);
	}

public:
	/** **/
	virtual void				OnIsolatedError() override;

	/** **/
	virtual	void				SetConnectedEventPosted(bool x = true) override {
		// useless
		(void)x;
	}

	/** Connect **/
	virtual int					Connect(void *base, std::string& sIp_or_Hostname, unsigned short nPort) override;
	virtual void				Reconnect() override;
	virtual void				DelayReconnect(int nDelaySeconds) override;

	/** **/
	virtual void				Reopen(int nDelaySeconds) override;

private:
	struct event_base *_base = nullptr;
	client_handle_t *_clnt = nullptr;

	IPacket *_packet = nullptr;

	// 
	struct event *_reconnectTimer = nullptr;

	//////////////////////////////////////////////////////////////////////////
	uint64_t				_connId = 0;
	unsigned int			_customId = 0;

	ITcpConnFactory			*_refConnFactory;
	ITcpEventManager		*_eventManager;

	bool					_bRunning = false;
	bool					_bConnected = false;	/// socket is connected (tcp/udp)
	bool					_bDisposed = false;		/// start close and delete flag
	bool					_bFlushed = true;

	unsigned int			_frontEndProduceNum = 0;
	unsigned int			_backEndConsumeNum = 0;
	unsigned int			_backEndProduceNum = 0;
	unsigned int			_frontEndConsumeNum = 0;

	bool					_bDelayReconnecting = false;

	std::string				_sIp;
	unsigned short			_nPort = 0;
};

/*EOF*/