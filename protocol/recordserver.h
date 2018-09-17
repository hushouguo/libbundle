/*
 * \file: recordserver.h
 * \brief: Created by hushouguo at 00:18:22 Sep 06 2018
 */
 
#ifndef __RECORDSERVER_H__
#define __RECORDSERVER_H__

BEGIN_NAMESPACE_BUNDLE {
#pragma pack(push, 1)
	// Serialize: create & update
	struct ObjectSerializeRequest : public Netmessage {
		enum { id = 0x101 };
		ObjectSerializeRequest() : Netmessage(id) {}
		u32  shard;
		char table[16];
		u64  objectid;
		size_t size() { return sizeof(*this) + this->datalen; }
		u32  datalen;
		char data[0];	// json encoding
	};
	
	struct ObjectSerializeResponse : public Netmessage {
		enum { id = 0x102 };
		ObjectSerializeResponse() : Netmessage(id) {}
		u32  shard;
		char table[16];
		u64  objectid;
		size_t size() { return sizeof(*this); }
		u32 retval;		// 0: OK, !0: errno
	};

	// Unserialize
	struct ObjectUnserializeRequest : public Netmessage {
		enum { id = 0x103 };
		ObjectUnserializeRequest() : Netmessage(id) {}
		u32  shard;
		char table[16];
		u64  objectid;
		size_t size() { return sizeof(*this); }
	};
	
	struct ObjectUnserializeResponse : public Netmessage {
		enum { id = 0x104 };
		ObjectUnserializeResponse() : Netmessage(id) {}
		u32  shard;
		char table[16];
		u64  objectid;
		size_t size() { return sizeof(*this) + this->datalen; }
		u32 retval;		// 0: OK, !0: errno
		u32  datalen;
		char data[0];	// json encoding
	};


	// Delete
	struct ObjectDeleteRequest : public Netmessage {
		enum { id = 0x105 };
		ObjectDeleteRequest() : Netmessage(id) {}
		u32  shard;
		char table[16];
		u64  objectid;
		size_t size() { return sizeof(*this); }
	};
	
	struct ObjectDeleteResponse : public Netmessage {
		enum { id = 0x106 };
		ObjectDeleteResponse() : Netmessage(id) {}
		u32  shard;
		char table[16];
		u64  objectid;
		size_t size() { return sizeof(*this); }
		u32 retval;		// 0: OK, !0: errno
	};

	// Select
	struct ObjectSelectRequest : public Netmessage {
		enum { id = 0x107 };
		ObjectSelectRequest() : Netmessage(id) {}
		u32  shard;
		char table[16];
		u64  objectid;
		size_t size() { return sizeof(*this) + this->length; }
		u32  length;
		char where[0];
	};

	struct ObjectSelectResponse : public Netmessage {
		enum { id = 0x108 };
		ObjectSelectResponse() : Netmessage(id) {}
		u32  shard;
		char table[16];
		u64  objectid;
		size_t size() { return sizeof(*this) + this->length; }
		u32  length;
		char objects[0];	// json encoding, {"id":"entitydata", "id":"entitydata"}
	};
#pragma pack(pop)	
}

#endif
