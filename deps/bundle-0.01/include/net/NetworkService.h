/*
 * \file: NetworkService.h
 * \brief: Created by hushouguo at 11:52:30 Aug 30 2018
 */
 
#ifndef __NETWORK_SERVICE_H__
#define __NETWORK_SERVICE_H__

BEGIN_NAMESPACE_BUNDLE {
	class NetworkTask;
	class NetworkService {
		public:
			NetworkService(
					std::function<void(NetworkService*, NetworkInterface*)> establishConnection,
					std::function<void(NetworkService*, NetworkInterface*)> lostConnection, 
					std::function<bool(NetworkService*, NetworkInterface*, const Netmessage*)> msgParser
					) 
					: _establishConnection(establishConnection)
					, _lostConnection(lostConnection)
					, _msgParser(msgParser)
			{}
			virtual ~NetworkService();

		public:
			inline bool isstop() { return this->_stop; }
			inline SocketServer* socketServer() { return this->_socketServer; }
			NetworkInterface* getNetworkInterface(SOCKET s);
			void releaseMessage(const Netmessage* netmsg);

		public:
			bool update();
			bool start(const char* address, int port);
			void stop();
			void close(NetworkInterface*);

		private:
			bool _stop = false;
			SocketServer* _socketServer = nullptr;
			std::unordered_map<SOCKET, NetworkTask*> _tasks;
			NetworkTask* spawnConnection(SOCKET s);
			void closeConnection(SOCKET s);
			std::function<void(NetworkService*, NetworkInterface*)> _establishConnection = nullptr;
			std::function<void(NetworkService*, NetworkInterface*)> _lostConnection = nullptr;
			std::function<bool(NetworkService*, NetworkInterface*, const Netmessage*)> _msgParser = nullptr;
	};
}

#endif
