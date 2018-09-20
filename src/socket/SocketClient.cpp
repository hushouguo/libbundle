/*
 * \file: SocketClient.cpp
 * \brief: Created by hushouguo at 01:57:59 Aug 09 2018
 */

#include "bundle.h"
#include "Socketmessage.h"
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
			Rawmessage* receiveMessage(bool& establish, bool& close) override;
			void sendMessage(const void* payload, size_t payload_len) override;
			//void sendMessage(const Rawmessage*) override;
			void releaseMessage(Rawmessage*) override;

		public:
			void clientProcess();
			
		private:
			std::thread* _threadClient = nullptr;
			
			SOCKET _fd = BUNDLE_INVALID_SOCKET;
			bool _stop = true, _active = false;
			
			bool _connect_in_progress = false;
			std::string _address;
			int _port = 0;
			
			std::mutex _rlocker, _wlocker;
			std::list<Socketmessage *> _rlist;
			std::list<Socketmessage *> _wlist;

			bool connectServer();

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
				
		Trace.cout("SocketClient establish with %s:%d", this->_address.c_str(), this->_port);	
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
			std::lock_guard<std::mutex> guard(this->_rlocker);
			this->_rlist.push_back(msg);
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

		auto writeMessage = [&](SOCKET s, Socketmessage* msg) {
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
				while (!this->_wlist.empty() && this->active() && this->_wlocker.try_lock()) {
					Socketmessage* msg = this->_wlist.front();
					this->_wlist.pop_front();
					this->_wlocker.unlock();
					assert(msg->magic == MAGIC);
					if (!getSocket(msg->s)) {
						bundle::releaseMessage(msg);
					}
					else {
						writeMessage(msg->s, msg);
					}
				}				
				poll.run(1, readSocket, writeSocket, removeSocket);
			}
		}

		SafeDelete(so);
	}

	//////////////////////////////////////////////////////////////////////////////////
	
	Rawmessage* SocketClientInternal::receiveMessage(bool& establish, bool& close) {
		establish = close = false;
		if (!this->_rlist.empty() && this->_rlocker.try_lock()) {
			Socketmessage* msg = this->_rlist.front();
			this->_rlist.pop_front();
			this->_rlocker.unlock();
			assert(msg->magic == MAGIC);
			switch (msg->opcode) {
				case SM_OPCODE_ESTABLISH: establish = true; return msg->rawmsg;
				case SM_OPCODE_CLOSE: close = true; return msg->rawmsg;
				case SM_OPCODE_MESSAGE: return msg->rawmsg;
				default: Error.cout("illegal opcode: %d", msg->opcode); break;
			}
		}
		return nullptr;
	}

	void SocketClientInternal::sendMessage(const void* payload, size_t payload_len) {
		assert(payload);
		assert(payload_len > 0);
		Socketmessage* msg = allocateMessage(this->fd(), SM_OPCODE_MESSAGE, payload, payload_len);
		std::lock_guard<std::mutex> guard(this->_wlocker);
		this->_wlist.push_back(msg);	
	}

#if 0
	void SocketClientInternal::sendMessage(const Rawmessage* rawmsg) {
		assert(rawmsg);
		assert(rawmsg->payload_len > 0);
		Socketmessage* msg = allocateMessage(this->fd(), SM_OPCODE_MESSAGE, rawmsg->payload, rawmsg->payload_len);
		std::lock_guard<std::mutex> guard(this->_wlocker);
		this->_wlist.push_back(msg);
	}
#endif

	void SocketClientInternal::releaseMessage(Rawmessage* rawmsg) {
		Socketmessage* msg = (Socketmessage *) ((Byte*) rawmsg - offsetof(Socketmessage, rawmsg));
		bundle::releaseMessage(msg);
	}
	
	void SocketClientInternal::stop() {
		if (!this->isstop()) {
			this->_stop = true;
			if (this->_threadClient && this->_threadClient->joinable()) {
				this->_threadClient->join();
			}

			// release rlist messages
			for (auto& msg : this->_rlist) {
				bundle::releaseMessage(msg);
			}

			// release wlist messages
			for (auto& msg : this->_wlist) {
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
