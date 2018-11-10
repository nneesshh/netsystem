#pragma once
//------------------------------------------------------------------------------
/**
@class CCamelReaderWriterQueueWL

	WL means "waterline"

(C) 2016 n.lee
*/
#include <memory>
#include <functional>

#include "concurrent/readerwriterqueue.h"

//------------------------------------------------------------------------------
/**
@brief CCamelReaderWriterQueueWL
*/
class CCamelReaderWriterQueueWL {
public:
	CCamelReaderWriterQueueWL();
	~CCamelReaderWriterQueueWL();

	using CallbackEntry = std::function<void()>;

	void RunOnce();
	void Close();

	bool Add(std::function<void()>&& workCb);

private:
	bool _close = false;

	moodycamel::ReaderWriterQueue<CallbackEntry> _callbacks;
};
using CCamelReaderWriterQueueWLPtr = std::shared_ptr<CCamelReaderWriterQueueWL>;

/*EOF*/