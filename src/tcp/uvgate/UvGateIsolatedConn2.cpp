//------------------------------------------------------------------------------
//  UvGateIsolatedConn2.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "UvGateIsolatedConn2.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../base/MyMacros.h"
#include "../../netsystem/RootContextDef.hpp"

#include "../tcp_packet_def.h"
#include "../TcpInnerPacket.h"

#include "../TcpConnManager.h"
#include "../TcpEventManager.h"

#include "UvGateConnFactory.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

#define RECONNECT_DELAY_SECONDS 10

//------------------------------------------------------------------------------
/**

*/
static void
on_sock_close(uv_handle_t* handle) {
	free(handle);
}

//------------------------------------------------------------------------------
/**

*/
static void
on_stream_close(uv_handle_t* handle) {
	client_handle_t *clnt = (client_handle_t *)handle->data;
	ITcpIsolated *pConn = (ITcpIsolated *)clnt->_connimpl;
	if (pConn) {
		// dispose and error
		pConn->SetFlushed(true);
		pConn->OnIsolatedError();
	}
	free(handle);
}

/**------------------------------------------------------------------------------
*
*/
static void
alloc_cb(uv_handle_t *stream1, size_t suggested_size, uv_buf_t *buf) {
	client_handle_t *clnt = (client_handle_t *)(stream1->data);
	size_t szreserved = suggested_size;
	buf->base = bip_buf_reserve(clnt->_bb, &szreserved);
	buf->len = szreserved;
}

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
static void
on_isolated_recv(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
	client_handle_t *clnt = (client_handle_t *)(stream->data);
	ITcpIsolated *pConn = (ITcpIsolated *)clnt->_connimpl;

	if (nread < 0) {
		/* Error or EOF */
		if (nread == UV_EOF
			|| nread == UV_ECONNRESET) {
			/* Free the read/write buffer */
			bip_buf_commit(clnt->_bb, 0);

			if (pConn) {
				// flush and error
				pConn->FlushStream();
				pConn->OnIsolatedError();
			}
			else {
				uv_close_((uv_handle_t *)clnt->_sockimpl, on_sock_close);
				bip_buf_destroy(clnt->_bb);
				free(clnt);
			}
		}
		else {
			/* Keep waiting */
		}
		return;
	}

	if (nread == 0) {
		/* Everything OK, but nothing read. */
		bip_buf_commit(clnt->_bb, 0);
		return;
	}

	// read ok
	bip_buf_commit(clnt->_bb, nread);

	// connection read data
	if (pConn && nread > 0) {
		pConn->OnRawData(*clnt->_bb);
	}
}

//------------------------------------------------------------------------------
/**

*/
static void
on_isolated_send(uv_write_t* req, int status) {
	write_req_t* wr;

	assert(req != NULL);

	/* Free the read/write buffer and the request */
	wr = (write_req_t*)req;

	//
	if (status) {
		if (ERROR_IO_PENDING != GetLastError()) {
			uv_tcp_t *tcp = (uv_tcp_t*)(req->handle);
			client_handle_t *clnt = (client_handle_t *)(tcp->data);
			ITcpIsolated *pConn = (ITcpIsolated *)clnt->_connimpl;
			if (pConn) {
				// flush and error
				pConn->FlushStream();
				pConn->OnIsolatedError();
			}
			else {
				uv_close_((uv_handle_t *)clnt->_sockimpl, on_sock_close);
				bip_buf_destroy(clnt->_bb);
				free(clnt);
			}

			fprintf(stderr, "write_cb error: %s - %s\n",
				uv_err_name(status), uv_strerror(status));

			fprintf(stderr, "(%Id)write_cb error XXXX: write_queue_size=(%zd).\n"
				, tcp->socket, tcp->write_queue_size);

			/* Free the read/write buffer and the request */
			free(wr->buf.base);
			free(wr);
		}
		return;
	}

	/* Free the read/write buffer and the request */
	free(wr->buf.base);
	free(wr);
}

