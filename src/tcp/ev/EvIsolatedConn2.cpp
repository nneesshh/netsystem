//------------------------------------------------------------------------------
//  EvIsolatedConn2.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "EvIsolatedConn2.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../base/MyMacros.h"
#include "../../netsystem/RootContextDef.hpp"

#include "../tcp_packet_def.h"
#include "../TcpOuterPacket.h"

#include "../TcpConnManager.h"
#include "../TcpEventManager.h"

#include "EvConnFactory.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

#define RECONNECT_DELAY_SECONDS 10

//------------------------------------------------------------------------------
/**

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
static void
on_isolated_recv(struct bufferevent *bev, void *arg) {
	struct event_base *base = bufferevent_get_base(bev);
	struct evbuffer *input = bufferevent_get_input(bev);
	struct evbuffer_iovec iovec[1];
	size_t nbytes;

	client_handle_t *clnt = (client_handle_t *)arg;
	ITcpIsolated *pConn = (ITcpIsolated *)clnt->_connimpl;

	//
	while ((nbytes = evbuffer_get_contiguous_space(input)) > 0) {
		//size_t szchains = evbuffer_peek(input, -1, nullptr, iovec, 1);
		size_t szoffersize = 4096;
		char *bufoffer = bip_buf_reserve(clnt->_bb, &szoffersize);
		if (szoffersize >= iovec[0].iov_len) {
			memcpy(bufoffer, iovec[0].iov_base, iovec[0].iov_len);
			bip_buf_commit(clnt->_bb, szoffersize);
			pConn->OnRawData(*clnt->_bb);
		}
		evbuffer_drain(input, iovec[0].iov_len);
	}
}

//------------------------------------------------------------------------------
/**

*/
static void
on_isolated_send(struct bufferevent *bev, void *arg) {

}

