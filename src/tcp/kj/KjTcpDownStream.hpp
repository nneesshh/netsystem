#pragma once

#include <functional>

#include "servercore/io/KjPipeEndpointIoContext.hpp"

#include "base/bip_buf.h"

class KjTcpDownStream : public kj::Refcounted {
public:
	using READ_CB = std::function<void(KjTcpDownStream&, bip_buf_t&)>;
	using CONNECT_CB = std::function<void(KjTcpDownStream&, uint64_t)>;
	using DISCONNECT_CB = std::function<void(KjTcpDownStream&, uint64_t)>;
	using ERROR_CB = std::function<void(KjTcpDownStream&, kj::Exception&& exception)>;

	struct stream_attach_t {
		std::size_t _readSize;
		READ_CB _readCb;
	};

public:
	//! ctor & dtor
	KjTcpDownStream(kj::Own<KjPipeEndpointIoContext> tioContext, kj::Own<kj::AsyncIoStream> stream);
	~KjTcpDownStream();

	//! copy ctor & assignment operator
	KjTcpDownStream(const KjTcpDownStream&) = delete;
	KjTcpDownStream& operator=(const KjTcpDownStream&) = delete;

	//! comparison operator
	bool operator==(const KjTcpDownStream& rhs) const;
	bool operator!=(const KjTcpDownStream& rhs) const;

	//! 
	uintptr_t GetStreamPtr() {
		return _streamid;
	}

	//! 
	void Write(const void* buffer, size_t size) {
		if (_stream)
			_stream->write(buffer, size);
	}

	//! 
	void FlushStream();

	//! 
	kj::Promise<void> StartReadOp(const READ_CB& readCb);

	//!
	kj::Promise<void> DisconnectWatcher() {
		return _disconnectPromise.addBranch();
	}

private:
	//! operation
	kj::Promise<void> AsyncReadLoop();

public:
	kj::Own<KjPipeEndpointIoContext> _tioContext;
	kj::Own<kj::AsyncIoStream> _stream;

	kj::ForkedPromise<void> _disconnectPromise = nullptr;
	kj::Own<kj::PromiseFulfiller<void>> _disconnectFulfiller;

private:
	uintptr_t _streamid = 0;

	stream_attach_t _streamAttach;
	bip_buf_t *_bb = nullptr;
};

/*EOF*/