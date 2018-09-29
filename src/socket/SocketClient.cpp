/*
 * \file: SocketClient.cpp
 * \brief: Created by hushouguo at 01:57:59 Aug 09 2018
 */

#include "bundle.h"
#include "Helper.h"
#include "Socket.h"
#include "Poll.h"
#include "WorkerProcess.h"

#define CONNECT_TIMEOUT		10
#define CONNECT_INTERVAL	5
#define WAKE_PROCESS_SIGNAL	SIGRTMIN

BEGIN_NAMESPACE_BUNDLE {
	class SocketClientInternal : public SocketClient {
		public:
			SocketClientInternal(MESSAGE_SPLITER splitMessage);
			~SocketClientInternal();

		public:
			SOCKET fd() override {	return this->_fd; }
			bool connect(const char* address, int port, u32 timeout) override;
			void stop() override;
			inline bool isstop() { return this->_stop; }
			bool active() override;
			inline bool connect_in_progress() { return this->_connect_in_progress; }
			const Socketmessage* receiveMessage(bool& establish, bool& close) override;			
			void sendMessage(const void* payload, size_t payload_len) override;
			void sendMessage(const Socketmessage*) override;

		private:
			WorkerProcess* _slotWorker = nullptr;
			LockfreeQueue<Socketmessage*> _readQueue; 
			
			SOCKET _fd = -1;
			bool _stop = true, _active = false;
			
			bool _connect_in_progress = false;
			std::string _address;
			int _port = 0;
			std::thread* _retry_thread = nullptr;

			bool connectServer(u32 timeout);
			bool connectAsync();

			MESSAGE_SPLITER _splitMessage = nullptr;
	};

	bool SocketClientInternal::connectServer(u32 timeout) {
		CHECK_RETURN(this->active() == false, true, "connectServer already establish");
		CHECK_RETURN(this->connect_in_progress() == false, false, "connectServer in progress");
		this->_connect_in_progress = true;
		
		this->_fd = ::socket(AF_INET, SOCK_STREAM, 0);
		CHECK_RETURN(this->_fd >= 0, false, "create socket failure: %d, %s", errno, strerror(errno));
		
		bool rc = connectSignal(this->fd(), this->_address.c_str(), this->_port, timeout);
		this->_connect_in_progress = false;
		if (!rc) {
			return false;
		}

		Debug.cout("SocketClient establish with %s:%d", this->_address.c_str(), this->_port);
		
		return true;
	}
	
	bool SocketClientInternal::connect(const char* address, int port, u32 timeout) {
		CHECK_RETURN(this->_stop, false, "SocketClient is running, stop it at first!");
		this->_stop = false;
		CHECK_RETURN(this->_active == false, false, "SocketClient is active, stop it at first!");
		this->_active = false;
		this->_address = address;
		this->_port = port;
		
		if (timeout > 0) {
			this->_active = this->connectServer(timeout);
			if (this->_active) {
				this->_slotWorker->addSocket(this->fd(), false);
			}
			return this->_active;
		}
		
		return this->connectAsync();
	}

	bool SocketClientInternal::connectAsync() {
		if (this->_retry_thread && this->_retry_thread->joinable()) {
			this->_retry_thread->join();
		}
		SafeDelete(this->_retry_thread);
		this->_retry_thread = new std::thread([this]() {
			for (; !this->isstop() && !this->active(); ) {
				SafeClose(this->_fd);
				std::this_thread::sleep_for(std::chrono::seconds(CONNECT_INTERVAL));
				if (this->connectServer(CONNECT_TIMEOUT)) {
					this->_active = true;
				}
			}			
			if (this->active()) {
				this->_slotWorker->addSocket(this->fd(), false);
			}
		});	
		return true;
	}

	//////////////////////////////////////////////////////////////////////////////////
	
	const Socketmessage* SocketClientInternal::receiveMessage(bool& establish, bool& close) {
		establish = close = false;
		while (!this->_readQueue.empty()) {
			Socketmessage* msg = this->_readQueue.pop_front();
			assert(msg);
			assert(msg->magic == MAGIC);
			assert(msg->s != BUNDLE_INVALID_SOCKET);
			switch (msg->opcode) {
				case SM_OPCODE_ESTABLISH: establish = true; return msg;
				case SM_OPCODE_CLOSE:
					if (msg->s == this->fd()) {
						Debug << "SocketClient: " << this->fd() << " lost, retry connect";
						this->_active = false;
						this->connectAsync();
					}
					close = true; return msg;
				case SM_OPCODE_MESSAGE: return msg;

				default:
				case SM_OPCODE_NEW_SOCKET:
				case SM_OPCODE_NEW_LISTENING: assert(false); break;
			}
		}
		return nullptr;
	}

	void SocketClientInternal::sendMessage(const void* payload, size_t payload_len) {
		this->_slotWorker->pushMessage(this->fd(), payload, payload_len);
	}
	
	void SocketClientInternal::sendMessage(const Socketmessage* msg) {
		this->_slotWorker->pushMessage(this->fd(), (Socketmessage*) msg);
	}

	void SocketClientInternal::stop() {
		if (!this->isstop()) {
			this->_stop = true;

			// release retry thread
			if (this->_retry_thread && this->_retry_thread->joinable()) {
				this->_retry_thread->join();
			}
			SafeDelete(this->_retry_thread);

			// release WorkerProcess
			SafeDelete(this->_slotWorker);

			// release readQueue messages
			for (;this->_readQueue.empty() == false;) {
				Socketmessage* msg = this->_readQueue.pop_front();
				assert(msg);
				bundle::releaseMessage(msg);
			}

			// close socket
			SafeClose(this->_fd);
		}
	}

	bool SocketClientInternal::active() {
		return this->_active;
	}

	SocketClientInternal::SocketClientInternal(MESSAGE_SPLITER splitMessage) {
		this->_splitMessage = splitMessage;
		this->_slotWorker = new WorkerProcess(0, this->_splitMessage, &this->_readQueue);
	}
	
	SocketClient::~SocketClient() {}
	SocketClientInternal::~SocketClientInternal() { 
		this->stop(); 
	}

	SocketClient* SocketClientCreator::create(MESSAGE_SPLITER splitMessage) {
		return new SocketClientInternal(splitMessage);
	}
}
