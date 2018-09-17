/*
 * \file: NetworkClient.h
 * \brief: Created by hushouguo at 08:40:57 Sep 01 2018
 */
 
#ifndef __NETWORK_CLIENT_H__
#define __NETWORK_CLIENT_H__
 
BEGIN_NAMESPACE_BUNDLE {
	class NetworkClient : public NetworkInterface {
		public:
			NetworkClient(
					std::function<void(NetworkClient*)> establishConnection,
					std::function<void(NetworkClient*)> lostConnection, 
					std::function<bool(NetworkClient*, const Netmessage*)> msgParser
					) 
					: _establishConnection(establishConnection)
					, _lostConnection(lostConnection)
					, _msgParser(msgParser)
			{}
			~NetworkClient();

		public:
			inline bool isstop() { return this->_stop; }
			inline SocketClient* socketClient() { return this->_socketClient; }
			void releaseMessage(const Netmessage* netmsg);

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
			SocketClient* _socketClient = nullptr;
			std::function<void(NetworkClient*)> _establishConnection = nullptr;
			std::function<void(NetworkClient*)> _lostConnection = nullptr;
			std::function<bool(NetworkClient*, const Netmessage*)> _msgParser = nullptr;
	};
}

#endif
