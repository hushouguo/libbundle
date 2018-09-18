/*
 * \file: Recordmessage.cpp
 * \brief: Created by hushouguo at 00:09:21 Sep 08 2018
 */

#include "global.h"
#include "protocol/recordserver.h"

Recordmessage::Recordmessage(SOCKET s, const Netmessage* netmsg) {
	this->s = s;
	this->netmsg = netmsg;
}

Recordmessage::~Recordmessage() {
}

