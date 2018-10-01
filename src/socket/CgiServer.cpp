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
		FASTCGI_ABORT_REQUEST      	= 2,
		FASTCGI_END_REQUEST        	= 3,
		FASTCGI_PARAMS             	= 4,
		FASTCGI_STDIN              	= 5,
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
		inline u32 content_length() { return (this->content_length_high << 8) + this->content_length_low; }
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
	
	class CgiServerInternal : public CgiServer {
		public:
			CgiServerInternal();
			~CgiServerInternal();

		public:
			SOCKET fd() override { return this->_socketServer->fd(); }
			bool start(const char* address, int port) override { return this->_socketServer->start(address, port); }
			void stop() override { this->_socketServer->stop(); }
			const Socketmessage* receiveMessage() override;
			void sendMessage(SOCKET s, const void*, size_t) override;
			void close(SOCKET s) override { this->_socketServer->close(s); }
			size_t size() override { this->_socketServer->size(); }
			bool setsockopt(int opt, const void* optval, size_t optlen) override { 
				return this->_socketServer->setsockopt(opt, optval, optlen); 
			}
			bool getsockopt(int opt, void* optval, size_t optlen) override {
				return this->_socketServer->getsockopt(opt, optval, optlen);
			}

		public:
			void sendString(SOCKET, const char*, size_t) override;
			void sendString(SOCKET, const std::string&) override;
			void sendString(SOCKET, const std::ostringstream&) override;
			void sendBinary(SOCKET, const void*, size_t) override;

		private:
			SocketServer* _socketServer = nullptr;
			bool parsePackage(SOCKET s, const void* buffer, size_t len);
			bool beginRequest(SOCKET s, const void* buffer, size_t len);
			bool readParams(SOCKET s, const void* buffer, size_t len);
			bool readStdin(SOCKET s, const void* buffer, size_t len);
			bool readData(SOCKET s, const void* buffer, size_t len);
	};

	bool CgiServerInternal::beginRequest(SOCKET s, const void* buffer, size_t len) {
		assert(len >= sizeof(fastcgi_begin_request));
		fastcgi_begin_request* request = (fastcgi_begin_request *) buffer;
		Debug << "role: " << request->role() << ", flags: " << request->flags;
		CHECK_REUTNR(request->role() == FASTCGI_RESPONDER, false, "Only support response role: %d", request->role());
		return true;
	}

	// For GET command, exist key is QUERY_STRING, value is like: id=1&name=hushouguo
	bool CgiServerInternal::readParams(SOCKET s, const void* buffer, size_t len) {
		if (len == 0) {
			return true;	// read params done, it's the last package with content_length == 0
		}
		
		size_t i = 0;
		const u8 * data = (const u8 *) buffer;

		// FASTCGI_PARAMS FORMAT: LENGTH_KEY|VALUE_LENGTH|KEY|VALUE
		// 	LENGTH:	1 or 4 bytes when LENGTH > 128 
		// 	Sample: \x0B\x02SERVER_PORT80\x0B\x0ESERVER_ADDR199.170.183.42
		//
		while (i < len) {
			u32 len_key = data[i++];
			CHECK_RETURN(i < len, false, "readParams overflow, i: %ld, len: %ld", i, len);
			if ((len_key & 0x80) != 0) {	// > 128
				len_key = ((len_key & 0x7f) << 24) + (data[i++] << 16) + (data[i++] << 8) + data[i++];
				CHECK_RETURN(i < len, false, "readParams overflow, i: %ld, len: %ld", i, len);
			}

			u32 len_value = data[i++];
			CHECK_RETURN(i < len, false, "readParams overflow, i: %ld, len: %ld", i, len);
			if ((len_value & 0x80) != 0) {	// > 128
				len_value = ((len_value & 0x7f) << 24) + (data[i++] << 16) + (data[i++] << 8) + data[i++];
				CHECK_RETURN(i < len, false, "readParams overflow, i: %ld, len: %ld", i, len);
			}

			std::string key, value;
			key.assign(&data[i], len_key);
			i += len_key;
			value.assign(&data[i], len_value);
			i += len_value;
			CHECK_RETURN(i < len, false, "readParams overflow, i: %ld, len: %ld", i, len);

			Debug << "Key: " << key << ", Value: " << value;
		}

		Debug << "readParams, i: " << i << ", len: " << len;
		return true;
	}
		
	// STDIN pass value of POST, just like: var1=1&var2=husosdgfsdfg&var3=%25E4%25BD%25A0%25E5
	bool CgiServerInternal::readStdin(SOCKET s, const void* buffer, size_t len) {
		if (len == 0) {
			return true;	// read stdin done, it's the last package with content_length == 0
		}
		//TODO: parse
		return true;
	}

	bool CgiServerInternal::readData(SOCKET s, const void* buffer, size_t len) {
		if (len == 0) {
			return true;	// read stdin done, it's the last package with content_length == 0
		}
		return true;
	}

	bool CgiServerInternal::parsePackage(SOCKET s, const void* buffer, size_t len) {
		// fastcgi_header
		assert(len >= sizeof(fastcgi_header));
		fastcgi_header* header = (fastcgi_header *) buffer;
		buffer = (u8*) buffer + sizeof(fastcgi_header);
		len -= sizeof(fastcgi_header);
		Debug.cout("fastcgi version: %d, type: %d, requestid: %d, content_length: %d, padding_length: %d",
				header->version, header->type, header->requestid(), header->content_length(), header->padding_length);

		switch (header->type) {
			// BeginRequest
			case FASTCGI_BEGIN_REQUEST: return this->beginRequest(s, buffer, len); break;
			// PARAMS: HEADERS
			case FASTCGI_PARAMS: return this->readParams(s, buffer, len); break;
			// STDIN: POST
			case FASTCGI_STDIN: return this->readStdin(s, buffer, len); break;
			// DATA
			case FASTCGI_DATA: return this->readData(s, buffer, len); break;
		}
	}

	const Socketmessage* CgiServerInternal::receiveMessage() {
        Socketmessage* msg = (Socketmessage *) this->_socketServer->receiveMessage();
        if (!msg) {
        	return nullptr;
        }

		SOCKET s = GET_MESSAGE_SOCKET(msg);

        if (IS_ESTABLISH_MESSAGE(msg)) {
        	bundle::releaseMessage(msg);
        	return nullptr;
        }

        if (IS_CLOSE_MESSAGE(msg)) {
        	return msg;	// connection close
        }
		
			
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
