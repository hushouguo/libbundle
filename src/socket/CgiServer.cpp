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
			inline bool receiveParams() { return this->_receive_params; }
			void receiveParams(bool value) { this->_receive_params = value; }
			inline bool receiveStdin() { return this->_receive_stdin; }
			void receiveStdin(bool value) { this->_receive_stdin = value; }
			inline SOCKET fd() { return this->_socket; }
			void fd(SOCKET s) { this->_socket = s; }
			bool addHeader(const char* key, const char* value);
			bool addVariable(const char* key, const char* value);

		public:
			u32 id() override { return this->_id; }
			const char* header(const char* name) override {
				auto i = this->_headers.find(name);
				return i != this->_headers.end() ? i->second.c_str() : nullptr;
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
						
			void sendString(const char*, size_t) override;
			void sendString(const std::string&) override;
			void sendString(const std::ostringstream&) override;
			void sendBinary(const void*, size_t) override;
			void done() override;

		private:
			u32 _id = 0;
			SocketServer* _socketServer = nullptr;
			SOCKET _socket = -1;
			bool _keepalive = false;
			bool _receive_params = false, _receive_stdin = false;
			FASTCGI_ROLE _role = FASTCGI_RESPONDER;
			std::unordered_map<std::string, std::string> _headers;
			std::unordered_map<std::string, std::string> _variables;
	};
			
	CgiRequestInternal::CgiRequestInternal(u32 id, SocketServer* socketServer) {
		this->_id = id;
		this->_socketServer = socketServer;
	}

	CgiRequest::~CgiRequest() {}
	CgiRequestInternal::~CgiRequestInternal() {
		this->_socketServer->close(this->_socket);
	}

	bool CgiRequestInternal::addHeader(const char* key, const char* value) {
		return this->_headers.insert(std::make_pair(key, value)).second;
	}

	bool CgiRequestInternal::addVariable(const char* key, const char* value) {
		return this->_variables.insert(std::make_pair(key, value)).second;
	}

	void CgiRequestInternal::sendString(const char* data, size_t len) {
		if (len > 0) {
			fastcgi_header header;
			header.version = FASTCGI_VERSION;
			header.type = FASTCGI_STDOUT;
			header.requestid(this->_id);	// requestid_high && requestid_low
			header.content_length(len);		// content_length_high && content_length_low && padding_length
			header.reserved = 0;
			this->_socketServer->sendMessage(this->_socket, &header, sizeof(fastcgi_header));
			this->_socketServer->sendMessage(this->_socket, data, len);
			if (header.padding_length > 0) {
				u8 padding[header.padding_length];
				this->_socketServer->sendMessage(this->_socket, padding, sizeof(padding));
			}
		}

		if (true) {
			fastcgi_header header;
			header.version = FASTCGI_VERSION;
			header.type = FASTCGI_STDOUT;
			header.requestid(this->_id);	// requestid_high && requestid_low
			header.content_length(0);		// end of stdout
			header.reserved = 0;
			this->_socketServer->sendMessage(this->_socket, &header, sizeof(fastcgi_header));
		}
	}

	void CgiRequestInternal::sendString(const std::string& s) {
		this->sendString(s.data(), s.length());
	}

	void CgiRequestInternal::sendString(const std::ostringstream& o) {
		const std::string& s = o.str();
		this->sendString(s);
	}

	void CgiRequestInternal::sendBinary(const void*, size_t) {
	}

	void CgiRequestInternal::done() {
		fastcgi_header header;
		header.version = FASTCGI_VERSION;
		header.type = FASTCGI_END_REQUEST;
		header.requestid(this->_id);			// requestid_high && requestid_low
		header.content_length(sizeof(fastcgi_end_request));	// content_length_high && content_length_low && padding_length
		header.reserved = 0;
		this->_socketServer->sendMessage(this->_socket, &header, sizeof(fastcgi_header));

		fastcgi_end_request body;
		body.protocolStatus = FASTCGI_REQUEST_COMPLETE;
		this->_socketServer->sendMessage(this->_socket, &body, sizeof(fastcgi_end_request));
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
			bool beginRequest(CgiRequestInternal*, const void* buffer, size_t len);
			bool readParams(CgiRequestInternal*, const void* buffer, size_t len);
			bool readStdin(CgiRequestInternal*, const void* buffer, size_t len);
			bool readData(CgiRequestInternal*, const void* buffer, size_t len);
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
				if (i.second->receiveParams() && i.second->receiveStdin()) {
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

			this->parsePackage(s, GET_MESSAGE_BY_PAYLOAD(msg), GET_MESSAGE_PAYLOAD_LENGTH(msg));
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
		buffer = (u8*) buffer + sizeof(fastcgi_header);
		len -= sizeof(fastcgi_header);
		Debug.cout("fastcgi version: %d, type: %d, requestid: %d, content_length: %d, padding_length: %d",
				header->version, header->type, header->requestid(), header->content_length(), header->padding_length);

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
				rc = this->beginRequest(request, buffer, len); break;
			case FASTCGI_PARAMS: 
				rc = this->readParams(request, buffer, len); break;
			case FASTCGI_STDIN: 
				rc = this->readStdin(request, buffer, len); break;
			case FASTCGI_DATA: 
				rc = this->readData(request, buffer, len); break;
			default: 
				Alarm << "Unhandle FASTCGI header: " << (u32) header->type; break;
		}

		if (!rc) {
			this->releaseRequest(request);
		}
		return rc;
	}

	bool CgiServerInternal::beginRequest(CgiRequestInternal* request, const void* buffer, size_t len) {
		assert(len >= sizeof(fastcgi_begin_request));
		fastcgi_begin_request* msg = (fastcgi_begin_request *) buffer;
		Debug << "role: " << msg->role() << ", flags: " << msg->flags;
		request->role((FASTCGI_ROLE) msg->role());
		request->keepalive(msg->flags & FASTCGI_KEEP_ALIVE);
		CHECK_RETURN(request->role() == FASTCGI_RESPONDER, false, "Only support response role: %d", request->role());
		return true;
	}

	// For GET command, exist key is QUERY_STRING, value is like: id=1&name=hushouguo
	bool CgiServerInternal::readParams(CgiRequestInternal* request, const void* buffer, size_t len) {
		if (len == 0) {
			request->receiveParams(true);
			return true;	// read params done, it's the last package with content_length == 0
		}
		
		size_t i = 0;
		const char* data = (const char *) buffer;

		// FASTCGI_PARAMS FORMAT: LENGTH_KEY|VALUE_LENGTH|KEY|VALUE
		// 	LENGTH:	1 or 4 bytes when LENGTH > 128 
		// 	Sample: \x0B\x02SERVER_PORT80\x0B\x0ESERVER_ADDR199.170.183.42
		//
		while (i < len) {
			u32 len_key = data[i++];
			CHECK_RETURN(i < len, false, "readParams overflow, i: %ld, len: %ld", i, len);
			if ((len_key & 0x80) != 0) {	// > 128
				u8 v1 = data[i++];
				u8 v2 = data[i++];
				u8 v3 = data[i++];
				len_key = ((len_key & 0x7f) << 24) + (v1 << 16) + (v2 << 8) + v3;
				CHECK_RETURN(i < len, false, "readParams overflow, i: %ld, len: %ld", i, len);
			}

			u32 len_value = data[i++];
			CHECK_RETURN(i < len, false, "readParams overflow, i: %ld, len: %ld", i, len);
			if ((len_value & 0x80) != 0) {	// > 128
				u8 v1 = data[i++];
				u8 v2 = data[i++];
				u8 v3 = data[i++];
				len_value = ((len_value & 0x7f) << 24) + (v1 << 16) + (v2 << 8) + v3;
				CHECK_RETURN(i < len, false, "readParams overflow, i: %ld, len: %ld", i, len);
			}

			std::string key, value;
			key.assign(&data[i], len_key);
			i += len_key;
			value.assign(&data[i], len_value);
			i += len_value;
			CHECK_RETURN(i < len, false, "readParams overflow, i: %ld, len: %ld", i, len);

			Debug << "Key: " << key << ", Value: " << value;

			request->addHeader(key.c_str(), value.c_str());
		}

		Debug << "readParams, i: " << i << ", len: " << len;
		return true;
	}
		
	// STDIN pass value of POST, just like: var1=1&var2=husosdgfsdfg&var3=%25E4%25BD%25A0%25E5
	bool CgiServerInternal::readStdin(CgiRequestInternal* request, const void* buffer, size_t len) {
		if (len == 0) {
			request->receiveStdin(true);
			return true;	// read stdin done, it's the last package with content_length == 0
		}
		//TODO: parse
		return true;
	}

	bool CgiServerInternal::readData(CgiRequestInternal* request, const void* buffer, size_t len) {
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