//------------------------------------------------------------------------------
/**

*/
static void
on_isolated_connect_error(uv_connect_t *req, int status) {
	client_handle_t *clnt = (client_handle_t *)req->data;
	uv_tcp_t *sockimpl = (uv_tcp_t *)clnt->_sockimpl;
	ITcpIsolated *pConn = (ITcpIsolated *)clnt->_connimpl;
	int r;

	if (0 == status) {

		{
			clnt->_fd = sockimpl->socket;

			r = uv_tcp_nodelay(sockimpl, 1);
			if (r != 0) {
				fprintf(stderr, "[fd(%d)][on_isolated_connect_error()] uv_tcp_nodelay() failed, err: %s - %s\n",
					clnt->_fd, uv_err_name(r), uv_strerror(r));

				uv_close_((uv_handle_t *)clnt->_sockimpl, on_sock_close);
				bip_buf_destroy(clnt->_bb);
				free(clnt);
				return;
			}

			r = uv_read_start((uv_stream_t *)sockimpl, alloc_cb, on_isolated_recv);
			if (r != 0) {
				fprintf(stderr, "[fd(%d)][on_isolated_connect_error()] read start error: %s - %s\n",
					clnt->_fd, uv_err_name(status), uv_strerror(status));

				uv_close_((uv_handle_t *)clnt->_sockimpl, on_sock_close);
				bip_buf_destroy(clnt->_bb);
				free(clnt);
				return;
			}

			/* connected */
			pConn->OnConnect();
		}
	}
	else {
		if (UV_EOF == status) {
			fprintf(stderr, "Connection closed.\n");
		}
		else {
			fprintf(stderr, "Got an error on the connection: %s - %s\n",
				uv_err_name(status), uv_strerror(status));
		}

		if (pConn) {
			// flush and error
			pConn->FlushStream();
			pConn->OnIsolatedError();
		}
		else {
			uv_close_((uv_handle_t *)clnt->_sockimpl, on_sock_close);
			bip_buf_destroy(clnt->_bb);
			free(clnt);
		}
	}

	//
	free(req);
}

//------------------------------------------------------------------------------
/**

*/
static void
on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res) {
	if (status == -1) {
		fprintf(stderr, "getaddrinfo callback error: %s - %s\n",
			uv_err_name(status), uv_strerror(status));
		return;
	}

	char addr[17] = { '\0' };
	uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);
	fprintf(stderr, "%s\n", addr);

	uv_connect_(resolver->loop, addr, 0, on_isolated_connect_error);
	uv_freeaddrinfo(res);
}

//------------------------------------------------------------------------------
/**

*/
static void
client_reconnect_timer_cb(uv_timer_t* timer) {
	CUvGateIsolatedConn2 *pConn = static_cast<CUvGateIsolatedConn2 *>(timer->data);
	if (pConn) {
		pConn->Reconnect();
	}
}

//------------------------------------------------------------------------------
/**

*/
CUvGateIsolatedConn2::CUvGateIsolatedConn2(uint64_t uConnId, ITcpConnFactory *pFactory)
	: _connId(uConnId)
	, _refConnFactory(pFactory)
	, _eventManager(new CTcpEventManager()) {

}

