//------------------------------------------------------------------------------
//  KjClientConn.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjClientConn.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "../../base/MyMacros.h"

#include "../tcp_packet_def.h"
#include "../TcpInnerPacket.h"

#include "KjConnFactory.hpp"
#include "KjServer.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CKjClientConn::CKjClientConn(uint64_t uConnId, std::string&& sPeerIp, ITcpServer *pServer)
	: _connId(uConnId)
	, _peerIp(std::move(sPeerIp))
	, _refServer(pServer) {

}

//------------------------------------------------------------------------------
/**

*/
CKjClientConn::~CKjClientConn() noexcept {
	_refServer = nullptr;
}

//------------------------------------------------------------------------------
/**

*/
void
CKjClientConn::OnConnect() {
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
CKjClientConn::OnDisconnect() {

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
CKjClientConn::OnGotPacket(const uint8_t *buf_in, size_t len) {

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
CKjClientConn::DisposeConnection() {
	
	if (!IsDisposed()) {
		SetDisposed(true);

		// dispose packet
		SAFE_DELETE(_thr_packet);

		// flush stream
		FlushStream();

		// dispose stream for thread
		_thr_tcpStream = nullptr;

		// dispose base for thread
		_thr_tasks = nullptr;
		_thr_endpointContext = nullptr;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjClientConn::FlushStream() {
	// 
	if (_thr_tcpStream
		&& !IsFlushed()) {
		_thr_tcpStream->FlushStream();
		SetFlushed(true);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjClientConn::PostPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {
	if (_thr_packet)
		_thr_packet->Post(std::make_tuple(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody)));
}

//------------------------------------------------------------------------------
/**

*/
size_t
CKjClientConn::SendPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {
	
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
CKjClientConn::SendPBMessage(google::protobuf::MessageLite *pMessage, uint64_t uInnerUuid, uint8_t uSerialNo) {
	
	std::string sTypeName = pMessage->GetTypeName();
	std::string sBody = pMessage->SerializeAsString();
	return SendPacket(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjClientConn::Disconnect() {
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
CKjClientConn::ConfirmClientIsReady(void *base, uintptr_t streamptr) {

	//fprintf(stderr, "[CKjClientConn::ConfirmClientIsReady()] confirm client is ready 1111111111111111++++++++++++++++++++++++++++++++\n");

	CKjServer *pServer = static_cast<CKjServer *>(_refServer);
	_thr_tcpStream = pServer->RemoveStream(streamptr);
	if (_thr_tcpStream) {
		// init base
		_thr_endpointContext = kj::addRef(*(KjPipeEndpointIoContext *)base);
		_thr_tasks = netsystem_get_servercore()->NewTaskSet(*this);

		// init packet
		if (nullptr == _thr_packet) {
			_thr_packet = new CTcpInnerPacket(this);
			_thr_packet->SetGotPacketCb(std::bind(&CKjClientConn::OnGotPacket, this, std::placeholders::_1, std::placeholders::_2));
		}

		//fprintf(stderr, "[CKjClientConn::ConfirmClientIsReady()] start read op 2222222222222222++++++++++++++++++++++++++++++++\n");

		// start read
		auto p1 = _thr_tcpStream->StartReadOp(
			[this](KjTcpDownStream&, bip_buf_t& bb) {
			//
			OnRawData(bb);
		});
		// client start read op
		_thr_tasks->add(kj::mv(p1));
	}

}

//------------------------------------------------------------------------------
/**

*/
void
CKjClientConn::taskFailed(kj::Exception&& exception) {
	CKjServer *pServer = static_cast<CKjServer *>(_refServer);

	// schedule eval later to avoid destroying self task set
	netsystem_get_servercore()->ScheduleEvalLaterFunc([this]() {

		// flush stream
		FlushStream();

		//
		_refServer->OnDisposeClient(this);
	});
}

/** -- EOF -- **/