/*
 * \file: Recordmessage.h
 * \brief: Created by hushouguo at 00:04:32 Sep 08 2018
 */
 
#ifndef __RECORD_MESSAGE_H__
#define __RECORD_MESSAGE_H__

struct Recordmessage {
	SOCKET s = -1;
	Netmessage* netmsg = nullptr;
	Recordmessage(SOCKET, Netmessage*);
	~Recordmessage();
};

#endif
