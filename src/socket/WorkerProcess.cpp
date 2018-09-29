/*
 * \file: WorkerProcess.cpp
 * \brief: Created by hushouguo at 05:42:03 Sep 22 2018
 */

#include "bundle.h"
#include "Helper.h"
#include "Socket.h"
#include "Poll.h"
#include "WorkerProcess.h"

#define GET_SOCKET(S) ({ ASSERT_SOCKET(S); this->_sockets[S]; })

BEGIN_NAMESPACE_BUNDLE {
	WorkerProcess::WorkerProcess(u32 id, MESSAGE_SPLITER splitMessage, LockfreeQueue<Socketmessage*>* recvQueue) 
		: Entry<u32>(id) {
		memset(this->_sockets, 0, sizeof(this->_sockets));
		this->_threadWorker = new std::thread([this]() {
				this->run();
				});
		this->_splitMessage = splitMessage;
		this->_recvQueue = recvQueue;
	}

	WorkerProcess::~WorkerProcess() {
		this->stop();
	}

	void WorkerProcess::stop() {
		if (!this->isstop()) {
			this->_stop = true;
			if (this->_threadWorker) {
				pthread_kill(this->_threadWorker->native_handle(), WAKE_WORKER_PROCESS_SIGNAL);
				if (this->_threadWorker->joinable()) {
					this->_threadWorker->join();
				}
			}

			// destroy worker thread
			SafeDelete(this->_threadWorker);
		
			// release Socket object
			for (auto& so : this->_sockets) {
				SafeDelete(so);
			}

			// clean sendQueue
			while (!this->_sendQueue.empty()) {
				const Socketmessage* msg = this->_sendQueue.pop_front();
				assert(msg);
				assert(msg->magic == MAGIC);
				if (msg->opcode == SM_OPCODE_NEW_SOCKET) {
					::close(msg->s);
				}
				bundle::releaseMessage(msg);
			}
		}
	}

	void WorkerProcess::run() {
		while (!this->isstop()) {
			this->checkSocket();
			this->handleMessage();
			this->_poll.run(-1, [this](SOCKET s) { this->readSocket(s); }, 
					[this](SOCKET s) { this->writeSocket(s); }, [this](SOCKET s) { this->errorSocket(s); });
		}
		Debug << "WorkerProcess: " << this->id << " thread exit, maxfd: " << this->_maxfd 
			<< ", recvQueue: " << this->_recvQueue->size() << ", sendQueue: " << this->_sendQueue.size();
	}

	void WorkerProcess::pushMessage(Socketmessage* msg) {
		this->_sendQueue.push_back(msg);
		if (this->_threadWorker) {
			pthread_kill(this->_threadWorker->native_handle(), WAKE_WORKER_PROCESS_SIGNAL);
		}
	}

	void WorkerProcess::addSocket(SOCKET s, bool is_listening) {
		Socketmessage* msg = allocateMessage(s, is_listening ? SM_OPCODE_NEW_LISTENING : SM_OPCODE_NEW_SOCKET);
		this->pushMessage(msg);
	}
	
	void WorkerProcess::closeSocket(SOCKET s) {
		Socketmessage* msg = allocateMessage(s, SM_OPCODE_CLOSE);
		this->pushMessage(msg);
	}
	
	void WorkerProcess::pushMessage(SOCKET s, Socketmessage* msg) {
		msg->s = s;
		this->pushMessage(msg);
	}

	void WorkerProcess::pushMessage(SOCKET s, const void* payload, size_t payload_len) {
		assert(payload);
		assert(payload_len > 0);
		Socketmessage* msg = allocateMessage(s, SM_OPCODE_MESSAGE, payload, payload_len);
		this->pushMessage(msg);	
	}
	
	void WorkerProcess::readSocket(SOCKET s) {
		Socket* so = GET_SOCKET(s);
		CHECK_RETURN(so, void(0), "readSocket: %d, not found Socket", s);
		if (so->is_listening()) {
			this->acceptSocket(so);	
		}
		else {
			this->readSocket(so);
		}
	}
	
	void WorkerProcess::acceptSocket(Socket* so) {
		assert(so);
		assert(so->is_listening());
		while (!this->isstop()) {
			struct sockaddr_in addr;
			socklen_t len = sizeof(addr);
			SOCKET newfd = ::accept(so->fd(), (struct sockaddr*)&addr, &len);
			if (newfd < 0) {
				if (interrupted()) {
					continue;
				}
				if (wouldblock()) {
					break; // no more connection
				}
				CHECK_RETURN(false, void(0), "accept error:%d,%s", errno, strerror(errno));
			}
			ASSERT_SOCKET(newfd);

			//
			// get newfd from Socket
			//
			Socketmessage* msg = allocateMessage(newfd, SM_OPCODE_NEW_SOCKET);
			this->_recvQueue->push_back(msg);
		}
	}

	void WorkerProcess::readSocket(Socket* so) {
		assert(so);
		assert(!so->is_listening());
		while (!this->isstop()) {
			Socketmessage* newmsg = nullptr;
			if (!so->receiveMessage(newmsg)) {
				//
				// read socket error happen
				//
				this->removeSocket(so->fd(), "readSocket error");
				return;
			}
			if (newmsg) {
				//
				// get newmsg from Socket
				//
				this->_recvQueue->push_back(newmsg);
				Debug << "slot: " << this->id << ", newmsg";
			}
			else {
				return; }
		}			
	}

	void WorkerProcess::writeSocket(SOCKET s) {
		Socket* so = GET_SOCKET(s);
		CHECK_RETURN(so, void(0), "writeSocket: %d, not found Socket", s);
		CHECK_RETURN(!so->is_listening(), void(0), "writeSocket: %d is listening", s);
		if (!so->sendMessage()) {
			//
			// write socket error happen
			//
			this->removeSocket(s, "writeSocket error");
		}
	}

	void WorkerProcess::errorSocket(SOCKET s) {
		Socket* so = GET_SOCKET(s);
		CHECK_RETURN(so, void(0), "errorSocket: %d, not found Socket", s);
		this->removeSocket(s, "errorSocket");
	}


	//========================================================================================
	//
	void WorkerProcess::newSocket(SOCKET newfd, bool is_listening) {
		Socket* so = GET_SOCKET(newfd);
		assert(so == nullptr);		
		this->_sockets[newfd] = so = new Socket(newfd, this->_splitMessage);		
		so->set_listening(is_listening);
		this->_poll.addSocket(newfd);
		if (newfd > this->_maxfd) { this->_maxfd = newfd; }
		Socketmessage* msg = allocateMessage(newfd, SM_OPCODE_ESTABLISH);
		//
		// throw establish message
		//
		this->_recvQueue->push_back(msg);
		++_totalConnections;
		Debug << "establish connection: " << newfd << ", worker: " << this->id;
	}

	void WorkerProcess::removeSocket(SOCKET s, const char* reason) {
		Socket* so = GET_SOCKET(s);
		CHECK_ALARM(so != nullptr, "socket: %d not exist", s);
		this->_sockets[s] = nullptr;
		SafeDelete(so);
		this->_poll.removeSocket(s);
		Socketmessage* msg = allocateMessage(s, SM_OPCODE_CLOSE);
		//
		// throw close message
		//
		this->_recvQueue->push_back(msg);
		assert(_totalConnections > 0);
		--_totalConnections;
		Debug << "lost connection: " << s << ", worker: " << this->id << ", reason: " << reason;
	}

	void WorkerProcess::sendMessage(SOCKET s, Socketmessage* msg) {
		Socket* so = GET_SOCKET(s);
		if (so && !so->sendMessage(msg)) {
			//
			// send message to socket error
			//
			this->removeSocket(s, "sendMessage");
		}
	}

	void WorkerProcess::multisendMessage(Socketmessage* msg) {
		assert(msg);
		assert(msg->payload_len > 0);
		for (SOCKET s = 0; s <= this->_maxfd; ++s) {
			ASSERT_SOCKET(s);
			Socket* so = this->_sockets[s];
			if (so) {
				Socketmessage* newmsg = allocateMessage(s, msg->opcode, msg->payload, msg->payload_len);
				this->sendMessage(s, newmsg);
			}
		}
		//
		// release origin msg
		//
		bundle::releaseMessage(msg);
	}

	void WorkerProcess::checkSocket() {
	}

	void WorkerProcess::handleMessage() {
		while (!this->_sendQueue.empty()) {
			Socketmessage* msg = this->_sendQueue.pop_front();
			assert(msg);
			assert(msg->magic == MAGIC);
			if (msg->s == -1) {
				this->multisendMessage(msg);
			}
			else {
				switch (msg->opcode) {
					case SM_OPCODE_CLOSE: 
						this->removeSocket(msg->s, "active");
						bundle::releaseMessage(msg);
						break;

					case SM_OPCODE_MESSAGE: 
						if (true) {
							Socket* so = GET_SOCKET(msg->s);
							if (!so) {
								//
								// socket maybe already be closed
								//
								bundle::releaseMessage(msg);
							}
							else {
								this->sendMessage(msg->s, msg); 
							}
						}
						break;

					case SM_OPCODE_NEW_SOCKET:
						this->newSocket(msg->s, false);
						bundle::releaseMessage(msg);
						break;

					case SM_OPCODE_NEW_LISTENING:
						this->newSocket(msg->s, true);
						bundle::releaseMessage(msg);
						break;
						
					default:
					case SM_OPCODE_ESTABLISH: assert(false); break;
				}					
			}
		}
	}
}
