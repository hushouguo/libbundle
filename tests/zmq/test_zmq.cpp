/*
 * \file: test_zmq.cpp
 * \brief: Created by hushouguo at 16:24:53 Nov 29 2018
 */

#include "bundle.h"

using namespace bundle;

void test_zmq() {
	MessageQueue Q(ZMQ_SERVER);
	bool rc = Q.init("0.0.0.0", 12306);
	assert(rc);
	while (!sConfig.halt) {
		Q.run();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
