/*
 * \file: main.cpp
 * \brief: Created by hushouguo at 00:17:08 Aug 31 2018
 */

#include "global.h"

int main(int argc, char* argv[]) {
	if (!init_runtime_environment(argc, argv)) { return 1; }
	
	CHECK_GOTO(sLogService.init(), exit_except, "LogService init failure");
	
	while (!sConfig.halt) {
		sTime.now();
		sLogService.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

exit_except:
	sLogService.stop();
	shutdown_bundle_library();
	return 0;	
}
