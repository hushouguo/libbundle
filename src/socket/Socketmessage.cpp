/*
 * \file: Socketmessage.cpp
 * \brief: Created by hushouguo at 09:14:31 Aug 10 2018
 */

#include "bundle.h"
#include "Socketmessage.h"

BEGIN_NAMESPACE_BUNDLE {
	//static std::atomic<int> allocated_messages = 0;
	Socketmessage* allocateMessage(SOCKET s, u8 opcode) {
		Socketmessage* msg = (Socketmessage*) ::malloc(sizeof(Socketmessage));
		msg->magic = MAGIC;
		msg->s = s;
		msg->opcode = opcode;
		msg->payload_len = 0;
		return msg;
	}

	Socketmessage* allocateMessage(SOCKET s, u8 opcode, size_t payload_len) {
		Socketmessage* msg = (Socketmessage*) ::malloc(sizeof(Socketmessage) + payload_len);
		msg->magic = MAGIC;
		msg->s = s;
		msg->opcode = opcode;
		msg->payload_len = payload_len;
		return msg;
	}

	Socketmessage* allocateMessage(SOCKET s, u8 opcode, const void* payload, size_t payload_len) {
		Socketmessage* msg = (Socketmessage*) ::malloc(sizeof(Socketmessage) + payload_len);
		msg->magic = MAGIC;
		msg->s = s;
		msg->opcode = opcode;
		msg->payload_len = payload_len;
		if (payload) {
			memcpy(msg->payload, payload, payload_len);
		}
		return msg;
	}
	
	void releaseMessage(Socketmessage* msg) {
		assert(msg->magic == MAGIC);
		SafeFree(msg);
	}
}

