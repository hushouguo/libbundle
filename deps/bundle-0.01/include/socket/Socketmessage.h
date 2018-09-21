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
	const void* messagePayload(const Socketmessage*);
	size_t messagePayloadLength(const Socketmessage*);
}

#endif
