/*
 * \file: RecordProcessManager.cpp
 * \brief: Created by hushouguo at 03:17:06 Sep 07 2018
 */

#include "global.h"
#include "protocol/recordserver.h"
	 
INITIALIZE_INSTANCE(RecordProcessManager);

bool RecordProcessManager::init() {
	XmlParser xmlParser;
	if (!xmlParser.open(sConfig.get("db.conf.filename", "conf/db.xml"))) {
		return false;
	}

	XmlParser::XML_NODE root = xmlParser.getRootNode();
	CHECK_RETURN(root, false, "not found root node");
	
	std::unordered_map<u32, std::vector<std::string>> confs;
	
	XmlParser::XML_NODE node_shard = xmlParser.getChildNode(root, "shard");
	for (; node_shard; node_shard = xmlParser.getNextNode(node_shard, "shard")) {
		u32 shard = xmlParser.getValueByInteger(node_shard, "id", 0);
		CHECK_RETURN(!ContainsKey(this->_recordProcesses, shard), false, "duplicate shard: %u", shard);
		
		std::string name = xmlParser.getValueByString(node_shard, "name", "");
		RecordProcess* recordProcess = new RecordProcess(shard, name.c_str());

		std::vector<std::string>& v = confs[shard];
		
		XmlParser::XML_NODE node_mysql = xmlParser.getChildNode(node_shard, "mysql");
		for (; node_mysql; node_mysql = xmlParser.getNextNode(node_mysql, "mysql")) {
			std::string conf = xmlParser.getValueByString(node_mysql, "conf", "");
			v.push_back(conf);
		}
		
		CHECK_RETURN(v.size() == 8, false, "fixed 8 mysql connection for per shard");

		bool rc = this->_recordProcesses.insert(std::make_pair(shard, recordProcess)).second;
		assert(rc);
	}	
	xmlParser.final();

	for (auto& iterator : this->_recordProcesses) {
		u32 shard = iterator.first;
		const std::vector<std::string>& v = confs[shard];
		assert(v.size() == 8);
		bool rc = iterator.second->init(v);
		CHECK_RETURN(rc, false, "shard: %u conf error", shard);
	}

	return true;
}

bool RecordProcessManager::request(u32 shard, u32 tableid, u64 objectid, SOCKET s, Netmessage* netmsg) {
	RecordProcess* recordProcess = FindOrNull(this->_recordProcesses, shard);
	CHECK_RETURN(recordProcess, false, "not configure shard: %u", shard);
	return recordProcess->request(shard, tableid, objectid, s, netmsg);
}

void RecordProcessManager::update() {
	for (auto& iterator : this->_recordProcesses) {
		iterator.second->update();
	}
}

void RecordProcessManager::stop() {
	for (auto& iterator : this->_recordProcesses) {
		iterator.second->stop();
	}
}

