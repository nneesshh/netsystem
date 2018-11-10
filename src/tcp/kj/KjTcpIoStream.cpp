//------------------------------------------------------------------------------
//  KjTcpIoStream.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjTcpIoStream.hpp"

#include <assert.h>

#pragma push_macro("ERROR")
#undef ERROR
#pragma push_macro("VOID")
#undef VOID

#include "thirdparty/g/capnp/kj/debug.h"

#pragma pop_macro("ERROR")
#pragma pop_macro("VOID")

#include "../tcp_def.h"

//------------------------------------------------------------------------------
/**
//! ctor & dtor
*/
KjTcpIoStream::KjTcpIoStream(kj::Own<KjSimpleThreadIoContext> tioContext, kj::Own<kj::AsyncIoStream> stream)
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
KjTcpIoStream::~KjTcpIoStream() {
	FlushStream();
	bip_buf_destroy(_bb);
}

//------------------------------------------------------------------------------
/**
//! comparison operator
*/
bool
KjTcpIoStream::operator==(const KjTcpIoStream& rhs) const {
	return _streamid == rhs._streamid;
}

//------------------------------------------------------------------------------
/**

*/
bool
KjTcpIoStream::operator!=(const KjTcpIoStream& rhs) const {
	return !operator==(rhs);
}

//------------------------------------------------------------------------------
/**

*/
void
KjTcpIoStream::Disconnect() {
	// flush stream
	FlushStream();
}

//------------------------------------------------------------------------------
/**

*/
kj::Promise<void>
KjTcpIoStream::StartReadOp(const READ_CB& readCb) {
	_streamAttach._readCb = readCb;
	return AsyncReadLoop()
		.exclusiveJoin(DisconnectWatcher());
}

//------------------------------------------------------------------------------
/**

*/
kj::Promise<void>
KjTcpIoStream::AsyncReadLoop() {

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