//------------------------------------------------------------------------------
/**

*/
static void
on_isolated_error(struct bufferevent *bev, short what, void *arg) {
	struct event_base *base = bufferevent_get_base(bev);
	client_handle_t *clnt = (client_handle_t *)arg;
	ITcpIsolated *pConn = (ITcpIsolated *)clnt->_connimpl;

	char yes = 1;

	//
	if (what & BEV_EVENT_CONNECTED) {

		{
			clnt->_fd = bufferevent_getfd(bev);

			if (0 == setsockopt(clnt->_fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes))) {
				pConn->OnConnect();
			}
			else {
				fprintf(stderr, "[fd(%d)]setsockopt(TCP_NODELAY) failed, err=%s\n", clnt->_fd, strerror(errno));
				bufferevent_free(bev);
			}
		}
	}
	else {
		if (what & BEV_EVENT_EOF) {
			fprintf(stderr, "Connection closed.\n");
		}
		else if (what & BEV_EVENT_ERROR) {
			fprintf(stderr, "Got an error on the connection: %s\n",
				strerror(errno));/*XXX win32*/
		}

		//
		if (pConn) {
			// flush and error
			pConn->FlushStream();
			pConn->OnIsolatedError();
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
static void
client_reconnect_timer_cb(evutil_socket_t fd, short what, void *arg) {
	CEvIsolatedConn2 *pConn = static_cast<CEvIsolatedConn2 *>(arg);
	if (pConn) {
		pConn->Reconnect();
	}
}

//------------------------------------------------------------------------------
/**

*/
CEvIsolatedConn2::CEvIsolatedConn2(uint64_t uConnId, ITcpConnFactory *pFactory)
	: _connId(uConnId)
	, _refConnFactory(pFactory)
	, _eventManager(new CTcpEventManager()) {

}

//------------------------------------------------------------------------------
/**

*/
CEvIsolatedConn2::~CEvIsolatedConn2() {
	// must clear because handler may be released already
	_eventManager->ClearAllEventHandlers();

#ifdef _VERBOSE_TRACE_
	StdLog *pLog = netsystem_get_log();
	if (pLog)
		pLog->logprint(LOG_LEVEL_NOTICE, "\n[~CEvIsolatedConn2()]: connection is destroyed, connid(%08llu).\n"
			, _connId);
#endif

	SAFE_DELETE(_eventManager);

	// free conn handle
	if (_clnt) {
		bip_buf_destroy(_clnt->_bb);
		free(_clnt);
		_clnt = nullptr;
	}

}

//------------------------------------------------------------------------------
/**

*/
void
CEvIsolatedConn2::OnConnect() {
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
CEvIsolatedConn2::OnDisconnect() {

	if (IsConnected()) {
		//
		SetConnected(false);

		// notify disconnected event
		_eventManager->OnEvent(CONNECTION_DISCONNECTED, this);

		// notice
		StdLog *pLog = netsystem_get_log();
		if (pLog)
			pLog->logprint(LOG_LEVEL_NOTICE, "\n\t!!!! [CEvIsolatedConn2::OnDisconnect()]: connid(%08llu) -- target_IP=(%s) target_port=(%d) !!!!\n",
				_connId, _sIp.c_str(), _nPort);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CEvIsolatedConn2::OnGotPacket(const uint8_t *buf_in, size_t len) {

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
CEvIsolatedConn2::DisposeConnection() {

	if (!IsDisposed()) {
		SetDisposed(true);

		// dispose packet
		SAFE_DELETE(_packet);

		// flush stream
		FlushStream();

		// kill reconnect timer
		if (_reconnectTimer) {
			ev_stop_and_free_timer_(_reconnectTimer);
			_reconnectTimer = nullptr;
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CEvIsolatedConn2::FlushStream() {
	// 
	if (_clnt
		&& _clnt->_sockimpl
		&& !IsFlushed()) {

		ev_close_((struct bufferevent *)_clnt->_sockimpl);
		_clnt->_sockimpl = nullptr;

		SetFlushed(true);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CEvIsolatedConn2::PostPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {
	if (_packet)
		_packet->Post(std::make_tuple(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody)));
}

//------------------------------------------------------------------------------
/**

*/
size_t
CEvIsolatedConn2::SendRaw(const uint8_t *buf, size_t len) {
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
CEvIsolatedConn2::SendPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

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
CEvIsolatedConn2::SendPBMessage(google::protobuf::MessageLite *pMessage, uint64_t uInnerUuid, uint8_t uSerialNo) {

	std::string sTypeName = pMessage->GetTypeName();
	std::string sBody = pMessage->SerializeAsString();
	return SendPacket(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
}

//------------------------------------------------------------------------------
/**

*/
void
CEvIsolatedConn2::Disconnect() {
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
CEvIsolatedConn2::OnIsolatedError() {
	// clear packet because isolated may be reused
	_packet->Clear();

	// trigger disconnect event
	OnDisconnect();

	// check running
	if (_bRunning) {
		DelayReconnect(RECONNECT_DELAY_SECONDS);
	}
}

//------------------------------------------------------------------------------
/**

*/
int
CEvIsolatedConn2::Connect(void *base, std::string& sIp_or_Hostname, unsigned short nPort) {

	assert(!IsConnected() && !IsDisposed() && IsFlushed() && !_bRunning);

	// is IP or hostname ?
	if (is_ip_address(sIp_or_Hostname.c_str())) {
		// init ip and port
		_sIp = sIp_or_Hostname;
		_nPort = nPort;
	}
	else {
		evutil_addrinfo *answer = ev_getaddrinfoforhost_(sIp_or_Hostname.c_str(), nPort);
		if (answer) {
			char chIP[64];
			sockaddr_in *_sin_ptr = (sockaddr_in *)answer->ai_addr;
			evutil_inet_ntop(AF_INET, &(_sin_ptr->sin_addr), chIP, sizeof(chIP));

			// init ip and port
			_sIp = sIp_or_Hostname;
			_nPort = nPort;
		}
		else
			return -1;
	}

	{
		// init base
		_base = (struct event_base *)base;

		// init reconnect timer
		_reconnectTimer = ev_create_timer_(_base, client_reconnect_timer_cb, this, 1);

		// low level connect
		_clnt = ev_connect_(
			_base,
			_sIp.c_str(),
			_nPort,
			on_isolated_recv,
			on_isolated_send,
			on_isolated_error);
		if (_clnt) {
			_clnt->_connimpl = this;
		}
		else {
			return -2;
		}

		// init packet
		if (nullptr == _packet) {
			_packet = new CTcpOuterPacket(this);
			_packet->SetGotPacketCb(std::bind(&CEvIsolatedConn2::OnGotPacket, this, std::placeholders::_1, std::placeholders::_2));
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
CEvIsolatedConn2::Reconnect() {
	// clear delay reconnecting flag
	if (_bDelayReconnecting)
		_bDelayReconnecting = false;

	assert(!IsDisposed());

	// recover running
	if (!_bRunning)
		_bRunning = true;

	// check flushed
	if (IsFlushed()) {
		SetFlushed(false);

		// low level connect
		ev_real_connect_(
			_clnt,
			_base,
			on_isolated_recv,
			on_isolated_send,
			on_isolated_error);
	}
	else {
		//
		fprintf(stderr, "\n\t!!!! [CEvIsolatedConn2::Reconnect()]: still in flushing, reconnect abort!!! connected(%d)disposed(%d)flushed(%d), connid(%08llu) -- target_IP=(%s) target_port=(%d)!\n",
			_bConnected, _bDisposed, _bFlushed, _connId, _sIp.c_str(), _nPort);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CEvIsolatedConn2::DelayReconnect(int nDelaySeconds) {

	if (!_bDelayReconnecting
		&& !IsDisposed()) {
		_bDelayReconnecting = true;

		fprintf(stderr, "[CEvIsolatedConn2::DelayReconnect()] reconnect after (%d) seconds, connected(%d)disposed(%d)flushed(%d), connid(%08llu) -- target_IP=(%s) target_port=(%d)!\n",
			nDelaySeconds, _bConnected, _bDisposed, _bFlushed, _connId, _sIp.c_str(), _nPort);

		// reconnect timer
		struct timeval timeout = { nDelaySeconds, 0 };
		ev_start_timer_(_reconnectTimer, &timeout);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CEvIsolatedConn2::Reopen(int nDelaySeconds) {

	if (!IsDisposed()) {

		if (IsConnected()) {
			Disconnect();
		}

		//
		_refConnFactory->IsolatedConnDelayReconnect(this, nDelaySeconds);
	}
}

/** -- EOF -- **/