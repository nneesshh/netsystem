//------------------------------------------------------------------------------
//  KjConnFactoryWorkQueue.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjConnFactoryWorkQueue.hpp"

#include <future>

//////////////////////////////////////////////////////////////////////////
struct gate_conn_factory_thread_worker_t {
	kj::AsyncIoStream                *_refStream;
	kj::Own<KjSimpleThreadIoContext>  _tioContext;
	kj::Own<kj::TaskSet>              _tasks;
};
static gate_conn_factory_thread_worker_t *s_tls_worker = nullptr;

static kj::Promise<void>
check_quit_loop(CKjConnFactoryWorkQueue& q, gate_conn_factory_thread_worker_t& worker, kj::PromiseFulfiller<void> *fulfiller) {
	if (!q.IsDone()) {
		// delay and check quit loop -- wait 500 ms
		return worker._tioContext->GetTimer().afterDelay(500 * kj::MICROSECONDS)
			.then([&q, &worker, fulfiller]() {
			// loop
			return check_quit_loop(q, worker, fulfiller);
		});
	}

	//
	fulfiller->fulfill();
	return kj::READY_NOW;
}

static kj::Promise<void>
read_pipe_loop(CKjConnFactoryWorkQueue& q, gate_conn_factory_thread_worker_t& worker, kj::AsyncIoStream& stream) {

	if (!q.IsDone()) {
		return stream.tryRead(&q._opCodeRecvBuf, 1, 1024)
			.then([&q, &worker, &stream](size_t amount) {
			//
			// Get next work item.
			//
			while (q.Callbacks().try_dequeue(q._opCmd)) {

				auto& cmd = q._opCmd;
				int r = 0;

				switch (cmd._type) {
				case CKjConnFactoryWorkQueue::net_cmd_t::OPEN_TCP_SERVER: {

					ITcpServer *pServer = static_cast<ITcpServer *>(cmd._handle);
					r = pServer->Open(worker._tioContext.get(), cmd._port);
					break;
				}

				case CKjConnFactoryWorkQueue::net_cmd_t::CLOSE_TCP_SERVER: {

					ITcpServer *pServer = static_cast<ITcpServer *>(cmd._handle);
					pServer->Close();
					break;
				}

				case CKjConnFactoryWorkQueue::net_cmd_t::DISPOSE_CONNECTION: {

					ITcpConn *pConn = static_cast<ITcpConn *>(cmd._handle);
					pConn->DisposeConnection();
					break;
				}

				case CKjConnFactoryWorkQueue::net_cmd_t::POST_PACKET: {

					ITcpConn *pConn = static_cast<ITcpConn *>(cmd._handle);
					if (pConn->IsReady()) {
						pConn->PostPacket(cmd._uInnerUuid, cmd._uSerialNo, cmd._typeName, cmd._body);
					}
					break;
				}

				case CKjConnFactoryWorkQueue::net_cmd_t::CONFIRM_CLIENT_IS_READY: {

					ITcpClient *pClient = static_cast<ITcpClient *>(cmd._handle);
					uintptr_t streamptr = cmd._streamptr;
					if (pClient->IsReady()) {
						pClient->ConfirmClientIsReady(worker._tioContext.get(), streamptr);
					}
					break;
				}

				case CKjConnFactoryWorkQueue::net_cmd_t::POST_BROADCAST_PACKET: {

					ITcpClient *pClient = static_cast<ITcpClient *>(cmd._handle);
					if (pClient->IsReady()) {
						//

					}
					break;
				}

				case CKjConnFactoryWorkQueue::net_cmd_t::ISOLATED_CONN_CONNECT: {
					
					ITcpIsolated *pIsolated = static_cast<ITcpIsolated *>(cmd._handle);
					pIsolated->Connect(worker._tioContext.get(), cmd._ip_or_hostname, cmd._port);
					break;
				}

				case CKjConnFactoryWorkQueue::net_cmd_t::ISOLATED_FLUSH_STREAM: {

					ITcpIsolated *pIsolated = static_cast<ITcpIsolated *>(cmd._handle);
					// flush and error
					pIsolated->FlushConnection();
					pIsolated->OnIsolatedError();
					break;
				}

				case CKjConnFactoryWorkQueue::net_cmd_t::ISOLATED_DELAY_RECONNECT: {

					ITcpIsolated *pIsolated = static_cast<ITcpIsolated *>(cmd._handle);
					int nDelaySeconds = (int)cmd._streamptr;
					pIsolated->DelayReconnect(nDelaySeconds);
					break;
				}

				default:
					break;
				}

				// result callback
				if (cmd._resultCb)
					cmd._resultCb(r);
			}
			return read_pipe_loop(q, worker, stream);
		});
	}
	return kj::READY_NOW;
}

static std::vector<uint64_t> s_vEmpty;
static std::string s_sEmpty;

