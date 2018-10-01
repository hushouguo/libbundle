/*
 * \file: CgiServer.h
 * \brief: Created by hushouguo at 14:51:35 Oct 01 2018
 */
 
#ifndef __CGISERVER_H__
#define __CGISERVER_H__

BEGIN_NAMESPACE_BUNDLE {
	class CgiRequest {
		public:
			virtual ~CgiRequest() = 0;

		public:
			virtual u32 id() = 0;
			virtual const char* header(const char* name) = 0;
			virtual const char* variable(const char* name) = 0;
			virtual const std::unordered_map<std::string, std::string>& headers() = 0;
			virtual const std::unordered_map<std::string, std::string>& variables() = 0;
						
			virtual void sendString(const char*, size_t) = 0;
			virtual void sendString(const std::string&) = 0;
			virtual void sendString(const std::ostringstream&) = 0;
			virtual void sendBinary(const void*, size_t) = 0;
			virtual void done() = 0;
	};

	class CgiServer {
		public:
			virtual ~CgiServer() = 0;

		public:
			virtual SOCKET fd() = 0;
			virtual bool start(const char* address, int port) = 0;
			virtual void stop() = 0;
			//virtual const Socketmessage* receiveMessage() = 0;
			//virtual void sendMessage(SOCKET s, const void*, size_t) = 0;
			//virtual void close(SOCKET s) = 0;
			//virtual size_t size() = 0;
			//virtual bool setsockopt(int opt, const void* optval, size_t optlen) = 0;
			//virtual bool getsockopt(int opt, void* optval, size_t optlen) = 0;

		public:
			virtual CgiRequest* getRequest() = 0;
			virtual void releaseRequest(CgiRequest*) = 0;
	};

	struct CgiServerCreator {
		static CgiServer* create();
	};
}

#endif
