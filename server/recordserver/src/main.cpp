/*
 * \file: main.cpp
 * \brief: Created by hushouguo at 22:07:32 Aug 24 2018
 */

#include "global.h"
 
int main(int argc, char* argv[]) {
	if (!init_runtime_environment(argc, argv)) { return 1; }

	CHECK_GOTO(sRecordProcessManager.init(), exit_except, "RecordProcessManager init failure");
	CHECK_GOTO(sRecordService.init(), exit_except, "RecordService init failure");

	while (!sConfig.halt) {
		sTime.now();
		sRecordService.update();
		sRecordProcessManager.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

exit_except:
	sRecordService.stop();
	sRecordProcessManager.stop();
	shutdown_bundle_library();
	return 0;
}


