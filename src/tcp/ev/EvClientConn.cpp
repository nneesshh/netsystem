//------------------------------------------------------------------------------
//  EvClientConn.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "EvClientConn.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "../tcp_packet_def.h"
#include "../TcpInnerPacket.h"

#include "EvConnFactory.h"
#include "EvServer.h"

#ifdef _MSC_VER
	#ifdef _DEBUG
		#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
	#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CEvClientConn::CEvClientConn(uint64_t uConnId, const std::string& sPeerIp, ITcpServer *pServer)
	: _connId(uConnId)
	, _peerIp(std::move(sPeerIp))
	, _refServer(pServer) {

}

//------------------------------------------------------------------------------
/**

*/
CEvClientConn::~CEvClientConn() {
	// free conn handle
	if (_clnt) {
		bip_buf_destroy(_clnt->_bb);
		free(_clnt);
		_clnt = nullptr;
	}

	_refServer = nullptr;
}

//------------------------------------------------------------------------------
/**

*/
void
CEvClientConn::OnConnect() {
	// connect event will be ignored if not running
	if (_bRunning
		&& !IsConnected()) {
		//
		SetConnected(true);

		// notify connected event
		GetEventManager().OnEvent(CONNECTION_CONNECTED, this);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CEvClientConn::OnDisconnect() {

	if (IsConnected()) {
		//
		SetConnected(false);

		// notify disconnected event
		GetEventManager().OnEvent(CONNECTION_DISCONNECTED, this);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CEvClientConn::OnGotPacket(const uint8_t *buf_in, size_t len) {

	size_t szHeaderLen = SIZE_OF_TCP_INNER_PACKET_LEADING;
	tcp_inner_packet_leading_t *pHeader = (tcp_inner_packet_leading_t *)buf_in;

	size_t szTypeNameLen = (size_t)pHeader->inner_name_len;
	std::string sTypeName((char *)(buf_in + szHeaderLen), szTypeNameLen);

	size_t szBodyLen = len - szTypeNameLen - szHeaderLen;
	std::string sBody((char *)(buf_in + szHeaderLen + pHeader->inner_name_len), szBodyLen);

	//
	_refServer->GetConnFactory().AddClientInnerPacketCb(this, GetConnId(), pHeader->inner_uuid, pHeader->inner_serial_no, std::move(sTypeName), std::move(sBody));
}

//------------------------------------------------------------------------------
/**

*/
void
CEvClientConn::DisposeConnection() {

	if (!IsDisposed()) {
		SetDisposed(true);

		// dispose packet
		SAFE_DELETE(_packet);

		// flush stream
		if (_clnt
			&& _clnt->_sockimpl
			&& !IsFlushed()) {
			
			ev_close_((struct bufferevent *)_clnt->_sockimpl);
			_clnt->_sockimpl = nullptr;
			
			SetFlushed(true);
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CEvClientConn::PostPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

	assert(IsReady());

	_packet->Post(uInnerUuid, uSerialNo, sTypeName, sBody);
}

//------------------------------------------------------------------------------
/**

*/
size_t
CEvClientConn::SendRaw(const uint8_t *buf, size_t len) {
	// null handle means "the connection is not initialized or already closed".
	assert(_clnt);

		// write
	struct bufferevent *bev = (struct bufferevent *)_clnt->_sockimpl;
	int r = bufferevent_write(bev, buf, len);
	if (r < 0) {
		DisposeConnection();
		return 0;
	}

	CTcpConnManager::s_send += len;
	return len;
}

//------------------------------------------------------------------------------
/**

*/
size_t
CEvClientConn::SendPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

	size_t szHeaderLen = SIZE_OF_TCP_INNER_PACKET_LEADING;
	size_t szTypeNameLen = sTypeName.length();
	size_t szBodyLen = sBody.length();

	//
	_refServer->GetConnFactory().PostPacket(this, uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
	return szHeaderLen + szTypeNameLen + szBodyLen;
}

//------------------------------------------------------------------------------
/**

*/
size_t
CEvClientConn::SendPBMessage(google::protobuf::MessageLite *pMessage, uint64_t uInnerUuid, uint8_t uSerialNo) {

	std::string sTypeName = pMessage->GetTypeName();
	std::string sBody = pMessage->SerializeAsString();
	return SendPacket(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
}

//------------------------------------------------------------------------------
/**

*/
void
CEvClientConn::Disconnect() {
	// disconnect means stop running and remove
	if (!_bDisposed
		&& _bRunning) {
		_bRunning = false;

		OnDisconnect();

		// remove and dispose conn
		_refServer->GetConnFactory().GetConnManager().OnRemoveClient(_refServer, this);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CEvClientConn::ConfirmClientIsReady(void *base, uintptr_t streamptr) {

	CEvServer *pServer = static_cast<CEvServer *>(_refServer);
	_clnt = pServer->RemoveStream(streamptr);
	if (_clnt) {
		// init base
		_base = (struct event_base *)base;

		// init packet
		if (nullptr == _packet) {
			_packet = new CTcpInnerPacket(this);
			_packet->SetGotPacketCb(std::bind(&CEvClientConn::OnGotPacket, this, std::placeholders::_1, std::placeholders::_2));
		}
	}

}

/** -- EOF -- **/