#pragma once
//------------------------------------------------------------------------------
/**
@class CKjConnFactoryWorkQueue

(C) 2016 n.lee
*/
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <thread>

#include "base/concurrent/readerwriterqueue.h"

#include "KjSimpleIoContext.hpp"
#include "KjSimpleThreadIoContext.hpp"

#include "../ITcpConnFactory.h"
#include "../ITcpServer.h"
#include "../ITcpClient.h"
#include "../ITcpIsolated.h"

//------------------------------------------------------------------------------
/**
@brief CKjConnFactoryWorkQueue
*/
class CKjConnFactoryWorkQueue : public kj::TaskSet::ErrorHandler {
public:
	explicit CKjConnFactoryWorkQueue(ITcpConnFactory *pFactory, kj::Own<KjSimpleIoContext> rootContext);
	~CKjConnFactoryWorkQueue();

	using result_cb_t = std::function<void(int)>;

	struct net_cmd_t {
		/*cmd type */
		enum CMD_TYPE {
			OPEN_TCP_SERVER = 1,
			CLOSE_TCP_SERVER = 2,
			DISPOSE_CONNECTION = 3,
			POST_PACKET = 4,

			CONFIRM_CLIENT_IS_READY = 11,
			POST_BROADCAST_PACKET = 12,

			ISOLATED_CONN_CONNECT = 21,
			ISOLATED_FLUSH_STREAM = 22,
			ISOLATED_DELAY_RECONNECT = 23,
		};

		/*cmd state */
		enum CMD_STATE {
			QUEUEING = 1,
			SENDING = 2,
			COMMITTING = 3,
			PROCESSING = 4,
			PROCESS_OVER = 5,
		};

		int _sn;
		CMD_TYPE _type;

		void *_handle;
		std::string _ip_or_hostname;
		unsigned short _port;
		std::vector<uint64_t> _targetList;

		uintptr_t _streamptr;
		uint64_t _uInnerUuid;
		uint8_t _uSerialNo;
		std::string _typeName;
		std::string _body;
		result_cb_t _resultCb;
		CMD_STATE _state;
	};
	using CallbackEntry = net_cmd_t;

	bool Add(net_cmd_t&& cmd);

	bool IsDone() {
		return _done;
	}

	moodycamel::ReaderWriterQueue<CallbackEntry>& Callbacks() {
		return _callbacks;
	}

	void Finish();

	int OpenTcpServer(ITcpServer *pServer, unsigned short nPort);
	void CloseTcpServer(ITcpServer *pServer);
	void DisposeConnection(ITcpConn *pConn);
	void PostPacket(ITcpConn *pConn, uint64_t uInnerUuid, uint8_t uSerialNo, std::string& sTypeName, std::string& sBody);

	void ConfirmClientIsReady(ITcpClient *pClient, uintptr_t streamptr);
	void PostBroadcastPacket(ITcpServer *pServer, std::vector<uint64_t>& vTarget, std::string& sTypeName, std::string& sBody);

	int IsolatedConnConnect(ITcpIsolated *pIsolated, std::string sIp_or_Hostname, unsigned short nPort);
	void IsolatedConnFlush(ITcpIsolated *pIsolated);
	void IsolatedConnDelayReconnect(ITcpIsolated *pIsolated, int nDelaySeconds);

private:
	void InitTasks();
	void Start();
	void Run(kj::AsyncIoProvider& ioProvider, kj::AsyncIoStream& stream, kj::WaitScope& waitScope);

	void Commit(
		net_cmd_t::CMD_TYPE eType,
		void *pHandle,
		std::vector<uint64_t>& vTarget,
		uintptr_t streamptr,
		uint64_t uInnerUuid,
		uint8_t uSearialNo,
		std::string& sTypeName,
		std::string& sBody,
		result_cb_t&& resultCb);

	int BlockingCommit(net_cmd_t::CMD_TYPE eType, void *pHandle, const std::string& sIp_or_Hostname, unsigned short nPort);

public:
	static net_cmd_t CreateCmdPipeline(
		int nSn,
		net_cmd_t::CMD_TYPE eType,
		void *pHandle,
		const std::string& sIp_or_Hostname,
		unsigned short nPort,
		std::vector<uint64_t>& vTarget,
		uintptr_t streamptr,
		uint64_t uInnerUuid,
		uint8_t uSerialNo,
		std::string& sTypeName,
		std::string& sBody,
		result_cb_t&& resultCb) {

		net_cmd_t cp;
		cp._sn = nSn;
		cp._type = eType;
		cp._handle = pHandle;
		cp._ip_or_hostname.append(sIp_or_Hostname);
		cp._port = nPort;
		cp._targetList = std::move(vTarget);
		cp._streamptr = streamptr;
		cp._uInnerUuid = uInnerUuid;
		cp._uSerialNo = uSerialNo;
		cp._typeName = std::move(sTypeName);
		cp._body = std::move(sBody);
		cp._resultCb = std::move(resultCb);
		cp._state = net_cmd_t::QUEUEING;
		return cp;
	}

private:
	//! 
	void taskFailed(kj::Exception&& exception) override;

private:
	ITcpConnFactory *_refConnFactory;
	kj::Own<KjSimpleIoContext> _rootContext;

	bool _done = false;
	moodycamel::ReaderWriterQueue<CallbackEntry> _callbacks;

	//! threads
	kj::AsyncIoProvider::PipeThread _pipeThread;

public:
	char _opCodeSend = 0;
	char _opCodeRecvBuf[1024];

	CallbackEntry _opCmd;
	int _nextSn = 0;
};
using CKjConnFactoryWorkQueuePtr = std::shared_ptr<CKjConnFactoryWorkQueue>;

/*EOF*/