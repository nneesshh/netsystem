//------------------------------------------------------------------------------
//  KjSimpleIoContext.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjSimpleIoContext.hpp"

#ifdef _MSC_VER
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__,__LINE__)
#endif
#endif

// test
class KjDummyEventPort : public kj::EventPort {
public:
	bool runnable = false;
	int callCount = 0;

	bool wait() override {
		KJ_FAIL_ASSERT("Nothing to wait for.");
	}

	bool poll() override { return false; }
	void setRunnable(bool runnable) override {
		this->runnable = runnable;
		++callCount;
	}
};

void
KjSimpleIoContext::taskFailed(kj::Exception&& exception) {
	fprintf(stderr, "\n[KjSimpleIoContext::taskFailed()] desc(%s) -- pause!!!\n", exception.getDescription().cStr());
	system("pause");
	kj::throwFatalException(kj::mv(exception));
}

/* -- EOF -- */