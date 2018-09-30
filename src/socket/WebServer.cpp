/*
 * \file: WebServer.cpp
 * \brief: Created by hushouguo at 13:41:25 Sep 30 2018
 */

#if 0
#include "bundle.h"
#include "Helper.h"
#include "Socket.h"

#define SP		' '		// 32
#define LF		'\n'	// 10
#define CR		'\r'	// 13

BEGIN_NAMESPACE_BUNDLE {

	class WebServerInternal : public WebServer {
		public:
			WebServerInternal();
			~WebServerInternal();
			
		public:
			SOCKET fd() override {	return this->_socketServer->fd(); }
			bool start(const char* address, int port) override { return this->_socketServer->start(address, port); }
			void stop() override { this->_socketServer->stop(); }
			const Socketmessage* receiveMessage() override;
			void sendMessage(SOCKET s, const void*, size_t) override;
			void close(SOCKET s) override { this->_socketServer->close(s); }
			size_t size() override { return this->_socketServer->size(); }
			bool setsockopt(int opt, const void* optval, size_t optlen) override { 
				return this->_socketServer->setsockopt(opt, optval, optlen); 
			}
			bool getsockopt(int opt, void* optval, size_t optlen) override { 
				return this->_socketServer->getsockopt(opt, optval, optlen); 
			}
		
		private:
			SocketServer* _socketServer = nullptr;
			std::set<std::string> _validMethods = { "GET", "POST" };
			std::set<std::string> _validVersions = { "HTTP/1.0", "HTTP/1.1", "HTTP/2" }; // UNUSED HTTP/0.9
			int parse_http_request(const char* buffer, size_t len);
	};

	WebServerInternal::WebServerInternal() {
		this->_socketServer = SocketServerCreator::create([this](const void* data, size_t len) -> int {
			const char* buffer = (const char*) data;

			if (len >= 3 && buffer[0] == 'G' && buffer[1] == 'E' && buffer[2] == 'T') {	// GET
				std::string Sec_WebSocket_Key;
				return this->parse_handshake_request(buffer, len, Sec_WebSocket_Key);
			}
			else {
				return this->parsePackage(buffer, len);
			}
		});
	}

	WebServer::~WebServer() {}
	WebServerInternal::~WebServerInternal() {
		this->stop();
	}

	WebServer* WebServerCreator::create() {
		return new WebServerInternal();
	}
			
	int WebServerInternal::parse_http_request(const char* buffer, size_t len) {
		std::istringstream is((const char*) buffer);

		// request line
		std::string request_line;
		try {
			// std::getline already filter '\n'
			std::getline(is, request_line);// METHOD[SP]URL[SP]HTTP/1.1[CRLF]
		} 
		catch(std::exception& e) {
			CHECK_RETURN(false, -1, "get request line exception: %s", e.what());
		}

		CHECK_RETURN(!request_line.empty(), -1, "request line is empty: %s", buffer);
		CHECK_RETURN(request_line[request_line.size()-1] == CR, -1, "lack CR: %s", buffer);
		request_line.erase(request_line.end() - 1);  // remove last char: CR

		// METHOD
		std::string::size_type i = request_line.find(":", 0);
		CHECK_RETURN(i != std::string::npos, -1, "lack METHOD: %s", request_line.c_str());
		std::string METHOD = request_line.substr(0, i); ++i;

		// URL
		std::string::size_type ii = request_line.find(":", i);
		CHECK_RETURN(ii != std::string::npos, -1, "lack URL: %s", request_line.c_str());
		std::string URL = request_line.substr(i, ii - i); ++ii;

		// VERSION
		CHECK_RETURN(ii != std::string::npos, -1, "lack VERSION: %s", request_line.c_str());
		std::string VERSION = request_line.substr(ii);
		
		// header
		std::string header;
		while (true) {
			try {
				// std::getline already filter '\n'
				std::getline(is, header);// header:[LWS]*data[CRLF]
			} 
			catch(std::exception& e) {
				CHECK_RETURN(false, -1, "get header exception: %s", e.what());
			}

			CHECK_RETURN(!header.empty(), 0, "incomplete HTTP request");
			CHECK_CONTINUE(header[header.size()-1] == CR, "there is a LF in the middle of header");

			header.erase(header.end() - 1);   // remove last char: CR
			if (header.empty()) {
				break; // end of headers, CRLF
			}

			std::string::size_type i = header.find(":", 0);
			CHECK_RETURN(i != std::string::npos, -1, "illegal HEADER: %s", header.c_str());
			std::string name = header.substr(0, i); ++i;
			std::string value = header.substr(i);
			if (!value.empty()) {
				value.erase(0, value.find_first_not_of(" "));	// ltrim
				value.erase(value.find_last_not_of(" ") + 1);	// rtrim
			}
		}
	
		// body	

	}
}

