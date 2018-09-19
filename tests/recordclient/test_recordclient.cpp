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

	char time_buffer[64];
	timestamp(time_buffer, sizeof(time_buffer), 0, nullptr);
	
	u32 shard = 10;
	const char* table = "player";
	u32 count = 1000;
	std::vector<std::string> v;
	for (u32 n = 0; n < count; ++n) {
		std::string nickname;
		randomString(nickname, 8, false, true, true);

		// 28 fields
		std::ostringstream o;
		o << "{\"id\":" << n;
		o << ",\"name\":\"" << nickname << "\"";
		o << ",\"level\":" << randomBetween(1, 100);
		o << ",\"diamond\":" << randomBetween(1, 100);
		o << ",\"vip\":" << randomBetween(1, 10);
		o << ",\"account\":\"" << nickname << "\"";
		o << ",\"gender\":" << randomBetween(0, 1);
		o << ",\"job\":" << randomBetween(1, 8);
		o << ",\"exp\":" << randomBetween(1, 10);
		o << ",\"gold\":" << randomBetween(1, 100);
		o << ",\"createtime\":" << time_buffer;
		o << ",\"strength\":" << randomBetween(1, 10);
		o << ",\"agility\":" << randomBetween(1, 10);
		o << ",\"intellect\":" << randomBetween(1, 10);
		o << ",\"hp\":" << randomBetween(1, 10);
		o << ",\"sp\":" << randomBetween(1, 10);
		o << ",\"attack_speed\":" << randomBetween(1, 10);
		o << ",\"move_speed\":" << randomBetween(1, 10);
		o << ",\"spirit\":" << randomBetween(1, 10);
		o << ",\"block\":" << randomBetween(1, 10);
		o << ",\"damage\":" << randomBetween(1, 10);
		o << ",\"defence\":" << randomBetween(1, 10);
		o << ",\"maxhp\":" << randomBetween(1, 10);
		o << ",\"maxsp\":" << randomBetween(1, 10);
		o << ",\"gametime\":" << randomBetween(1, 10);
		o << ",\"bag\":" << "" << "\"";
		o << ",\"mailbox\":" << "" << "\"";
		o << ",\"friendlist\":" << "" << "\"";
		o << "}";
		v.push_back(o.str());
	}

	u32 res_ok = 0, res_fail = 0;
	
	Time t1;
	for (u32 n = 0; n < count; ++n) {
		recordClient.serialize(shard, table, n, v[n], 
				[&recordClient, &res_ok, &res_fail, count](u32 shard, std::string table, u64 objectid, u32 retval){
					//std::ostringstream o;
					//o << "serialize response, shard: " << shard;
					//o << "table: " << table;
					//o << "objectid: " << objectid;				
					//o << "retval: " << retval;
					//Trace << o.str();
					retval == 0 ? ++res_ok : ++res_fail;
					if ((res_ok + res_fail) == count) {
						recordClient.stop();
					}
				});
	}
	

#if 0
	recordClient.unserialize(10, "player", 1000, 
			[&recordClient](u32 shard, std::string table, u64 objectid, u32 retval, const char* data, size_t datalen) {
				std::ostringstream o;
				o << "unserialize response, shard: " << shard;
				o << "table: " << table;
				o << "objectid: " << objectid;				
				o << "retval: " << retval;
				o << "datalen: " << datalen;
				o << "data: " << data;
				Trace << o.str();
			});
			
	recordClient.addKey(10, "player", 1000, "level", 
			[&recordClient](u32 shard, std::string table, u64 objectid, u32 retval){
				std::ostringstream o;
				o << "addKey response, shard: " << shard;
				o << "table: " << table;
				o << "objectid: " << objectid;				
				o << "retval: " << retval;
				Trace << o.str();
			});
#endif
	
	while (!recordClient.isstop()) {
		recordClient.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	Time t2;
	Trace << "benchmark: serialize " << count << " objects, cost milliseconds: " << t2 - t1;
	Trace << "    Success: " << res_ok << ", Fail: " << res_fail;

	System << "test recordclient OK";
}

