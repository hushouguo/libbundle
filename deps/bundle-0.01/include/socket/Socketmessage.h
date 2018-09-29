/*
 * \file: Socketmessage.h
 * \brief: Created by hushouguo at 07:46:33 Sep 21 2018
 */
 
#ifndef __SOCKETMESSAGE_H__
#define __SOCKETMESSAGE_H__

BEGIN_NAMESPACE_BUNDLE {
	class Socketmessage;
	Socketmessage* allocateMessage(size_t);
	Socketmessage* allocateMessage(size_t, const void*);
	void releaseMessage(const Socketmessage*);
	
	SOCKET GET_MESSAGE_SOCKET(const Socketmessage*);
	const void* GET_MESSAGE_PAYLOAD(const Socketmessage*);
	size_t GET_MESSAGE_PAYLOAD_LENGTH(const Socketmessage*);
	const Socketmessage* GET_MESSAGE_BY_PAYLOAD(const void*);
	bool IS_ESTABLISH_MESSAGE(const Socketmessage*);
	bool IS_CLOSE_MESSAGE(const Socketmessage*);
}

#endif
