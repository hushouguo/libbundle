/*
 * \file: Rawmessage.h
 * \brief: Created by hushouguo at 20:47:56 Sep 07 2018
 */
 
#ifndef __RAWMESSAGE_H__
#define __RAWMESSAGE_H__
 
BEGIN_NAMESPACE_BUNDLE {
#pragma pack(push, 1)
	struct Rawmessage {
		u32 payload_len;
		u8 payload[0];
	};
#pragma pack(pop)
}

#endif
