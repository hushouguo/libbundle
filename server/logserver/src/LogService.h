/*
 * \file: LogService.h
 * \brief: Created by hushouguo at 09:34:23 Sep 01 2018
 */
 
#ifndef __LOG_SERVICE_H__
#define __LOG_SERVICE_H__

class LogService : public NetworkService {
	public:
		LogService();

	public:
		bool init();

	private:
		bool msgParser(NetworkInterface* task, const Netmessage* netmsg);
};

#define sLogService bundle::Singleton<LogService>::getInstance()

#endif
