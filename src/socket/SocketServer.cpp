/*
 * \file: SocketServer.cpp
 * \brief: Created by hushouguo at 01:55:44 Aug 09 2018
 */

#include "bundle.h"
#include "Helper.h"
#include "Socket.h"
#include "Poll.h"
#include "WorkerProcess.h"

#define MAX_CONNECTION_FD				65536
#define ASSERT_SOCKET(S)				assert(S >= 0 && S < MAX_CONNECTION_FD)
#define WAKE_WORKER_PROCESS_SIGNAL		SIGRTMIN

BEGIN_NAMESPACE_BUNDLE {
	class SocketServerInternal : public SocketServer {
		public:
			SocketServerInternal(MESSAGE_SPLITER splitMessage);
			~SocketServerInternal();

		public:
			SOCKET fd() override {	return this->_fd_listening; }
			bool setWorkerNumber(u32) override;
			bool start(const char* address, int port) override;
			void stop() override;
			inline bool isstop() { return this->_stop; }
			const Socketmessage* receiveMessage(SOCKET& s, bool& establish, bool& close) override;
			void sendMessage(SOCKET s, const void*, size_t) override;
			void sendMessage(SOCKET s, const Socketmessage*) override;
			void close(SOCKET s) override;
			size_t size() override;
			bool setsockopt(int opt, const void* optval, size_t optlen) override;
			bool getsockopt(int opt, void* optval, size_t optlen) override;

		private:
			u32 _workerNumber = std::thread::hardware_concurrency();
			LockfreeQueue<Socketmessage*> _readQueue;
			std::vector<WorkerProcess*> _processes;
			WorkerProcess* getWorkerProcess(SOCKET);

			SOCKET _fd_listening = BUNDLE_INVALID_SOCKET;
			bool _stop = true;
			size_t _opts[BUNDLE_SOL_MAX];

			MESSAGE_SPLITER _splitMessage = nullptr;
	};

	bool SocketServerInternal::setWorkerNumber(u32 worker_number) {
		CHECK_RETURN(this->_stop, false, "SockerServer is running, stop it at first!");
		CHECK_RETURN(worker_number < std::thread::hardware_concurrency() * 8, false, 
		"woker number: %d too large, hardware: %d, suggest: %d", worker_number, std::thread::hardware_concurrency(), std::thread::hardware_concurrency() * 2);
		this->_workerNumber = worker_number;
		return true;
	}

	bool SocketServerInternal::start(const char* address, int port) {
		CHECK_RETURN(this->_stop, false, "SocketServer is running, stop it at first!");
		this->_stop = false;
		if (this->_fd_listening != BUNDLE_INVALID_SOCKET) {
			::close(this->_fd_listening);
		}

		this->_fd_listening = ::socket(AF_INET, SOCK_STREAM, 0);
		CHECK_RETURN(this->_fd_listening >= 0, false, "create socket failure: %d, %s", errno, strerror(errno));
		bool rc = reuseableAddress(this->fd());
		CHECK_RETURN(rc, false, "reuseableAddress failure: %d, %s", errno, strerror(errno));
		rc = nonblocking(this->fd());
		CHECK_RETURN(rc, false, "nonblocking failure: %d, %s", errno, strerror(errno));

		struct sockaddr_in sockaddr;
		memset(&sockaddr, 0, sizeof(sockaddr));
		sockaddr.sin_family = AF_INET;
		inet_aton(address, &(sockaddr.sin_addr));
		sockaddr.sin_port = htons(port);

		int val = ::bind(this->fd(), (struct sockaddr*)&sockaddr, sizeof(sockaddr));
		CHECK_RETURN(val == 0, false, "bind failure: %d, %s", errno, strerror(errno));

		val = ::listen(this->fd(), SOMAXCONN);
		CHECK_RETURN(val == 0, false, "listen failure: %d, %s", errno, strerror(errno));

		for (auto slot : this->_processes) {
			SafeDelete(slot);
		}
		this->_processes.clear();

		// create master process
		WorkerProcess* master = new WorkerProcess(0, this->_splitMessage, &this->_readQueue);
		master->addSocket(this->_fd_listening, true);
		this->_processes.push_back(master);

		// make worker process
		for (u32 i = 0; i < this->_workerNumber; ++i) {
			WorkerProcess* slot = new WorkerProcess(i + 1, this->_splitMessage, &this->_readQueue);			
			this->_processes.push_back(slot);
		}
				
		Debug.cout("SocketServer listening on %s:%d with workerProcess: %d", address, port, this->_workerNumber);
		
		return true;
	}

	WorkerProcess* SocketServerInternal::getWorkerProcess(SOCKET s) {
		assert(this->_processes.empty() == false);
		return this->_processes.size() == 1 ? this->_processes[0] 
					: this->_processes[(s % (this->_processes.size() - 1)) + 1];
	}


	////////////////////////////////////////////////////////////////////////////////////////

	
	const Socketmessage* SocketServerInternal::receiveMessage(SOCKET& s, bool& establish, bool& close) {
		s = BUNDLE_INVALID_SOCKET;
		establish = close = false;
		while (!this->_readQueue.empty()) {
			Socketmessage* msg = this->_readQueue.pop_front();
			assert(msg);
			assert(msg->magic == MAGIC);
			assert(msg->s != BUNDLE_INVALID_SOCKET);
			s = msg->s;
			switch (msg->opcode) {
				case SM_OPCODE_ESTABLISH: establish = true; return msg;
				case SM_OPCODE_CLOSE: close = true; return msg;
				case SM_OPCODE_MESSAGE: return msg;
				case SM_OPCODE_NEW_SOCKET:
					if (true) {
						WorkerProcess* slot = this->getWorkerProcess(msg->s);
						slot->pushMessage(s, msg);
					}
					break;
				default:
				case SM_OPCODE_NEW_LISTENING: assert(false); break;
			}			
		}
		return nullptr;
	}

	void SocketServerInternal::close(SOCKET s) {
		assert(s != BUNDLE_INVALID_SOCKET);
		WorkerProcess* slot = this->getWorkerProcess(s);
		slot->closeSocket(s);
	}
	
	void SocketServerInternal::sendMessage(SOCKET s, const void* payload, size_t payload_len) {
		assert(s != BUNDLE_INVALID_SOCKET);
		WorkerProcess* slot = this->getWorkerProcess(s);
		slot->pushMessage(s, payload, payload_len);
	}

	void SocketServerInternal::sendMessage(SOCKET s, const Socketmessage* msg) {
		assert(s != BUNDLE_INVALID_SOCKET);
		WorkerProcess* slot = this->getWorkerProcess(s);
		slot->pushMessage(s, (Socketmessage*) msg);
	}

	void SocketServerInternal::stop() {
		if (!this->isstop()) {
			this->_stop = true;
			for (auto slot : this->_processes) {
				SafeDelete(slot);
			}

			// release readQueue messages
			for (;this->_readQueue.empty() == false;) {
				Socketmessage* msg = this->_readQueue.pop_front();
				assert(msg);
				bundle::releaseMessage(msg);
			}

			// close socket
			SafeClose(this->_fd_listening);
		}
	}

	size_t SocketServerInternal::size() {
		u32 value = 0;
		for (auto slot : this->_processes) {
			value += slot->totalConnections();
		}
		return value;
	}

	bool SocketServerInternal::setsockopt(int opt, const void* optval, size_t optlen) {
		CHECK_RETURN(opt > 0 && opt < BUNDLE_SOL_MAX, false, "illegal opt: %d", opt);
		CHECK_RETURN(optval, false, "illegal optval");
		CHECK_RETURN(optlen <= sizeof(size_t), false, "illegal optlen");
		memcpy(&this->_opts[opt], optval, optlen);
		return true;
	}
	
	bool SocketServerInternal::getsockopt(int opt, void* optval, size_t optlen) {
		CHECK_RETURN(opt > 0 && opt < BUNDLE_SOL_MAX, false, "illegal opt: %d", opt);
		CHECK_RETURN(optval, false, "illegal optval");
		CHECK_RETURN(optlen <= sizeof(size_t), false, "illegal optlen");
		memcpy(optval, &this->_opts[opt], optlen);
		return true;
	}	

	SocketServerInternal::SocketServerInternal(MESSAGE_SPLITER splitMessage) {
		this->_splitMessage = splitMessage;
		memset(this->_opts, 0, sizeof(this->_opts));
	}

	SocketServer::~SocketServer() {}
	SocketServerInternal::~SocketServerInternal() { 
		this->stop(); 
	}

	SocketServer* SocketServerCreator::create(MESSAGE_SPLITER splitMessage) {
		return new SocketServerInternal(splitMessage);
	}
}
