/*
 * \file: WebServer.cpp
 * \brief: Created by hushouguo at 13:41:25 Sep 30 2018
 */

#include "bundle.h"
#include "Helper.h"
#include "Socket.h"

BEGIN_NAMESPACE_BUNDLE {
	class WebRequestInternal : public WebRequest {
		public:
			WebRequestInternal(u32 id, struct evhttp_request* evrequest, const char* decode_uri);
			~WebRequestInternal();

		public:
			u32 id() override { return this->_id; }
			evhttp_cmd_type cmd() override {
				return evhttp_request_get_command(this->_evrequest);
			}
			const char* url() override {
				return evhttp_request_get_uri(this->_evrequest);
			}
			const char* header(const char* name) override {
				return evhttp_find_header(this->_evrequest->input_headers, name);
			}
			const char* remote_host() override {
				return this->_evrequest->remote_host ? this->_evrequest->remote_host : "UNDEFINED";
			}
			int remote_port() override {
				return this->_evrequest->remote_port;
			}
			const char* variable(const char* name) override {
				auto i = this->_variables.find(name);
				return i != this->_variables.end() ? i->second.c_str() : nullptr;
			}
			const std::unordered_map<std::string, std::string>& headers() override {
				return this->_headers;
			}
			const std::unordered_map<std::string, std::string>& variables() override {
				return this->_variables;
			}

			void addHeader(const char* name, const char* value) override {
				evhttp_add_header(this->_evrequest->output_headers, name, value);
			}
			void removeHeader(const char* name) override {
				evhttp_remove_header(this->_evrequest->output_headers, name);
			}
			void clearHeader() override {
				evhttp_clear_headers(this->_evrequest->output_headers);
			}
			void pushString(const char* content) override {
				evbuffer_add(this->_evbuffer, content, strlen(content));
			}
			void pushString(const std::string& content) override {
				evbuffer_add(this->_evbuffer, content.c_str(), content.length());
			}
			void pushBinary(const void* data, size_t len) override {
				evbuffer_add(this->_evbuffer, data, len);
			}
			void send() override {
				evhttp_send_reply(this->_evrequest, HTTP_OK, "OK", this->_evbuffer);
				//the _evrequest will be freed in the callback function evhttp_send_reply or evhttp_send_done  
				//when server finishes writing.
				this->_evrequest = nullptr; 
			}

		private:
			u32 _id = 0;
			struct evbuffer* _evbuffer = nullptr;
			struct evhttp_request* _evrequest = nullptr;
			std::unordered_map<std::string, std::string> _headers;
			std::unordered_map<std::string, std::string> _variables;
	};


	WebRequestInternal::WebRequestInternal(u32 id, struct evhttp_request* evrequest, const char* decode_uri) {
		this->_id = id;
		this->_evbuffer = evbuffer_new();
		this->_evrequest = evrequest;
		assert(decode_uri);
		Debug << "decode uri: " << decode_uri;
		struct evhttp_uri* evuri = evhttp_uri_parse(decode_uri);
		if (evuri) {
			//Debug << "scheme: " << evhttp_uri_get_scheme(evuri);
			//Debug << "userinfo: " << evhttp_uri_get_userinfo(evuri);
			//Debug << "host: " << evhttp_uri_get_host(evuri);
			//Debug << "port: " << evhttp_uri_get_port(evuri);
			//Debug << "path: " << evhttp_uri_get_path(evuri);
			//Debug << "query: " << evhttp_uri_get_query(evuri);
			//Debug << "fragment: " << evhttp_uri_get_fragment(evuri);
			struct evkeyvalq headers;
			int rc = evhttp_parse_query_str(evhttp_uri_get_query(evuri), &headers);
			if (rc == 0) {
				for (struct evkeyval* header = headers.tqh_first; header; header = header->next.tqe_next) {
					this->_variables.insert(std::make_pair(header->key, header->value));
				}
			}
			if (true) {
				struct evkeyvalq* headers = evhttp_request_get_input_headers(evrequest);
				for (struct evkeyval* header = headers->tqh_first; header; header = header->next.tqe_next) {
					this->_headers.insert(std::make_pair(header->key, header->value));
				}		
			}
			evhttp_uri_free(evuri);
		}
		else {
			Error << "fail to parse decode uri:" << decode_uri;
		}
		SafeFree(decode_uri);
	}

	WebRequest::~WebRequest() {}
	WebRequestInternal::~WebRequestInternal() {
		evbuffer_free(this->_evbuffer);
		//evhttp_request_free(this->_evrequest);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	
	class WebServerInternal : public WebServer {
		public:
			WebServerInternal();
			~WebServerInternal();
			
		public:
			bool start(const char* address, int port) override;
			void stop() override;
			inline bool isstop() { return this->_stop; }
			void run() override;
			WebRequest* getRequest() override;
			void releaseRequest(WebRequest*) override;
			void createRequest(struct evhttp_request* evrequest);
		
		private:
			u32 _baseid = 0;
			bool _stop = false;
			struct evhttp* _evhttp = nullptr;
			struct event_base* _evbase = nullptr;
			std::list<WebRequestInternal*> _requests;
			const char* requestGet(struct evhttp_request* evrequest);
			const char* requestPost(struct evhttp_request* evrequest);
	};

	WebRequest* WebServerInternal::getRequest() {
		WebRequest* request = nullptr;
		if (!this->_requests.empty()) {
			request = this->_requests.front();
			this->_requests.pop_front();
		}
		return request;
	}
			
	void WebServerInternal::releaseRequest(WebRequest* request) {
		//Debug << "release request: " << request->id();
		SafeDelete(request);
	}

	const char* WebServerInternal::requestGet(struct evhttp_request* evrequest) {
		const char* uri = evhttp_request_get_uri(evrequest);
		char* decode_uri = evhttp_decode_uri(uri);
		//Debug << "HTTP GET, URI: " << decode_uri;
		return decode_uri;
	}

	//
	// form-data: 
	//  POST /do.php HTTP/1.1
	//  Host: 192.168.1.188:12306
	//  User-Agent: PostmanRuntime/7.1.5
	//  Content-Length: 268
	//  Accept: */*
	//  Accept-Encoding: gzip, deflate
	//  Cache-Control: no-cache
	//  Content-Type: multipart/form-data; boundary=--------------------------292034440064387749135212
	//  Postman-Token: 35f9b8bd-36e2-4e2c-837c-7825c781da1c
	//  X-Lantern-Version: 4.8.2
	//
	//  ----------------------------292034440064387749135212
	//  Content-Disposition: form-data; name="body1"
	//
	//  2
	//  ----------------------------292034440064387749135212
	//  Content-Disposition: form-data; name="body2"
	//
	//  amy
	//  ----------------------------292034440064387749135212--
	//
	// x-www-form-urlencoded:
	//	POST /do.php HTTP/1.1
	//	Host: 192.168.1.188:12306
	//	User-Agent: PostmanRuntime/7.1.5
	//	Content-Length: 24
	//	Accept: */*
	//	Accept-Encoding: gzip, deflate
	//	Cache-Control: no-cache
	//	Content-Type: application/x-www-form-urlencoded
	//	Postman-Token: c2a86fe6-d2ed-4971-bb22-8c2f0ec9d129
	//	X-Lantern-Version: 4.8.2
	//
	//	var1=1&var2=husosdgfsdfg
	//
	const char* WebServerInternal::requestPost(struct evhttp_request* evrequest) {
		std::ostringstream o;
		o << evhttp_request_get_uri(evrequest) << "?";
		struct evbuffer* buffer = evhttp_request_get_input_buffer(evrequest);
		while (evbuffer_get_length(buffer)) {
			char data[1024];
			int n = evbuffer_remove(buffer, data, sizeof(data) - 1);
			if (n > 0) {
				data[n] = '\0';
				o << data;
			}
		}
		//Debug << "HTTP POST, buffer: " << o.str();
		char* decode_uri = evhttp_decode_uri(o.str().c_str());
		//Debug << "HTTP POST, URI: " << decode_uri;
		return decode_uri;
	}

	void WebServerInternal::createRequest(struct evhttp_request* evrequest) {
		const char* decode_uri = nullptr;
		enum evhttp_cmd_type cmd = evhttp_request_get_command(evrequest);
		switch (cmd) {
			case EVHTTP_REQ_GET: decode_uri = this->requestGet(evrequest); break;
			case EVHTTP_REQ_POST: decode_uri = this->requestPost(evrequest); break;
			default: CHECK_RETURN(false, void(0), "unhandled http request cmd: %d\n", cmd);
		}

		if (!decode_uri) {
			evhttp_request_free(evrequest);
			return;
		}

		WebRequestInternal* request = new WebRequestInternal(this->_baseid++, evrequest, decode_uri);
		this->_requests.push_back(request);

		//Debug << "HTTP: new request: " << request->id();
	}

	static void RequestCallback(struct evhttp_request* evrequest, void* p) {
		WebServerInternal* thisServer = static_cast<WebServerInternal*>(p);
		thisServer->createRequest(evrequest);
	}
	
	bool WebServerInternal::start(const char* address, int port) {
		int rc = evhttp_bind_socket(this->_evhttp, address, port);
		CHECK_RETURN(rc == 0, false, "evhttp_bind_socket failure: %s,%d\n", address, port);
		evhttp_set_gencb(this->_evhttp, RequestCallback, this);
		Debug << "HTTP listening on: " << address << ", " << port;
		return true;
	}

	void WebServerInternal::run() {
		if (!this->isstop()) {
			//int retval = event_base_loopexit(this->_evbase, &tv); //Memory Leak !!!
			int retval = event_base_loop(this->_evbase, EVLOOP_ONCE | EVLOOP_NONBLOCK);
			if (retval != 0) {
				Error << "event_base_loop error: " << retval;
			}
		}
	}

	WebServerInternal::WebServerInternal() {
		this->_evbase = event_base_new();
		this->_evhttp = evhttp_new(this->_evbase);
	}

	void WebServerInternal::stop() {
		if (!this->isstop()) {
			this->_stop = true;
			if (this->_evhttp) {
				evhttp_free(this->_evhttp);
				this->_evhttp = nullptr;
			}
			if (this->_evbase) {
				event_base_free(this->_evbase);
				this->_evbase = nullptr;
			}
		}
	}

	WebServer::~WebServer() {}
	WebServerInternal::~WebServerInternal() {
		this->stop();
	}

	WebServer* WebServerCreator::create() {
		return new WebServerInternal();
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
