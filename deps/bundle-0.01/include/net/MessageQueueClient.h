/*
 * \file: MessageQueueClient.h
 * \brief: Created by hushouguo at 08:00:33 Nov 30 2018
 */
 
#ifndef __MESSAGE_QUEUE_CLIENT_H__
#define __MESSAGE_QUEUE_CLIENT_H__

BEGIN_NAMESPACE_BUNDLE {
	class MessageQueueClient : public NetworkInterface {
		public:
			MessageQueueClient(
					std::function<void(MessageQueueClient*)> establishConnection,
					std::function<void(MessageQueueClient*)> lostConnection, 
					std::function<bool(MessageQueueClient*, const Netmessage*)> msgParser
					) 
					: _establishConnection(establishConnection)
					, _lostConnection(lostConnection)
					, _msgParser(msgParser)
			{}
			~MessageQueueClient();

		public:
			inline bool isstop() { return this->_stop; }

		public:
			bool update();
			bool connect(const char* address, int port);
			void stop();

		public:
			SOCKET fd() override;
			void sendMessage(const Netmessage*) override;
			void sendMessage(s32, const google::protobuf::Message*) override;
			void sendMessage(const void*, size_t) override;

		private:
			bool _stop = false;
			void* _zmq_ctx = nullptr;
			void* _zmq_socket = nullptr;
			std::function<void(MessageQueueClient*)> _establishConnection = nullptr;
			std::function<void(MessageQueueClient*)> _lostConnection = nullptr;
			std::function<bool(MessageQueueClient*, const Netmessage*)> _msgParser = nullptr;
			void handleMessage(void* socket);
	};
}

#endif
