/*
 * \file: CentralClient.cpp
 * \brief: Created by hushouguo at 08:36:22 Sep 01 2018
 */

#include "global.h"

INITIALIZE_INSTANCE(CentralClient);

CentralClient::CentralClient() 
	: NetworkClient(
			nullptr,
			[](NetworkClient* nc) {
				Error << "lost CentralServer";
				sConfig.syshalt();
			},
			[this](NetworkClient* nc, const Netmessage* netmsg) -> bool {
				if (!this->msgParser(nc, netmsg)) { this->stop(); }
				return true;
			})
{
}

bool CentralClient::init() {
	return this->connect(
		sConfig.get("centralserver.address", "127.0.0.1"),
		sConfig.get("centralserver.port", 0u)
	);
}

DECLARE_MESSAGE();
bool CentralClient::msgParser(NetworkInterface* task, const Netmessage* netmsg) {
	return DISPATCH_MESSAGE(task, netmsg);
}


//Note: ON_MSG(MSGID, STRUCTURE) 
//		ON_MSG(NetworkInterface* task, STRUCTURE* msg, Netmessage* netmsg)
//


