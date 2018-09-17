/*
 * \file: RecordService.h
 * \brief: Created by hushouguo at 22:08:50 Aug 24 2018
 */
 
#ifndef __RECORD_SERVICE_H__
#define __RECORD_SERVICE_H__

class RecordService : public NetworkService {
	public:
		RecordService();

	public:
		bool init();

	private:
		bool msgParser(NetworkInterface* task, Netmessage* netmsg);
};

#define sRecordService bundle::Singleton<RecordService>::getInstance()

#endif
