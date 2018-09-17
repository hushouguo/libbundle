/*
 * \file: RecordService.cpp
 * \brief: Created by hushouguo at 22:08:59 Aug 24 2018
 */
 
#include "global.h"
#include "protocol/recordserver.h"
	 
INITIALIZE_INSTANCE(RecordService);

RecordService::RecordService() 
	: NetworkService(
			nullptr,
			[](NetworkService*, NetworkInterface* networkInterface) {
			},
			[this](NetworkService* networkService, NetworkInterface* networkInterface, Netmessage* netmsg) -> bool {
				bool rc = this->msgParser(networkInterface, netmsg);
				if (!rc) {
					networkService->close(networkInterface);
					return true; // release this message right now
				}
				return false;// Delayed release message, Manual release this Netmessage
			})
{
}

bool RecordService::init() {
	return this->start(
			sConfig.get("service.address", "0.0.0.0"), 
			sConfig.get("service.port", 9990)
			);
}

bool RecordService::msgParser(NetworkInterface* task, Netmessage* netmsg) {
	switch (netmsg->id) {
		case ObjectSerializeRequest::id:
			if (true) {
				ObjectSerializeRequest* request = (ObjectSerializeRequest*) netmsg;
				CHECK_RETURN(netmsg->len == request->size(), false, "illegal SerializeRequest message:%u, %lu", netmsg->len, request->size());
				return sRecordProcessManager.request(request->shard, request->tableid, request->objectid, task->fd(), netmsg);
			}
			break;

		case ObjectUnserializeRequest::id:
			if (true) {
				ObjectUnserializeRequest* request = (ObjectUnserializeRequest*) netmsg;
				CHECK_RETURN(netmsg->len == request->size(), false, "illegal UnserializeRequest message:%u, %lu", netmsg->len, request->size());
				return sRecordProcessManager.request(request->shard, request->tableid, request->objectid, task->fd(), netmsg);
			}
			break;

		default: CHECK_RETURN(false, false, "unhandled Netmessage: %d", netmsg->id);
	}
	return true;
}

