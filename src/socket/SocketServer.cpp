/*
 * \file: SocketServer.cpp
 * \brief: Created by hushouguo at 01:55:44 Aug 09 2018
 */

#include "bundle.h"
#include "Helper.h"
#include "Socket.h"
#include "Poll.h"

#define MAX_CONNECTION_FD		65536
#define NM_CONNECTION_THREAD	4

BEGIN_NAMESPACE_BUNDLE {
	class SocketServerInternal : public SocketServer {
		public:
			SocketServerInternal(std::function<int(const Byte*, size_t)> splitMessage);
			~SocketServerInternal();

		public:
			SOCKET fd() override {	return this->_fd; }
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

		public:
			void acceptProcess();
			void connectionProcess(SlotConnection* slot);

		private:
			std::thread* _threadAccept = nullptr;
			LockfreeQueue<Socketmessage> _readQueue;
			
			struct SlotConnection {
				std::thread* threadConnection = nullptr;
				LockfreeQueue<Socketmessage> writeQueue;
				Spinlocker fdslocker;
				std::list<SOCKET> connfds;
			} _slotConnection[NM_CONNECTION_THREAD];			

			SOCKET _fd = BUNDLE_INVALID_SOCKET;
			bool _stop = true;
			size_t _opts[BUNDLE_SOL_MAX];
			std::atomic<size_t> _size = {0};
						
			void pushMessage(const Socketmessage* msg);

			std::function<int(const Byte*, size_t)> _splitMessage = nullptr;
	};
		
	bool SocketServerInternal::start(const char* address, int port) {
		CHECK_RETURN(this->_stop, false, "SocketServer is running, stop it at first!");
		this->_stop = false;
		if (this->_fd != BUNDLE_INVALID_SOCKET) {
			::close(this->_fd);
		}

		this->_fd = ::socket(AF_INET, SOCK_STREAM, 0);
		CHECK_RETURN(this->_fd >= 0, false, "create socket failure: %d, %s", errno, strerror(errno));
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

		for (auto& slot : this->_slotConnection) {
			SafeDelete(slot.threadConnection);
			slot.threadConnection = new std::thread([this]() {
				this->connectionProcess();
			}, &slot);
		}
				
		SafeDelete(this->_threadAccept);
		this->_threadAccept = new std::thread([this]() {
			this->acceptProcess();
		});
		
		Debug.cout("SocketServer listening on %s:%d", address, port);
		
		return true;
	}

	void SocketServerInternal::acceptProcess() {
		struct sigaction act;
        act.sa_handler = [](int sig) {
			Debug << "acceptProcess receive signal: " << sig;
		}; // sa_handler will not take effect if it is not set, different with connectSignal implement
        sigemptyset(&act.sa_mask);  
        sigaddset(&act.sa_mask, SIGTERM);
        act.sa_flags = SA_INTERRUPT; //The system call that is interrupted by this signal will not be restarted automatically
        sigaction(SIGTERM, &act, nullptr);

		blocking(this->_fd);
		while (!this->isstop()) {
			struct sockaddr_in clientaddr;
			socklen_t len = sizeof(clientaddr);
			SOCKET s = ::accept(this->_fd, (struct sockaddr*)&clientaddr, &len);
			if (s == 0) {
				CHECK_RETURN(false, void(0), "accept error:%d,%s", errno, strerror(errno));
			}
			else if (s < 0) {
				if (interrupted()) {
					continue;
				}
				if (wouldblock()) {
					//std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue; // no more connection
				}
				CHECK_RETURN(false, void(0), "accept error:%d,%s", errno, strerror(errno));
			}

			auto& slot = this->_slotConnection[s % NM_CONNECTION_THREAD];
			slot.fdslocker.lock();
			slot.connfds.push_back(s);
			slot.fdslocker.unlock();
		}
		Debug << "acceptProcess thread exit";
	}

	void SocketServerInternal::connectionProcess(SlotConnection* slot) {
		struct sigaction act;
        act.sa_handler = [](int sig) {
			Debug << "connectionProcess receive signal: " << sig;
		}; // sa_handler will not take effect if it is not set, different with connectSignal implement
        sigemptyset(&act.sa_mask);
        sigaddset(&act.sa_mask, SIGALRM);
        act.sa_flags = SA_INTERRUPT; //The system call that is interrupted by this signal will not be restarted automatically
        sigaction(SIGALRM, &act, nullptr);

	
		Poll poll;
		Socket* sockets[MAX_CONNECTION_FD];
		memset(sockets, 0, sizeof(sockets));
		SOCKET maxfd = BUNDLE_INVALID_SOCKET;
		
		auto getSocket = [&](SOCKET s) -> Socket* {
			assert(s >= 0 && s < MAX_CONNECTION_FD);
			return sockets[s];			
		};

		auto pushMessage = [&](Socketmessage* msg) {
			this->_readQueue.push_back(msg);
		};
		
		auto addSocket = [&](SOCKET s) {
			if (this->_opts[BUNDLE_SOL_MAXSIZE] == 0 || this->_size < this->_opts[BUNDLE_SOL_MAXSIZE]) {
				Socket* so = getSocket(s);
				assert(so == nullptr);
				so = sockets[s] = new Socket(s, this->_splitMessage);
				poll.addSocket(so->fd());
				++this->_size;
				if (s > maxfd) { maxfd = s; }
				Socketmessage* msg = allocateMessage(s, SM_OPCODE_ESTABLISH);
				pushMessage(msg);
			}
			else { ::close(s); }
		};
		
		auto removeSocket = [&](SOCKET s) {
			Socket* so = getSocket(s);
			assert(so != nullptr);
			poll.removeSocket(s);
			sockets[s] = nullptr;
			SafeDelete(so);
			--this->_size;
			Socketmessage* msg = allocateMessage(s, SM_OPCODE_CLOSE);
			pushMessage(msg);
		};
		
		auto readSocket = [&](SOCKET s) {
			Socket* so = getSocket(s);
			while (!this->isstop() && so) {
				Socketmessage* msg = nullptr;
				if (!so->receiveMessage(msg)) {
					removeSocket(s);
					return;
				}
				if (!msg) {	return; }
				pushMessage(msg);
			}
		};

		auto writeSocket = [&](SOCKET s) {
			Socket* so = getSocket(s);
			if (so && !so->sendMessage()) {
				removeSocket(s);
			}
		};

		auto writeMessage = [&](SOCKET s, const Socketmessage* msg) {
			Socket* so = getSocket(s);
			if (so && !so->sendMessage(msg)) {
				removeSocket(s);
			}
		};

		auto broadcastMessage = [&](const Socketmessage* msg) {
			assert(msg);
			assert(msg->payload_len > 0);
			for (SOCKET s = 0; s < maxfd; ++s) {
				Socket* so = getSocket(s);
				if (!so) { 
					continue;
				}

				Socketmessage* newmsg = allocateMessage(s, msg->opcode, msg->payload, msg->payload_len);
				writeMessage(s, newmsg);
			}
			bundle::releaseMessage(msg);
		};

		auto checkConnection = [&]() {
			if (this->_opts[BUNDLE_SOL_SILENCE_SECOND] > 0 || this->_opts[BUNDLE_SOL_THRESHOLD_MESSAGE] > 0) {
				u64 nowtime = timeSecond();
				for (SOCKET s = 0; s <= maxfd; ++s) {
					Socket* so = getSocket(s);
					if (!so) { 
						continue;
					}

					if (this->_opts[BUNDLE_SOL_SILENCE_SECOND] > 0 && (nowtime - so->lastSecond()) >= this->_opts[BUNDLE_SOL_SILENCE_SECOND]) {
						Alarm.cout("Connection:%d, No messages has been received in the last %ld seconds, allow: (%ld)", s, nowtime - so->lastSecond(), this->_opts[BUNDLE_SOL_SILENCE_SECOND]);
						removeSocket(s);
					}
					else if (this->_opts[BUNDLE_SOL_THRESHOLD_MESSAGE] > 0 && this->_opts[BUNDLE_SOL_THRESHOLD_INTERVAL] > 0) {
						u32 total = so->recentMessage(this->_opts[BUNDLE_SOL_THRESHOLD_INTERVAL]);
						if (total > this->_opts[BUNDLE_SOL_THRESHOLD_MESSAGE]) {
							Alarm.cout("Connection:%d, More than %d(%ld) messages have been received in the last %ld seconds", s, total, this->_opts[BUNDLE_SOL_THRESHOLD_MESSAGE], this->_opts[BUNDLE_SOL_THRESHOLD_INTERVAL]);
							removeSocket(s);
						}
					}
				}
			}
		};
		
		while (!this->isstop()) {
			checkConnection();
			
			while (!slot->connfds.empty()) {
				slot->fdslocker.lock();
				SOCKET s = slot->connfds.front();
				slot->connfds.pop_front();
				slot->fdslocker.unlock();
				addSocket(s);
			}

			while (slot->writeQueue.size() > 0) {
				const Socketmessage* msg = slot->writeQueue.pop_front();
				assert(msg);
				assert(msg->magic == MAGIC);
				if (msg->s == BUNDLE_BROADCAST_SOCKET) {
					broadcastMessage(msg);
				}
				else {
					if (!getSocket(msg->s)) {
						Alarm.cout("socket: %d not found", msg->s);
						bundle::releaseMessage(msg);
					}
					else {
						switch (msg->opcode) {
							case SM_OPCODE_ESTABLISH: 
								Error.cout("illegal establish message");
								bundle::releaseMessage(msg);
								break;
								
							case SM_OPCODE_CLOSE: 
								removeSocket(msg->s);
								bundle::releaseMessage(msg);
								break;
								
							case SM_OPCODE_MESSAGE: 
								writeMessage(msg->s, msg); 
								break;
								
							default: 
								Error.cout("illegal opcode: %d", msg->opcode); 
								bundle::releaseMessage(msg); 
								break;
						}					
					}
				}
			}

			poll.run(-1, readSocket, writeSocket, removeSocket);
		}
		
		for (auto& so : sockets) {
			SafeDelete(so);
		}
		
		Debug << "connectionProcess thread exit, maxfd: " << maxfd << ", writeQueue: " << slot->writeQueue.size() << ", connfds: " << slot->connfds.size();
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
		auto& slot = this->_slotConnection[msg->s % NM_CONNECTION_THREAD];
		slot.writeQueue.push_back((Socketmessage*)msg);
		if (slot.threadConnection) {
			pthread_kill(slot.threadConnection->native_handle(), SIGALRM);
		}
	}
	
	void SocketServerInternal::stop() {
		if (!this->isstop()) {
			this->_stop = true;
			if (this->_threadAccept) {
				pthread_kill(this->_threadAccept->native_handle(), SIGTERM);
				if (this->_threadAccept->joinable()) {
					this->_threadAccept->join();
				}
			}


			for (auto& slot : this->_slotConnection) {
				if (slot.threadConnection) {
					pthread_kill(slot.threadConnection->native_handle(), SIGALRM);	
					if (slot.threadConnection->joinable()) {
						slot.threadConnection->join();
					}
				}
				
				// release writeQueue messages
				for (;;) {
					Socketmessage* msg = slot.writeQueue.pop_front();
					if (!msg) {
						break;
					}
					bundle::releaseMessage(msg);
				}

				SafeDelete(slot.threadConnection);
			}
			

			// release readQueue messages
			for (;;) {
				Socketmessage* msg = this->_readQueue.pop_front();
				if (!msg) {
					break;
				}
				bundle::releaseMessage(msg);
			}


			// close connection
			for (auto s : this->_connfds) {
				::close(s);
			}

			// close listening port
			if (this->_fd != BUNDLE_INVALID_SOCKET) {
				::close(this->_fd);
				this->_fd = BUNDLE_INVALID_SOCKET;
			}

			// destroy accept thread & i/o thread
			SafeDelete(this->_threadAccept);
		}
	}

	size_t SocketServerInternal::size() {
		return this->_size;
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
