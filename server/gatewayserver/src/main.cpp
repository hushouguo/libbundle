/*
 * \file: main.cpp
 * \brief: Created by hushouguo at 00:17:08 Aug 31 2018
 */

#include "global.h"

int main(int argc, char* argv[]) {
	if (!sConfig.init(argc, argv)) { return 1; }

	Easylog::syslog()->set_level((EasylogSeverityLevel) sConfig.get("log.level", GLOBAL));
	Easylog::syslog()->set_autosplit_day(sConfig.get("log.autosplit_day", true));
	Easylog::syslog()->set_autosplit_hour(sConfig.get("log.autosplit_hour", false));
	Easylog::syslog()->set_destination(sConfig.get("log.dir", ".logs"));
	Easylog::syslog()->set_tofile(GLOBAL, "gatewayserver");
	Easylog::syslog()->set_tostdout(GLOBAL, sConfig.runasdaemon ? false : true);
	
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	//CHECK_GOTO(sCentralClient.init(), exit_except, "CentralClient init failure");
	CHECK_GOTO(sGatewayService.init(), exit_except, "GatewayService init failure");
	
	if (true) {
		sEntityDescriptor.loadDescriptor(1, "./player.xml");
		Entity* entity = new Entity(1000, 1);
		entity->dump();
	}


	while (!sConfig.halt) {
		sTime.now();
		//sCentralClient.update();
		sGatewayService.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

exit_except:

	sCentralClient.stop();
	sGatewayService.stop();
	Easylog::syslog()->stop();
	
	// Optional:  Delete all global objects allocated by libprotobuf.
	google::protobuf::ShutdownProtobufLibrary();
	
	return 0;	
}
