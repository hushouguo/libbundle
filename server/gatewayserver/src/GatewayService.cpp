/*
 * \file: GatewayService.cpp
 * \brief: Created by hushouguo at 09:35:50 Sep 01 2018
 */

#include "global.h"

INITIALIZE_INSTANCE(GatewayService);

GatewayService::GatewayService() 
	: NetworkService(
			nullptr,
			[](NetworkService*, NetworkInterface* networkInterface) {
				ClientTask* clientTask = sClientTaskManager.find(networkInterface->fd());
				if (clientTask) {
				}
			},
			[this](NetworkService* networkService, NetworkInterface* networkInterface, Netmessage* netmsg) -> bool {
				bool rc = this->msgParser(networkInterface, netmsg);
				if (!rc) {
					networkService->close(networkInterface);
				}
				return true;
			})
{
}

bool GatewayService::init() {
	return this->start(
			sConfig.get("service.address", "0.0.0.0"), 
			sConfig.get("service.port", 9900)
	);
}

DECLARE_MESSAGE();
bool GatewayService::msgParser(NetworkInterface* task, Netmessage* netmsg) {
	return DISPATCH_MESSAGE(task, netmsg);
}

//Note: ON_MSG(MSGID, STRUCTURE) 
//		ON_MSG(NetworkInterface* task, STRUCTURE* msg, Netmessage* netmsg)
//
//


