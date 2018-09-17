/*
 * \file: RecordProcessManager.h
 * \brief: Created by hushouguo at 03:16:26 Sep 07 2018
 */
 
#ifndef __RECORD_PROCESS_MANAGER_H__
#define __RECORD_PROCESS_MANAGER_H__

class RecordProcessManager {
	public:
		bool init();
		void stop();
		void update();
		bool request(u32 shard, u32 tableid, u64 objectid, SOCKET s, Netmessage* netmsg);

	private:
		std::unordered_map<u32, RecordProcess*> _recordProcesses;
};

#define sRecordProcessManager bundle::Singleton<RecordProcessManager>::getInstance()

#endif
