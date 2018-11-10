//------------------------------------------------------------------------------
//  KjIsolatedConn.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjIsolatedConn.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../tcp_packet_def.h"
#include "../TcpInnerPacket.h"

#include "../TcpEventManager.h"

#include "KjConnFactory.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

#define RECONNECT_DELAY_SECONDS 10

/**------------------------------------------------------------------------------
*
*/
static bool
is_ip_address(const char *ip) {
	unsigned int i;
	for (i = 0;i < strlen(ip);i++) {
		if (isdigit(ip[i]) == 0 && ip[i] != '.')
			return false;
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
CKjIsolatedConn::CKjIsolatedConn(uint64_t uConnId, ITcpConnFactory *pFactory)
	: _connId(uConnId)
	, _refConnFactory(pFactory)
	, _eventManager(new CTcpEventManager()) {

}

//------------------------------------------------------------------------------
/**

*/
CKjIsolatedConn::~CKjIsolatedConn() {
	// must clear because handler may be released already
	_eventManager->ClearAllEventHandlers();

#ifdef _VERBOSE_TRACE_
	StdLog *pLog = _refConnFactory->GetLogHandler();
	if (pLog)
		pLog->logprint(LOG_LEVEL_NOTICE, "\n[~CKjIsolatedConn()]: connection is destroyed, connid(%08llu).\n"
			, _connId);
#endif

	SAFE_DELETE(_eventManager);
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::OnConnect() {
	// connect event will be ignored if not running
	if (_bRunning
		&& !IsConnected()) {
		//
		SetConnected(true);

		// notify connected event
		_eventManager->OnEvent(CONNECTION_CONNECTED, this);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::OnDisconnect() {

	if (IsConnected()) {
		//
		SetConnected(false);

		// notify disconnected event
		_eventManager->OnEvent(CONNECTION_DISCONNECTED, this);

		// notice
		StdLog *pLog = _refConnFactory->GetLogHandler();
		if (pLog)
			pLog->logprint(LOG_LEVEL_NOTICE, "\n\t!!!! [CKjIsolatedConn::OnDisconnect()]: connid(%08llu) -- target_IP=(%s) target_port=(%d) !!!!\n",
				_connId, _sIp.c_str(), _nPort);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::OnGotPacket(const uint8_t *buf_in, size_t len) {

	size_t szHeaderLen = SIZE_OF_TCP_INNER_PACKET_LEADING;
	tcp_inner_packet_leading_t *pHeader = (tcp_inner_packet_leading_t *)buf_in;

	size_t szTypeNameLen = (size_t)pHeader->inner_name_len;
	std::string sTypeName((char *)(buf_in + szHeaderLen), szTypeNameLen);

	size_t szBodyLen = len - szTypeNameLen - szHeaderLen;
	std::string sBody((char *)(buf_in + szHeaderLen + pHeader->inner_name_len), szBodyLen);

	//
	_refConnFactory->AddIsolatedInnerPacketCb(this, GetConnId(), pHeader->inner_uuid, pHeader->inner_serial_no, std::move(sTypeName), std::move(sBody));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::OnLowLevelConnect(KjTcpConnection&, uint64_t uConnId) {
	// connect cb to front end
	if (!IsConnected())
		_refConnFactory->AddIsolatedConnectCb(this, &_refConnFactory->GetConnManager());

	// start read always
	auto p1 = _thr_tcpConnection->StartReadOp(
		[this](KjTcpConnection&, bip_buf_t& bb) {
		//
		OnRawData(bb);
	});
	// isolated start read op
	_thr_tasks->add(kj::mv(p1));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::OnLowLevelDisconnect(KjTcpConnection&, uint64_t uConnId) {
	// disconnect cb to front end
	// Note: No need to tell main thread if already in [ false == IsConnected() ] state, because isolated never REMOVE by error
	if (IsConnected())
		_refConnFactory->AddIsolatedDisconnectCb(this, &_refConnFactory->GetConnManager());
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::DisposeConnection() {

	if (!IsDisposed()) {
		SetDisposed(true);
		
		// dispose packet
		SAFE_DELETE(_thr_packet);

		// flush stream
		FlushConnection();

		// dispose stream
		_thr_tcpConnection = nullptr;

		// dispose base
		_thr_tioContext = nullptr;
		_thr_tasks = nullptr;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::PostPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

	std::tuple<uint64_t, uint8_t, std::string, std::string> content = std::make_tuple(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));

	assert(IsReady());

	// stream write maybe trigger exception at once, so we must catch it manually
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
CKjIsolatedConn::SendPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {
	
	size_t szHeaderLen = SIZE_OF_TCP_INNER_PACKET_LEADING;
	size_t szTypeNameLen = sTypeName.length();
	size_t szBodyLen = sBody.length();

	//
	_refConnFactory->PostPacket(this, uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
	return szHeaderLen + szTypeNameLen + szBodyLen;
}

//------------------------------------------------------------------------------
/**

*/
size_t
CKjIsolatedConn::SendPBMessage(google::protobuf::MessageLite *pMessage, uint64_t uInnerUuid, uint8_t uSerialNo) {
	
	std::string sTypeName = pMessage->GetTypeName();
	std::string sBody = pMessage->SerializeAsString();
	return SendPacket(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::Disconnect() {
	// disconnect means stop running and flush stream
	// , but not disposed
	if (!_bDisposed
		&& _bRunning) {
		_bRunning = false;

		OnDisconnect();
		
		// flush stream
		_refConnFactory->IsolatedConnFlush(this);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::OnIsolatedError() {
	// clear packet because isolated may be reused
	_thr_packet->Clear();

	// event -- disconnect
	if (_thr_tcpConnection) {
		OnLowLevelDisconnect(*_thr_tcpConnection.get(), _connId);
	}

	// check running
	if (_bRunning) {
		DelayReconnect(RECONNECT_DELAY_SECONDS);
	}
}

//------------------------------------------------------------------------------
/**

*/
int
CKjIsolatedConn::Connect(void *base, std::string& sIp_or_Hostname, unsigned short nPort) {

	assert(!IsConnected() && !IsDisposed() && IsFlushed() && !_bRunning);

	// is ip or hostname ?
	if (is_ip_address(sIp_or_Hostname.c_str())) {
		// init ip and port
		_sIp = sIp_or_Hostname;
		_nPort = nPort;
	}
	else {
		return -1;
	}

	{
		// init base
		_thr_tioContext = kj::addRef(*(KjSimpleThreadIoContext *)base);
		_thr_tasks = _thr_tioContext->CreateTaskSet(*this);

		// create tcp connection
		_thr_tcpConnection = kj::heap<KjTcpConnection>(kj::addRef(*_thr_tioContext), _connId);

		//
		auto p1 = _thr_tcpConnection->Connect(
			_sIp,
			_nPort,
			std::bind(&CKjIsolatedConn::OnLowLevelConnect, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&CKjIsolatedConn::OnLowLevelDisconnect, this, std::placeholders::_1, std::placeholders::_2)
		);
		// isolated connect
		_thr_tasks->add(kj::mv(p1));

		// init packet
		if (nullptr == _thr_packet) {
			_thr_packet = new CTcpInnerPacket(this);
			_thr_packet->SetGotPacketCb(std::bind(&CKjIsolatedConn::OnGotPacket, this, std::placeholders::_1, std::placeholders::_2));
		}
	}
	
	//
	_bRunning = true;
	SetFlushed(false);
	return 0;
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::Reconnect() {
	
	assert(!IsDisposed());

	// recover running
	if (!_bRunning)
		_bRunning = true;

	// check flushed
	if (IsFlushed()) {
		SetFlushed(false);

		//
		if (!_thr_tcpConnection) {
			// create tcp connection
			_thr_tcpConnection = kj::heap<KjTcpConnection>(kj::addRef(*_thr_tioContext), _connId);
		}

		// kjisolated conn reconnect
		_thr_tasks->add(_thr_tcpConnection->Reconnect());
	}
	else {
		//
		fprintf(stderr, "\n\t!!!! [CKjIsolatedConn::Reconnect()]: still in flushing, reconnect abort!!! connected(%d)disposed(%d)flushed(%d), connid(%08llu) -- target_IP=(%s) target_port=(%d)!\n",
			_bConnected, _bDisposed, _bFlushed, _connId, _sIp.c_str(), _nPort);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::FlushConnection() {
	// flush tcp stream only
	if (_thr_tcpConnection
		&& !IsFlushed()) {

		_thr_tcpConnection->FlushStream();
		
		SetFlushed(true);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::DelayReconnect(int nDelaySeconds) {

	if (!_bDelayReconnecting
		&& !IsDisposed()) {
		_bDelayReconnecting = true;

		fprintf(stderr, "[CKjIsolatedConn::DelayReconnect()] reconnect after (%d) seconds, connected(%d)disposed(%d)flushed(%d), connid(%08llu) -- target_IP=(%s) target_port=(%d)!\n",
			nDelaySeconds, _bConnected, _bDisposed, _bFlushed, _connId, _sIp.c_str(), _nPort);

		// reconnect delay
		auto p1 = _thr_tioContext->AfterDelay(nDelaySeconds * kj::SECONDS)
			.then([this]() {

			if (_bDelayReconnecting
				&& !IsDisposed()) {

				_bDelayReconnecting = false;

				// reconnect
				Reconnect();
			}
		});
		// isolated delay reconnect
		_thr_tasks->add(kj::mv(p1));
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::Reopen(int nDelaySeconds) {

	if (!IsDisposed()) {

		if (IsConnected()) {
			Disconnect();
		}

		//
		_refConnFactory->IsolatedConnDelayReconnect(this, nDelaySeconds);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjIsolatedConn::taskFailed(kj::Exception&& exception) {
	char chDesc[1024];
#if defined(__CYGWIN__) || defined( _WIN32)
	_snprintf_s(chDesc, sizeof(chDesc), "\n[CKjIsolatedConn::taskFailed()] desc(%s)!!! connected(%d)disposed(%d)flushed(%d), connid(%08llu) -- target_IP=(%s) target_port=(%d).\n",
		exception.getDescription().cStr(), _bConnected, _bDisposed, _bFlushed, _connId, _sIp.c_str(), _nPort);
#else
	snprintf(chDesc, sizeof(chDesc), "\n[CKjIsolatedConn::taskFailed()] desc(%s)!!! connected(%d)disposed(%d)flushed(%d), connid(%08llu) -- target_IP=(%s) target_port=(%d).\n",
		exception.getDescription().cStr(), _bConnected, _bDisposed, _bFlushed, _connId, _sIp.c_str(), _nPort);
#endif
	fprintf(stderr, "%s", chDesc);

	// eval later to avoid destroying self task set
	auto p1 = _thr_tioContext->EvalForResult(
		[this]() {

		// force flush stream
		FlushConnection();

		// error
		OnIsolatedError();
	});
	// isolated conn dispose
	_thr_tioContext->AddTask(kj::mv(p1));
}

/** -- EOF -- **/