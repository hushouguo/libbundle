/*
 * \file: CgiServer.cpp
 * \brief: Created by hushouguo at 14:55:35 Oct 01 2018
 */

#include "bundle.h"
#include "Helper.h"
#include "Socket.h"

BEGIN_NAMESPACE_BUNDLE {
	enum FASTCGI_REQUEST_TYPE {
		FASTCGI_BEGIN_REQUEST      	= 1,
		FASTCGI_ABORT_REQUEST      	= 2,	// CgiServer-side to WebServer
		FASTCGI_END_REQUEST        	= 3,
		FASTCGI_PARAMS             	= 4,	// on PHP: $_SERVERS, QUERY_STRING: $GETS
		FASTCGI_STDIN              	= 5,	// $POSTS
		FASTCGI_STDOUT             	= 6,
		FASTCGI_STDERR             	= 7,
		FASTCGI_DATA               	= 8,
		FASTCGI_GET_VALUES         	= 9,
		FASTCGI_GET_VALUES_RESULT  	= 10,
		FASTCGI_UNKOWN_TYPE        	= 11
	};

	enum FASTCGI_ROLE {
		FASTCGI_RESPONDER      		= 1,
		FASTCGI_AUTHORIZER     		= 2,
		FASTCGI_FILTER         		= 3
	};

	enum FASTCGI_PROTOCOL_STATUS {
		FASTCGI_REQUEST_COMPLETE 	= 0,
		FASTCGI_CANT_MPX_CONN 		= 1,
		FASTCGI_OVERLOADED 			= 2,
		FASTCGI_UNKNOWN_ROLE 		= 3
	};

#define FASTCGI_VERSION				1
#define FASTCGI_KEEP_ALIVE			1	

#pragma pack(push, 1)
	struct fastcgi_header {
		unsigned char version;
		unsigned char type;
		unsigned char requestid_high;
		unsigned char requestid_low;
		unsigned char content_length_high;
		unsigned char content_length_low;
		unsigned char padding_length;
		unsigned char reserved;
		inline u32 requestid() { return (this->requestid_high << 8) + this->requestid_low; }
		inline void requestid(u32 value) {
			this->requestid_high = (value >> 8) & 0xff;
			this->requestid_low = value & 0xff;
		}
		inline u32 content_length() { return (this->content_length_high << 8) + this->content_length_low; }
		inline void content_length(u32 len) {
			this->content_length_high = (len >> 8) & 0xff;
			this->content_length_low = len & 0xff;
			this->padding_length = (len % 8) > 0 ? 8 - (len % 8) : 0;	// align 8 bytes
		}
		inline u32 package_length() { return sizeof(fastcgi_header) + this->content_length() + this->padding_length; }
	};

	struct fastcgi_begin_request {
		unsigned char role_high;
		unsigned char role_low;
		unsigned char flags;
		unsigned char reserved[5];
		inline u32 role() { return (this->role_high << 8) + this->role_low; }
		inline bool keepalive() { return (this->flags & FASTCGI_KEEP_ALIVE) != 0; }
	};

	struct fastcgi_end_request {
		unsigned char appStatusB3;
		unsigned char appStatusB2;
		unsigned char appStatusB1;
		unsigned char appStatusB0;
		unsigned char protocolStatus;
		unsigned char reserved[3];
	};
#pragma pack(pop)
	
	class CgiRequestInternal : public CgiRequest {
		public:
			CgiRequestInternal(u32 id, SocketServer* socketServer);
			~CgiRequestInternal();

		public:
			inline bool keepalive() { return this->_keepalive; }
			void keepalive(bool value) { this->_keepalive = value; }
			inline FASTCGI_ROLE role() { return this->_role; }
			void role(FASTCGI_ROLE value) { this->_role = value; }
			inline bool isParamsDone() { return this->_recv_params_done; }
			void receiveParamsDone();
			inline bool isStdinDone() { return this->_recv_stdin_done; }
			void receiveStdinDone();
			inline SOCKET fd() { return this->_socket; }
			void fd(SOCKET s) { this->_socket = s; }
			void addInputHeader(const char* key, const char* value);
			void addInputVariable(const char* key, const char* value);

		public:
			u32 id() override { return this->_id; }
			HTTP_COMMAND cmd() override { return this->_cmd; }
			void cmd(HTTP_COMMAND value) { this->_cmd = value; }
			const char* url() override { return this->_url.c_str(); }
			void url(const char* value) { this->_url = value; }
			const char* inputHeader(const char* name) override {
				auto i = this->_input_headers.find(name);
				return i != this->_input_headers.end() ? i->second.c_str() : nullptr;
			}
			const char* inputVariable(const char* name) override {
				auto i = this->_input_variables.find(name);
				return i != this->_input_variables.end() ? i->second.c_str() : nullptr;
			}
			const std::unordered_map<std::string, std::string>& inputHeaders() override {
				return this->_input_headers;
			}
			const std::unordered_map<std::string, std::string>& inputVariables() override {
				return this->_input_variables;
			}
			
			// response
			void addHeader(const char* name, const char* value) override;			
			void addString(const char*, size_t) override;
			void addString(const std::string&) override;
			void addString(const std::ostringstream&) override;
			//void addBinary(const void*, size_t) override;
			void send() override;

			// cookie
			void setCookie(const char* name, u32 seconds) override {
				u64 expires = currentSecond() + seconds;
				char time_buffer[64];
				timestamp_gmt(time_buffer, sizeof(time_buffer), expires);

				std::ostringstream o;
				o << name << ";" << "expires=" << time_buffer;

				this->addHeader("Set-Cookie", o.str().c_str());
			}
			void setCookie(const char* name, const char* value, u32 seconds) override {
				u64 expires = currentSecond() + seconds;
				char time_buffer[64];
				timestamp_gmt(time_buffer, sizeof(time_buffer), expires);

				std::ostringstream o;
				o << name << "=" << value << ";" << "expires=" << time_buffer;

				this->addHeader("Set-Cookie", o.str().c_str());
			}

			void addInputCookie(const char* name, const char* value);
			const char* cookie(const char* name) override {
				auto i = this->_cookies.find(name);
				return i != this->_cookies.end() ? i->second.c_str() : nullptr;
			}
			const std::unordered_map<std::string, std::string>& cookies() override { return this->_cookies; }

		private:
			u32 _id = 0;
			SOCKET _socket = -1;
			SocketServer* _socketServer = nullptr;
			bool _keepalive = false;
			bool _recv_params_done = false, _recv_stdin_done = false;
			std::string _url;
			HTTP_COMMAND _cmd = GET;
			FASTCGI_ROLE _role = FASTCGI_RESPONDER;
			std::unordered_map<std::string, std::string> _input_headers;
			std::unordered_map<std::string, std::string> _input_variables;
			std::unordered_map<std::string, std::string> _cookies;

		private:
			std::unordered_map<std::string, std::string> _output_headers;
			std::ostringstream _output_content;
	};
			
	CgiRequestInternal::CgiRequestInternal(u32 id, SocketServer* socketServer) {
		this->_id = id;
		this->_socketServer = socketServer;
		//
		// initialize response header, Only support type of text
		this->addHeader("Content-type", "text/html");
	}

	CgiRequest::~CgiRequest() {}
	CgiRequestInternal::~CgiRequestInternal() {
		this->_socketServer->close(this->_socket);
	}

	void CgiRequestInternal::receiveParamsDone() {
		this->_recv_params_done = true;
	}
	
	void CgiRequestInternal::receiveStdinDone() { 
		this->_recv_stdin_done = true;
		if (true) {
			//HTTP_COOKIE:username=hushouguo; id=100; level
			const char* value = this->inputHeader("HTTP_COOKIE");
			CHECK_RETURN(value, void(0), "not found `HTTP_COOKIE`");
			std::vector<std::string> fields;
			bool rc = splitString(value, ';', fields);
			CHECK_RETURN(rc, void(0), "split HTTP_COOKIE: %s error", value);
			for (auto& s : fields) {
				std::vector<std::string> v;
				rc = splitString(s.c_str(), '=', v);
				CHECK_RETURN(rc, void(0), "split field: %s error", s.c_str());
				CHECK_RETURN(v.size() == 1 || v.size() == 2, void(0), "illegal field: %s", s.c_str());
				addInputCookie(v[0].c_str(), v.size() == 2 ? v[1].c_str() : "");
			}
		}

		if (true) {
			//REQUEST_METHOD:GET
			const char* value = this->inputHeader("REQUEST_METHOD");
			CHECK_RETURN(value, void(0), "not found `REQUEST_METHOD`");
			u32 hashValue = hashString(value);
			if (hashValue == hashString("GET")) { this->_cmd = GET;	
			} else if (hashValue == hashString("POST")) { this->_cmd = POST;
			} else {
				Alarm << "Unsupport REQUEST_METHOD: " << value;
			}
		}

		// url: http://192.168.1.188/account/do.php?aa=1&bb=adsdasd
		// REQUEST_URI:   	/account/do.php?aa=1&bb=adsdasd
		// DOCUMENT_URI : 	/account/do.php
		// SCRIPT_NAME:   	/account/do.php
		// SCRIPT_FILENAME:	/usr/local/html/account/do.php
		// QUERY_STRING:	aa=1&bb=adsdasd
		if (true) {
			const char* value = this->inputHeader("SCRIPT_NAME");
			CHECK_RETURN(value, void(0), "not found `SCRIPT_NAME`");
			this->_url = value;
		}

		if (true) {
			const char* value = this->inputHeader("QUERY_STRING");
			CHECK_RETURN(value, void(0), "not found `QUERY_STRING`");

			std::string value_decoded, value_raw = value;
			bool rc = url_decode(value_raw, value_decoded);
			CHECK_RETURN(rc, void(0), "decode QUERY_STRING: %s error", value);

			std::vector<std::string> fields;
			rc = splitString(value_decoded.c_str(), '&', fields);
			CHECK_RETURN(rc, void(0), "split QUERY_STRING: %s error", value);
			for (auto& s : fields) {
				std::vector<std::string> v;
				rc = splitString(s.c_str(), '=', v);
				CHECK_RETURN(rc, void(0), "split field: %s error", s.c_str());
				CHECK_RETURN(v.size() == 2, void(0), "illegal field: %s", s.c_str());
				addInputVariable(v[0].c_str(), v[1].c_str());
			}
		}
	}

	void CgiRequestInternal::addInputCookie(const char* key, const char* value) {
		this->_cookies[key] = value;
	}

	void CgiRequestInternal::addInputHeader(const char* key, const char* value) {
		this->_input_headers[key] = value;
	}

	void CgiRequestInternal::addInputVariable(const char* key, const char* value) {
		this->_input_variables[key] = value;
	}

	void CgiRequestInternal::addHeader(const char* name, const char* value) {
		bool rc = this->_output_headers.insert(std::make_pair(name, value)).second;
		CHECK_ALARM(rc, "addHeader: %s already exist, value: %s", name, value);
	}

	void CgiRequestInternal::addString(const char* data, size_t len) {
		//Note: urlencode
		this->_output_content << data;
	}

	void CgiRequestInternal::addString(const std::string& s) {
		this->addString(s.data(), s.length());
	}

	void CgiRequestInternal::addString(const std::ostringstream& o) {
		const std::string& s = o.str();
		this->addString(s);
	}

//	void CgiRequestInternal::addBinary(const void*, size_t) {
//		//Note: Unrealized
//	}

	void CgiRequestInternal::send() {
		// send response header
		if (true) {
			std::ostringstream o;
			for (auto& i : this->_output_headers) {
				o << i.first << ": " << i.second << "\r\n";
			}
			o << "\r\n";	// end of headers

			const std::string& s = o.str();

			fastcgi_header header;
			header.version = FASTCGI_VERSION;
			header.type = FASTCGI_STDOUT;
			header.requestid(this->_id);		// requestid_high && requestid_low
			header.content_length(s.length());	// content_length_high && content_length_low && padding_length
			header.reserved = 0;
			this->_socketServer->sendMessage(this->_socket, &header, sizeof(fastcgi_header));
			this->_socketServer->sendMessage(this->_socket, s.data(), s.length());
			if (header.padding_length > 0) {
				u8 padding[header.padding_length];
				this->_socketServer->sendMessage(this->_socket, padding, sizeof(padding));
			}
		}
	
		// send response content
		const std::string& s = this->_output_content.str();
		if (s.length() > 0) {
			fastcgi_header header;
			header.version = FASTCGI_VERSION;
			header.type = FASTCGI_STDOUT;
			header.requestid(this->_id);		// requestid_high && requestid_low
			header.content_length(s.length());	// content_length_high && content_length_low && padding_length
			header.reserved = 0;
			this->_socketServer->sendMessage(this->_socket, &header, sizeof(fastcgi_header));
			this->_socketServer->sendMessage(this->_socket, s.data(), s.length());
			if (header.padding_length > 0) {
				u8 padding[header.padding_length];
				this->_socketServer->sendMessage(this->_socket, padding, sizeof(padding));
			}
		}

		// send end of stdout
		if (true) {
			fastcgi_header header;
			header.version = FASTCGI_VERSION;
			header.type = FASTCGI_STDOUT;
			header.requestid(this->_id);	// requestid_high && requestid_low
			header.content_length(0);		// end of stdout
			header.reserved = 0;
			this->_socketServer->sendMessage(this->_socket, &header, sizeof(fastcgi_header));
		}

		// send end of request header
		if (true) {
			fastcgi_header header;
			header.version = FASTCGI_VERSION;
			header.type = FASTCGI_END_REQUEST;
			header.requestid(this->_id);			// requestid_high && requestid_low
			header.content_length(sizeof(fastcgi_end_request));	// content_length_high && content_length_low && padding_length
			header.reserved = 0;
			this->_socketServer->sendMessage(this->_socket, &header, sizeof(fastcgi_header));
		}

		// send end of request body
		if (true) {
			fastcgi_end_request body;
			body.protocolStatus = FASTCGI_REQUEST_COMPLETE;
			this->_socketServer->sendMessage(this->_socket, &body, sizeof(fastcgi_end_request));
		}
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//
		
	class CgiServerInternal : public CgiServer {
		public:
			CgiServerInternal();
			~CgiServerInternal();

		public:
			SOCKET fd() override { return this->_socketServer->fd(); }
			bool start(const char* address, int port) override { return this->_socketServer->start(address, port); }
			void stop() override;
//			bool setsockopt(int opt, const void* optval, size_t optlen) override { 
//				return this->_socketServer->setsockopt(opt, optval, optlen); 
//			}
//			bool getsockopt(int opt, void* optval, size_t optlen) override {
//				return this->_socketServer->getsockopt(opt, optval, optlen);
//			}
			CgiRequest* getRequest() override;
			void releaseRequest(CgiRequest*) override;

		private:
			SocketServer* _socketServer = nullptr;
			std::unordered_map<u32, CgiRequestInternal*> _requests;
			CgiRequest* getCompleteRequest();
			bool parsePackage(SOCKET, const void* buffer, size_t len);
			bool beginRequest(CgiRequestInternal*, fastcgi_header* header, const void* buffer, size_t len);
			bool readParams(CgiRequestInternal*, fastcgi_header* header, const void* buffer, size_t len);
			bool readStdin(CgiRequestInternal*, fastcgi_header* header, const void* buffer, size_t len);
			bool readData(CgiRequestInternal*, fastcgi_header* header, const void* buffer, size_t len);
			const char* requestString(u8 type) {
				static const char* __typestring[] = {
					[0]							=	"NONE",
					[FASTCGI_BEGIN_REQUEST]		=	"BEGIN_REQUEST",
					[FASTCGI_ABORT_REQUEST]		=	"ABORT_REQUEST",
					[FASTCGI_END_REQUEST]		=	"END_REQUEST",
					[FASTCGI_PARAMS]			=	"PARAMS",
					[FASTCGI_STDIN]				=	"STDIN",
					[FASTCGI_STDOUT]			=	"STDOUT",
					[FASTCGI_STDERR]			=	"STDERR",
					[FASTCGI_DATA]				=	"DATA",
					[FASTCGI_GET_VALUES]		=	"GET_VALUES",
					[FASTCGI_GET_VALUES_RESULT]	=	"GET_VALUES_RESULT",
					[FASTCGI_UNKOWN_TYPE]		=	"UNKNOWN_TYPE"
				};
				return type < sizeof(__typestring) ? __typestring[type] : "NONE";
			}
	};
			
	void CgiServerInternal::stop() {
		Debug << "CgiServer unhandled request: " << this->_requests.size();
		for (auto& i : this->_requests) {
			CgiRequest* request = i.second;
			SafeDelete(request);
		}
		SafeDelete(this->_socketServer);
	}

	CgiRequest* CgiServerInternal::getCompleteRequest() {
		CgiRequest* request = nullptr;
		if (!this->_requests.empty()) {
			u32 requestid = 0;
			for (auto& i : this->_requests) {
				if (i.second->isParamsDone() && i.second->isStdinDone()) {
					requestid = i.first;
					break;
				}
			}
			auto i = this->_requests.find(requestid);
			if (i != this->_requests.end()) {
				request = i->second;
				this->_requests.erase(i);
			}
		}
		return request;
	}

	CgiRequest* CgiServerInternal::getRequest() {
		while (true) {
			Socketmessage* msg = (Socketmessage *) this->_socketServer->receiveMessage();
			if (!msg) {
				return this->getCompleteRequest();
			}

			SOCKET s = GET_MESSAGE_SOCKET(msg);
			if (IS_ESTABLISH_MESSAGE(msg) || IS_CLOSE_MESSAGE(msg)) {
				bundle::releaseMessage(msg);
				continue;
			}

			this->parsePackage(s, GET_MESSAGE_PAYLOAD(msg), GET_MESSAGE_PAYLOAD_LENGTH(msg));
		}
	}

	void CgiServerInternal::releaseRequest(CgiRequest* request) {
		Debug << "release request: " << request->id();
		auto i = this->_requests.find(request->id());
		if (i != this->_requests.end()) {
			this->_requests.erase(i);
		}
		SafeDelete(request);
	}

	bool CgiServerInternal::parsePackage(SOCKET s, const void* buffer, size_t len) {
		// fastcgi_header
		assert(len >= sizeof(fastcgi_header));
		fastcgi_header* header = (fastcgi_header *) buffer;
		assert(len == header->package_length());
		assert(len >= header->content_length());
		buffer = (u8*) buffer + sizeof(fastcgi_header);
		len -= sizeof(fastcgi_header);
		Debug.cout("fastcgi version: %d, type: %s, requestid: %d, content: %d, padding: %d, package: %d, len: %ld",
				header->version, this->requestString(header->type), header->requestid(), 
				header->content_length(), header->padding_length, header->package_length(), len);

		CgiRequestInternal* request = FindOrNull(this->_requests, header->requestid());
		if (!request) {
			request = new CgiRequestInternal(header->requestid(), this->_socketServer);
			request->fd(s);
			this->_requests.insert(std::make_pair(request->id(), request));
			Debug << "spawn new request: " << request->id() << ", fd: " << request->fd();
		}
		assert(request->fd() == s);

		bool rc = false;
		switch (header->type) {
			case FASTCGI_BEGIN_REQUEST: 
				rc = this->beginRequest(request, header, buffer, len); break;
			case FASTCGI_PARAMS: 
				rc = this->readParams(request, header, buffer, len); break;
			case FASTCGI_STDIN: 
				rc = this->readStdin(request, header, buffer, len); break;
			case FASTCGI_DATA: 
				rc = this->readData(request, header, buffer, len); break;
			default: 
				Alarm << "Unhandle FASTCGI header: " << (u32) header->type; break;
		}

		if (!rc) {
			this->releaseRequest(request);
		}
		return rc;
	}

	bool CgiServerInternal::beginRequest(CgiRequestInternal* request, fastcgi_header* header, const void* buffer, size_t len) {
		assert(len >= sizeof(fastcgi_begin_request));
		fastcgi_begin_request* msg = (fastcgi_begin_request *) buffer;
		Debug << "role: " << msg->role() << ", flags: " << (u32) msg->flags;
		request->role((FASTCGI_ROLE) msg->role());
		request->keepalive(msg->flags & FASTCGI_KEEP_ALIVE);
		CHECK_RETURN(request->role() == FASTCGI_RESPONDER, false, "Only support response role: %d", request->role());
		return true;
	}

	// For GET command, exist key is QUERY_STRING, value is like: id=1&name=hushouguo
	bool CgiServerInternal::readParams(CgiRequestInternal* request, fastcgi_header* header, const void* buffer, size_t len) {
		if (len == 0) {
			request->receiveParamsDone();
			return true;	// read params done, it's the last package with content_length == 0
		}
		
		size_t i = 0;
		const char* data = (const char *) buffer;

		CHECK_RETURN(len >= header->padding_length, false, "illegal len: %ld, padding_length: %d", len, header->padding_length);
		size_t realen = len - header->padding_length;

		// FASTCGI_PARAMS FORMAT: LENGTH_KEY|VALUE_LENGTH|KEY|VALUE
		// 	LENGTH:	1 or 4 bytes when LENGTH > 128 
		// 	Sample: \x0B\x02SERVER_PORT80\x0B\x0ESERVER_ADDR199.170.183.42
		//
		//Debug << "headers: ";
		while (i < realen) {
			u32 len_key = data[i++];
			CHECK_RETURN(i < realen, false, "readParams overflow, i: %ld, len: %ld", i, realen);
			if ((len_key & 0x80) != 0) {	// > 128
				u8 v1 = data[i++];
				u8 v2 = data[i++];
				u8 v3 = data[i++];
				len_key = ((len_key & 0x7f) << 24) + (v1 << 16) + (v2 << 8) + v3;
				CHECK_RETURN(i < realen, false, "readParams overflow, i: %ld, len: %ld", i, realen);
			}

			u32 len_value = data[i++];
			CHECK_RETURN(i < realen, false, "readParams overflow, i: %ld, len: %ld", i, realen);
			if ((len_value & 0x80) != 0) {	// > 128
				u8 v1 = data[i++];
				u8 v2 = data[i++];
				u8 v3 = data[i++];
				len_value = ((len_value & 0x7f) << 24) + (v1 << 16) + (v2 << 8) + v3;
				CHECK_RETURN(i <= realen, false, "readParams overflow, i: %ld, len: %ld", i, realen);
			}

			std::string key, value;
			key.assign(&data[i], len_key);
			i += len_key;
			value.assign(&data[i], len_value);
			i += len_value;
			CHECK_RETURN(i <= realen, false, "readParams overflow, i: %ld, len: %ld", i, realen);

			//Debug << "    Key: " << key << ", Value: " << value << ", length: " << value.length();

			request->addInputHeader(key.c_str(), value.c_str());
		}

		Debug << "readParams, i:" << i << ", len:" << len << ", realen:" << realen;
		return true;
	}
		
	// STDIN pass value of POST, just like: var1=1&var2=husosdgfsdfg&var3=%25E4%25BD%25A0%25E5
	bool CgiServerInternal::readStdin(CgiRequestInternal* request, fastcgi_header* header, const void* buffer, size_t len) {
		if (len == 0) {
			request->receiveStdinDone();
			return true;	// read stdin done, it's the last package with content_length == 0
		}

		assert(header->content_length() <= len);
		std::string data;
		data.assign((const char*)buffer, header->content_length());
		request->addInputHeader("QUERY_STRING", data.c_str());

		return true;
	}

	bool CgiServerInternal::readData(CgiRequestInternal* request, fastcgi_header* header, const void* buffer, size_t len) {
		if (len == 0) {
			return true;	// read stdin done, it's the last package with content_length == 0
		}
		return true;
	}


	CgiServerInternal::CgiServerInternal() {
		this->_socketServer = SocketServerCreator::create([this](const void* buffer, size_t len) -> int {
			return len < sizeof(fastcgi_header) ? 0 : (
				len >= ((fastcgi_header*)buffer)->package_length() ? ((fastcgi_header*)buffer)->package_length() : 0);
		});
	}

	CgiServer::~CgiServer() {}
	CgiServerInternal::~CgiServerInternal() {
		this->stop();
	}

	CgiServer* CgiServerCreator::create() {
		return new CgiServerInternal();
	}
}
