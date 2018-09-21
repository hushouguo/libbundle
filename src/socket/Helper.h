/*
 * \file: Helper.h
 * \brief: Created by hushouguo at 20:06:34 Aug 09 2018
 */
 
#ifndef __HELPER_H__
#define __HELPER_H__

#define MAGIC	0x12345678	
#define INTERNAL_SIGN	0x77abc3694dfb225e

BEGIN_NAMESPACE_BUNDLE {
	bool interrupted();
	bool wouldblock();
	bool connectionlost();
	bool reuseableAddress(SOCKET s);
	bool reuseablePort(SOCKET s);
	bool blocking(SOCKET s);
	bool nonblocking(SOCKET s);	
	bool nodelay(SOCKET s, bool enable);	
	bool connectSignal(SOCKET s, const char* address, int port, int seconds);
	bool connectSelect(SOCKET s, const char* address, int port, int milliseconds);

	// Socketmessage
	enum {
		SM_OPCODE_MESSAGE		=	0x1,
		SM_OPCODE_ESTABLISH 	=	0x2,
		SM_OPCODE_CLOSE 		=	0x3,
	};

#pragma pack(push, 1)
	struct Socketmessage {
		u32 magic;
		SOCKET s;
		u8 opcode;
		u32 payload_len;
		u8 payload[0];
	};
#pragma pack(pop)

	Socketmessage* allocateMessage(SOCKET s, u8 opcode);
	Socketmessage* allocateMessage(SOCKET s, u8 opcode, size_t payload_len);
	Socketmessage* allocateMessage(SOCKET s, u8 opcode, const void* payload, size_t payload_len);
	//void releaseMessage(const Socketmessage* msg);
}

#endif
