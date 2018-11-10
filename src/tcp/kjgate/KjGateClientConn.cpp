//------------------------------------------------------------------------------
//  KjGateClientConn.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjGateClientConn.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "../tcp_packet_def.h"
#include "../TcpOuterPacket.h"

#include "KjGateConnFactory.hpp"
#include "KjGateServer.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
CKjGateClientConn::CKjGateClientConn(uint64_t uConnId, const std::string& sPeerIp, ITcpServer *pServer)
	: _connId(uConnId)
	, _peerIp(std::move(sPeerIp))
	, _refServer(pServer) {

}

//------------------------------------------------------------------------------
/**

*/
CKjGateClientConn::~CKjGateClientConn() {
	_refServer = nullptr;
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateClientConn::OnConnect() {
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
CKjGateClientConn::OnDisconnect() {

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
CKjGateClientConn::OnGotPacket(const uint8_t *buf_in, size_t len) {

	size_t szHeaderLen = SIZE_OF_TCP_OUTER_PACKET_LEADING;
	tcp_outer_packet_leading_t *pHeader = (tcp_outer_packet_leading_t *)buf_in;


	size_t szTypeNameLen = (size_t)pHeader->name_len;
	std::string sTypeName((char *)(buf_in + szHeaderLen), szTypeNameLen);

	size_t szBodyLen = len - szTypeNameLen - szHeaderLen;
	std::string sBody((char *)(buf_in + szHeaderLen + pHeader->name_len), szBodyLen);

	//
	_refServer->GetConnFactory().AddClientPacketCb(this, GetConnId(), pHeader->serial_no, std::move(sTypeName), std::move(sBody));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateClientConn::DisposeConnection() {

	if (!IsDisposed()) {
		SetDisposed(true);

		// dispose packet
		SAFE_DELETE(_thr_packet);

		// flush stream
		if (_thr_tcpStream
			&& !IsFlushed()) {

			_thr_tcpStream->Disconnect();

			SetFlushed(true);
		}

		// dispose stream
		_thr_tcpStream = nullptr;

		// dispose base
		_thr_tioContext = nullptr;
		_thr_tasks = nullptr;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateClientConn::PostPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

	assert(IsReady());

	std::tuple<uint64_t, uint8_t, std::string, std::string> content = std::make_tuple(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
	KJ_IF_MAYBE(e, kj::runCatchingExceptions(
		kj::mvCapture(content, [this](std::tuple<uint64_t, uint8_t, std::string, std::string>&& content) {
		_thr_packet->Post(std::get<0>(content), std::get<1>(content), std::get<2>(content), std::get<3>(content));
	}))) {
		taskFailed(kj::mv(*e));
	}
}

//------------------------------------------------------------------------------
/**

*/
size_t
CKjGateClientConn::SendPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

	size_t szHeaderLen = SIZE_OF_TCP_OUTER_PACKET_LEADING;
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
CKjGateClientConn::SendPBMessage(google::protobuf::MessageLite *pMessage, uint64_t uInnerUuid, uint8_t uSerialNo) {

	std::string sTypeName = pMessage->GetTypeName();
	std::string sBody = pMessage->SerializeAsString();
	return SendPacket(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateClientConn::Disconnect() {
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
CKjGateClientConn::ConfirmClientIsReady(void *base, uintptr_t streamptr) {

	CKjGateServer *pServer = static_cast<CKjGateServer *>(_refServer);
	_thr_tcpStream = pServer->RemoveStream(streamptr);
	if (_thr_tcpStream) {
		// init base
		_thr_tioContext = kj::addRef(*(KjSimpleThreadIoContext *)base);
		_thr_tasks = _thr_tioContext->CreateTaskSet(*this);

		// start read
		auto p1 = _thr_tcpStream->StartReadOp(
			[this](KjTcpIoStream&, bip_buf_t& bb) {
			//
			OnRawData(bb);
		});
		// gate client start read op
		_thr_tasks->add(kj::mv(p1));

		// init packet
		if (nullptr == _thr_packet) {
			_thr_packet = new CTcpOuterPacket(this);
			_thr_packet->SetGotPacketCb(std::bind(&CKjGateClientConn::OnGotPacket, this, std::placeholders::_1, std::placeholders::_2));
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateClientConn::taskFailed(kj::Exception&& exception) {
	// eval later to avoid destroying self task set
	CKjGateServer *pServer = static_cast<CKjGateServer *>(_refServer);
	auto p1 = pServer->_thr_tioContext->EvalForResult(
		[this]() {

		// flush stream
		if (_thr_tcpStream
			&& !IsFlushed()) {

			_thr_tcpStream->Disconnect();

			SetFlushed(true);
		}
		_refServer->OnDisposeClient(this);
	});
	// gate server dispose client
	pServer->_thr_tasks->add(kj::mv(p1));
}

/** -- EOF -- **/