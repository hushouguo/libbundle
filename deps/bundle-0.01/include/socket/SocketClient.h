/*
 * \file: SocketClient.h
 * \brief: Created by hushouguo at 01:56:02 Aug 09 2018
 */
 
#ifndef __SOCKET_CLIENT_H__
#define __SOCKET_CLIENT_H__

BEGIN_NAMESPACE_BUNDLE {
	class Socketmessage;
	class SocketClient {
		public:
			virtual ~SocketClient() = 0;
			
		public:
			virtual SOCKET fd() = 0;
			virtual bool connect(const char* address, int port, bool wait) = 0;
			virtual void stop() = 0;
			virtual bool active() = 0;
			virtual const Socketmessage* receiveMessage(bool& establish, bool& close) = 0;
			virtual void sendMessage(const void*, size_t) = 0;
			virtual Socketmessage* initMessage(size_t) = 0;
			virtual void* getMessageData(Socketmessage*) = 0;
			virtual void sendMessage(const Socketmessage*) = 0;
			virtual void releaseMessage(const Socketmessage*) = 0;
	};

	struct SocketClientCreator {
		static SocketClient* create(std::function<int(const Byte*, size_t)> splitMessage);
	};
}

#endif
