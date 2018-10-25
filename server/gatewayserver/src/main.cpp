/*
 * \file: main.cpp
 * \brief: Created by hushouguo at 00:17:08 Aug 31 2018
 */

#include "global.h"

int main(int argc, char* argv[]) {
	printf("sizeof(suseconds_t): %ld\n", sizeof(suseconds_t));

	if (!init_runtime_environment(argc, argv)) { return 1; }
	
	//CHECK_GOTO(sCentralClient.init(), exit_except, "CentralClient init failure");
	CHECK_GOTO(sGatewayService.init(), exit_except, "GatewayService init failure");
	
	if (false) {
		sEntityDescriptor.loadDescriptor(1, "./player.xml");
		Entity* entity = new Entity(1000);
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
	shutdown_bundle_library();
	return 0;	
}
