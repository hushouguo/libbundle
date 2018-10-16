/*
 * \file: CgiServer.h
 * \brief: Created by hushouguo at 14:51:35 Oct 01 2018
 */
 
#ifndef __CGISERVER_H__
#define __CGISERVER_H__

BEGIN_NAMESPACE_BUNDLE {
	enum HTTP_COMMAND {
		GET, POST	// HEAD, PUT, DELETE, CONNECT, OPTIONS, TRACE		
	};

	class CgiRequest {
		public:
			virtual ~CgiRequest() = 0;

		public:
			virtual u32 id() = 0;
			virtual HTTP_COMMAND cmd() = 0;
			virtual const char* url() = 0;
			virtual const char* inputHeader(const char* name) = 0;
			virtual const char* inputVariable(const char* name) = 0;
			virtual const std::unordered_map<std::string, std::string>& inputHeaders() = 0;
			virtual const std::unordered_map<std::string, std::string>& inputVariables() = 0;
						
			virtual void addHeader(const char* name, const char* value) = 0;
			virtual void addString(const char*, size_t) = 0;
			virtual void addString(const std::string&) = 0;
			virtual void addString(const std::ostringstream&) = 0;
			virtual void send() = 0;

			virtual void setCookie(const char* name, u32 seconds) = 0;			
			virtual void setCookie(const char* name, const char* value, u32 seconds) = 0;
			virtual const char* cookie(const char* name) = 0;
			virtual const std::unordered_map<std::string, std::string>& cookies() = 0;
	};

	class CgiServer {
		public:
			virtual ~CgiServer() = 0;

		public:
			virtual SOCKET fd() = 0;
			virtual bool start(const char* address, int port) = 0;
			virtual void stop() = 0;

		public:
			virtual CgiRequest* getRequest() = 0;
			virtual void releaseRequest(CgiRequest*) = 0;
	};

	struct CgiServerCreator {
		static CgiServer* create();
	};
}

#endif
