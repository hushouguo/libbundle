/*
 * \file: NetworkService.cpp
 * \brief: Created by hushouguo at 14:42:00 Aug 30 2018
 */

#include "bundle.h"

BEGIN_NAMESPACE_BUNDLE {
	class NetworkTask : public NetworkInterface {
		public:
			virtual ~NetworkTask() {}
			NetworkTask(NetworkService* ns, SOCKET s) : _ns(ns), _socket(s) {}

		public:
			SOCKET fd() override { return this->_socket; }
			void sendMessage(const Netmessage* netmsg) override {
				this->_ns->socketServer()->sendMessage(this->_socket, netmsg, netmsg->len);
			}
			void sendMessage(s32 msgid, const google::protobuf::Message* msg) override {
				int dataSize = msg->ByteSize();
				char buffer[sizeof(Netmessage) + dataSize];
				Netmessage* netmsg = (Netmessage *) buffer;
				netmsg->len = sizeof(Netmessage) + dataSize;
				netmsg->id = msgid;
				netmsg->flags = 0;
				netmsg->timestamp = 0;
				bool rc = msg->SerializeToArray(netmsg->payload, dataSize);
				CHECK_RETURN(rc, void(0), "SerializeToArray error");
				this->_ns->socketServer()->sendMessage(this->_socket, netmsg, netmsg->len);
			}
			void sendMessage(const void* payload, size_t payload_len) override {
				this->_ns->socketServer()->sendMessage(this->_socket, payload, payload_len);
			}

		private:
			NetworkService* _ns = nullptr;
			SOCKET _socket = -1;
	};

	NetworkInterface* NetworkService::getNetworkInterface(SOCKET s) {
		return FindOrNull(this->_tasks, s);
	}

	NetworkTask* NetworkService::spawnConnection(SOCKET s) {
		NetworkTask* task = new NetworkTask(this, s);
		bool rc = this->_tasks.insert(std::make_pair(s, task)).second;
		if (!rc) {
			SafeDelete(task);
			CHECK_RETURN(false, nullptr, "duplicate NetworkTask: %d", s);
		}
		return task;
	}

	void NetworkService::closeConnection(SOCKET s) {
		auto i = this->_tasks.find(s);
		CHECK_RETURN(i != this->_tasks.end(), void(0), "not exist NetworkTask: %d", s);
		this->_tasks.erase(i);
	}

	bool NetworkService::start(const char* address, int port, u32 worker) {
		SafeDelete(this->_socketServer);
		this->_socketServer = SocketServerCreator::create([](const void* buffer, size_t len) -> int {
				Netmessage* netmsg = (Netmessage*) buffer;
				return len < sizeof(Netmessage) || len < netmsg->len ? 0 : netmsg->len;
				});

		bool rc = this->_socketServer->setWorkerNumber(worker) 
					&& this->_socketServer->start(address, port);
		if (!rc) {
			SafeDelete(this->_socketServer);
			return false;
		}

		size_t maxsize = sConfig.get("service.security.maxsize", 0u);
		if (maxsize > 0) {
			this->_socketServer->setsockopt(BUNDLE_SOL_MAXSIZE, &maxsize, sizeof(maxsize));
		}

		size_t silence_second = sConfig.get("service.security.silence_second", 0u);
		if (silence_second > 0) {
			this->_socketServer->setsockopt(BUNDLE_SOL_SILENCE_SECOND, &silence_second, sizeof(silence_second));
		}

		size_t threshold_interval = sConfig.get("service.security.threshold_interval", 0u);
		if (threshold_interval > 0) {
			this->_socketServer->setsockopt(BUNDLE_SOL_THRESHOLD_INTERVAL, &threshold_interval, sizeof(threshold_interval));
		}

		size_t threshold_message = sConfig.get("service.security.threshold_message", 0u);
		if (threshold_message > 0) {
			this->_socketServer->setsockopt(BUNDLE_SOL_THRESHOLD_MESSAGE, &threshold_message, sizeof(threshold_message));
		}

		Trace.cout("NetworkService listening on %s:%d", address, port);
		return true;
	}

	bool NetworkService::update() {
		CHECK_RETURN(this->_socketServer, false, "please call `start` to init service");
		while (!this->isstop()) {
			const Socketmessage* msg = this->_socketServer->receiveMessage();
			if (!msg) {
				return true;
			}

			SOCKET s = GET_MESSAGE_SOCKET(msg);
			
			bool rc = true;
			if (IS_ESTABLISH_MESSAGE(msg)) {
				NetworkTask* task = this->spawnConnection(s);
				if (this->_establishConnection) {
					this->_establishConnection(this, task);
				}
			}
			else if (IS_CLOSE_MESSAGE(msg)) {
				NetworkTask* task = FindOrNull(this->_tasks, s);
				if (task) {
					if (this->_lostConnection) {
						this->_lostConnection(this, task);
					}
					this->closeConnection(s);
				}
				else {
					Error << "not found NetworkTask: " << s;
				}
			}
			else {
				NetworkTask* task = FindOrNull(this->_tasks, s);
				if (task && this->_msgParser) {
					const Netmessage* netmsg = (const Netmessage *) GET_MESSAGE_PAYLOAD(msg);
					size_t payload_len = GET_MESSAGE_PAYLOAD_LENGTH(msg);
					assert(payload_len == netmsg->len);
					rc = this->_msgParser(this, task, netmsg);
				}
			}
			
			//
			// msgParser return true means this msg no longer used
			if (rc) {
				bundle::releaseMessage(msg);
			}
		}
		return !this->isstop();
	}

	void NetworkService::releaseMessage(const Netmessage* netmsg) {
		const Socketmessage* msg = GET_MESSAGE_BY_PAYLOAD(netmsg);
		bundle::releaseMessage(msg);
	}

	void NetworkService::close(NetworkInterface* task) {
		if (this->_socketServer) {
			this->_socketServer->close(task->fd());
		}
	}

	void NetworkService::stop() {
		if (!this->isstop()) {
			this->_stop = true;
			for (auto& i : this->_tasks) {
				NetworkTask* task = i.second;
				SafeDelete(task);
			}
			this->_tasks.clear();
			if (this->_socketServer) {
				this->_socketServer->stop();
			}
		}
	}

	NetworkService::~NetworkService() {
		this->stop();
	}
}
