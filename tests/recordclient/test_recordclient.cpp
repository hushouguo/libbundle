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
	recordClient.serialize(10, "player", 1000, "{\"id\":1000,\"name\":\"hushouguo\",\"level\":\"138\",\"vip\":true}", 
			[&recordClient](u32 shard, std::string table, u64 objectid, u32 retval){
				Trace.cout("receive response:%d,%s,%ld, %d", shard, table.c_str(), objectid, retval);
				recordClient.stop();
			});
#if 0	
	recordClient.unserialize(10, "player", 8, 
			[&recordClient](u32 shard, std::string table, u64 objectid, u32 retval, const char* data, size_t datalen) {
				Trace.cout("unserialize: %d,%s,%ld, %d, %s, %ld", shard, table.c_str(), objectid, retval, data, datalen);
				recordClient.stop();
			});
#endif
	while (!recordClient.isstop()) {
		recordClient.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	System << "test recordclient OK";
}

