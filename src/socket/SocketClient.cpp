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
			bool connect(const char* address, int port, bool wait) override;
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
			
			SOCKET _fd = BUNDLE_INVALID_SOCKET;
			bool _stop = true, _active = false;
			
			bool _connect_in_progress = false;
			std::string _address;
			int _port = 0;			

			bool connectServer();

			MESSAGE_SPLITER _splitMessage = nullptr;
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

		this->_slotWorker->addSocket(this->fd(), false);
				
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
		
		//SafeDelete(this->_slotWorker);
		//this->_slotWorker = new WorkerProcess(0, this->_splitMessage, &this->_readQueue);
		//std::this_thread::yield();
		
		return true;
	}

#if 0
	void SocketClientInternal::workerProcess() {

		while (!this->isstop()) {
			if (!this->active()) {
				if (this->fd() != BUNDLE_INVALID_SOCKET) {	::sleep(CONNECT_INTERVAL); }
				if (!this->connectServer()) {
					continue;
				}
			}
			else {
				addSocket(this->fd());
				while (this->_writeQueue.size() > 0) {
					const Socketmessage* msg = this->_writeQueue.pop_front();
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
		Debug << "clientProcess exit, readQueue: " << this->_readQueue.size() << ", writeQueue: " << this->_writeQueue.size();
	}
#endif

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
				case SM_OPCODE_CLOSE: close = true; return msg;
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
			SafeDelete(this->_slotWorker);

			// release readQueue messages
			for (;;) {
				Socketmessage* msg = this->_readQueue.pop_front();
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
		}
	}

	bool SocketClientInternal::active() {
		return this->_active;
	}

	SocketClientInternal::SocketClientInternal(MESSAGE_SPLITER splitMessage) {
		this->_splitMessage = splitMessage;
		this->_slotWorker = new WorkerProcess(0, splitMessage, &this->_readQueue);
	}
	
	SocketClient::~SocketClient() {}
	SocketClientInternal::~SocketClientInternal() { 
		this->stop(); 
	}

	SocketClient* SocketClientCreator::create(MESSAGE_SPLITER splitMessage) {
		return new SocketClientInternal(splitMessage);
	}
}
