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
			virtual bool connect(const char* address, int port, u32 timeout) = 0;
			virtual void stop() = 0;
			virtual bool active() = 0;
			virtual const Socketmessage* receiveMessage(bool& establish, bool& close) = 0;
			virtual void sendMessage(const void*, size_t) = 0;
			virtual void sendMessage(const Socketmessage*) = 0;
	};

	struct SocketClientCreator {
		static SocketClient* create(MESSAGE_SPLITER splitMessage);
	};
}

#endif
