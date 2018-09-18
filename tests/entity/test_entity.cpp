/*
 * \file: test_entity.cpp
 * \brief: Created by hushouguo at 17:42:05 Sep 07 2018
 */

#include "bundle.h"

using namespace bundle;

void test_entity() {
	Entity entity(1);
	entity["u8"] = 1;
	entity["s8"] = -1;
	entity["u32"] = 100;
	entity["s32"] = -100;
	entity["u64"] = 1234567890;
	entity["s64"] = -1234567890;
	entity["float"] = 0.1f;
	entity["double"] = 1001.005f;
	entity["bool"] = false;
	entity["string"] = "hushouguo";
	std::string s = "士大夫";
	entity["string-utf8"] = s;
	Trace << "dump entity:";
	entity.dump();
	std::ostringstream o;
	bool rc = entity.SerializeToString(o, true);
	assert(rc);
	Trace << "o: " << o.str();

	Entity entity2(2);
	rc = entity2.ParseFromString(o.str().c_str(), o.str().length());
	assert(rc);
	Trace << "dump entity2:";
	entity2.dump();
}
