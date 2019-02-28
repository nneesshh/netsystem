//------------------------------------------------------------------------------
//  KjTcpDownStream.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjTcpDownStream.hpp"

#include <assert.h>

#include "../tcp_def.h"

//------------------------------------------------------------------------------
/**
//! ctor & dtor
*/
KjTcpDownStream::KjTcpDownStream(kj::Own<KjPipeEndpointIoContext> tioContext, kj::Own<kj::AsyncIoStream> stream)
	: _tioContext(kj::mv(tioContext))
	, _stream(kj::mv(stream))
	, _streamid((uintptr_t)this) {

	auto paf = kj::newPromiseAndFulfiller<void>();
	_disconnectPromise = paf.promise.fork();
	_disconnectFulfiller = kj::mv(paf.fulfiller);
	
	_streamAttach._readSize = TCP_STREAM_READ_SIZE;
	_bb = bip_buf_create(TCP_STREAM_READ_SIZE);
}

//------------------------------------------------------------------------------
/**

*/
KjTcpDownStream::~KjTcpDownStream() {
	FlushStream();
	bip_buf_destroy(_bb);
}

//------------------------------------------------------------------------------
/**
//! comparison operator
*/
bool
KjTcpDownStream::operator==(const KjTcpDownStream& rhs) const {
	return _streamid == rhs._streamid;
}

//------------------------------------------------------------------------------
/**

*/
bool
KjTcpDownStream::operator!=(const KjTcpDownStream& rhs) const {
	return !operator==(rhs);
}

//------------------------------------------------------------------------------
/**

*/
void
KjTcpDownStream::FlushStream() {

	if (_stream) {
		// flush stream at once with shutdown gracefully: 1) abort read 2) shutdown write
		try {
			_stream->abortRead();
			_stream->shutdownWrite();
		}
		catch(std::exception& e) {
			fprintf(stderr, "[KjTcpDownStream::FlushStream()] abortRead or shutdownWrite exception -- what(%s)!!!"
				, e.what());
		}
		_stream = nullptr;
	}

	if (_disconnectFulfiller) {
		_disconnectFulfiller->fulfill();
		_disconnectFulfiller = nullptr;
	}
}

//------------------------------------------------------------------------------
/**

*/
kj::Promise<void>
KjTcpDownStream::StartReadOp(const READ_CB& readCb) {
	_streamAttach._readCb = readCb;
	return AsyncReadLoop()
		.exclusiveJoin(DisconnectWatcher());
}

//------------------------------------------------------------------------------
/**

*/
kj::Promise<void>
KjTcpDownStream::AsyncReadLoop() {

	size_t buflen = _streamAttach._readSize;
	char *bufbase = bip_buf_reserve(_bb, &buflen);

	if (bufbase && buflen > 0) {

		return _stream->read(bufbase, 1, buflen)
			.then([this](size_t amount) {
			//
			bip_buf_commit(_bb, (int)amount);

			if (_streamAttach._readCb) {
				_streamAttach._readCb(*this, *_bb);
			}
			return AsyncReadLoop();
		});
	}
	else {
		// delay and read
		const unsigned int uDelayMilliSeconds = 200;
		// kjstream async read loop
		return _tioContext->AfterDelay(uDelayMilliSeconds * kj::MILLISECONDS)
			.then([this]() {

			return AsyncReadLoop();
		});
	}
}

/* -- EOF -- */