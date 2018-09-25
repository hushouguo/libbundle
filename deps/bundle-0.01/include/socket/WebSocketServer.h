/*
 * \file: WebSocketServer.h
 * \brief: Created by hushouguo at 07:46:17 Aug 10 2018
 */
 
#ifndef __WEB_SOCKET_SERVER_H__
#define __WEB_SOCKET_SERVER_H__

BEGIN_NAMESPACE_BUNDLE {
	class Socketmessage;
	class WebSocketServer {
		public:
			virtual ~WebSocketServer() = 0;

		public:
			virtual SOCKET fd() = 0;
			virtual bool start(const char* address, int port) = 0;
			virtual void stop() = 0;
			virtual const Socketmessage* receiveMessage(SOCKET& s, bool& establish, bool& close) = 0;
			virtual void sendMessage(SOCKET s, const void*, size_t) = 0;
			virtual void releaseMessage(const Socketmessage*) = 0;
			virtual void close(SOCKET s) = 0;
			virtual size_t size() = 0;
			virtual bool setsockopt(int opt, const void* optval, size_t optlen) = 0;
			virtual bool getsockopt(int opt, void* optval, size_t optlen) = 0;
	};

	struct WebSocketServerCreator {
		static WebSocketServer* create();
	};
}

#endif
