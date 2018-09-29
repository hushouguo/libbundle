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
			SOCKET s = -1;
			bool establish = false, close = false;
			const Socketmessage* msg = ss->receiveMessage(s, establish, close);
			if (msg) {
				if (establish) {
					//fprintf(stderr, "SocketServer: establish: %d\n", s);
				}
				else if (close) {
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
			SOCKET s = -1;
			bool establish = false, close = false;
			const Socketmessage* msg = ss->receiveMessage(s, establish, close);
			if (msg) {
				if (establish) {
					//fprintf(stderr, "SocketServer: establish: %d\n", s);
				}
				else if (close) {
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

void test_net() {
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
		SOCKET s = -1;
		bool establish = false, close = false;
		const Socketmessage* msg = ss->receiveMessage(s, establish, close);
		if (msg) {
			if (establish) {
				//fprintf(stderr, "SocketServer: establish: %d\n", s);
			}
			else if (close) {
				//fprintf(stderr, "SocketServer: lostConnection: %d\n", s);
			}
			else {
				fprintf(stderr, "receive Socketmessage: %d, payload:%s, payload_len:%ld\n", 
						n, (const char*)messagePayload(msg), messagePayloadLength(msg));
				++n;
			}
			bundle::releaseMessage(msg);
		}
	};

	auto doClientMessage = []() {
		bool establish = false, close = false;
		const Socketmessage* msg = cs->receiveMessage(establish, close);
		if (msg) {
			if (establish) {
				fprintf(stderr, "SocketClient: establish\n");
			}
			else if (close) {
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

