/*
 * \file: MessageQueue.h
 * \brief: Created by hushouguo at 01:48:07 Sep 21 2018
 */
 
#ifndef __MESSAGEQUEUE_H__
#define __MESSAGEQUEUE_H__

BEGIN_NAMESPACE_BUNDLE {
	class MessageQueue {
		public:
			MessageQueue(int socket_type);
			~MessageQueue();

		public:
			bool init(const char* address, int port);
			void run();

		public:
//			void sendMessage(rawmessage* rawmsg, uint32_t clientid = 0);
//			void sendMessage(NetData::MSGID msgid, const google::protobuf::Message* msg, uint32_t clientid = 0, uint64_t target = 0);

//			virtual void lostConnection(uint32_t clientid) = 0;
//			virtual void handleMessage(uint32_t clientid, rawmessage* rawmsg) = 0;

		private:
			int _socket_type = 0;
			void * _zmq_ctx = nullptr;
			void * _zmq_socket = nullptr;
			//std::unordered_map<uint32_t, uint64_t> _clients;
			void handleMessage(void* socket);
	};
}

#endif
