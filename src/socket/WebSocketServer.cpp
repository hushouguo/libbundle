/*
 * \file: WebSocket.cpp
 * \brief: Created by hushouguo at 07:46:24 Aug 10 2018
 */

#include "bundle.h"
#include "Helper.h"
#include "Socket.h"

#define MAGIC_KEY 	"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

BEGIN_NAMESPACE_BUNDLE {

#pragma pack(push, 1)
		struct WS_HEADER {
			u8	opcode:4;
			u8	RSV3:1;
			u8	RSV2:1;
			u8	RSV1:1;
			u8	FIN:1;
			u8	payload_len:7;
			u8	MASK:1;
	
			void dump() {
				Trace.cout("FIN:%d, RSV1:%d, RSV2:%d, RSV:%d, opcode:%d, MASK:%d, payload_len:%d", this->FIN, this->RSV1, this->RSV2, this->RSV3, this->opcode, this->MASK, this->payload_len);
			}
		};
#pragma pack(pop)
	
	enum {
		WS_OPCODE_CONTINUE	=	0x0,
		WS_OPCODE_TEXT		=	0x1,
		WS_OPCODE_BINARY	=	0x2,
		WS_OPCODE_CLOSE 	=	0x8,
		WS_OPCODE_PING		=	0X9,
		WS_OPCODE_PONG		=	0xa
	};

	class WebSocketServerInternal : public WebSocketServer {
		public:
			WebSocketServerInternal();
			~WebSocketServerInternal();
			
		public:
			SOCKET fd() override {	return this->_socketServer->fd(); }
			bool start(const char* address, int port) override { return this->_socketServer->start(address, port); }
			void stop() override { this->_socketServer->stop(); }
			const Socketmessage* receiveMessage(SOCKET& s, bool& establish, bool& close) override;
			void sendMessage(SOCKET s, const void*, size_t) override;
			//void sendMessage(SOCKET s, const Socketmessage*) override;
			void close(SOCKET s) override { this->_socketServer->close(s); }
			size_t size() override { return this->_socketServer->size(); }
			bool setsockopt(int opt, const void* optval, size_t optlen) override { return this->_socketServer->setsockopt(opt, optval, optlen); }
			bool getsockopt(int opt, void* optval, size_t optlen) override { return this->_socketServer->getsockopt(opt, optval, optlen); }
		
		private:
			SocketServer* _socketServer = nullptr;
			int parse_handshake_request(const Byte* buffer, size_t len, std::string& Sec_WebSocket_Key);
			void send_handshake_response(SOCKET s, const std::string& Sec_WebSocket_Key);
			int parsePackage(const Byte* buffer, size_t len);
			bool parsePackage(SOCKET s, Socketmessage*& msg, bool& forward);
			void sendPing(SOCKET s);
			void sendPong(SOCKET s);
	};

	void WebSocketServerInternal::sendPing(SOCKET s) {
		WS_HEADER header;
		header.FIN = 1;
		header.RSV1 = header.RSV2 = header.RSV3 = 0;
		header.opcode = WS_OPCODE_PING;
		header.MASK = 0;
		header.payload_len = 0;
		this->_socketServer->sendMessage(s, &header, sizeof(header));
	}
	
	void WebSocketServerInternal::sendPong(SOCKET s) {
		WS_HEADER header;
		header.FIN = 1;
		header.RSV1 = header.RSV2 = header.RSV3 = 0;
		header.opcode = WS_OPCODE_PONG;
		header.MASK = 0;
		header.payload_len = 0;
		this->_socketServer->sendMessage(s, &header, sizeof(header));
	}

	void WebSocketServerInternal::sendMessage(SOCKET s, const void* data, size_t datasize) {
		u64 len = sizeof(WS_HEADER) + datasize;
		u64 payload_len = 0;
		if (datasize < 126) {
			payload_len = datasize;
		}
		else if (datasize < 65025) {
			len += 2;
			payload_len = 126;
		}
		else {
			len += 8;
			payload_len = 127;
		}
				
		char buffer[len];
		u64 offset = sizeof(WS_HEADER);
		
		WS_HEADER* header = (WS_HEADER*) buffer;
		header->FIN = 1;
		header->RSV1 = header->RSV2 = header->RSV3 = 0;
		header->opcode = WS_OPCODE_TEXT;
		header->MASK = 0;
		header->payload_len = payload_len;
		if (payload_len == 126) {
			* (u16 *) (&buffer[offset]) = datasize;
			offset += 2;
		}
		else if (payload_len == 127) {
			* (u64 *) (&buffer[offset]) = datasize;
			offset += 8;
		}
		
		memcpy(&buffer[offset], data, datasize);
		offset += datasize;
		assert(offset == len);
		
		//header->dump();
		//Trace.cout("len:%ld, payload_len:%ld", len, payload_len);

		this->_socketServer->sendMessage(s, buffer, len);
	}

//	void WebSocketServerInternal::sendMessage(SOCKET s, const Socketmessage*) {
//	}

	/////////////////////////////////////////////////////////////////
	
	const Socketmessage* WebSocketServerInternal::receiveMessage(SOCKET& s, bool& establish, bool& close) {
        Socketmessage* msg = (Socketmessage *) this->_socketServer->receiveMessage(s, establish, close);
        if (!msg) {
        	return nullptr;
        }

        if (establish) {
        	bundle::releaseMessage(msg);
        	return nullptr; // Websocket needs to wait for the handshake to establish.
        }

        if (close) {
        	return msg;	// disconnect
        }

		if (msg->payload_len >= 3 && msg->payload[0] == 'G' && msg->payload[1] == 'E' && msg->payload[2] == 'T') {	// GET
			std::string Sec_WebSocket_Key;
			int len = this->parse_handshake_request(msg->payload, msg->payload_len, Sec_WebSocket_Key);
			assert(len > 0 && size_t(len) == msg->payload_len);
			this->send_handshake_response(s, Sec_WebSocket_Key);
			establish = true, close = false;
			return msg;
		}

		bool forward = false;
		if (!this->parsePackage(s, msg, forward)) {
			this->_socketServer->close(s);
			bundle::releaseMessage(msg);
			return nullptr;
		}

		if (!forward) {
			bundle::releaseMessage(msg);
			return nullptr;
		}

		return msg;
	}

	bool WebSocketServerInternal::parsePackage(SOCKET s, Socketmessage*& msg, bool& forward) {
		size_t offset = sizeof(WS_HEADER);
		CHECK_RETURN(msg->payload_len >= offset, false, "illegal WS_PACKAGE size");

		WS_HEADER* header = (WS_HEADER *) msg->payload;
		CHECK_RETURN(header->RSV1 == 0, false, "illegal WS_PACKAGE RSV1");
		CHECK_RETURN(header->RSV2 == 0, false, "illegal WS_PACKAGE RSV2");
		CHECK_RETURN(header->RSV3 == 0, false, "illegal WS_PACKAGE RSV3");
		CHECK_RETURN(header->FIN == 1, false, "NOT SUPPORT MULTI-FRAME PROTOCOL");

		u64 payload_len = header->payload_len;
		if (header->payload_len == 126) {
			CHECK_RETURN(msg->payload_len >= (offset + 2), false, "126, not enough data");
			payload_len = ntohs(*(u16 *)(msg->payload + offset));
			offset += 2;
		}
		else if (header->payload_len == 127) {
			CHECK_RETURN(msg->payload_len >= (offset + 8), false, "127, not enough data");
			payload_len = ntohl(*(u64 *)(msg->payload + offset));
			offset += 8;
		}

		Byte* mask = nullptr;
		if (header->MASK == 1) {
			CHECK_RETURN(msg->payload_len >= (offset + 4), false, "mask, not enough data");
			mask = msg->payload + offset;
			offset += 4;
		}

		CHECK_RETURN(msg->payload_len >= (offset + payload_len), false, "payload_len not enough");

		Byte* data = msg->payload + offset;
		if (header->MASK == 1) {
			for (size_t i = 0; i < payload_len; ++i) {
				data[i] ^= mask[i % 4];
			}
		}

		switch (header->opcode) {
			case WS_OPCODE_BINARY: 
			case WS_OPCODE_TEXT:
			case WS_OPCODE_CONTINUE:
				if (true) {
					Socketmessage* newmsg = allocateMessage(s, SM_OPCODE_MESSAGE, msg->payload + offset, payload_len);
					bundle::releaseMessage(msg);
					//memmove(msg->payload, msg->payload + offset, payload_len);
					//msg->payload_len = payload_len;
					msg = newmsg;
					forward = true;
				}
				return true;
			
			case WS_OPCODE_CLOSE: 
				if (payload_len > 0) { 
					Alarm.cout("receive OPCODE_CLOSE: %s", (const char*) data); 
				}
				return false;
				
			case WS_OPCODE_PING: 
				this->sendPong(s); 
				forward = false;
				return true;
				
			case WS_OPCODE_PONG:
				//NOTE: count
				forward = false;
				return true;
				
			default: CHECK_RETURN(false, false, "illegal OPCODE: %d", header->opcode);
		}

		return true;
	}
		
	/* handshake request format: 
	 * GET /chat HTTP/1.1
	 * Upgrade: websocket
	 * Connection: Upgrade
	 * Host: www.zendstudio.net:9108
	 * Origin: http://www.zendstudio.net
	 * Sec-WebSocket-Key: U00QUfV1CRfIIU0NkcUCnA==
	 * Sec-WebSocket-Version: 13
	 * Sec-WebSocket-Extensions: x-webkit-deflate-frame
	 */
	int WebSocketServerInternal::parse_handshake_request(const Byte* buffer, size_t len, std::string& Sec_WebSocket_Key) {
		std::unordered_map<std::string, std::string> headers;
		
		std::istringstream is((const char*) buffer);
		std::string request;
		
		try {
			std::getline(is, request);// GET / HTTP/1.1
		} catch(std::exception& e) {
			CHECK_RETURN(false, -1, "parse request exception:%s", e.what());
		}
		
		std::string header;
		while (std::getline(is, header)) {
			if (header == "\r") {
				break;// endofistream
			}
		
			if (header[header.size()-1] != '\r') {
				continue; //end
			} 
			else {
				header.erase(header.end()-1);	//remove last char
			}
		
			std::string::size_type end = header.find(": ", 0);
			if (end != std::string::npos) {
				std::string key = header.substr(0, end);
				std::string value = header.substr(end + 2);
				headers[key] = value;
			}
		}
		
		std::unordered_map<std::string, bool> keys = {
			{ "Host", true },
			{ "Upgrade", true },
			{ "Connection", true }, 
			{ "Sec-WebSocket-Key", true },
			{ "Sec-WebSocket-Version", true },
			
			{ "Origin", false },					// optional
			{ "Sec-WebSocket-Protocol", false },	// optional
			{ "Sec-WebSocket-Extensions", false },	// optional 			
			{ "Pragma", false },					// optional
			{ "Cache-Control", false }, 			// optional
			{ "User-Agent", false },				// optional
			{ "Accept-Language", false },			// optional
			{ "Accept-Encoding", false }			// optional
		};
		
		for (auto& i : keys) {
			CHECK_RETURN(i.second == false || headers.find(i.first) != headers.end(), 0, "lack header: %s", i.first.c_str());
		}
	
		Sec_WebSocket_Key = headers["Sec-WebSocket-Key"];		
		return len; 
	}
	
	/* handshake response format:
	 * HTTP/1.1 101 Switching Protocols
	 * Upgrade: websocket
	 * Connection: Upgrade
	 * Sec-WebSocket-Accept: 7GGzyIJjf9bX7pej+3tc5Vv87S0=
	 * WebSocket-Origin: http://www.zendstudio.net
	 * WebSocket-Location: ws://www.zendstudio.net:9108/chat	 
	 */
	void WebSocketServerInternal::send_handshake_response(SOCKET s, const std::string& Sec_WebSocket_Key) {
		std::string response;
		response  = "HTTP/1.1 101 Switching Protocols\r\n";
		response += "Connection: upgrade\r\n";
		response += "Sec-WebSocket-Accept: ";
		
		std::string server_key = Sec_WebSocket_Key;
		server_key += MAGIC_KEY;
		
		SHA1 sha;
		unsigned int message_digest[5];
		sha.Reset();
		sha << server_key.c_str();
		
		sha.Result(message_digest);
		for (int i = 0; i < 5; i++) {
			message_digest[i] = htonl(message_digest[i]);
		}
		
		std::string ret_string;
		base64_encode(reinterpret_cast<const unsigned char*>(message_digest), 20, ret_string);
		ret_string += "\r\n";
		
		response += ret_string;
		response += "Upgrade: websocket\r\n\r\n";
		
		//Trace.cout("response: %s", response.c_str());
	
		this->_socketServer->sendMessage(s, response.data(), response.length());
	}

	int WebSocketServerInternal::parsePackage(const Byte* buffer, size_t len) {
		size_t offset = sizeof(WS_HEADER);
		if (len < offset) {
			return 0;
		}

		WS_HEADER* header = (WS_HEADER *) buffer;
		CHECK_RETURN(header->RSV1 == 0, -1, "illegal WS_PACKAGE RSV1");
		CHECK_RETURN(header->RSV2 == 0, -1, "illegal WS_PACKAGE RSV2");
		CHECK_RETURN(header->RSV3 == 0, -1, "illegal WS_PACKAGE RSV3");
		CHECK_RETURN(header->FIN == 1, -1, "NOT SUPPORT MULTI-FRAME PROTOCOL");

		u64 payload_len = header->payload_len;
		if (header->payload_len == 126) {
			if (len < (offset + 2)) {
				return 0;
			}
			payload_len = ntohs(*(u16 *)(buffer + offset));
			offset += 2;
		}
		else if (header->payload_len == 127) {
			if (len < (offset + 8)) {
				return 0;
			}
			payload_len = ntohl(*(u64 *)(buffer + offset));
			offset += 8;
		}

		if (header->MASK == 1) {
			if (len < (offset + 4)) {
				return 0;
			}
			offset += 4;
		}

		if (len < (offset + payload_len)) {
			return 0;
		}

		return offset + payload_len;
	}

	WebSocketServerInternal::WebSocketServerInternal() {
		this->_socketServer = SocketServerCreator::create([this](const void* data, size_t len) -> int {
			const Byte* buffer = (const Byte*) data;
			if (len >= 3 && buffer[0] == 'G' && buffer[1] == 'E' && buffer[2] == 'T') {	// GET
				std::string Sec_WebSocket_Key;
				return this->parse_handshake_request(buffer, len, Sec_WebSocket_Key);
			}
			else {
				return this->parsePackage(buffer, len);
			}
		});
	}

	WebSocketServer::~WebSocketServer() {}
	WebSocketServerInternal::~WebSocketServerInternal() {
		this->stop();
	}

	WebSocketServer* WebSocketServerCreator::create() {
		return new WebSocketServerInternal();
	}
}

