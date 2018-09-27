/*
 * \file: SocketServer.cpp
 * \brief: Created by hushouguo at 01:55:44 Aug 09 2018
 */

#include "bundle.h"
#include "Helper.h"
#include "Socket.h"
#include "Poll.h"

#define MAX_CONNECTION_FD				65536
#define ASSERT_SOCKET(S)				assert(S >= 0 && S < MAX_CONNECTION_FD)
#define WAKE_WORKER_PROCESS_SIGNAL		SIGRTMIN

BEGIN_NAMESPACE_BUNDLE {
	class SocketServerInternal : public SocketServer {
		public:
			SocketServerInternal(std::function<int(const Byte*, size_t)> splitMessage);
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
			struct SlotProcess {
				Poll poll;
				Socket* sockets[MAX_CONNECTION_FD];
				SOCKET maxfd = BUNDLE_INVALID_SOCKET;
				std::thread* threadWorker = nullptr;
				LockfreeQueue<Socketmessage> writeQueue;
				Spinlocker fdslocker;
				std::list<SOCKET> fdslist;
				std::function<void(SlotProcess*, SOCKET)> readSocket = nullptr;
				std::function<void(SlotProcess*, SOCKET)> writeSocket = nullptr;
				std::function<void(SlotProcess*, SOCKET)> addSocket = nullptr;
				std::function<void(SlotProcess*, SOCKET)> removeSocket = nullptr;
				std::function<void(SlotProcess*)> doMessage = nullptr;
				std::function<void(SlotProcess*, SOCKET, Socketmessage*)> sendMessage = nullptr;
				std::function<void(SlotProcess*, Socketmessage*)> broadcastMessage = nullptr;
				std::function<void(SlotProcess*)> checkSocket = nullptr;
				std::function<void(SlotProcess*)> acceptSocket = nullptr;
				u32 id = -1;
			};
			LockfreeQueue<Socketmessage> _readQueue;
			std::vector<SlotProcess*> _slotProcesses;
			void releaseProcess();
			SlotProcess* getWorkerProcess(SOCKET);

			SOCKET _fd_listening = BUNDLE_INVALID_SOCKET;
			bool _stop = true;
			size_t _opts[BUNDLE_SOL_MAX];
			std::atomic<size_t> _totalConnections = {0};
						
			void pushMessage(const Socketmessage* msg);

			std::function<int(const Byte*, size_t)> _splitMessage = nullptr;

		public:
			void workerProcess(SlotProcess* slot);
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

		this->releaseProcess();

		// make master process
		SlotProcess* slot = new SlotProcess();
		slot->id = 0; // master process id
		memset(slot->sockets, 0, sizeof(slot->sockets));
		slot->threadWorker = new std::thread([this](SlotProcess* slot) {
				this->workerProcess(slot);
			}, slot);

		slot->readSocket = [this](SlotProcess* slot, SOCKET s) {
			assert(s == this->fd());
			while (!this->isstop()) {
				struct sockaddr_in addr;
				socklen_t len = sizeof(addr);
				SOCKET newfd = ::accept(s, (struct sockaddr*)&addr, &len);
				if (newfd < 0) {
					if (interrupted()) {
						continue;
					}
					if (wouldblock()) {
						break; // no more connection
					}
					CHECK_RETURN(false, void(0), "accept error:%d,%s", errno, strerror(errno));
				}

				SlotProcess* slot_worker = this->getWorkerProcess(newfd);
				if (slot_worker == slot) {
					slot_worker->fdslist.push_back(newfd);
				}
				else {
					slot_worker->fdslocker.lock();
					slot_worker->fdslist.push_back(newfd);
					slot_worker->fdslocker.unlock();
					if (slot_worker->threadWorker) {
						pthread_kill(slot_worker->threadWorker->native_handle(), WAKE_WORKER_PROCESS_SIGNAL);
					}
				}
			}
		};

		slot->removeSocket = [this](SlotProcess* slot, SOCKET s) {
			assert(s == this->fd());
			Error << "listen fd should not be removed!";
		};
		this->_slotProcesses.push_back(slot);

		//
		// add fd listening to poll
		//
		slot->poll.addSocket(this->fd());

		// make worker process
		for (u32 i = 0; i < this->_workerNumber; ++i) {
			SlotProcess* slot = new SlotProcess();
			slot->id = i + 1; // worker process id
			memset(slot->sockets, 0, sizeof(slot->sockets));
			slot->threadWorker = new std::thread([this](SlotProcess* slot) {
				this->workerProcess(slot);
			}, slot);
						
			slot->readSocket = [this](SlotProcess* slot, SOCKET s) {
				ASSERT_SOCKET(s);
				Socket* so = slot->sockets[s];
				while (!this->isstop() && so) {
					Socketmessage* newmsg = nullptr;
					if (!so->receiveMessage(newmsg)) {
						//
						// read socket error
						//
						slot->removeSocket(slot, s);
						return;
					}
					if (newmsg) {
						//
						// get newmsg from Socket
						//
						this->_readQueue.push_back(newmsg);
						Debug << "newmsg from slot: " << slot->id;
					}
					else {
						return; }
				}			
			};
			
			slot->writeSocket = [this](SlotProcess* slot, SOCKET s) {
				ASSERT_SOCKET(s);
				Socket* so = slot->sockets[s];
				if (so && !so->sendMessage()) {
					//
					// write socket error
					//
					slot->removeSocket(slot, s);
				}
			};

			slot->sendMessage = [this](SlotProcess* slot, SOCKET s, Socketmessage* msg) {
				ASSERT_SOCKET(s);
				Socket* so = slot->sockets[s];
				if (so && !so->sendMessage(msg)) {
					//
					// write message to socket error
					//
					slot->removeSocket(slot, s);
				}
			};

			slot->addSocket = [this](SlotProcess* slot, SOCKET newfd) {
				ASSERT_SOCKET(newfd);
				if (this->_opts[BUNDLE_SOL_MAXSIZE] == 0 || this->_totalConnections < this->_opts[BUNDLE_SOL_MAXSIZE]) {
					Socket* so = slot->sockets[newfd];
					assert(so == nullptr);
					so = slot->sockets[newfd] = new Socket(newfd, this->_splitMessage);
					slot->poll.addSocket(so->fd());
					++this->_totalConnections;
					if (newfd > slot->maxfd) { slot->maxfd = newfd; }
					Socketmessage* msg = allocateMessage(newfd, SM_OPCODE_ESTABLISH);
					//
					// throw establish message
					//
					this->_readQueue.push_back(msg);
					Debug << "establish connection: " << newfd << ", slot: " << slot->id;
				}
				else { // exceeding this maximum connections limit
					::close(newfd); }
			};
			
			slot->removeSocket = [this](SlotProcess* slot, SOCKET s) {
				ASSERT_SOCKET(s);
				Socket* so = slot->sockets[s];
				assert(so != nullptr);
				slot->poll.removeSocket(s);
				slot->sockets[s] = nullptr;
				SafeDelete(so);
				--this->_totalConnections;
				Socketmessage* msg = allocateMessage(s, SM_OPCODE_CLOSE);
				//
				// throw close message
				//
				this->_readQueue.push_back(msg);
				Debug << "lost connection: " << s << ", slot: " << slot->id;
			};

			slot->broadcastMessage = [this](SlotProcess* slot, Socketmessage* msg) {
				assert(msg);
				assert(msg->payload_len > 0);
				for (SOCKET s = 0; s < slot->maxfd; ++s) {
					ASSERT_SOCKET(s);
					Socket* so = slot->sockets[s];
					if (!so) {
						continue;
					}				
					Socketmessage* newmsg = allocateMessage(s, msg->opcode, msg->payload, msg->payload_len);
					slot->sendMessage(slot, s, newmsg);
				}
				//
				// release origin msg
				//
				bundle::releaseMessage(msg);			
			};

			slot->doMessage = [this](SlotProcess* slot) {
				while (slot->writeQueue.size() > 0) {
					Socketmessage* msg = slot->writeQueue.pop_front();
					assert(msg);
					assert(msg->magic == MAGIC);
					if (msg->s == BUNDLE_BROADCAST_SOCKET) {
						slot->broadcastMessage(slot, msg);
					}
					else {
						ASSERT_SOCKET(msg->s);
						Socket* so = slot->sockets[msg->s];
						
						if (so == nullptr) {
							//
							// socket maybe already be closed
							//
							bundle::releaseMessage(msg);
						}
						else {
							switch (msg->opcode) {
								case SM_OPCODE_CLOSE: 
									slot->removeSocket(slot, msg->s);
									bundle::releaseMessage(msg);
									break;
									
								case SM_OPCODE_MESSAGE: 
									slot->sendMessage(slot, msg->s, msg); 
									break;
									
								default: 
								case SM_OPCODE_ESTABLISH: assert(false); break;
							}					
						}
					}
				}				
			};

			slot->checkSocket = [this](SlotProcess* slot) {
				if (this->_opts[BUNDLE_SOL_SILENCE_SECOND] > 0 || this->_opts[BUNDLE_SOL_THRESHOLD_MESSAGE] > 0) {
					u64 nowtime = timeSecond();
					for (SOCKET s = 0; s <= slot->maxfd; ++s) {
						ASSERT_SOCKET(s);
						Socket* so = slot->sockets[s];
						if (!so) {
							continue;
						}
				
						if (this->_opts[BUNDLE_SOL_SILENCE_SECOND] > 0 && (nowtime - so->lastSecond()) >= this->_opts[BUNDLE_SOL_SILENCE_SECOND]) {
							Alarm.cout("Connection:%d, No messages has been received in the last %ld seconds, allow: (%ld)", s, nowtime - so->lastSecond(), this->_opts[BUNDLE_SOL_SILENCE_SECOND]);
							//
							// silence time too long
							//
							slot->removeSocket(slot, s);
						}
						else if (this->_opts[BUNDLE_SOL_THRESHOLD_MESSAGE] > 0 && this->_opts[BUNDLE_SOL_THRESHOLD_INTERVAL] > 0) {
							u32 total = so->recentMessage(this->_opts[BUNDLE_SOL_THRESHOLD_INTERVAL]);
							if (total > this->_opts[BUNDLE_SOL_THRESHOLD_MESSAGE]) {
								Alarm.cout("Connection:%d, More than %d(%ld) messages have been received in the last %ld seconds", s, total, this->_opts[BUNDLE_SOL_THRESHOLD_MESSAGE], this->_opts[BUNDLE_SOL_THRESHOLD_INTERVAL]);
								//
								// send message too fast
								//
								slot->removeSocket(slot, s);
							}
						}
					}
				}
			};

			slot->acceptSocket = [this](SlotProcess* slot) {
				while (!slot->fdslist.empty()) {
					slot->fdslocker.lock();
					SOCKET newfd = slot->fdslist.front();
					slot->fdslist.pop_front();
					slot->fdslocker.unlock();
					slot->addSocket(slot, newfd);
				}
			};
			
			this->_slotProcesses.push_back(slot);
		}
				
		Debug.cout("SocketServer listening on %s:%d with workerProcess: %d", address, port, this->_workerNumber);
		
		return true;
	}

	void SocketServerInternal::workerProcess(SlotProcess* slot) {
		setSignal(WAKE_WORKER_PROCESS_SIGNAL);
			
		while (!this->isstop()) {
			if (slot->checkSocket != nullptr) {
				slot->checkSocket(slot);
			}

			if (slot->acceptSocket != nullptr) {
				slot->acceptSocket(slot);
			}

			if (slot->doMessage) {
				slot->doMessage(slot);
			}
			
			slot->poll.run(-1, 
					[slot](SOCKET s) {
						if (slot->readSocket) {
							slot->readSocket(slot, s);
						}
					},
					[slot](SOCKET s) {
						if (slot->writeSocket) {
							slot->writeSocket(slot, s);
						}
					},
					[slot](SOCKET s) {
						if (slot->removeSocket) {
							slot->removeSocket(slot, s);
						}
					});
		}
		
		Debug << "workerProcess thread exit, maxfd: " << slot->maxfd << ", writeQueue: " << slot->writeQueue.size() << ", fdslist: " << slot->fdslist.size();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	
	const Socketmessage* SocketServerInternal::receiveMessage(SOCKET& s, bool& establish, bool& close) {
		s = BUNDLE_INVALID_SOCKET;
		establish = close = false;
		const Socketmessage* msg = this->_readQueue.pop_front();
		if (msg) {
			assert(msg->magic == MAGIC);
			assert(msg->s != BUNDLE_INVALID_SOCKET);
			s = msg->s;
			switch (msg->opcode) {
				case SM_OPCODE_ESTABLISH: establish = true; break;;
				case SM_OPCODE_CLOSE: close = true; break;;
				case SM_OPCODE_MESSAGE: break;;
				default: assert(false); break;
			}
		}
		return msg;
	}

	void SocketServerInternal::close(SOCKET s) {
		Socketmessage* msg = allocateMessage(s, SM_OPCODE_CLOSE);
		this->pushMessage(msg);
	}
	
	void SocketServerInternal::sendMessage(SOCKET s, const void* payload, size_t payload_len) {
		assert(payload);
		assert(payload_len > 0);
		Socketmessage* msg = allocateMessage(s, SM_OPCODE_MESSAGE, payload, payload_len);
		this->pushMessage(msg);
	}

	void SocketServerInternal::sendMessage(SOCKET s, const Socketmessage* msg) {
		((Socketmessage*)msg)->s = s;
		this->pushMessage(msg);
	}

	void SocketServerInternal::pushMessage(const Socketmessage* msg) {
		assert(msg->s != BUNDLE_INVALID_SOCKET);
		auto slot = this->getWorkerProcess(msg->s);
		slot->writeQueue.push_back((Socketmessage*)msg);
		if (slot->threadWorker) {
			pthread_kill(slot->threadWorker->native_handle(), WAKE_WORKER_PROCESS_SIGNAL);
		}
	}

	SocketServerInternal::SlotProcess* SocketServerInternal::getWorkerProcess(SOCKET s) {
		assert(this->_slotProcesses.empty() == false);
		return this->_slotProcesses.size() == 1 ? this->_slotProcesses[0] 
					: this->_slotProcesses[(s % (this->_slotProcesses.size() - 1)) + 1];
	}

	void SocketServerInternal::releaseProcess() {
		for (auto& slot : this->_slotProcesses) {
			// destroy worker thread
			SafeDelete(slot->threadWorker);
		
			// close connfd
			for (auto s : slot->fdslist) {
				::close(s);
			}
			slot->fdslist.clear();
		
			// release Socket object
			for (auto& so : slot->sockets) {
				SafeDelete(so);
			}

			// clean writeQueue
			while (slot->writeQueue.size() > 0) {
				const Socketmessage* msg = slot->writeQueue.pop_front();
				assert(msg);
				assert(msg->magic == MAGIC);
				bundle::releaseMessage(msg);
			}
			
			SafeDelete(slot);
		}
		this->_slotProcesses.clear();	
	}
		
	void SocketServerInternal::stop() {
		if (!this->isstop()) {
			this->_stop = true;
			for (auto& slot : this->_slotProcesses) {
				if (slot->threadWorker) {
					pthread_kill(slot->threadWorker->native_handle(), WAKE_WORKER_PROCESS_SIGNAL);
					if (slot->threadWorker->joinable()) {
						slot->threadWorker->join();
					}
				}
			}
			
			this->releaseProcess();

			// release readQueue messages
			for (;;) {
				Socketmessage* msg = this->_readQueue.pop_front();
				if (!msg) {
					break;
				}
				bundle::releaseMessage(msg);
			}

			// close listening port
			if (this->_fd_listening != BUNDLE_INVALID_SOCKET) {
				::close(this->_fd_listening);
				this->_fd_listening = BUNDLE_INVALID_SOCKET;
			}
		}
	}

	size_t SocketServerInternal::size() {
		return this->_totalConnections;
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

	SocketServerInternal::SocketServerInternal(std::function<int(const Byte*, size_t)> splitMessage) {
		this->_splitMessage = splitMessage;
		memset(this->_opts, 0, sizeof(this->_opts));
	}

	SocketServer::~SocketServer() {}
	SocketServerInternal::~SocketServerInternal() { 
		this->stop(); 
	}

	SocketServer* SocketServerCreator::create(std::function<int(const Byte*, size_t)> splitMessage) {
		return new SocketServerInternal(splitMessage);
	}
}
