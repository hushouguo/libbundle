/*
 * \file: test_recordclient.cpp
 * \brief: Created by hushouguo at 00:14:18 Sep 07 2018
 */

#include "bundle.h"

using namespace bundle;

void test_recordclient(const char* address, int port) {
	RecordClient recordClient;
	bool rc = recordClient.connect(address, port);
	CHECK_RETURN(rc, void(0), "unreachable recordserver: %s,%d", address, port);
	recordClient.serialize(11, 1, 1000, "{\"id\":1000,\"name\":\"hushouguo\"}", 
			[&recordClient](u32 shard, u32 tableid, u64 objectid){
				Trace.cout("receive response:%d,%d,%ld", shard, tableid, objectid);
				recordClient.stop();
			});
	while (!recordClient.isstop()) {
		recordClient.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	System << "test recordclient OK";
}

