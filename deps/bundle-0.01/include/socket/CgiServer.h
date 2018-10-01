/*
 * \file: CgiServer.h
 * \brief: Created by hushouguo at 14:51:35 Oct 01 2018
 */
 
#ifndef __CGISERVER_H__
#define __CGISERVER_H__

BEGIN_NAMESPACE_BUNDLE {
	class CgiServer {
		public:
			virtual ~CgiServer() = 0;

		public:
			virtual SOCKET fd() = 0;
			virtual bool start(const char* address, int port) = 0;
			virtual void stop() = 0;
			virtual const Socketmessage* receiveMessage() = 0;
			virtual void sendMessage(SOCKET s, const void*, size_t) = 0;
			virtual void close(SOCKET s) = 0;
			virtual size_t size() = 0;
			virtual bool setsockopt(int opt, const void* optval, size_t optlen) = 0;
			virtual bool getsockopt(int opt, void* optval, size_t optlen) = 0;

		public:
			virtual void sendString(SOCKET, const char*, size_t) = 0;
			virtual void sendString(SOCKET, const std::string&) = 0;
			virtual void sendString(SOCKET, const std::ostringstream&) = 0;
			virtual void sendBinary(SOCKET, const void*, size_t) = 0;
	};

	struct CgiServerCreator {
		static CgiServer* create();
	};
}

#endif
