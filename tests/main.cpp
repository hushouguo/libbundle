/*
 * \file: main.cpp
 * \brief: Created by hushouguo at 16:43:49 Sep 06 2018
 */

#include <libgen.h>

#include "bundle.h"
#include "easylog/test_easylog.h"
#include "tools/test_tools.h"
#include "net/test_net.h"
#include "csv/test_csv.h"
#include "xml/test_xml.h"
#include "recordclient/test_recordclient.h"
#include "entity/test_entity.h"
#include "lockfree/test_lockfree.h"

int main(int argc, char** argv) {	
	if (!bundle::init_runtime_environment(argc, argv)) { return 1; }
	bundle::Easylog::syslog()->set_tostdout(bundle::GLOBAL, true);

	std::string s = "101";
	try {
		int value = std::stoi(s);
		fprintf(stderr, "s:%s, value:%d\n", s.c_str(), value);
	} catch(std::exception& e) {
		fprintf(stderr, "s: %s, exception: %s\n", s.c_str(), e.what());
	}
	
	s = "10111aas2";
	try {
		int value = std::stoi(s);
		fprintf(stderr, "s:%s, value:%d\n", s.c_str(), value);
	} catch(...) {
		fprintf(stderr, "s:%s, exception\n", s.c_str());
	}

	int value = strtol(s.c_str(), (char**)NULL, 10);
	if (errno == EINVAL || errno == ERANGE) {
		fprintf(stderr, "invalid s:%s\n", s.c_str());
	}	
	else {
		fprintf(stderr, "s:%s, value:%d\n", s.c_str(), value);
	}

	//test_tools();
	test_net();
	//test_csv("./csv/test.csv");
	//test_xml("./xml/conf.xml");
	//test_xml("../server/recordserver/conf/db.xml");
	//test_xml2("../server/recordserver/conf/db.xml");
	//test_easylog();
	//test_recordclient("127.0.0.1", 9990);
	//test_entity();
	//test_lockfree();

	bundle::shutdown_bundle_library();
	return 0;
}
