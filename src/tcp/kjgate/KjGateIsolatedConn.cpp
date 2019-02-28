//------------------------------------------------------------------------------
//  KjGateIsolatedConn.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjGateIsolatedConn.hpp"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../base/MyMacros.h"

#include "../tcp_packet_def.h"
#include "../TcpOuterPacket.h"

#include "../TcpConnManager.h"
#include "../TcpEventManager.h"

#include "KjGateConnFactory.hpp"

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
CKjGateIsolatedConn::CKjGateIsolatedConn(uint64_t uConnId, ITcpConnFactory *pFactory)
	: _connId(uConnId)
	, _refConnFactory(pFactory)
	, _eventManager(new CTcpEventManager()) {

}

//------------------------------------------------------------------------------
/**

*/
CKjGateIsolatedConn::~CKjGateIsolatedConn() noexcept {
	// must clear because handler may be released already
	_eventManager->ClearAllEventHandlers();

#ifdef _VERBOSE_TRACE_
	StdLog *pLog = netsystem_get_log();
	if (pLog)
		pLog->logprint(LOG_LEVEL_NOTICE, "\n[~CKjGateIsolatedConn()]: connection is destroyed, connid(%08llu).\n"
			, _connId);
#endif

	SAFE_DELETE(_eventManager);
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateIsolatedConn::OnConnect() {
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
CKjGateIsolatedConn::OnDisconnect() {

	if (IsConnected()) {
		//
		SetConnected(false);

		// notify disconnected event
		_eventManager->OnEvent(CONNECTION_DISCONNECTED, this);

		// notice
		StdLog *pLog = netsystem_get_log();
		if (pLog)
			pLog->logprint(LOG_LEVEL_NOTICE, "\n\t!!!! [CKjGateIsolatedConn::OnDisconnect()]: connid(%08llu) -- target_IP=(%s) target_port=(%d) !!!!\n",
				_connId, _sIp.c_str(), _nPort);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateIsolatedConn::OnGotPacket(const uint8_t *buf_in, size_t len) {

	size_t szHeaderLen = SIZE_OF_TCP_OUTER_PACKET_LEADING;
	tcp_outer_packet_leading_t *pHeader = (tcp_outer_packet_leading_t *)buf_in;

	size_t szTypeNameLen = (size_t)pHeader->name_len;
	std::string sTypeName((char *)(buf_in + szHeaderLen), szTypeNameLen);

	size_t szBodyLen = len - szTypeNameLen - szHeaderLen;
	std::string sBody((char *)(buf_in + szHeaderLen + pHeader->name_len), szBodyLen);

	//
	_refConnFactory->AddIsolatedPacketCb(this, GetConnId(), pHeader->serial_no, std::move(sTypeName), std::move(sBody));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateIsolatedConn::OnLowLevelConnect(KjTcpConnection&, uint64_t uConnId) {
	// connect cb to front end
	if (!IsConnected()) {
		_refConnFactory->AddIsolatedConnectCb(this, &_refConnFactory->GetConnManager());

		// used to avoid too many "OnDisconnect()" event -- "OnDisconnect()" event will only after on "OnConnect()" event
		SetConnectedEventPosted(true);
	}

	// start read always
	auto p1 = _thr_tcpStream->StartReadOp(
		[this](KjTcpConnection&, bip_buf_t& bb) {
		//
		OnRawData(bb);
	});
	// gate isolated conn start read op
	_thr_tasks->add(kj::mv(p1));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateIsolatedConn::OnLowLevelDisconnect(KjTcpConnection&, uint64_t uConnId) {
	// disconnect cb to front end
	// Note: No need to tell main thread if already in [ false == IsConnected() ] state, because isolated never REMOVE by error
	if (IsConnected()) {
		_refConnFactory->AddIsolatedDisconnectCb(this, &_refConnFactory->GetConnManager());

		// can post "OnDisconnect() event again
		SetConnectedEventPosted(false);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateIsolatedConn::DisposeConnection() {
	
	if (!IsDisposed()) {
		SetDisposed(true);
		
		// dispose packet
		SAFE_DELETE(_thr_packet);

		// flush stream
		FlushStream();

		// dispose stream for thread
		_thr_tcpStream = nullptr;

		// dispose base for thread
		_thr_endpointContext = nullptr;
		_thr_tasks = nullptr;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateIsolatedConn::FlushStream() {
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
CKjGateIsolatedConn::PostPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {
	if (_thr_packet)
		_thr_packet->Post(std::make_tuple(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody)));
}

//------------------------------------------------------------------------------
/**

*/
size_t
CKjGateIsolatedConn::SendPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

	size_t szHeaderLen = SIZE_OF_TCP_OUTER_PACKET_LEADING;
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
CKjGateIsolatedConn::SendPBMessage(google::protobuf::MessageLite *pMessage, uint64_t uInnerUuid, uint8_t uSerialNo) {
	
	std::string sTypeName = pMessage->GetTypeName();
	std::string sBody = pMessage->SerializeAsString();
	return SendPacket(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateIsolatedConn::Disconnect() {
	// disconnect means stop running and flush stream
	// , but not disposed
	if (!_bDisposed
		&& _bRunning) {
		_bRunning = false;

		OnDisconnect();

		// release
		_refConnFactory->ReleaseConnection(this);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateIsolatedConn::OnIsolatedError() {
	// clear packet because isolated may be reused
	_thr_packet->Clear();

	// event -- disconnect
	if (_thr_tcpStream) {
		OnLowLevelDisconnect(*_thr_tcpStream.get(), _connId);
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
CKjGateIsolatedConn::Connect(void *base, std::string& sIp_or_Hostname, unsigned short nPort) {

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
		_thr_endpointContext = kj::addRef(*(KjPipeEndpointIoContext *)base);
		_thr_tasks = netsystem_get_servercore()->NewTaskSet(*this);

		// create tcp connection
		_thr_tcpStream = kj::heap<KjTcpConnection>(kj::addRef(*_thr_endpointContext), _connId);

		//
		auto p1 = _thr_tcpStream->Connect(
			_sIp,
			_nPort,
			std::bind(&CKjGateIsolatedConn::OnLowLevelConnect, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&CKjGateIsolatedConn::OnLowLevelDisconnect, this, std::placeholders::_1, std::placeholders::_2)
		);
		// gate isolated connect
		_thr_tasks->add(kj::mv(p1));

		// init packet
		if (nullptr == _thr_packet) {
			_thr_packet = new CTcpOuterPacket(this);
			_thr_packet->SetGotPacketCb(std::bind(&CKjGateIsolatedConn::OnGotPacket, this, std::placeholders::_1, std::placeholders::_2));
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
CKjGateIsolatedConn::Reconnect() {

	assert(!IsDisposed());

	// recover running
	if (!_bRunning)
		_bRunning = true;

	// check flushed
	if (IsFlushed()) {
		SetFlushed(false);

		//
		if (!_thr_tcpStream) {
			// create tcp connection
			_thr_tcpStream = kj::heap<KjTcpConnection>(kj::addRef(*_thr_endpointContext), _connId);
		}

		// kjgate isolated conn reconnect
		_thr_tasks->add(_thr_tcpStream->Reconnect());
	}
	else {
		//
		fprintf(stderr, "\n\t!!!! [CKjGateIsolatedConn::Reconnect()]: still in flushing, reconnect abort!!! connected(%d)disposed(%d)flushed(%d), connid(%08llu) -- target_IP=(%s) target_port=(%d)!\n",
			_bConnected, _bDisposed, _bFlushed, _connId, _sIp.c_str(), _nPort);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateIsolatedConn::DelayReconnect(int nDelaySeconds) {

	if (!_bDelayReconnecting
		&& !IsDisposed()) {
		_bDelayReconnecting = true;

		fprintf(stderr, "[CKjGateIsolatedConn::DelayReconnect()] reconnect after (%d) seconds, connected(%d)disposed(%d)flushed(%d), connid(%08llu) -- target_IP=(%s) target_port=(%d)!\n",
			nDelaySeconds, _bConnected, _bDisposed, _bFlushed, _connId, _sIp.c_str(), _nPort);

		// reconnect delay
		auto p1 = _thr_endpointContext->AfterDelay(nDelaySeconds * kj::SECONDS)
			.then([this]() {

			if (_bDelayReconnecting
				&& !IsDisposed()) {

				_bDelayReconnecting = false;

				// reconnect
				Reconnect();
			}
		});
		// gate isolated delay reconnect
		_thr_tasks->add(kj::mv(p1));
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CKjGateIsolatedConn::Reopen(int nDelaySeconds) {

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
CKjGateIsolatedConn::taskFailed(kj::Exception&& exception) {
	char chDesc[1024];
#if defined(__CYGWIN__) || defined( _WIN32)
	_snprintf_s(chDesc, sizeof(chDesc), "\n[CKjGateIsolatedConn::taskFailed()] desc(%s)!!! connected(%d)disposed(%d)flushed(%d), connid(%08llu) -- target_IP=(%s) target_port=(%d).\n",
		exception.getDescription().cStr(), _bConnected, _bDisposed, _bFlushed, _connId, _sIp.c_str(), _nPort);
#else
	snprintf(chDesc, sizeof(chDesc), "\n[CKjGateIsolatedConn::taskFailed()] desc(%s)!!! connected(%d)disposed(%d)flushed(%d), connid(%08llu) -- target_IP=(%s) target_port=(%d).\n",
		exception.getDescription().cStr(), _bConnected, _bDisposed, _bFlushed, _connId, _sIp.c_str(), _nPort);
#endif
	fprintf(stderr, "%s", chDesc);

	// schedule eval later to avoid destroying self task set
	netsystem_get_servercore()->ScheduleEvalLaterFunc([this]() {

		// flush stream
		FlushStream();

		// error
		OnIsolatedError();
	});
}

/** -- EOF -- **/