/*
 * \file: test_net.cpp
 * \brief: Created by hushouguo at 16:31:49 Sep 06 2018
 */

#include "bundle.h"

using namespace bundle;

//Time t1, t2;
struct timeval t1, t2;
u32 n = 0, N = 3;

SocketServer* ss = nullptr;
SocketClient* cs = nullptr;
std::thread* loop = nullptr;

void createServer(u32 msgsize) {
	ss = SocketServerCreator::create([=](const void* buffer, size_t len) -> int{
			if (len >= msgsize) {
#if 0			
				gettimeofday(&t2, nullptr);
				u64 t1_us = t1.tv_sec * 1000 * 1000 + t1.tv_usec;
				u64 t2_us = t2.tv_sec * 1000 * 1000 + t2.tv_usec;
				Trace << "receive cost us: " << t2_us - t1_us;
#endif				
			}
			return len >= msgsize ? msgsize : 0;
			});
	assert(ss);
	bool rc = ss->start("0.0.0.0", 12306);
	assert(rc);

	auto runnable = []() {
		u32 n = 0;
		while (n < N) {
			const Socketmessage* msg = ss->receiveMessage();
			if (msg) {
				if (IS_ESTABLISH_MESSAGE(msg)) {
					//fprintf(stderr, "SocketServer: establish: %d\n", s);
				}
				else if (IS_CLOSE_MESSAGE(msg)) {
					//fprintf(stderr, "SocketServer: lostConnection: %d\n", s);
				}
				else {
					//fprintf(stderr, "receive Socketmessage: %d\n", n);
					++n;
				}
				bundle::releaseMessage(msg);
			}
			else {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));	
			}
		}

		//t2.now();
		gettimeofday(&t2, nullptr);
	};
	
	SafeDelete(loop);
	loop = new std::thread(runnable);
}

void createClient(u32 msgsize) {
	cs = SocketClientCreator::create([=](const void* buffer, size_t len) -> int{
			return len >= msgsize ? msgsize : 0;
			});
	assert(cs);
	bool rc = cs->connect("127.0.0.1", 12306, true);
	assert(rc);

	//t1.now();
	gettimeofday(&t1, nullptr);

	sleep(1);

	struct timeval ta, tb, tc;
	for (u32 i = 0; i < N; ++i) {
		gettimeofday(&ta, nullptr);
		Socketmessage* msg = allocateMessage(msgsize);
		gettimeofday(&tb, nullptr);
		cs->sendMessage(msg);
		gettimeofday(&tc, nullptr);

#if 0
		u64 ta_us = ta.tv_sec * 1000 * 1000 + ta.tv_usec;
		u64 tb_us = tb.tv_sec * 1000 * 1000 + tb.tv_usec;
		u64 tc_us = tc.tv_sec * 1000 * 1000 + tc.tv_usec;
		Trace << "initMessage cost us: " << tb_us - ta_us << ", sendMessage: " << tc_us - tb_us;
#endif		
	}
}

void test_net2() {
//	Easylog::syslog()->set_level(LEVEL_DEBUG);
	Easylog::syslog()->set_level(LEVEL_TRACE);

	u32 sizes[] = {
		4*KB, 8*KB, 16*KB, 32*KB, 64*KB, 128*KB, 256*KB, 512*KB, 
//		1*MB, 2*MB, 4*MB, 8*MB, 16*MB, 32*MB, 64*MB, 128*MB, 256*MB
//		256*MB
	};

	for (auto size : sizes) {
		createServer(size);
		createClient(size);
		loop->join();
		u64 t1_us = t1.tv_sec * 1000 * 1000 + t1.tv_usec;
		u64 t2_us = t2.tv_sec * 1000 * 1000 + t2.tv_usec;
		Trace << "benchmark: net i/o count: " << N << " messages, message size: " << size << ", cost us: " << t2_us - t1_us;
		SafeDelete(cs);
		SafeDelete(ss);
	}

	System << "test net OK";
}

