//------------------------------------------------------------------------------
//  EvServer.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "EvServer.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../TcpConnManager.h"
#include "../TcpEventManager.h"

#include "EvConnFactory.h"
#include "EvClientConn.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

//------------------------------------------------------------------------------
/**

*/
static int
on_server_accept_client(server_handle_t *srvr, client_handle_t *clnt, const char *peerip) {
	// accept client cb
	CEvServer *pServer = static_cast<CEvServer *>(srvr->_userdata);

	if (!pServer->IsReady()) {
		// flush stream at once
		ev_close_((struct bufferevent *)clnt->_sockimpl);
		clnt->_sockimpl = nullptr;
		return -1;
	}
	else {
		//
		ITcpConn *pConn = pServer->OnAcceptClient((uintptr_t)clnt, std::string(peerip));
		if (nullptr == pConn) {
			// bind connection impl failed
			clnt->_connimpl = nullptr;
			return -1;
		}

		// mark as connected
		if (!pConn->IsDisposed()) {
			pConn->SetConnected(true);
		}
		return 0;
	}
}

//------------------------------------------------------------------------------
/**

*/
static void
on_client_read(struct bufferevent *bev, void *arg) {
	struct event_base *base = bufferevent_get_base(bev);
	struct evbuffer *input = bufferevent_get_input(bev);
	struct evbuffer_iovec iovec[1];
	size_t nbytes;

	client_handle_t *clnt = (client_handle_t *)arg;
	ITcpConn *pConn = (ITcpConn *)clnt->_connimpl;

	// echo sample
	// 	{
	// 		bufferevent_write_buffer(bev, bufferevent_get_input(bev));
	// 	}

	//
	while (pConn && (nbytes = evbuffer_get_contiguous_space(input)) > 0) {
		//size_t szchains = evbuffer_peek(input, -1, nullptr, iovec, 1);
		size_t szoffersize = 4096;
		char *bufoffer = bip_buf_reserve(clnt->_bb, &szoffersize);
		if (szoffersize >= iovec[0].iov_len) {
			memcpy(bufoffer, iovec[0].iov_base, iovec[0].iov_len);
			bip_buf_commit(clnt->_bb, szoffersize);
			pConn->OnRawData(*clnt->_bb);
		}
		//bufferevent_write(bev, (const uint8_t *)iovec[0].iov_base, iovec[0].iov_len); // echo sample
		evbuffer_drain(input, iovec[0].iov_len);
	}

}

//------------------------------------------------------------------------------
/**

*/
static void
on_client_write(struct bufferevent *bev, void *arg) {

}

/**------------------------------------------------------------------------------
*
*/
static void
on_client_error(struct bufferevent *bev, short what, void *arg) {
	client_handle_t *clnt = (client_handle_t *)arg;

	if (what & BEV_EVENT_EOF) {
		//		printf("Connection closed.\n");
	}
	else if (what & BEV_EVENT_ERROR) {
		// 		printf("Got an error on the connection: %s\n",
		// 		    strerror(errno));/*XXX win32*/
	}

	ITcpConn *pConn = (ITcpConn *)clnt->_connimpl;
	if (pConn) {
		pConn->Disconnect();
	}
	else {
		// don't close bufferevent again, it is already closed.
	}
}

//------------------------------------------------------------------------------
/**

*/
CEvServer::CEvServer(ITcpConnFactory *pFactory)
	: _refConnFactory(pFactory)
	, _eventManager(new CTcpEventManager()) {

	memset(&_srvr, 0, sizeof(_srvr));
	_srvr._userdata = this;
}

//------------------------------------------------------------------------------
/**

*/
CEvServer::~CEvServer() {

	SAFE_DELETE(_eventManager);
	_refConnFactory = nullptr;
}

//------------------------------------------------------------------------------
/**

*/
ITcpClient *
CEvServer::OnAcceptClient(uintptr_t streamptr, std::string&& sPeerIp) {
	//
	ITcpClient *pClient = _refConnFactory->CreateTcpClientOnServer(std::move(sPeerIp), this);
	if (pClient) {
		// connection is ready
		_refConnFactory->ConfirmClientIsReady(pClient, streamptr);

		// event
		pClient->OnConnect();
	}
	return pClient;
}

//------------------------------------------------------------------------------
/**

*/
void
CEvServer::OnDisposeClient(ITcpClient *pClient) {

	if (pClient) {
		// dispose
		pClient->DisposeConnection();

		// disconnect cb to front end
		// Note: Client may be [ false == IsConnected() ], but we always need main thread to [ RemoveClient() ]
		_refConnFactory->AddClientDisconnectCb(this, pClient, &_refConnFactory->GetConnManager());
	}
}

//------------------------------------------------------------------------------
/**

*/
int
CEvServer::Open(void *base, unsigned short port) {

	int nResult = 0;

	// init base
	struct evsrvr_impl_t *srvr_impl = (struct evsrvr_impl_t *)malloc(sizeof(struct evsrvr_impl_t));
	srvr_impl->_base = (struct event_base *)_refConnFactory->GetLoopBase();
	srvr_impl->_server_closed = 0;
	srvr_impl->_acceptcb = on_server_accept_client;
	srvr_impl->_readcb = on_client_read;
	srvr_impl->_writecb = on_client_write;
	srvr_impl->_errorcb = on_client_error;

	// init port
	_srvr._port = port;

	// init listener
	nResult = ev_server_init_(&_srvr, srvr_impl);
	if (0 == nResult) {

		fprintf(stderr, "server init on port(%d) fd(%d)...\n",
			_srvr._port, (int)_srvr._fd);
	}
	return nResult;
}

//------------------------------------------------------------------------------
/**

*/
void
CEvServer::Close() {

	if (!_closed) {
		//
		for (auto& iter : _tcpStreamDict) {
			auto& clnt = iter.second;
			
			// flush stream
			ev_close_((struct bufferevent *)clnt->_sockimpl);
			clnt->_sockimpl = nullptr;

			// free conn handle
			bip_buf_destroy(clnt->_bb);
			free(clnt);
		}
		_tcpStreamDict.clear();

		// dispose base
		struct evsrvr_impl_t *srvr_impl = (struct evsrvr_impl_t *)_srvr._impl;
		srvr_impl->_server_closed = 1;
		ev_server_dispose_(&_srvr);
	
		//
		_refConnFactory->GetConnManager().DisposeDownStreams(this);

		//
		_closed = true;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CEvServer::FlushDownStream(uintptr_t streamptr) {
	client_handle_t *clnt = nullptr;
	auto iter = _tcpStreamDict.find(streamptr);
	if (iter != _tcpStreamDict.end()) {

		clnt = iter->second;
		_tcpStreamDict.erase(iter);
	}

	// flush stream
	ev_close_((struct bufferevent *)clnt->_sockimpl);
	clnt->_sockimpl = nullptr;

	// free conn handle
	bip_buf_destroy(clnt->_bb);
	free(clnt);
	clnt = NULL;
}

/** -- EOF -- **/