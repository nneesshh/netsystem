//------------------------------------------------------------------------------
//  KjConnFactoryWorkQueue.cpp
//  (C) 2016 n.lee
//------------------------------------------------------------------------------
#include "KjConnFactoryWorkQueue.hpp"

#include <future>
#include "servercore/io/KjPipeEndpointIoContext.hpp"

#include "KjConnFactory.hpp"

//////////////////////////////////////////////////////////////////////////
struct conn_factory_thread_env_t {
	svrcore_pipeworker_t           *worker;

	kj::Own<kj::TaskSet>            tasks;
};
static thread_local conn_factory_thread_env_t *stl_env = nullptr;

static kj::Promise<void>
check_quit_loop(CKjConnFactoryWorkQueue& q, conn_factory_thread_env_t& env, kj::PromiseFulfiller<void> *fulfiller) {
	if (!q.IsDone()) {
		// delay and check quit loop -- wait 500 ms
		return env.worker->endpointContext->GetTimer().afterDelay(500 * kj::MICROSECONDS)
			.then([&q, &env, fulfiller]() {
			// loop
			return check_quit_loop(q, env, fulfiller);
		});
	}

	//
	fulfiller->fulfill();
	return kj::READY_NOW;
}

static kj::Promise<void>
read_pipe_loop(CKjConnFactoryWorkQueue& q, conn_factory_thread_env_t& env, kj::AsyncIoStream& stream) {

	if (!q.IsDone()) {
		return stream.tryRead(&q._thrOpCodeRecvBuf, 1, 1024)
			.then([&q, &env, &stream](size_t amount) {
			//
			// Get next work item.
			//
			while (q.Callbacks().try_dequeue(q._opCmd)) {

				auto& cmd = q._opCmd;
				int r = 0;

				switch (cmd._type) {
				case CKjConnFactoryWorkQueue::net_cmd_t::OPEN_TCP_SERVER: {

					ITcpServer *pServer = static_cast<ITcpServer *>(cmd._handle);
					r = pServer->Open(env.worker->endpointContext.get(), cmd._port);
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
					// "pConn->PostPacket" always success even if the connection is not ready
					ITcpConn *pConn = static_cast<ITcpConn *>(cmd._handle);
					pConn->PostPacket(cmd._uInnerUuid, cmd._uSerialNo, std::move(cmd._typeName), std::move(cmd._body));
					break;
				}

				case CKjConnFactoryWorkQueue::net_cmd_t::FLUSH_TCP_SERVER_DOWNSTREAM: {
					//
					ITcpServer *pServer = static_cast<ITcpServer *>(cmd._handle);
					uintptr_t streamptr = cmd._streamptr;
					pServer->FlushDownStream(streamptr);
					break;
				}

				case CKjConnFactoryWorkQueue::net_cmd_t::CONFIRM_CLIENT_IS_READY: {

					ITcpClient *pClient = static_cast<ITcpClient *>(cmd._handle);
					uintptr_t streamptr = cmd._streamptr;
					if (pClient->IsReady()) {
						pClient->ConfirmClientIsReady(env.worker->endpointContext.get(), streamptr);
					}
					break;
				}

				case CKjConnFactoryWorkQueue::net_cmd_t::POST_BROADCAST_PACKET: {

					ITcpServer *pServer = static_cast<ITcpServer *>(cmd._handle);
					if (!pServer->IsClosed()) {
						//

					}
					break;
				}

				case CKjConnFactoryWorkQueue::net_cmd_t::ISOLATED_CONN_CONNECT: {
					
					ITcpIsolated *pIsolated = static_cast<ITcpIsolated *>(cmd._handle);
					pIsolated->Connect(env.worker->endpointContext.get(), cmd._ip_or_hostname, cmd._port);
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
			return read_pipe_loop(q, env, stream);
		});
	}
	return kj::READY_NOW;
}

static std::vector<uint64_t> s_vEmpty;
static std::string s_sEmpty;

//------------------------------------------------------------------------------
/**

*/
CKjConnFactoryWorkQueue::CKjConnFactoryWorkQueue(CKjConnFactoryBase *pFactory)
	: _refConnFactory(pFactory)
	, _callbacks(256) {

}

//------------------------------------------------------------------------------
/**

*/
CKjConnFactoryWorkQueue::~CKjConnFactoryWorkQueue() {
	_refConnFactory = nullptr;
}

//------------------------------------------------------------------------------
/**

*/
void
CKjConnFactoryWorkQueue::Run(svrcore_pipeworker_t *worker) {
	// 
	stl_env = new conn_factory_thread_env_t;
	stl_env->worker = worker;
	stl_env->tasks = netsystem_get_servercore()->NewTaskSet(*this);

	//
	InitTasks();

	// thread dispose
	stl_env->tasks = nullptr;

	delete stl_env;
	stl_env = nullptr;

	_finished = true;
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

	// write opcode to trunk pipe
	++_thrOpCodeSend;
	netsystem_get_servercore()->PipeNotify(*_refConnFactory->_refPipeWorker->pipeThread.pipe.get(), _thrOpCodeSend);
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

	// wait until finished
	while (!_finished) {
		util_sleep(10);
	}
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
CKjConnFactoryWorkQueue::FlushTcpServerDownStream(ITcpServer *pServer, uintptr_t streamptr) {
	// 
	auto resultCb = [](int r) {};

	Commit(
		net_cmd_t::FLUSH_TCP_SERVER_DOWNSTREAM,
		pServer,
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
CKjConnFactoryWorkQueue::PostPacket(ITcpConn *pClient, uint64_t uInnerUuid, uint8_t uSerialNo, std::string&& sTypeName, std::string&& sBody) {
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
CKjConnFactoryWorkQueue::PostBroadcastPacket(ITcpServer *pServer, std::vector<uint64_t>& vTarget, std::string&& sTypeName, std::string&& sBody) {
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
	stl_env->tasks->add(
		check_quit_loop(
			*this,
			*stl_env,
			paf.fulfiller.get()));

	// read pipe
	stl_env->tasks->add(
		read_pipe_loop(
			*this,
			*stl_env,
			stl_env->worker->endpointContext->GetEndpoint()));

	//
	paf.promise.wait(stl_env->worker->endpointContext->GetWaitScope());
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
	// fatal
	StdLog *pLog = netsystem_get_log();
	if (pLog)
		pLog->logprint(LOG_LEVEL_FATAL, "\n\n\n!!![CKjConnFactoryWorkQueue::taskFailed()] exception_desc(%s)!!!\n\n\n",
			exception.getDescription().cStr());

	fprintf(stderr, "\n\n\n!!![CKjConnFactoryWorkQueue::taskFailed()] exception_desc(%s)!!!\n\n\n", 
		exception.getDescription().cStr());
	kj::throwFatalException(kj::mv(exception));
}

/** -- EOF -- **/