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

void test_net() {
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

