/*
 * \file: MessageQueueService.h
 * \brief: Created by hushouguo at 07:59:20 Nov 30 2018
 */
 
#ifndef __MESSAGE_QUEUE_SERVICE_H__
#define __MESSAGE_QUEUE_SERVICE_H__

BEGIN_NAMESPACE_BUNDLE {
	class NetworkTask;
	class MessageQueueService {
		public:
			MessageQueueService(
					std::function<void(MessageQueueService*, NetworkInterface*)> establishConnection,
					std::function<void(MessageQueueService*, NetworkInterface*)> lostConnection, 
					std::function<bool(MessageQueueService*, NetworkInterface*, const Netmessage*)> msgParser
					) 
					: _establishConnection(establishConnection)
					, _lostConnection(lostConnection)
					, _msgParser(msgParser)
			{}
			virtual ~MessageQueueService();

		public:
			inline bool isstop() { return this->_stop; }
			inline void* socket() { return this->_zmq_socket; }
			NetworkInterface* getNetworkInterface(SOCKET s);
			void releaseMessage(const Netmessage* netmsg);

		public:
			bool update();
			bool start(const char* address, int port);
			void stop();
			void close(NetworkInterface*);

		private:
			bool _stop = false;
			void* _zmq_ctx = nullptr;
			void* _zmq_socket = nullptr;
			std::unordered_map<u32, NetworkTask*> _tasks;
			NetworkTask* spawnConnection(u32 routeid);
			void closeConnection(u32 routeid);
			std::function<void(MessageQueueService*, NetworkInterface*)> _establishConnection = nullptr;
			std::function<void(MessageQueueService*, NetworkInterface*)> _lostConnection = nullptr;
			std::function<bool(MessageQueueService*, NetworkInterface*, const Netmessage*)> _msgParser = nullptr;
			void handleMessage(void* socket);
	};
}

#endif
