/*
 * \file: test_net.cpp
 * \brief: Created by hushouguo at 16:31:49 Sep 06 2018
 */

#include "bundle.h"

using namespace bundle;
void test_net() {
	SocketServer* ss = SocketServerCreator::create([=](const Byte* buffer, size_t len) -> int{
			return len;
			});
	assert(ss);
	bool rc = ss->start("0.0.0.0", 12306);
	assert(rc);
	
	SocketClient* cs = SocketClientCreator::create([](const Byte* buffer, size_t len) -> int{
			return len;
			});
	assert(cs);
	rc = cs->connect("127.0.0.1", 12306, true);
	assert(rc);

	SOCKET s = -1;
	auto socketserver_run = [ss, &s]() -> char {
		char c = 0;
		while (true) {
			//SOCKET s = -1;
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
					if (rawmsg->payload_len > 0) {
						c = rawmsg->payload[0];
						break;
						//fprintf(stderr, "payload: %s\n", rawmsg->payload);
					}
				}
				ss->releaseMessage(rawmsg);
			}
			else {
				usleep(1);
				//break;
			}
		}
		return c;
	};

	auto socketclient_run = [cs]() -> char {
		char c = 0;
		while (true) {
			bool establish = false, close = false;
			Rawmessage* rawmsg = cs->receiveMessage(establish, close);
			if (rawmsg) {
				if (establish) {
					//fprintf(stderr, "SocketClient: establish\n");
				}
				else if (close) {
					//fprintf(stderr, "SocketClient: disconnect\n");
				}
				else {
					//printf("receive rawmsg: %d \n", rawmsg->payload_len);
					if (rawmsg->payload_len > 0) {
						//printf("payload: %s\n", rawmsg->payload);
						c = rawmsg->payload[0];
						break;
					}
				}
				cs->releaseMessage(rawmsg);
			}
			else {
				usleep(1);
				//break;
			}
		}
		return c;
	};

	char c = 'a';
	cs->sendMessage(&c, sizeof(c));
	char res = socketserver_run();
	if (c == res) {
		Trace << "client to server result OK";
	}
	else {
		Error << "client to server result Fail";
	}
	ss->sendMessage(s, &c, sizeof(c));
	res = socketclient_run();
	if (c == res) {
		Trace << "server to client result OK";
	}
	else {
		Error << "server to client result Fail";
	}

	SafeDelete(ss);
	SafeDelete(cs);

	System << "test net OK";
}
