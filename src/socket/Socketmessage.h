/*
 * \file: Socketmessage.h
 * \brief: Created by hushouguo at 09:13:16 Aug 10 2018
 */
 
#ifndef __SOCKET_MESSAGE_H__
#define __SOCKET_MESSAGE_H__

#define MAGIC	0x12345678	
#define INTERNAL_SIGN	0x77abc3694dfb225e

BEGIN_NAMESPACE_BUNDLE {
	enum {
		SM_OPCODE_MESSAGE		=	0x0,
		SM_OPCODE_ESTABLISH 	=	0x1,
		SM_OPCODE_CLOSE 		=	0x2,
	};
#pragma pack(push, 1)
	struct Socketmessage {
		u32 magic;
		SOCKET s;
		u8 opcode;
		Rawmessage rawmsg[0];
	};
#pragma pack(pop)
	Socketmessage* allocateMessage(SOCKET s, u8 opcode);
	Socketmessage* allocateMessage(SOCKET s, u8 opcode, const void* payload, size_t payload_len);
	void releaseMessage(Socketmessage* msg);
}

#endif
