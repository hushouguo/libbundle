/*
 * \file: WebServer.h
 * \brief: Created by hushouguo at 13:40:07 Sep 30 2018
 */
 
#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#if 0
BEGIN_NAMESPACE_BUNDLE {
	enum HTTP_METHOD {
		HTTP_METHOD_GET		=	1,
		HTTP_METHOD_POST	=	2,
		HTTP_METHOD_HEAD	=	3,
		HTTP_METHOD_PUT		=	4,
		HTTP_METHOD_DELETE	=	5,
		HTTP_METHOD_OPTIONS	=	6,
		HTTP_METHOD_TRACE	=	7,
		HTTP_METHOD_CONNECT	=	8,
		HTTP_METHOD_PATCH	=	9,
	};

	class WebRequest {
		public:
			virtual ~WebRequest() = 0;

		public:
			virtual SOCKET fd() = 0;
			virtual void close() = 0;

		public:
			// request fields
			virtual HTTP_METHOD method() = 0;
			virtual const char* url() = 0;
			virtual const char* version() = 0;
			virtual const char* header(const char* name) = 0;
			virtual const char* variable(const char* name) = 0;
			virtual const char* rawdata() = 0;
			virtual const std::unordered_map<std::string, std::string>& HEADERS() = 0;
			virtual const std::unordered_map<std::string, std::string>& GETS() = 0;
			virtual const std::unordered_map<std::string, std::string>& POSTS() = 0;
			virtual const std::unordered_map<std::string, std::string>& COOKIES() = 0;
	};

	class WebServer {
		public:
			virtual ~WebServer() = 0;

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
			//request
			virtual const char* url(SOCKET) = 0;
			//response
			virtual void addHeader(SOCKET, const char* name, const char* value) = 0;
			virtual void removeHeader(SOCKET, const char* name) = 0;
			virtual void clearHeader(SOCKET) = 0;
			virtual void pushText(SOCKET, const char* content) = 0;
			virtual void pushData(SOCKET, const void* buffer, size_t len) = 0;

			using KV = std::unordered_map<std::string, std::string>;
			virtual const KV& HEADERS() = 0;
			virtual const KV& GETS() = 0;
			virtual const KV& POSTS() = 0;
	};

	struct WebServerCreator {
		static WebServer* create();
	};
}

#endif

#endif
