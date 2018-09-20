/*
 * \file: MessageQueue.cpp
 * \brief: Created by hushouguo at 01:49:19 Sep 21 2018
 */

#include "bundle.h"

BEGIN_NAMESPACE_BUNDLE {
#if 0	
	MessageQueue::MessageQueue(int socket_type) {
		this->_socket_type = socket_type;
		this->_zmq_ctx = zmq_ctx_new();
		this->_zmq_socket = zmq_socket(this->_zmq_ctx, socket_type);
	}

	MessageQueue::~MessageQueue() {
		zmq_close(this->_zmq_socket);
		zmq_ctx_destroy(this->_zmq_ctx);
	}

	bool MessageQueue::init(const char* address, int port) {
		char buffer[64];
		snprintf(buffer, sizeof(buffer), "tcp://%s:%d", address, port);
		if (this->_socket_type == ZMQ_SERVER) {
			int rc = zmq_bind(this->_zmq_socket, buffer);
			CHECK_RETURN(rc == 0, false, "zmq_bind error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
			log_trace("Server bind on: %s", buffer);
		}
		else {
			int rc = zmq_connect(this->_zmq_socket, buffer);
			CHECK_RETURN(rc == 0, false, "zmq_connect error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
			log_trace("Client connect to: %s", buffer);
		}
		return true;
	}

	void MessageQueue::run() {
		// Wait until something happens.
		zmq_pollitem_t sockets[] = {
			{ this->_zmq_socket, 0, ZMQ_POLLIN, 0 },
		};
		
		// milliseconds timeout, 0: return immediately, -1: wait forever
		int events = zmq_poll(sockets, sizeof(sockets)/sizeof(sockets[0]), 0); 
		CHECK_RETURN(events >= 0, void(0), "zmq_poll error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
		for (int i = 0; i < events; ++i) {
			if (sockets[i].revents & ZMQ_POLLIN) {
				this->handleMessage(sockets[i].socket);
			}
		}
	}

	void MessageQueue::handleMessage(void* socket) {
		zmq_msg_t msg;
		int rc = zmq_msg_init(&msg);
		CHECK_RETURN(rc == 0, void(0), "zmq_msg_init_size error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
		rc = zmq_msg_recv(&msg, socket, ZMQ_NOBLOCK); // msg MUST ready !!
		CHECK_GOTO(rc >= 0, exit_except, "zmq_msg_recv error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));

		if (true) {
			size_t size = zmq_msg_size(&msg);
			rawmessage* rawmsg = (rawmessage *) zmq_msg_data(&msg);
			CHECK_GOTO(size == rawmsg->len, exit_except, "error rawmsg->len:%d, size:%ld", rawmsg->len, size);
			CHECK_GOTO(size >= sizeof(rawmessage), exit_except, 
					"illegal msg size:%ld, expect:%ld", size, sizeof(rawmessage));

			//Note: we need check max message size ?
			
			uint32_t clientid = zmq_msg_routing_id(&msg);// for ZMQ_SERVER, clientid != 0, for ZMQ_CLIENT, clientid == 0
			this->handleMessage(clientid, rawmsg);
		}

exit_except:
		rc = zmq_msg_close(&msg);
		CHECK_RETURN(rc == 0, void(0), "zmq_msg_close error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
	}

	void MessageQueue::sendMessage(rawmessage* rawmsg, uint32_t clientid) {
		CHECK_RETURN(rawmsg->len > 0, void(0), "illegal rawmsg->len:%u", rawmsg->len);

		zmq_msg_t msg;
		int rc = zmq_msg_init_size(&msg, rawmsg->len);
		CHECK_RETURN(rc == 0, void(0), "zmq_msg_init_size error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));

		memcpy(zmq_msg_data(&msg), rawmsg, rawmsg->len);

		if (this->_socket_type == ZMQ_SERVER) {
			CHECK_GOTO(clientid > 0, except_exit, "illegal clientid: %d", clientid);
			rc = zmq_msg_set_routing_id(&msg, clientid);
			CHECK_GOTO(rc == 0, except_exit, "zmq_msg_set_routing_id error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
		}

		rc = zmq_msg_send(&msg, this->_zmq_socket, ZMQ_SNDMORE);
		CHECK_GOTO(rc == -1, except_exit, "zmq_msg_send:MORE error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));

		rc = zmq_msg_send(&msg, this->_zmq_socket, ZMQ_NOBLOCK);
		CHECK_GOTO(rc == (int)rawmsg->len, except_exit, 
				"zmq_msg_send error:%d,%s, rc:%d, len:%u", zmq_errno(), zmq_strerror(zmq_errno()), rc, rawmsg->len);

except_exit:
		rc = zmq_msg_close(&msg);
		CHECK_RETURN(rc == 0, void(0), "zmq_msg_close error:%d,%s", zmq_errno(), zmq_strerror(zmq_errno()));
	}

	void MessageQueue::sendMessage(NetData::MSGID msgid, const google::protobuf::Message* msg, uint32_t clientid, uint64_t target)
	{
		int dataSize = msg->ByteSize();
		char buffer[sizeof(rawmessage) + dataSize];
		rawmessage* rawmsg = (rawmessage *) buffer;
		rawmsg->len = sizeof(rawmessage) + dataSize;
		rawmsg->type = MESSAGE_TYPE_PROTOBUF;
		rawmsg->id = msgid;
		rawmsg->flags = 0;
		rawmsg->timestamp = 0;
		rawmsg->target = target;
		bool rc = msg->SerializeToArray(rawmsg->data, dataSize);
		CHECK_RETURN(rc, void(0), "SerializeToArray error");
		this->sendMessage(rawmsg, clientid);
	}

//#if 0
	void MessageQueue::sendRetcode(NetData::RC rc, uint32_t clientid)
	{
		NetData::ErrorCode errmsg;
		errmsg.set_rc(rc);
		this->sendMessage(NetData::MSGID_ERROR_CODE, &errmsg, clientid);
	}
#endif
}