void test_net3() {
	ss = SocketServerCreator::create([=](const void* buffer, size_t len) -> int{
			return len;
			});
	assert(ss);
	ss->setWorkerNumber(4);
	bool rc = ss->start("0.0.0.0", 12306);
	assert(rc);

	auto runnable = []() {
		u32 n = 0;
		while (!sConfig.halt) {
			const Socketmessage* msg = ss->receiveMessage();
			if (msg) {
				if (IS_ESTABLISH_MESSAGE(msg)) {
					//fprintf(stderr, "SocketServer: establish: %d\n", s);
				}
				else if (IS_CLOSE_MESSAGE(msg)) {
					//fprintf(stderr, "SocketServer: lostConnection: %d\n", s);
				}
				else {
					//fprintf(stderr, "receive Socketmessage: %d\n", n);
					++n;
				}
				bundle::releaseMessage(msg);
			}
			else {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));	
			}
		}
	};

	runnable();

	SafeDelete(ss);
}

void test_net4() {
	ss = SocketServerCreator::create([=](const void* buffer, size_t len) -> int{
			return len;
			});
	assert(ss);
	ss->setWorkerNumber(4);
	bool rc = ss->start("0.0.0.0", 12306);
	assert(rc);
	
	cs = SocketClientCreator::create([=](const void* buffer, size_t len) -> int{
			return len;
			});
	assert(cs);
	rc = cs->connect("127.0.0.1", 12306, 10);
	assert(rc);

	auto doServerMessage = []() {
		const Socketmessage* msg = ss->receiveMessage();
		if (msg) {
			if (IS_ESTABLISH_MESSAGE(msg)) {
				//fprintf(stderr, "SocketServer: establish: %d\n", s);
			}
			else if (IS_CLOSE_MESSAGE(msg)) {
				//fprintf(stderr, "SocketServer: lostConnection: %d\n", s);
			}
			else {
				fprintf(stderr, "receive Socketmessage: %d, payload:%s, payload_len:%ld\n", 
						n, (const char*) GET_MESSAGE_PAYLOAD(msg), GET_MESSAGE_PAYLOAD_LENGTH(msg));
				++n;
			}
			bundle::releaseMessage(msg);
		}
	};

	auto doClientMessage = []() {
		const Socketmessage* msg = cs->receiveMessage();
		if (msg) {
			if (IS_ESTABLISH_MESSAGE(msg)) {
				fprintf(stderr, "SocketClient: establish\n");
			}
			else if (IS_CLOSE_MESSAGE(msg)) {
				fprintf(stderr, "SocketClient: lostConnection\n");
			}
			else {
			}
			bundle::releaseMessage(msg);
		}
	};

	while (!sConfig.halt) {
		if (ss) {
			doServerMessage();
			if (n >= N) {
				SafeDelete(ss);
			}
		}
		if (cs) {
			doClientMessage();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));	
	}

	SafeDelete(cs);
	SafeDelete(ss);
}

void test_net5() {
	HttpParser parser;
	ss = SocketServerCreator::create([&](const void* buffer, size_t len) -> int{
			return parser.parse((const char*) buffer, len);
			});
	assert(ss);
	ss->setWorkerNumber(4);
	bool rc = ss->start("0.0.0.0", 12306);
	assert(rc);
	
	auto doServerMessage = [&]() {
		const Socketmessage* msg = ss->receiveMessage();
		if (msg) {
			if (IS_ESTABLISH_MESSAGE(msg)) {
				//fprintf(stderr, "SocketServer: establish: %d\n", s);
			}
			else if (IS_CLOSE_MESSAGE(msg)) {
				//fprintf(stderr, "SocketServer: lostConnection: %d\n", s);
			}
			else {
				SOCKET s = GET_MESSAGE_SOCKET(msg);
				std::string req = (const char*) GET_MESSAGE_PAYLOAD(msg);
				size_t len = GET_MESSAGE_PAYLOAD_LENGTH(msg);
				fprintf(stderr, "req.length: %ld, len: %ld\n", req.length(), len);

				//fprintf(stderr, "receive Socketmessage: %d, payload:%s, payload_len:%ld\n", 
				//		n, (const char*) GET_MESSAGE_PAYLOAD(msg), GET_MESSAGE_PAYLOAD_LENGTH(msg));

				Trace << "request: " << req;

				parser.clear();
				size_t nread = parser.parse(req.c_str(), len);
				if (nread > 0) {
					parser.dump();

					//
					// send response
					const char* response = 
						"HTTP/1.1 200 OK\r\n"
						"Content-Type: text/html; charset=UTF-8\r\n"
						"Content-Length: 11\r\n"
						"Connection: close\r\n"
						"Date: Thu, 31 Dec 2009 20:55:48 +0000\r\n"
						"\r\n"
						"hello world";
						
					ss->sendMessage(s, response, strlen(response));
				}

				++n;
			}
			bundle::releaseMessage(msg);
		}
	};
	
	while (!sConfig.halt) {
		if (ss) {
			doServerMessage();
			//if (n >= N) {
			//	SafeDelete(ss);
			//}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));	
	}

	SafeDelete(ss);
}

void test_net6() {
	WebServer* ws = WebServerCreator::create();
	assert(ws);
	bool rc = ws->start("0.0.0.0", 12306);
	assert(rc);
	while (!sConfig.halt) {
		ws->run();
		WebRequest* request = ws->getRequest();
		if (request) {
			auto& headers = request->headers();
			Debug << "headers:";
			for (auto& i : headers) {
				Debug << "    Key:" << i.first << "," << i.second;
			}
			auto& variables = request->variables();
			Debug << "variables:";
			for (auto& i : variables) {
				Debug << "    Key:" << i.first << "," << i.second;
			}

			char time_buffer[64];
			timestamp(time_buffer, sizeof(time_buffer));
			request->pushString(time_buffer);
			request->send();
			ws->releaseRequest(request);
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));	
		}
	}
	SafeDelete(ws);
}

