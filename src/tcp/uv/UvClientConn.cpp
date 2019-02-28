//------------------------------------------------------------------------------
//  UvClientConn.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "UvClientConn.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "../../base/MyMacros.h"

#include "../tcp_packet_def.h"
#include "../TcpInnerPacket.h"

#include "UvConnFactory.h"
#include "UvServer.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
static void
on_stream_close(uv_handle_t* handle) {
	client_handle_t *clnt = (client_handle_t *)handle->data;
	ITcpIsolated *pConn = (ITcpIsolated *)clnt->_connimpl;
	if (pConn) {
		pConn->SetFlushed(true);
	}
	free(handle);
}

//------------------------------------------------------------------------------
/**

*/
CUvClientConn::CUvClientConn(uint64_t uConnId, std::string&& sPeerIp, ITcpServer *pServer)
	: _connId(uConnId)
	, _peerIp(std::move(sPeerIp))
	, _refServer(pServer) {

}

//------------------------------------------------------------------------------
/**

*/
CUvClientConn::~CUvClientConn() {
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
CUvClientConn::OnConnect() {
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
CUvClientConn::OnDisconnect() {

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
CUvClientConn::OnGotPacket(const uint8_t *buf_in, size_t len) {

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
CUvClientConn::DisposeConnection() {

	if (!IsDisposed()) {
		SetDisposed(true);

		// dispose packet
		SAFE_DELETE(_packet);

		// flush stream
		FlushStream();
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CUvClientConn::FlushStream() {
	//
	if (_clnt
		&& _clnt->_sockimpl
		&& !IsFlushed()) {

		uv_close_((uv_handle_t *)_clnt->_sockimpl, on_stream_close);
		_clnt->_sockimpl = nullptr;

		SetFlushed(true);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CUvClientConn::PostPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {
	if (_packet)
		_packet->Post(std::make_tuple(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody)));
}

//------------------------------------------------------------------------------
/**

*/
size_t
CUvClientConn::SendRaw(const uint8_t *buf, size_t len) {
	// null handle means "the connection is not initialized or already closed".
	assert(_clnt);

	// write
	write_req_t *wr = (write_req_t *)malloc(sizeof(write_req_t));
	wr->buf = uv_buf_init((char *)malloc(len), (unsigned int)len);
	memcpy(wr->buf.base, buf, len);

	CUvServer *pServer = static_cast<CUvServer *>(_refServer);
	struct uvsrvr_impl_t *srvr_impl = (struct uvsrvr_impl_t *)pServer->_srvr._impl;

	int r = uv_write((uv_write_t *)wr, (uv_stream_t *)_clnt->_sockimpl, &wr->buf, 1, srvr_impl->_writecb);
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
CUvClientConn::SendPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

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
CUvClientConn::SendPBMessage(google::protobuf::MessageLite *pMessage, uint64_t uInnerUuid, uint8_t uSerialNo) {

	std::string sTypeName = pMessage->GetTypeName();
	std::string sBody = pMessage->SerializeAsString();
	return SendPacket(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
}

//------------------------------------------------------------------------------
/**

*/
void
CUvClientConn::Disconnect() {
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
CUvClientConn::ConfirmClientIsReady(void *base, uintptr_t streamptr) {

	CUvServer *pServer = static_cast<CUvServer *>(_refServer);
	_clnt = pServer->RemoveStream(streamptr);
	if (_clnt) {
		// init base
		_base = (uv_loop_t *)base;

		// init packet
		if (nullptr == _packet) {
			_packet = new CTcpInnerPacket(this);
			_packet->SetGotPacketCb(std::bind(&CUvClientConn::OnGotPacket, this, std::placeholders::_1, std::placeholders::_2));
		}
	}

}

/** -- EOF -- **/