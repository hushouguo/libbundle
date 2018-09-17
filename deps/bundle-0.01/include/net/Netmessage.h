/*
 * \file: Netmessage.h
 * \brief: Created by hushouguo at 20:50:27 Sep 07 2018
 */
 
#ifndef __NETMESSAGE_H__
#define __NETMESSAGE_H__
 
BEGIN_NAMESPACE_BUNDLE {
#pragma pack(push, 1)
	struct Netmessage {
		Netmessage(u16 _id) : len(sizeof(Netmessage)), id(_id), flags(0), timestamp(0) {}
		u32 len;
		u16 id;
		u16 flags;
		u32 timestamp;
		char payload[0];
	};
#pragma pack(pop)	

#define NEW_MSG(STRUCTURE, SIZE, ...)	\
	char __message_buffer__[SIZE];\
	STRUCTURE* newmsg = Constructor((STRUCTURE *)(__message_buffer__), ##__VA_ARGS__)


#define MAX_MESSAGE_ID	((u16)(-1))
#define DECLARE_MESSAGE() \
	typedef bool (*MESSAGE_ROUTINE)(NetworkInterface* task, const Netmessage* netmsg);\
	struct MessageTable {\
		MESSAGE_ROUTINE table[MAX_MESSAGE_ID];\
	};\
	static MessageTable __t;\
	__attribute__((constructor)) static void __t_init() {\
		memset(__t.table, 0, sizeof(__t.table));\
	}
	

#define ON_MSG(MSGID, STRUCTURE) \
	static bool onMessage_raw_##STRUCTURE(NetworkInterface* task, const Netmessage* netmsg);\
	static void onMessage_##STRUCTURE(NetworkInterface* task, STRUCTURE* msg, const Netmessage* netmsg);\
	__attribute__((constructor)) static void __##STRUCTURE() {\
		assert(MSGID >= 0 && MSGID < MAX_MESSAGE_ID);\
		if (__t.table[MSGID]) {\
			fprintf(stderr, "duplicate message id:%d, %s\n", MSGID, #STRUCTURE);\
			::abort();\
		}\
		else {\
			__t.table[MSGID] = onMessage_raw_##STRUCTURE;\
		}\
	}\
	static bool onMessage_raw_##STRUCTURE(NetworkInterface* task, const Netmessage* netmsg) {\
		STRUCTURE msg;\
		bool rc = msg.ParseFromArray(netmsg->payload, netmsg->len - sizeof(Netmessage));\
		CHECK_RETURN(rc, false, "%s ParseFromArray failure:%d", #STRUCTURE, netmsg->len);\
		onMessage_##STRUCTURE(task, &msg, netmsg);\
		return rc;\
	}\
	static void onMessage_##STRUCTURE(NetworkInterface* task, const STRUCTURE* msg, const Netmessage* netmsg)
	


#define DISPATCH_MESSAGE(task, netmsg) \
	({\
		bool rc = false;\
		if (netmsg->id < 0 || netmsg->id >= MAX_MESSAGE_ID) {\
			Error.cout("illegal netmsg->id:%d", netmsg->id);\
		}\
		else {\
			rc = true;\
			if (__t.table[netmsg->id]) {\
				rc = __t.table[netmsg->id](task, netmsg);\
			}\
			else {\
				Error.cout("unhandled netmsg:%d", netmsg->id);\
			}\
		}\
		rc;\
	})
}

#endif
