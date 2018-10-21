/*
 * \file: LogService.cpp
 * \brief: Created by hushouguo at 09:35:50 Sep 01 2018
 */

#include "global.h"

INITIALIZE_INSTANCE(LogService);

LogService::LogService() 
	: NetworkService(
			nullptr,
			[](NetworkService*, NetworkInterface* networkInterface) {
				ClientTask* clientTask = sClientTaskManager.find(networkInterface->fd());
				if (clientTask) {
				}
			},
			[this](NetworkService* networkService, NetworkInterface* networkInterface, const Netmessage* netmsg) -> bool {
				bool rc = this->msgParser(networkInterface, netmsg);
				if (!rc) {
					networkService->close(networkInterface);
				}
				return true;
			})
{
}

bool LogService::init() {
	return this->start(sConfig.get("service.address", "0.0.0.0"), sConfig.get("service.port", 9800), sConfig.get("service.worker", 0));
}

DECLARE_MESSAGE();
bool LogService::msgParser(NetworkInterface* task, const Netmessage* netmsg) {
	return DISPATCH_MESSAGE(task, netmsg);
}

//Note: ON_MSG(MSGID, STRUCTURE) 
//		ON_MSG(NetworkInterface* task, const STRUCTURE* msg, const Netmessage* netmsg)
//
//


