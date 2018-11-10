//------------------------------------------------------------------------------
//  CamelReaderWriterQueueWL.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "CamelReaderWriterQueueWL.h"

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN 1
# include <windows.h>
#endif

static void
util_sleep(unsigned int milliseconds) {
#if defined(WIN32) || defined(_WIN32)
	Sleep(milliseconds);
#else
	int seconds = milliseconds / 1000;
	int useconds = (milliseconds % 1000) * 1000;

	sleep(seconds);
	usleep(useconds);
#endif
}

#ifdef _DEBUG
# define RWQUEUE_CONSUME_WATERLINE	128
#else
# define RWQUEUE_CONSUME_WATERLINE	2048
#endif

#define RWQUEUE_PRODUCE_WATERLINE	8192

//------------------------------------------------------------------------------
/**

*/
CCamelReaderWriterQueueWL::CCamelReaderWriterQueueWL()
	: _callbacks(256) {

}

//------------------------------------------------------------------------------
/**

*/
CCamelReaderWriterQueueWL::~CCamelReaderWriterQueueWL() {

}

//------------------------------------------------------------------------------
/**

*/
void
CCamelReaderWriterQueueWL::RunOnce() {
	// work queue
	int ncount = 0;
	CallbackEntry workCb;
	while (!_close 
		&& ncount < RWQUEUE_CONSUME_WATERLINE
		&& _callbacks.try_dequeue(workCb)) {

		workCb();

		//
		++ncount;
	}
}

//------------------------------------------------------------------------------
/**

*/
void
CCamelReaderWriterQueueWL::Close() {
	//
	// Set done flag and notify.
	//
	_close = true;
}

//------------------------------------------------------------------------------
/**

*/
bool
CCamelReaderWriterQueueWL::Add(std::function<void()>&& workCb) {
	if (_close) {
		// error
		fprintf(stderr, "[CCamelWorkQueue::Add()] can't enqueue, callback is dropped!!!");
		return false;
	}

	//
	// Add work item.
	//
	if (!_callbacks.enqueue(std::move(workCb))) {
		// error
		fprintf(stderr, "[CCamelWorkQueue::Add()] enqueue failed, callback is dropped!!!");
		return false;
	}

	// decrease produce speed, wait main thread to consume
	if (_callbacks.size_approx() >= RWQUEUE_PRODUCE_WATERLINE) {
		util_sleep(15);
	}
	return true;
}

/** -- EOF -- **/