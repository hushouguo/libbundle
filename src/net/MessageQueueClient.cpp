/*
 * \file: MessageQueueClient.cpp
 * \brief: Created by hushouguo at 08:03:19 Nov 30 2018
 */

#include "bundle.h"

BEGIN_NAMESPACE_BUNDLE {
	bool MessageQueueClient::connect(const char* address, int port) {
		this->stop();

		this->_zmq_ctx = zmq_ctx_new();
		this->_zmq_socket = zmq_socket(this->_zmq_ctx, ZMQ_CLIENT);

		std::ostringstream o;
		o << "tcp://" << address << ":" << port;

		int rc = zmq_connect(this->_zmq_socket, o.str().c_str());
		CHECK_RETURN(rc == 0, false, "zmq_connect error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
		
		this->_stop = false;
		return true;
	}

	void MessageQueueClient::stop() {
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

	MessageQueueClient::~MessageQueueClient() {
		this->stop();
	}
			
	bool MessageQueueClient::update() {
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
			
	void MessageQueueClient::handleMessage(void* socket) {
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
			
			//uint32_t routeid = zmq_msg_routing_id(&msg);// for ZMQ_SERVER, routeid != 0, for ZMQ_CLIENT, routeid == 0
			if (this->_msgParser) {
				rc = this->_msgParser(this, netmsg) ? 0 : -1;
			}
		}

exit_except:
		rc = zmq_msg_close(&msg);
		//CHECK_RETURN(rc == 0, void(0), "zmq_msg_close error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
	}

	void MessageQueueClient::sendMessage(const Netmessage* netmsg) {
		this->sendMessage(netmsg, netmsg->len);	
	}

	void MessageQueueClient::sendMessage(s32 msgid, const google::protobuf::Message* msg) {
		CHECK_RETURN(this->_zmq_socket, void(0), "please call `connect` to init client");
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

	void MessageQueueClient::sendMessage(const void* payload, size_t payload_len) {
		CHECK_RETURN(payload_len > 0, void(0), "illegal payload_len:%u", payload_len);

		zmq_msg_t msg;
		int rc = zmq_msg_init_size(&msg, payload_len);
		CHECK_RETURN(rc == 0, void(0), "zmq_msg_init_size error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));

		memcpy(zmq_msg_data(&msg), payload, payload_len);

		rc = zmq_msg_send(&msg, this->_zmq_socket, ZMQ_SNDMORE);
		CHECK_GOTO(rc == -1, except_exit, "zmq_msg_send:MORE error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));

		rc = zmq_msg_send(&msg, this->_zmq_socket, ZMQ_NOBLOCK);
		CHECK_GOTO(rc == (int) payload_len, except_exit, 
				"zmq_msg_send error:%d,%s, rc:%d, len:%ld", zmq_errno(), zmq_strerror(zmq_errno()), rc, payload_len);

except_exit:
		rc = zmq_msg_close(&msg);
		//CHECK_RETURN(rc == 0, void(0), "zmq_msg_close error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
	}

	SOCKET MessageQueueClient::fd() {
		return 0;
		//TODO:
	}
}
