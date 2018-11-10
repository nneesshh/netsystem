#pragma once

#include <functional>

#include "base/bip_buf.h"
#include "KjSimpleThreadIoContext.hpp"

class KjTcpIoStream : public kj::Refcounted {
public:
	using READ_CB = std::function<void(KjTcpIoStream&, bip_buf_t&)>;
	using CONNECT_CB = std::function<void(KjTcpIoStream&, uint64_t)>;
	using DISCONNECT_CB = std::function<void(KjTcpIoStream&, uint64_t)>;
	using ERROR_CB = std::function<void(KjTcpIoStream&, kj::Exception&& exception)>;

	struct stream_attach_t {
		std::size_t _readSize;
		READ_CB _readCb;
	};

public:
	//! ctor & dtor
	KjTcpIoStream(kj::Own<KjSimpleThreadIoContext> tioContext, kj::Own<kj::AsyncIoStream> stream);
	~KjTcpIoStream();

	//! copy ctor & assignment operator
	KjTcpIoStream(const KjTcpIoStream&) = delete;
	KjTcpIoStream& operator=(const KjTcpIoStream&) = delete;

	//! comparison operator
	bool operator==(const KjTcpIoStream& rhs) const;
	bool operator!=(const KjTcpIoStream& rhs) const;

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
	void Disconnect();

	//! 
	void FlushStream() {

		if (_stream) {
			try {
				_stream->abortRead();
				_stream->shutdownWrite();
			}
			catch(std::exception& e) {
				fprintf(stderr, "[KjTcpIoStream::FlushStream()] abortRead or shutdownWrite exception -- what(%s)!!!"
					, e.what());
			}
			_stream = nullptr;
		}

		if (_disconnectFulfiller) {
			_disconnectFulfiller->fulfill();
			_disconnectFulfiller = nullptr;
		}
	}

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
	kj::Own<KjSimpleThreadIoContext> _tioContext;
	kj::Own<kj::AsyncIoStream> _stream;

	kj::ForkedPromise<void> _disconnectPromise = nullptr;
	kj::Own<kj::PromiseFulfiller<void>> _disconnectFulfiller;

private:
	uintptr_t _streamid = 0;

	stream_attach_t _streamAttach;
	bip_buf_t *_bb = nullptr;
};

/*EOF*/