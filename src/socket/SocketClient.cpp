/*
 * \file: SocketClient.cpp
 * \brief: Created by hushouguo at 01:57:59 Aug 09 2018
 */

#include "bundle.h"
#include "Helper.h"
#include "Socket.h"
#include "Poll.h"

#define CONNECT_TIMEOUT		10
#define CONNECT_INTERVAL	5

BEGIN_NAMESPACE_BUNDLE {
	class SocketClientInternal : public SocketClient {
		public:
			SocketClientInternal(std::function<int(const Byte*, size_t)> splitMessage);
			~SocketClientInternal();

		public:
			SOCKET fd() override {	return this->_fd; }
			bool connect(const char* address, int port, bool wait) override;
			void stop() override;
			inline bool isstop() { return this->_stop; }
			bool active() override;
			inline bool connect_in_progress() { return this->_connect_in_progress; }
			const Socketmessage* receiveMessage(bool& establish, bool& close) override;			
			void sendMessage(const void* payload, size_t payload_len) override;
			void sendMessage(const Socketmessage*) override;

		public:
			void clientProcess();
			
		private:
			std::thread* _threadClient = nullptr;
			
			SOCKET _fd = BUNDLE_INVALID_SOCKET;
			bool _stop = true, _active = false;
			
			bool _connect_in_progress = false;
			std::string _address;
			int _port = 0;
			
			LockfreeQueue<Socketmessage> _rQueue, _wQueue; 

			bool connectServer();
			void pushMessage(const Socketmessage* msg);

			std::function<int(const Byte*, size_t)> _splitMessage = nullptr;
	};

	bool SocketClientInternal::connectServer() {
		CHECK_RETURN(this->active() == false, true, "connection already establish");
		CHECK_RETURN(this->connect_in_progress() == false, false, "connect in progress");
		this->_connect_in_progress = true;
		
		if (this->_fd != BUNDLE_INVALID_SOCKET) {
			::close(this->_fd);
			this->_fd = BUNDLE_INVALID_SOCKET;
		}
		this->_fd = ::socket(AF_INET, SOCK_STREAM, 0);
		CHECK_RETURN(this->_fd >= 0, false, "create socket failure: %d, %s", errno, strerror(errno));
		
		bool rc = connectSignal(this->fd(), this->_address.c_str(), this->_port, CONNECT_TIMEOUT);
		this->_connect_in_progress = false;
		if (!rc) {
			return false;
		}
				
		Debug.cout("SocketClient establish with %s:%d", this->_address.c_str(), this->_port);
		
		return this->_active = true;
	}
	
	bool SocketClientInternal::connect(const char* address, int port, bool wait) {
		CHECK_RETURN(this->_stop, false, "SocketClient is running, stop it at first!");
		this->_stop = false;
		CHECK_RETURN(this->_active == false, false, "SocketClient is active, stop it at first!");
		this->_active = false;
		this->_address = address;
		this->_port = port;

		if (wait && !this->connectServer()) {
			return false;
		}
		
		SafeDelete(this->_threadClient);
		this->_threadClient = new std::thread([this]() {
			this->clientProcess();
		});
		
		return true;
	}

	void SocketClientInternal::clientProcess() {
		Poll poll;
		Socket* so = nullptr;

		auto getSocket = [&](SOCKET s) -> Socket* {
			assert(s == this->fd() && s != BUNDLE_INVALID_SOCKET);
			return so;
		};

		auto pushMessage = [&](Socketmessage* msg) {
			this->_rQueue.push_back(msg);
		};

		auto addSocket = [&](SOCKET s) {
			if (!so || s != so->fd()) {
				SafeDelete(so);// discard all unsent messages!
				so = new Socket(s, this->_splitMessage);
				poll.addSocket(s);
				Socketmessage* msg = allocateMessage(s, SM_OPCODE_ESTABLISH);
				pushMessage(msg);
			}
		};

		auto removeSocket = [&](SOCKET s) {
			Socketmessage* msg = allocateMessage(s, SM_OPCODE_CLOSE);
			pushMessage(msg);
			poll.removeSocket(s);
			SafeDelete(so);
			this->_active = false;
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

		while (!this->isstop()) {
			if (!this->active()) {
				if (this->fd() != BUNDLE_INVALID_SOCKET) {	::sleep(CONNECT_INTERVAL); }
				if (!this->connectServer()) {
					continue;
				}
			}
			else {
				addSocket(this->fd());
				if (this->_wQueue.size() > 0) {
					const Socketmessage* msg = this->_wQueue.pop_front();
					assert(msg->magic == MAGIC);
					if (!getSocket(msg->s)) {
						bundle::releaseMessage(msg);
					}
					else {
						writeMessage(msg->s, msg);
					}
				}				
				poll.run(-1, readSocket, writeSocket, removeSocket);
			}
		}

		SafeDelete(so);
	}

	//////////////////////////////////////////////////////////////////////////////////
	
	const Socketmessage* SocketClientInternal::receiveMessage(bool& establish, bool& close) {
		establish = close = false;
		const Socketmessage* msg = this->_rQueue.pop_front();
		if (msg) {
			assert(msg->magic == MAGIC);
			switch (msg->opcode) {
				case SM_OPCODE_ESTABLISH: establish = true; break;
				case SM_OPCODE_CLOSE: close = true; break;
				case SM_OPCODE_MESSAGE: break;
				default: assert(false); break;
			}
		}
		return msg;
	}

	void SocketClientInternal::sendMessage(const void* payload, size_t payload_len) {
		assert(payload);
		assert(payload_len > 0);
		Socketmessage* msg = allocateMessage(this->fd(), SM_OPCODE_MESSAGE, payload, payload_len);
		this->pushMessage(msg);
	}
	
	void SocketClientInternal::sendMessage(const Socketmessage* msg) {
		((Socketmessage*)msg)->s = this->fd();
		this->pushMessage(msg);
	}

	void SocketClientInternal::pushMessage(const Socketmessage* msg) {
		this->_wQueue.push_back((Socketmessage*)msg);
		//TODO: send signal to connectionProcess or wakeup epoll_wait
	}
		
	void SocketClientInternal::stop() {
		if (!this->isstop()) {
			this->_stop = true;
			if (this->_threadClient && this->_threadClient->joinable()) {
				this->_threadClient->join();
			}

			// release rQueue messages
			for (;;) {
				Socketmessage* msg = this->_rQueue.pop_front();
				if (!msg) {
					break;
				}
				bundle::releaseMessage(msg);
			}

			// release wQueue messages
			for (;;) {
				Socketmessage* msg = this->_wQueue.pop_front();
				if (!msg) {
					break;
				}
				bundle::releaseMessage(msg);
			}

			// close connected port
			if (this->_fd != BUNDLE_INVALID_SOCKET) {
				::close(this->_fd);
				this->_fd = BUNDLE_INVALID_SOCKET;
			}

			// destroy client thread
			SafeDelete(this->_threadClient);
		}
	}

	bool SocketClientInternal::active() {
		return this->_active;
	}

	SocketClientInternal::SocketClientInternal(std::function<int(const Byte*, size_t)> splitMessage) {
		this->_splitMessage = splitMessage;
	}
	
	SocketClient::~SocketClient() {}
	SocketClientInternal::~SocketClientInternal() { 
		this->stop(); 
	}

	SocketClient* SocketClientCreator::create(std::function<int(const Byte*, size_t)> splitMessage) {
		return new SocketClientInternal(splitMessage);
	}
}
