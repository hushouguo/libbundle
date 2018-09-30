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

	bundle::decode_jscode("wx4c05f6310ae5922d", "8e73b42717adb805086e7d76198e24be", "021K1vv72iOXpK0Tmrx723RLv72K1vvI", nullptr,
			[](bool rc, std::string session_key, std::string openid, void* context) {
				Trace << "rc: " << rc;
				Trace << "session_key: " << session_key << ", openid: " << openid;
				Trace << "context: " << context;
				sConfig.syshalt();
	});

	while (!sConfig.halt) {
		sleep(1);
	}

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

	bundle::shutdown_bundle_library();
	return 0;
}