//------------------------------------------------------------------------------
/**

*/
CKjConnFactoryWorkQueue::CKjConnFactoryWorkQueue(ITcpConnFactory *pFactory, kj::Own<KjSimpleIoContext> rootContext)
	: _refConnFactory(pFactory)
	, _rootContext(kj::addRef(*rootContext))
	, _callbacks(256) {
	//
	Start();
}

//------------------------------------------------------------------------------
/**

*/
CKjConnFactoryWorkQueue::~CKjConnFactoryWorkQueue() {
	_pipeThread.pipe = nullptr;
	_pipeThread.thread = nullptr;

	_rootContext = nullptr;
	_refConnFactory = nullptr;
}

//------------------------------------------------------------------------------
/**

*/
bool
CKjConnFactoryWorkQueue::Add(net_cmd_t&& cmd) {

	if (_done) {
		// error
		fprintf(stderr, "[CKjConnFactoryWorkQueue::Add()] can't enqueue, callback is dropped!!!");
		return false;
	}

	//
	// Add work item.
	//
	if (!_callbacks.enqueue(std::move(cmd))) {
		// error
		fprintf(stderr, "[CKjConnFactoryWorkQueue::Add()] enqueue failed, callback is dropped!!!");
		return false;
	}

	// write opcode to pipe
	++_opCodeSend;

	// pipe write maybe trigger exception at once, so we must catch it manually
	KJ_IF_MAYBE(e, kj::runCatchingExceptions([this]() {
		_pipeThread.pipe->write((const void *)&_opCodeSend, 1);
	})) {
		fprintf(stderr, "[CKjRedisSubscriberWorkQueue::Add()] desc(%s) -- pause!!!\n", e->getDescription().cStr());
		system("pause");
		kj::throwFatalException(kj::mv(*e));
	}
	return true;
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::Finish() {
	//
	// Set done flag and notify.
	//
	_done = true;

	// join
//  	if (_pipeThread.thread->joinable()) {
// 		_pipeThread.thread->join();
//  	}
}

//------------------------------------------------------------------------------
/**

*/
int
CKjConnFactoryWorkQueue::OpenTcpServer(ITcpServer *pServer, unsigned short nPort) {

	return BlockingCommit(
		net_cmd_t::OPEN_TCP_SERVER,
		pServer,
		"",
		nPort);
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::CloseTcpServer(ITcpServer *pServer) {

	BlockingCommit(
		net_cmd_t::CLOSE_TCP_SERVER,
		pServer,
		"",
		0);
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::DisposeConnection(ITcpConn *pConn) {
	// reference and dereference
	auto resultCb = [pConn](int r) {
		//
		pConn->IncrBackEndConsumeNum();
	};
	pConn->IncrFrontEndProduceNum();

	Commit(
		net_cmd_t::DISPOSE_CONNECTION,
		pConn,
		s_vEmpty,
		(uintptr_t)0,
		0L,
		(uint8_t)0,
		s_sEmpty,
		s_sEmpty,
		std::move(resultCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::PostPacket(ITcpConn *pClient, uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody) {
	// reference and dereference
	auto resultCb = [pClient](int r) {
		//
		pClient->IncrBackEndConsumeNum();
	};
	pClient->IncrFrontEndProduceNum();
	
	Commit(
		net_cmd_t::POST_PACKET,
		pClient,
		s_vEmpty,
		(uintptr_t)0,
		uInnerUuid,
		uSerialNo,
		std::move(sTypeName),
		std::move(sBody),
		std::move(resultCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::ConfirmClientIsReady(ITcpClient *pClient, uintptr_t streamptr) {
	// reference and dereference
	auto resultCb = [pClient](int r) {
		//
		pClient->IncrBackEndConsumeNum();
	};
	pClient->IncrFrontEndProduceNum();

	Commit(
		net_cmd_t::CONFIRM_CLIENT_IS_READY,
		pClient,
		s_vEmpty,
		streamptr,
		0L,
		(uint8_t)0,
		s_sEmpty,
		s_sEmpty,
		std::move(resultCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::PostBroadcastPacket(ITcpServer *pServer, std::vector<uint64_t>& vTarget, std::string& sTypeName, std::string& sBody) {
	// 
	auto resultCb = [](int r) {	};

	Commit(
		net_cmd_t::POST_BROADCAST_PACKET,
		pServer,
		std::move(vTarget),
		(uintptr_t)0,
		0L,
		(uint8_t)0,
		std::move(sTypeName),
		std::move(sBody),
		std::move(resultCb));
}

//------------------------------------------------------------------------------
/**

*/
int
CKjConnFactoryWorkQueue::IsolatedConnConnect(ITcpIsolated *pIsolated, std::string sIp_or_Hostname, unsigned short nPort) {

	return BlockingCommit(
		net_cmd_t::ISOLATED_CONN_CONNECT,
		pIsolated,
		sIp_or_Hostname,
		nPort);
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::IsolatedConnFlush(ITcpIsolated *pIsolated) {

	// reference and dereference
	auto resultCb = [pIsolated](int r) {
		//
		pIsolated->IncrBackEndConsumeNum();
	};
	pIsolated->IncrFrontEndProduceNum();

	Commit(
		net_cmd_t::ISOLATED_FLUSH_STREAM,
		pIsolated,
		s_vEmpty,
		0,
		0L,
		(uint8_t)0,
		s_sEmpty,
		s_sEmpty,
		std::move(resultCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::IsolatedConnDelayReconnect(ITcpIsolated *pIsolated, int nDelaySeconds) {

	// reference and dereference
	auto resultCb = [pIsolated](int r) {
		//
		pIsolated->IncrBackEndConsumeNum();
	};
	pIsolated->IncrFrontEndProduceNum();

	Commit(
		net_cmd_t::ISOLATED_DELAY_RECONNECT,
		pIsolated,
		s_vEmpty,
		nDelaySeconds,
		0L,
		(uint8_t)0,
		s_sEmpty,
		s_sEmpty,
		std::move(resultCb));
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::InitTasks() {

	auto paf = kj::newPromiseAndFulfiller<void>();

	// check quit
	s_tls_worker->_tasks->add(
		check_quit_loop(
			*this,
			*s_tls_worker,
			paf.fulfiller.get()));

	// read pipe
	s_tls_worker->_tasks->add(
		read_pipe_loop(
			*this,
			*s_tls_worker,
			*s_tls_worker->_refStream));

	//
	paf.promise.wait(s_tls_worker->_tioContext->GetWaitScope());
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::Start() {
	//
	_pipeThread = _rootContext->NewPipeThread(
		[this](kj::AsyncIoProvider& ioProvider, kj::AsyncIoStream& stream, kj::WaitScope& waitScope) {
		//
		Run(ioProvider, stream, waitScope);
	});
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::Run(kj::AsyncIoProvider& ioProvider, kj::AsyncIoStream& stream, kj::WaitScope& waitScope) {
	// thread entry
	s_tls_worker = new gate_conn_factory_thread_worker_t;
	s_tls_worker->_refStream = &stream;
	s_tls_worker->_tioContext = kj::refcounted<KjSimpleThreadIoContext>(ioProvider, stream, waitScope);
	s_tls_worker->_tasks = s_tls_worker->_tioContext->CreateTaskSet(*this);

	//
	InitTasks();

	// thread dispose
	delete s_tls_worker;
	s_tls_worker = nullptr;
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::Commit(
	net_cmd_t::CMD_TYPE eType,
	void *pHandle,
	std::vector<uint64_t>& vTarget,
	uintptr_t streamptr,
	uint64_t uInnerUuid,
	uint8_t uSerialNo,
	std::string& sTypeName,
	std::string& sBody,
	result_cb_t&& resultCb) {

	auto cp = CreateCmdPipeline(
		++_nextSn,
		eType,
		pHandle,
		"",
		0,
		std::move(vTarget),
		streamptr,
		uInnerUuid,
		uSerialNo,
		std::move(sTypeName),
		std::move(sBody),
		std::move(resultCb)
	);
	Add(std::move(cp));
}

//------------------------------------------------------------------------------
/**

*/
int
CKjConnFactoryWorkQueue::BlockingCommit(net_cmd_t::CMD_TYPE eType, void *pHandle, const std::string& sIp_or_Hostname, unsigned short nPort) {

	int nResult = 0;

	auto prms = std::make_shared<std::promise<void>>();
	auto resultCb = [&nResult, &prms](int r) {
		nResult = r;
		prms->set_value();
	};

	auto cp = CreateCmdPipeline(
		++_nextSn,
		eType,
		pHandle,
		sIp_or_Hostname,
		nPort,
		s_vEmpty,
		(uintptr_t)0,
		0L,
		(uint8_t)0,
		s_sEmpty,
		s_sEmpty,
		std::move(resultCb)
	);
	Add(std::move(cp));

	prms->get_future().get();
	return nResult;
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::taskFailed(kj::Exception&& exception) {
	// alert
	StdLog *pLog = _refConnFactory->GetLogHandler();
	if (pLog)
		pLog->logprint(LOG_LEVEL_ALERT, "\n\n\n!!![CKjConnFactoryWorkQueue::taskFailed()] exception_desc(%s)!!!\n\n\n",
			exception.getDescription().cStr());

	fprintf(stderr, "\n\n\n!!![CKjConnFactoryWorkQueue::taskFailed()] exception_desc(%s)!!!\n\n\n", 
		exception.getDescription().cStr());

	// reset tasks
	InitTasks();
}

/** -- EOF -- **/