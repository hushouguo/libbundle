/*
 * \file: SocketClient.h
 * \brief: Created by hushouguo at 01:56:02 Aug 09 2018
 */
 
#ifndef __SOCKET_CLIENT_H__
#define __SOCKET_CLIENT_H__

BEGIN_NAMESPACE_BUNDLE {
	class SocketClient {
		public:
			virtual ~SocketClient() = 0;
			
		public:
			virtual SOCKET fd() = 0;
			virtual bool connect(const char* address, int port, bool wait) = 0;
			virtual void stop() = 0;
			virtual bool active() = 0;
			virtual Rawmessage* receiveMessage(bool& establish, bool& close) = 0;
			virtual void sendMessage(const void*, size_t) = 0;
			virtual Rawmessage* initMessage(size_t) = 0;
			virtual void* getMessageData(Rawmessage*) = 0;
			virtual void sendMessage(const Rawmessage*) = 0;
			virtual void releaseMessage(Rawmessage*) = 0;
	};

	struct SocketClientCreator {
		static SocketClient* create(std::function<int(const Byte*, size_t)> splitMessage);
	};
}

#endif
