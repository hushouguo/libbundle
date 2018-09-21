/*
 * \file: test_net.cpp
 * \brief: Created by hushouguo at 16:31:49 Sep 06 2018
 */

#include "bundle.h"

using namespace bundle;

//Time t1, t2;
struct timeval t1, t2;
u32 N = 1;

SocketServer* ss = nullptr;
SocketClient* cs = nullptr;
std::thread* loop = nullptr;

void createServer(u32 msgsize) {
	ss = SocketServerCreator::create([=](const Byte* buffer, size_t len) -> int{
			if (len >= msgsize) {
				gettimeofday(&t2, nullptr);
				u64 t1_us = t1.tv_sec * 1000 * 1000 + t1.tv_usec;
				u64 t2_us = t2.tv_sec * 1000 * 1000 + t2.tv_usec;
				Trace << "receive cost us: " << t2_us - t1_us;
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
			Rawmessage* rawmsg = ss->receiveMessage(s, establish, close);
			if (rawmsg) {
				if (establish) {
					//fprintf(stderr, "SocketServer: establish: %d\n", s);
				}
				else if (close) {
					//fprintf(stderr, "SocketServer: lostConnection: %d\n", s);
				}
				else {
					//printf("receive rawmsg: %d from SOCKET: %d\n", rawmsg->payload_len, s);
					++n;
				}
				ss->releaseMessage(rawmsg);
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
	cs = SocketClientCreator::create([=](const Byte* buffer, size_t len) -> int{
			return len >= msgsize ? msgsize : 0;
			});
	assert(cs);
	bool rc = cs->connect("127.0.0.1", 12306, true);
	assert(rc);

	//t1.now();
	gettimeofday(&t1, nullptr);

	struct timeval ta, tb, tc;
	for (u32 i = 0; i < N; ++i) {
		gettimeofday(&ta, nullptr);
		Rawmessage* msg = cs->initMessage(msgsize);
		gettimeofday(&tb, nullptr);
		cs->sendMessage(msg);
		gettimeofday(&tc, nullptr);

		u64 ta_us = ta.tv_sec * 1000 * 1000 + ta.tv_usec;
		u64 tb_us = tb.tv_sec * 1000 * 1000 + tb.tv_usec;
		u64 tc_us = tc.tv_sec * 1000 * 1000 + tc.tv_usec;
		Trace << "initMessage cost us: " << tb_us - ta_us << ", sendMessage: " << tc_us - tb_us;
	}
}

void test_net() {
	u32 sizes[] = {
//		4*KB, 8*KB, 16*KB, 32*KB, 64*KB, 128*KB, 256*KB, 512*KB, 
//		1*MB, 2*MB, 4*MB, 8*MB, 16*MB, 32*MB, 64*MB, 128*MB, 256*MB
		256*MB
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