//------------------------------------------------------------------------------
/**

*/
CUvGateIsolatedConn2::~CUvGateIsolatedConn2() {
	// must clear because handler may be released already
	_eventManager->ClearAllEventHandlers();

#ifdef _VERBOSE_TRACE_
	StdLog *pLog = netsystem_get_log();
	if (pLog)
		pLog->logprint(LOG_LEVEL_NOTICE, "\n[~CUvGateIsolatedConn2()]: connection is destroyed, connid(%08llu).\n"
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
CUvGateIsolatedConn2::OnConnect() {
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
CUvGateIsolatedConn2::OnDisconnect() {

	if (IsConnected()) {
		//
		SetConnected(false);

		// notify disconnected event
		_eventManager->OnEvent(CONNECTION_DISCONNECTED, this);

		// notice
		StdLog *pLog = netsystem_get_log();
		if (pLog)
			pLog->logprint(LOG_LEVEL_NOTICE, "\n\t!!!! [CUvGateIsolatedConn2::OnDisconnect()]: connid(%08llu) -- target_IP=(%s) target_port=(%d) !!!!\n",
				_connId, _sIp.c_str(), _nPort);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CUvGateIsolatedConn2::OnGotPacket(const uint8_t *buf_in, size_t len) {

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
CUvGateIsolatedConn2::DisposeConnection() {
	
	if (!IsDisposed()) {
		SetDisposed(true);

		// dispose packet
		SAFE_DELETE(_packet);

		// flush stream
		FlushStream();

		// kill reconnect timer
		if (_reconnectTimer) {
			uv_stop_and_free_timer_(_reconnectTimer);
			_reconnectTimer = nullptr;
		}
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CUvGateIsolatedConn2::FlushStream() {
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
CUvGateIsolatedConn2::PostPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {
	if (_packet)
		_packet->Post(std::make_tuple(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody)));
}

//------------------------------------------------------------------------------
/**

*/
size_t
CUvGateIsolatedConn2::SendRaw(const uint8_t *buf, size_t len) {
	// null handle means "the connection is not initialized or already closed".
	assert(_clnt);

	// write
	write_req_t *wr = (write_req_t *)malloc(sizeof(write_req_t));
	wr->buf = uv_buf_init((char *)malloc(len), (unsigned int)len);
	memcpy(wr->buf.base, buf, len);

	int r = uv_write((uv_write_t *)wr, (uv_stream_t *)_clnt->_sockimpl, &wr->buf, 1, on_isolated_send);
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
CUvGateIsolatedConn2::SendPacket(uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {

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
CUvGateIsolatedConn2::SendPBMessage(google::protobuf::MessageLite *pMessage, uint64_t uInnerUuid, uint8_t uSerialNo) {

	std::string sTypeName = pMessage->GetTypeName();
	std::string sBody = pMessage->SerializeAsString();
	return SendPacket(uInnerUuid, uSerialNo, std::move(sTypeName), std::move(sBody));
}

//------------------------------------------------------------------------------
/**

*/
void
CUvGateIsolatedConn2::Disconnect() {
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
CUvGateIsolatedConn2::OnIsolatedError() {
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
CUvGateIsolatedConn2::Connect(void *base, std::string& sIp_or_Hostname, unsigned short nPort) {

	assert(!IsConnected() && !IsDisposed() && IsFlushed() && !_bRunning);

	// is IP or hostname ?
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
		_base = (uv_loop_t *)base;

		// init packet
		if (nullptr == _packet) {
			_packet = new CTcpInnerPacket(this);
			_packet->SetGotPacketCb(std::bind(&CUvGateIsolatedConn2::OnGotPacket, this, std::placeholders::_1, std::placeholders::_2));
		}

		// init reconnect timer
		_reconnectTimer = uv_create_timer_(_base, this);

		// low level connect
		_clnt = uv_connect_(_base, _sIp.c_str(), _nPort, on_isolated_connect_error);
		if (_clnt) {
			_clnt->_connimpl = this;
		}
		else {
			return -2;
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
CUvGateIsolatedConn2::Reconnect() {
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
		uv_real_connect_(_clnt, _base, on_isolated_connect_error);
	}
	else {
		//
		fprintf(stderr, "\n\t!!!! [CUvGateIsolatedConn2::Reconnect()]: still in flushing, reconnect abort!!! connected(%d)disposed(%d)flushed(%d), connid(%08llu) -- target_IP=(%s) target_port=(%d)!\n",
			_bConnected, _bDisposed, _bFlushed, _connId, _sIp.c_str(), _nPort);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CUvGateIsolatedConn2::DelayReconnect(int nDelaySeconds) {

	if (!_bDelayReconnecting
		&& !IsDisposed()) {
		_bDelayReconnecting = true;

		fprintf(stderr, "[CUvGateIsolatedConn2::DelayReconnect()] reconnect after (%d) seconds, connected(%d)disposed(%d)flushed(%d), connid(%08llu) -- target_IP=(%s) target_port=(%d)!\n",
			nDelaySeconds, _bConnected, _bDisposed, _bFlushed, _connId, _sIp.c_str(), _nPort);

		// reconnect timer
		uint64_t uDelaySecondsInMs = nDelaySeconds * 1000;
		uv_start_timer_(_reconnectTimer, client_reconnect_timer_cb, uDelaySecondsInMs, 0);
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CUvGateIsolatedConn2::Reopen(int nDelaySeconds) {

	if (!IsDisposed()) {

		if (IsConnected()) {
			Disconnect();
		}

		//
		_refConnFactory->IsolatedConnDelayReconnect(this, nDelaySeconds);
	}
}

/** -- EOF -- **/