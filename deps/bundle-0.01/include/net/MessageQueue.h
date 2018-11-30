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
			inline bool isstop() { return this->_stop; }
			void stop();
			bool init(const char* address, int port);
			void run();

		public:
//			void sendMessage(Netmessage* netmsg, uint32_t cid = 0);
//			void sendMessage(s32 msgid, const google::protobuf::Message* msg, uint32_t cid = 0);

//			virtual void lostConnection(uint32_t clientid) = 0;
//			virtual void handleMessage(uint32_t clientid, rawmessage* rawmsg) = 0;

		private:
			bool _stop = false;
			int _socket_type = 0;
			void * _zmq_ctx = nullptr;
			void * _zmq_socket = nullptr;
			void handleMessage(void* socket);
	};
}

#endif
