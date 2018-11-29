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
#include "uv/test_uv.h"
#include "zmq/test_zmq.h"

int main(int argc, char** argv) {	
	if (!bundle::init_runtime_environment(argc, argv)) { return 1; }

	//test_tools();
	//test_net();
	//test_csv("./csv/test.csv");
	//test_xml("./xml/conf.xml");
	//test_xml("../server/recordserver/conf/db.xml");
	//test_xml2("../server/recordserver/conf/db.xml");
	//test_easylog();
	//test_recordclient("127.0.0.1", 9990);
	//test_entity();
	//test_lockfree();
	//test_uv();
	test_zmq();

	bundle::shutdown_bundle_library();
	return 0;
}
