/*
 * \file: main.cpp
 * \brief: Created by hushouguo at 22:07:32 Aug 24 2018
 */

#include "global.h"
 
int main(int argc, char* argv[]) {
	if (!sConfig.init(argc, argv)) { return 1; }

	Easylog::syslog()->set_level((EasylogSeverityLevel) sConfig.get("log.level", GLOBAL));
	Easylog::syslog()->set_autosplit_day(sConfig.get("log.autosplit_day", true));
	Easylog::syslog()->set_autosplit_hour(sConfig.get("log.autosplit_hour", false));
	Easylog::syslog()->set_destination(sConfig.get("log.dir", ".logs"));
	Easylog::syslog()->set_tofile(GLOBAL, "recordserver");
	Easylog::syslog()->set_tostdout(GLOBAL, sConfig.runasdaemon ? false : true);

	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;

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
	Easylog::syslog()->stop();

	// Optional:  Delete all global objects allocated by libprotobuf.
	google::protobuf::ShutdownProtobufLibrary();

	return 0;	 
}


