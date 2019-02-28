//------------------------------------------------------------------------------
//  KjTcpConnection.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjTcpConnection.hpp"

#include <assert.h>

#include "../tcp_def.h"

//------------------------------------------------------------------------------
/**
//! ctor & dtor
*/
KjTcpConnection::KjTcpConnection(kj::Own<KjPipeEndpointIoContext> tioContext, uint64_t connid)
	: _tioContext(kj::mv(tioContext))
	, _connid(connid) {
	
	_connAttach._readSize = TCP_STREAM_READ_SIZE;
	_bb = bip_buf_create(TCP_STREAM_READ_SIZE);
}

//------------------------------------------------------------------------------
/**

*/
KjTcpConnection::~KjTcpConnection() {
	FlushStream();
	bip_buf_destroy(_bb);
}

//------------------------------------------------------------------------------
/**
//! comparison operator
*/
bool
KjTcpConnection::operator==(const KjTcpConnection& rhs) const {
	return _connid == rhs._connid;
}

//------------------------------------------------------------------------------
/**

*/
bool
KjTcpConnection::operator!=(const KjTcpConnection& rhs) const {
	return !operator==(rhs);
}

//------------------------------------------------------------------------------
/**

*/
kj::Promise<void>
KjTcpConnection::Connect(
	kj::StringPtr host,
	kj::uint port,
	CONNECT_CB connectCb,
	DISCONNECT_CB disconnectCb) {

	auto paf = kj::newPromiseAndFulfiller<void>();
	_disconnectPromise = paf.promise.fork();
	_disconnectFulfiller = kj::mv(paf.fulfiller);

	_connAttach._connectCb = connectCb;
	_connAttach._disconnectCb = disconnectCb;

	_host = host;
	_port = port;

	auto p1 = _tioContext->GetNetwork().parseAddress(_host, _port)
		.then([this](kj::Own<kj::NetworkAddress>&& addr) {

		_addr = kj::mv(addr);
		return StartConnect();
	}).exclusiveJoin(DisconnectWatcher());
	return p1;
}

//------------------------------------------------------------------------------
/**

*/
void
KjTcpConnection::Disconnect() {
	//
	if (_bConnected) {
		_bConnected = false;

		if (_connAttach._disconnectCb)
			_connAttach._disconnectCb(*this, _connid);
	}

	// stream disconnect means disposed
	if (!_bDisposed) {
		_bDisposed = true;

		// flush stream
		FlushStream();
	}
}

//------------------------------------------------------------------------------
/**

*/
void
KjTcpConnection::FlushStream() {

	if (_stream) {
		// flush stream at once
		/*try {
			_stream->abortRead();
			_stream->shutdownWrite();
		}
		catch (std::exception& e) {
			fprintf(stderr, "[KjTcpConnection::FlushStream()] abortRead or shutdownWrite exception -- what(%s)!!!"
				, e.what());
		}*/
		_stream = nullptr;
	}

	if (_disconnectFulfiller) {
		_disconnectFulfiller->fulfill();
		_disconnectFulfiller = nullptr;
	}

	// clear bb
	bip_buf_reset(_bb);
}

//------------------------------------------------------------------------------
/**

*/
kj::Promise<void>
KjTcpConnection::Reconnect() {

	assert(!_bDisposed);

	if (!_disconnectFulfiller) {
		auto paf = kj::newPromiseAndFulfiller<void>();
		_disconnectPromise = paf.promise.fork();
		_disconnectFulfiller = kj::mv(paf.fulfiller);
	}

	return StartConnect()
		.exclusiveJoin(DisconnectWatcher());
}

//------------------------------------------------------------------------------
/**

*/
kj::Promise<void>
KjTcpConnection::StartReadOp(const READ_CB& readCb) {
	_connAttach._readCb = readCb;
	return AsyncReadLoop()
		.exclusiveJoin(DisconnectWatcher());
}

//------------------------------------------------------------------------------
/**

*/
kj::Promise<void>
KjTcpConnection::StartConnect() {

	return _addr->connect()
		.then([this](kj::Own<kj::AsyncIoStream>&& stream) {

		_stream = kj::mv(stream);
		_bConnected = true;

		if (_connAttach._connectCb) {
			_connAttach._connectCb(*this, _connid);
		}

	});
}

//------------------------------------------------------------------------------
/**

*/
kj::Promise<void>
KjTcpConnection::AsyncReadLoop() {

	size_t buflen = _connAttach._readSize;
	char *bufbase = bip_buf_reserve(_bb, &buflen);

	if (bufbase && buflen > 0) {

		return _stream->read(bufbase, 1, buflen)
			.then([this](size_t amount) {
			//
			bip_buf_commit(_bb, (int)amount);

			if (_connAttach._readCb) {
				_connAttach._readCb(*this, *_bb);
			}
			return AsyncReadLoop();
		});
	}
	else {
		// delay and read
		const unsigned int uDelayMilliSeconds = 200;
		// kjconnection async read loop
		return _tioContext->AfterDelay(uDelayMilliSeconds * kj::MILLISECONDS)
			.then([this]() {

			return AsyncReadLoop();
		});
	}
}

/* -- EOF -- */