//
// HT: horizontal-tab
// SP: white space
// CR: carriage return \r
// LF: linefeed \n
// LWS:linear whitespace  
//
// request:
//	HTTP 0.9:
//		1: GET[SP]URL[CRLF]
//		[CRLF](end of headers)
//		...(request body)
//
//	HTTP 1.0/1.1:
//		1:METHOD[SP]URL[SP]HTTP/1.1[CRLF]
//		2:header:[LWS]*data[CRLF]
//		3:header:[LWS]*data[CRLF]
//		n:...[CRLF]
//		[CRLF](end of headers)
//		...(request body)
//		LWS = SP|HT
//
//
//
// response:
//
//
//
// List of HTTP header fields
//	A-IM	Acceptable instance-manipulations for the request[7].	A-IM: feed
//	Accept	Media type(s) that is(/are) acceptable for the response. See Content negotiation.	Accept: text/html
//	Accept-Charset	Character sets that are acceptable.	Accept-Charset: utf-8
//	Accept-Encoding	List of acceptable encodings. See HTTP compression.	Accept-Encoding: gzip, deflate
//	Accept-Language	List of acceptable human languages for response. See Content negotiation.	Accept-Language: en-US
//	Accept-Datetime	Acceptable version in time.	Accept-Datetime: Thu, 31 May 2007 20:35:00 GMT
//	Access-Control-Request-Method,
//	Access-Control-Request-Headers[8]	Initiates a request for cross-origin resource sharing with Origin (below).	Access-Control-Request-Method: GET
//	Authorization	Authentication credentials for HTTP authentication.	Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==
//	Cache-Control	Used to specify directives that must be obeyed by all caching mechanisms along the request-response chain.	Cache-Control: no-cache
//	Connection	Control options for the current connection and list of hop-by-hop request fields.[9]
//	Must not be used with HTTP/2.[10]
//
//	Connection: keep-alive
//	Connection: Upgrade
//	Content-Length	The length of the request body in octets (8-bit bytes).	Content-Length: 348
//	Content-MD5	A Base64-encoded binary MD5 sum of the content of the request body.	Content-MD5: Q2hlY2sgSW50ZWdyaXR5IQ==
//	Content-Type	The Media type of the body of the request (used with POST and PUT requests).	Content-Type: application/x-www-form-urlencoded
//	Cookie	An HTTP cookie previously sent by the server with Set-Cookie (below).	Cookie: $Version=1; Skin=new;
//	Date	The date and time that the message was originated (in "HTTP-date" format as defined by RFC 7231 Date/Time Formats).	Date: Tue, 15 Nov 1994 08:12:31 GMT
//	Expect	Indicates that particular server behaviors are required by the client.	Expect: 100-continue
//	Forwarded	Disclose original information of a client connecting to a web server through an HTTP proxy.[12]	Forwarded: for=192.0.2.60;proto=http;by=203.0.113.43 Forwarded: for=192.0.2.43, for=198.51.100.17
//	From	The email address of the user making the request.	From: user@example.com
//	Host	The domain name of the server (for virtual hosting), and the TCP port number on which the server is listening. The port number may be omitted if the port is the standard port for the service requested.
//	Mandatory since HTTP/1.1.[13] If the request is generated directly in HTTP/2, it should not be used.[14]
//
//	Host: en.wikipedia.org:8080
//	Host: en.wikipedia.org
//	If-Match	Only perform the action if the client supplied entity matches the same entity on the server. This is mainly for methods like PUT to only update a resource if it has not been modified since the user last updated it.	If-Match: "737060cd8c284d8af7ad3082f209582d"
//	If-Modified-Since	Allows a 304 Not Modified to be returned if content is unchanged.	If-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT
//	If-None-Match	Allows a 304 Not Modified to be returned if content is unchanged, see HTTP ETag.	If-None-Match: "737060cd8c284d8af7ad3082f209582d"
//	If-Range	If the entity is unchanged, send me the part(s) that I am missing; otherwise, send me the entire new entity.	If-Range: "737060cd8c284d8af7ad3082f209582d"
//	If-Unmodified-Since	Only send the response if the entity has not been modified since a specific time.	If-Unmodified-Since: Sat, 29 Oct 1994 19:43:31 GMT
//	Max-Forwards	Limit the number of times the message can be forwarded through proxies or gateways.	Max-Forwards: 10
//	Origin[8]	Initiates a request for cross-origin resource sharing (asks server for Access-Control-* response fields).	Origin: http://www.example-social-network.com
//	Pragma	Implementation-specific fields that may have various effects anywhere along the request-response chain.	Pragma: no-cache
//	Proxy-Authorization	Authorization credentials for connecting to a proxy.	Proxy-Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==
//	Range	Request only part of an entity. Bytes are numbered from 0. See Byte serving.	Range: bytes=500-999
//	Referer [sic]	This is the address of the previous web page from which a link to the currently requested page was followed. (The word “referrer” has been misspelled in the RFC as well as in most implementations to the point that it has become standard usage and is considered correct terminology)	Referer: http://en.wikipedia.org/wiki/Main_Page
//	TE	The transfer encodings the user agent is willing to accept: the same values as for the response header field Transfer-Encoding can be used, plus the "trailers" value (related to the "chunked" transfer method) to notify the server it expects to receive additional fields in the trailer after the last, zero-sized, chunk.
//	Only trailers is supported in HTTP/2.[10]
//
//	TE: trailers, deflate
//	User-Agent	The user agent string of the user agent.	User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:12.0) Gecko/20100101 Firefox/12.0
//	Upgrade	Ask the server to upgrade to another protocol.
//	Must not be used in HTTP/2.[10]
//
//	Upgrade: h2c, HTTPS/1.3, IRC/6.9, RTA/x11, websocket
//	Via	Informs the server of proxies through which the request was sent.	Via: 1.0 fred, 1.1 example.com (Apache/1.1)
//	Warning	A general warning about possible problems with the entity body.	Warning: 199 Miscellaneous warning
//
// List of HTTP reponse header fields:
//

#endif
