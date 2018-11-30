/*
 * \file: MessageQueueService.cpp
 * \brief: Created by hushouguo at 11:00:13 Nov 30 2018
 */

#include "bundle.h"

BEGIN_NAMESPACE_BUNDLE {
	class NetworkTask : public NetworkInterface {
		public:
			virtual ~NetworkTask() {}
			NetworkTask(MessageQueueService* ns, uint32_t routeid) : _ns(ns), _routeid(routeid) {}

		public:
			SOCKET fd() override { return this->_routeid; }
			void sendMessage(const Netmessage* netmsg) override {
				this->sendMessage(netmsg, netmsg->len);
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
				CHECK_RETURN(rc, void(0), "SerializeToArray error, msgid: %d", msgid);
				this->sendMessage(netmsg);
			}
			void sendMessage(const void* payload, size_t payload_len) override {
				CHECK_RETURN(this->_ns->socket(), void(0), "not init zmq_socket");
				CHECK_RETURN(payload_len > 0, void(0), "illegal payload_len:%u", payload_len);

				zmq_msg_t msg;
				int rc = zmq_msg_init_size(&msg, payload_len);
				CHECK_RETURN(rc == 0, void(0), "zmq_msg_init_size error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));

				memcpy(zmq_msg_data(&msg), payload, payload_len);

				if (true) {
					CHECK_GOTO(this->_routeid > 0, except_exit, "illegal routeid: %d", this->_routeid);
					rc = zmq_msg_set_routing_id(&msg, this->_routeid);
					CHECK_GOTO(rc == 0, except_exit, "zmq_msg_set_routing_id error: %d,%s", 
							zmq_errno(), zmq_strerror(zmq_errno()));
				}
				
				rc = zmq_msg_send(&msg, this->_ns->socket(), ZMQ_SNDMORE);
				CHECK_GOTO(rc == -1, except_exit, "zmq_msg_send:MORE error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));

				rc = zmq_msg_send(&msg, this->_ns->socket(), ZMQ_NOBLOCK);
				CHECK_GOTO(rc == (int) payload_len, except_exit, 
						"zmq_msg_send error:%d,%s, rc:%d, len:%ld", zmq_errno(), zmq_strerror(zmq_errno()), rc, payload_len);

except_exit:
				rc = zmq_msg_close(&msg);
				//CHECK_RETURN(rc == 0, void(0), "zmq_msg_close error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
			}

		private:
			MessageQueueService* _ns = nullptr;
			uint32_t _routeid = 0;
	};
	

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	bool MessageQueueService::start(const char* address, int port) {
		this->stop();

		this->_zmq_ctx = zmq_ctx_new();
		this->_zmq_socket = zmq_socket(this->_zmq_ctx, ZMQ_SERVER);

		std::ostringstream o;
		o << "tcp://" << address << ":" << port;

		int rc = zmq_bind(this->_zmq_socket, o.str().c_str());
		CHECK_RETURN(rc == 0, false, "zmq_bind error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
		
		this->_stop = false;
		return true;
	}
			
	void MessageQueueService::stop() {
		if (!this->isstop()) {
			this->_stop = true;
			if (this->_zmq_socket) {
				zmq_close(this->_zmq_socket);
				this->_zmq_socket = nullptr;
			}
			if (this->_zmq_ctx) {
				zmq_ctx_destroy(this->_zmq_ctx);
				this->_zmq_ctx = nullptr;
			}
		}
	}

	MessageQueueService::~MessageQueueService() {
		this->stop();
	}

	bool MessageQueueService::update() {
		CHECK_RETURN(this->_zmq_socket, false, "please call `connect` to init client");

		// Wait until something happens.
		zmq_pollitem_t sockets[] = {
			{ this->_zmq_socket, 0, ZMQ_POLLIN, 0 },
		};
		
		// milliseconds timeout, 0: return immediately, -1: wait forever
		int events = zmq_poll(sockets, sizeof(sockets)/sizeof(sockets[0]), 0); 
		CHECK_RETURN(events >= 0, false, "zmq_poll error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
		for (int i = 0; i < events; ++i) {
			if (sockets[i].revents & ZMQ_POLLIN) {
				this->handleMessage(sockets[i].socket);
			}
		}

		return !this->isstop();
	}
			
	void MessageQueueService::handleMessage(void* socket) {
		zmq_msg_t msg;
		int rc = zmq_msg_init(&msg);
		CHECK_RETURN(rc == 0, void(0), "zmq_msg_init_size error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
		rc = zmq_msg_recv(&msg, socket, ZMQ_NOBLOCK); // msg MUST ready !!
		CHECK_GOTO(rc >= 0, exit_except, "zmq_msg_recv error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));

		if (true) {
			size_t size = zmq_msg_size(&msg);
			Netmessage* netmsg = (Netmessage*) zmq_msg_data(&msg);
			CHECK_GOTO(size == netmsg->len, exit_except, "error netmsg->len:%d, size:%ld", netmsg->len, size);
			CHECK_GOTO(size >= sizeof(Netmessage), exit_except, "illegal msg size:%ld, expect:%ld", size, sizeof(Netmessage));

			//Note: we need check max message size ?

			uint32_t routeid = zmq_msg_routing_id(&msg);// for ZMQ_SERVER, routeid != 0, for ZMQ_CLIENT, routeid == 0
			Trace << "routeid: " << routeid;
			if (this->_msgParser) {
				NetworkInterface* networkInterface = this->getNetworkInterface(routeid);
				if (!networkInterface) {
					//Note: new connection arrived, callback `establishConnection`
					networkInterface = this->spawnConnection(routeid);
					assert(networkInterface);
				}
				rc = this->_msgParser(this, networkInterface, netmsg) ? 0 : -1;
			}
		}

exit_except:
		rc = zmq_msg_close(&msg);
		//CHECK_RETURN(rc == 0, void(0), "zmq_msg_close error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
	}

	NetworkTask* MessageQueueService::spawnConnection(u32 routeid) {
		NetworkTask* task = new NetworkTask(this, routeid);
		bool rc = this->_tasks.insert(std::make_pair(routeid, task)).second;
		if (!rc) {
			SafeDelete(task);
			CHECK_RETURN(false, nullptr, "duplicate NetworkTask: %d", routeid);
		}
		return task;
	}
			
	void MessageQueueService::closeConnection(u32 routeid) {
		auto i = this->_tasks.find(routeid);
		CHECK_RETURN(i != this->_tasks.end(), void(0), "not exist NetworkTask: %d", routeid);
		this->_tasks.erase(i);
	}
			
	NetworkInterface* MessageQueueService::getNetworkInterface(SOCKET s) {
		return FindOrNull(this->_tasks, s);
	}
			
	void MessageQueueService::close(NetworkInterface* networkInterface) {
		//TODO:
	}
}
