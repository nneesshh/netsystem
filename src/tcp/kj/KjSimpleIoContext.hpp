#pragma once
//------------------------------------------------------------------------------
/**
@class CKjSimpleIoContext

(C) 2016 n.lee
*/
#include <algorithm>

#include "../../common/UsingMyToolkitMini.h"

class KjSimpleIoContext : public kj::Refcounted, public kj::TaskSet::ErrorHandler {
public:
	KjSimpleIoContext() : _ioContext(kj::setupAsyncIo()) {}
	~KjSimpleIoContext() noexcept(false) = default;

	kj::WaitScope&				GetWaitScope() {
		return _ioContext.waitScope;
	}

	kj::Network&				GetNetwork() {
		return _ioContext.provider->getNetwork();
	}

	kj::Timer&					GetTimer() {
		return _ioContext.provider->getTimer();
	}

	kj::Promise<void>			AfterDelay(kj::Duration delay) {
		return GetTimer().afterDelay(delay);
	}

	template <typename T>
	kj::Promise<T>				TimeoutAfter(kj::Duration delay, kj::Promise<T>&& p) {
		return GetTimer().timeoutAfter(delay, kj::mv(p));
	}

	template <typename Func>
	kj::PromiseForResult<Func, void> EvalForResult(Func&& func) {
		return kj::evalLater(kj::mv(func));
	}

	template <typename T>
	kj::ForkedPromise<T>		ForkPromise(kj::Promise<T>&& promise) {
		return promise.fork();
	}

	kj::Own<kj::TaskSet>		CreateTaskSet() {
		return kj::heap<kj::TaskSet>(*this);
	}

	kj::Own<kj::TaskSet>		CreateTaskSet(kj::TaskSet::ErrorHandler& errorHandler) {
		return kj::heap<kj::TaskSet>(errorHandler);
	}

	kj::AsyncIoProvider::PipeThread NewPipeThread(kj::Function<void(kj::AsyncIoProvider&, kj::AsyncIoStream&, kj::WaitScope&)>&& startFunc) {
		return _ioContext.provider->newPipeThread(kj::mv(startFunc));
	}

private:
	void taskFailed(kj::Exception&& exception) override;

	kj::AsyncIoContext _ioContext;
};

/*EOF*/