#pragma pack(push, 1)
struct fcgi_header {
    unsigned char version;
    unsigned char type;
    unsigned char requestidB1;
    unsigned char requestidB0;
    unsigned char contentlengthB1;
    unsigned char contentlengthB0;
    unsigned char paddinglength;
    unsigned char reserved;
	u32 contentlength() {
		return (this->contentlengthB1 << 8) + this->contentlengthB0;
	}
};

enum fcgi_request_type {
    FCGI_BEGIN_REQUEST      = 1,
    FCGI_ABORT_REQUEST      = 2,
    FCGI_END_REQUEST        = 3,
    FCGI_PARAMS             = 4,
    FCGI_STDIN              = 5,
    FCGI_STDOUT             = 6,
    FCGI_STDERR             = 7,
    FCGI_DATA               = 8,
    FCGI_GET_VALUES         = 9,
    FCGI_GET_VALUES_RESULT  = 10,
    FCGI_UNKOWN_TYPE        = 11
};

struct FCGI_BeginRequestBody {
	unsigned char roleB1;
	unsigned char roleB0;
	unsigned char flags;
	unsigned char reserved[5];
};

struct FCGI_EndRequestBody {
    unsigned char appStatusB3;
    unsigned char appStatusB2;
    unsigned char appStatusB1;
    unsigned char appStatusB0;
    unsigned char protocolStatus;
    unsigned char reserved[3];
};

enum fcgi_role {
    FCGI_RESPONDER      = 1,
    FCGI_AUTHORIZER     = 2,
    FCGI_FILTER         = 3
};

enum protocolStatus {
    FCGI_REQUEST_COMPLETE = 0,
    FCGI_CANT_MPX_CONN = 1,
    FCGI_OVERLOADED = 2,
    FCGI_UNKNOWN_ROLE = 3
};

#pragma pack(pop)


