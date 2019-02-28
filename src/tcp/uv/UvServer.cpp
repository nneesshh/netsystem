//------------------------------------------------------------------------------
//  UvServer.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "UvServer.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../../base/MyMacros.h"

#include "../TcpConnManager.h"
#include "../TcpEventManager.h"

#include "UvConnFactory.h"
#include "UvClientConn.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

#define SERVER_SELF_CHECK_TIME 5000L

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
		pConn->SetFlushed(true);
	}
	free(handle);
}

//------------------------------------------------------------------------------
/**

*/
static void
on_echo_write(uv_write_t* req, int status) {
	write_req_t* wr;

	assert(req != NULL);

	wr = (write_req_t*)req;

	/* Free the read/write buffer and the request */
	free(wr->buf.base);
	free(wr);
}

//------------------------------------------------------------------------------
/**

*/
static void
on_client_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
	client_handle_t *clnt = (client_handle_t *)(stream->data);
	ITcpConn *pConn = (ITcpConn *)clnt->_connimpl;

	if (nread < 0) {
		/* Error or EOF */
		if (nread == UV_EOF
			|| nread == UV_ECONNRESET) {
			/* Free the read/write buffer */
			bip_buf_commit(clnt->_bb, 0);

			if (pConn) {
				pConn->Disconnect();
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
on_client_write(uv_write_t* req, int status) {
	write_req_t* wr;

	assert(req != NULL);

	/* Free the read/write buffer and the request */
	wr = (write_req_t*)req;

	//
	if (status) {
		if (ERROR_IO_PENDING != GetLastError()) {
			uv_tcp_t *tcp = (uv_tcp_t*)(req->handle);
			client_handle_t *clnt = (client_handle_t *)(tcp->data);

			ITcpConn *pConn = (ITcpConn *)clnt->_connimpl;
			if (pConn) {
				pConn->Disconnect();
			}
			else {
				uv_close_((uv_handle_t *)clnt->_sockimpl, on_sock_close);
				bip_buf_destroy(clnt->_bb);
				free(clnt);
			}

			fprintf(stderr,	"write_cb error: %s - %s\n",
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
on_server_close(uv_handle_t* handle) {
	server_handle_t *srvr = (server_handle_t *)handle->data;
	struct uvsrvr_impl_t *srvr_impl = (struct uvsrvr_impl_t *)srvr->_impl;
	uv_tcp_t *stream = (uv_tcp_t *)srvr_impl->_server;

	srvr_impl->_server_closed = 1;
	free(stream);
}

//------------------------------------------------------------------------------
/**

*/
static int
on_server_accept_client(server_handle_t *srvr, client_handle_t *clnt, const char *peerip) {
	// accept client cb
	CUvServer *pServer = static_cast<CUvServer *>(srvr->_userdata);

	if (!pServer->IsReady()) {
		// flush stream at once
		uv_close_((uv_handle_t *)clnt->_sockimpl, on_stream_close);
		clnt->_sockimpl = nullptr;
		return -1;
	}
	else {
		//
		uintptr_t streamptr = pServer->AddStream(clnt);
		ITcpConn *pConn = pServer->OnAcceptClient(streamptr, std::string(peerip));
		if (nullptr == pConn) {
			// bind connection impl failed
			clnt->_connimpl = nullptr;
			return -1;
		}
		
		clnt->_connimpl = pConn;
		return 0;
	}
}

//------------------------------------------------------------------------------
/**

*/
CUvServer::CUvServer(ITcpConnFactory *pFactory)
	: _refConnFactory(pFactory)
	, _eventManager(new CTcpEventManager()) {

	memset(&_srvr, 0, sizeof(_srvr));
	_srvr._userdata = this;
}

//------------------------------------------------------------------------------
/**

*/
CUvServer::~CUvServer() {
	
	SAFE_DELETE(_eventManager);
	_refConnFactory = nullptr;
}

//------------------------------------------------------------------------------
/**

*/
ITcpClient *
CUvServer::OnAcceptClient(uintptr_t streamptr, std::string&& sPeerIp) {
	//
	ITcpClient *pClient = _refConnFactory->CreateTcpClientOnServer(std::string(sPeerIp), this);
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
CUvServer::OnDisposeClient(ITcpClient *pClient) {

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
CUvServer::Open(void *base, unsigned short port) {

	int nResult = 0;

	// init base
	struct uvsrvr_impl_t *srvr_impl = (struct uvsrvr_impl_t *)malloc(sizeof(struct uvsrvr_impl_t));
	srvr_impl->_loop = (uv_loop_t *)_refConnFactory->GetLoopBase();
	srvr_impl->_acceptcb = on_server_accept_client;
	srvr_impl->_closecb = on_server_close;
	srvr_impl->_readcb = on_client_read;
	srvr_impl->_writecb = on_client_write;
	srvr_impl->_server = NULL;
	srvr_impl->_server_closed = 0;

	// init port
	_srvr._port = port;

	// init listener
	nResult = uv_server_init_(&_srvr, srvr_impl);
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
CUvServer::Close() {

	if (!_closed) {
		//
		for (auto& iter : _tcpStreamDict) {
			auto& clnt = iter.second;

			// flush stream
			uv_close_((uv_handle_t *)clnt->_sockimpl, on_stream_close);
			clnt->_sockimpl = nullptr;
			
			// free conn handle
			bip_buf_destroy(clnt->_bb);
			free(clnt);
		}
		_tcpStreamDict.clear();

		// dispose base
		struct uvsrvr_impl_t *srvr_impl = (struct uvsrvr_impl_t *)_srvr._impl;
		srvr_impl->_server_closed = 1;
		uv_server_dispose_(&_srvr);

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
CUvServer::FlushDownStream(uintptr_t streamptr) {
	client_handle_t *clnt = nullptr;
	auto iter = _tcpStreamDict.find(streamptr);
	if (iter != _tcpStreamDict.end()) {

		clnt = iter->second;
		_tcpStreamDict.erase(iter);
	}

	uv_close_((uv_handle_t *)clnt->_sockimpl, on_sock_close);
	bip_buf_destroy(clnt->_bb);
	free(clnt);
	clnt = NULL;
}

/** -- EOF -- **/