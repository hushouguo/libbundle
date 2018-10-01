/*
 * \file: WebServer.h
 * \brief: Created by hushouguo at 13:40:07 Sep 30 2018
 */
 
#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

BEGIN_NAMESPACE_BUNDLE {
	class WebRequest {
		public:
			virtual ~WebRequest() = 0;

		public:
			// request fields
			virtual u32 id() = 0;
			virtual evhttp_cmd_type cmd() = 0;
			virtual const char* url() = 0;
			virtual const char* header(const char* name) = 0;
			virtual const char* remote_host() = 0;
			virtual int remote_port() = 0;
			virtual const char* variable(const char* name) = 0;
			// response routine
			virtual void addHeader(const char* name, const char* value) = 0;
			virtual void removeHeader(const char* name) = 0;
			virtual void clearHeader() = 0;
			virtual void pushString(const char* content) = 0;
			virtual void pushString(const std::string& content) = 0;
			virtual void pushBinary(const void* data, size_t len) = 0;
			virtual void send() = 0;
			// general response
			// like: 100 OK, 505 etc..
	};

	class WebServer {
		public:
			virtual ~WebServer() = 0;

		public:
			virtual bool start(const char* address, int port) = 0;
			virtual void stop() = 0;

		public:
			virtual void run() = 0;
			virtual WebRequest* getRequest() = 0;
			virtual void releaseRequest(WebRequest*) = 0;
	};

	struct WebServerCreator {
		static WebServer* create();
	};
}

#endif
