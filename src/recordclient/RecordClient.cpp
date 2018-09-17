/*
 * \file: RecordClient.cpp
 * \brief: Created by hushouguo at 22:05:23 Aug 24 2018
 */

#include "bundle.h"
#include "protocol/recordserver.h"

BEGIN_NAMESPACE_BUNDLE {
	RecordClient::RecordClient() 
		: NetworkClient(
			nullptr,
			[](NetworkClient*) {
				Error << "lost recordserver";
			},
			[this](NetworkClient* networkClient, const Netmessage* netmsg) -> bool {
				if (!this->msgParser(networkClient, netmsg)) { networkClient->stop(); }
				return true;
			})
	{
	}

	void RecordClient::serialize(u32 shard, std::string table, u64 objectid, const std::string& js, std::function<void(u32, u32, u64)> func) {
		NEW_MSG(ObjectSerializeRequest, js.length() + sizeof(ObjectSerializeRequest));
		newmsg->shard = shard;
		strncpy(newmsg->table, table.c_str(), sizeof(newmsg->table));
		newmsg->objectid = objectid;
		newmsg->datalen = js.length();
		memcpy(newmsg->data, js.data(), js.length());
		newmsg->len = newmsg->size();
		this->sendMessage(newmsg, newmsg->len);
		this->_serializeCallbacks.push_back(func);
	}
	
	void RecordClient::unserialize(u32 shard, u32 tableid, u64 objectid, std::function<void(u32, u32, u64, const char*, size_t)> func) {
		NEW_MSG(ObjectUnserializeRequest, sizeof(ObjectUnserializeRequest));
		newmsg->shard = shard;
		strncpy(newmsg->table, table.c_str(), sizeof(newmsg->table));
		newmsg->objectid = objectid;
		newmsg->len = newmsg->size();
		this->sendMessage(newmsg, newmsg->len);
		this->_unserializeCallbacks.push_back(func);
	}

	bool RecordClient::msgParser(NetworkInterface* task, Netmessage* netmsg) {
		switch (netmsg->id) {
			case ObjectSerializeResponse::id:
				if (!this->_serializeCallbacks.empty()) {
					ObjectSerializeResponse* response = (ObjectSerializeResponse *) netmsg;
					CHECK_RETURN(netmsg->len == response->size(), false, "illegal SerializeResponse");
					std::function<void(u32, u32, u64)> func = this->_serializeCallbacks.front();
					this->_serializeCallbacks.pop_front();
					if (func) {
						func(response->shard, response->table, response->objectid);
					}
				}
				break;

			case ObjectUnserializeResponse::id:
				if (!this->_unserializeCallbacks.empty()) {
					ObjectUnserializeResponse* response = (ObjectUnserializeResponse *) netmsg;
					CHECK_RETURN(netmsg->len == response->size(), false, "illegal UnserializeResponse");
					std::function<void(u32, u32, u64, const char*, size_t)> func = this->_unserializeCallbacks.front();
					this->_unserializeCallbacks.pop_front();
					if (func) {
						func(response->shard, response->table, response->objectid, response->data, response->datalen);
					}
				}
				break;
				
			default: Error << "unhandled recordmessage: " << netmsg->id; return false;
		}
		return true;
	}
}