void test_net7() {
	HttpParser parser;
	ss = SocketServerCreator::create([&](const void* buffer, size_t len) -> int{
			//fprintf(stderr, "len:%ld, fcgi_header:%ld\n", len, sizeof(fcgi_header));
			if (len < sizeof(fcgi_header)) {
				return 0;
			}
			fcgi_header* header = (fcgi_header*) buffer;
			u32 contentlength = (header->contentlengthB1 << 8) + header->contentlengthB0;
			u32 package_len = sizeof(fcgi_header) + contentlength + header->paddinglength;
			//fprintf(stderr, "contentlength:%d, B0:%d, B1:%d\n", contentlength, header->contentlengthB0, header->contentlengthB1);
			return len >= package_len ? package_len : 0;
			});
	assert(ss);
	ss->setWorkerNumber(4);
	bool rc = ss->start("0.0.0.0", 9000);
	assert(rc);
	
	auto doServerMessage = [&]() {
		const Socketmessage* msg = ss->receiveMessage();
		if (msg) {
			SOCKET s = GET_MESSAGE_SOCKET(msg);
			if (IS_ESTABLISH_MESSAGE(msg)) {
				fprintf(stderr, "fastcgi: establish: %d\n", s);
			}
			else if (IS_CLOSE_MESSAGE(msg)) {
				fprintf(stderr, "fastcgi: lostConnection: %d\n", s);
			}
			else {
				size_t len = GET_MESSAGE_PAYLOAD_LENGTH(msg);
				fprintf(stderr, "s:%d, payload len: %ld\n", s, len);
				
				const void* payload = GET_MESSAGE_PAYLOAD(msg);
				assert(len >= sizeof(fcgi_header));
				len -= sizeof(fcgi_header);

				fcgi_header* header = (fcgi_header*) payload;
				payload = (u8*) payload + sizeof(fcgi_header);
				u32 requestid = (header->requestidB1 << 8) + header->requestidB0;
				//u32 contentlength = (header->contentlengthB1 << 8) + header->contentlengthB0;
				u32 contentlength = header->contentlength();
				Debug << "version:" << (u32) header->version;
				Debug << "type:" << (u32) header->type;
				Debug << "requestid:" << requestid;
				Debug << "contentlength:" << contentlength;
				Debug << "paddinglength:" << (u32) header->paddinglength;

				switch (header->type) {
					case FCGI_BEGIN_REQUEST:
						if (true) {
							Debug << "FCGI_BEGIN_REQUEST";
							Debug << "leave len: " << len << ", FCGI_BeginRequestBody: " << sizeof(FCGI_BeginRequestBody);
							assert(len >= sizeof(FCGI_BeginRequestBody));
							len -= sizeof(FCGI_BeginRequestBody);
							FCGI_BeginRequestBody* beginrequest = (FCGI_BeginRequestBody*) payload;
							payload = (u8*) payload + sizeof(FCGI_BeginRequestBody);
							u32 role = (beginrequest->roleB1 << 8) + beginrequest->roleB0;
							Debug << "role: " << role;
							Debug << "flags: " << (u32) beginrequest->flags;
						} 
						break;

					case FCGI_PARAMS:
						if (true) {
							Debug << "FCGI_PARAMS";
							Debug << "leave len: " << len;
							//Debug << "read params: " << (const char*) payload;
						}
						break;

					case FCGI_STDIN:
						if (true) {
							Debug << "FCGI_STDIN";
							Debug << "leave len: " << len;
							//Debug << "stdin: " << (const char*) payload;
						}
						break;

					case FCGI_DATA:
						if (true) {
							Debug << "FCGI_DATA";
							Debug << "leave len: " << len;
						}

					default: Error << "unhandled type: " << header->type; break; 
				}

				//ss->sendMessage(s, response, strlen(response));

				++n;
			}
			bundle::releaseMessage(msg);
		}
	};
	
	while (!sConfig.halt) {
		if (ss) {
			doServerMessage();
			//if (n >= N) {
			//	SafeDelete(ss);
			//}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));	
	}

	SafeDelete(ss);
}


// CgiServer
void test_net() {
	CgiServer* cgiserver = CgiServerCreator::create();
	assert(cgiserver);
	bool rc = cgiserver->start("127.0.0.1", 9000);
	assert(rc);
	while (!sConfig.halt) {
		CgiRequest* request = cgiserver->getRequest();
		if (request) {
			auto& headers = request->inputHeaders();
			Debug << "headers: " << headers.size();
			for (auto& i : headers) {
				Debug << "    Key:" << i.first << "," << i.second;
			}
			auto& variables = request->inputVariables();
			Debug << "variables: " << variables.size();
			for (auto& i : variables) {
				Debug << "    Key:" << i.first << "," << i.second;
			}
			auto& cookies = request->cookies();
			Debug << "cookies: " << cookies.size();
			for (auto& i : cookies) {
				Debug << "    Key:" << i.first << "," << i.second;
			}

			char time_buffer[64];
			timestamp(time_buffer, sizeof(time_buffer));

			std::ostringstream o;
			o << "hello, 当前时间: " << time_buffer;
			request->addString(o.str());
			request->setCookie(time_buffer, 10);
			request->send();

			cgiserver->releaseRequest(request);
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));	
		}
	}
	SafeDelete(cgiserver);
}

