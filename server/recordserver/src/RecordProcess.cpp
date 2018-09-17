/*
 * \file: RecordProcess.cpp
 * \brief: Created by hushouguo at 03:15:51 Sep 07 2018
 */

#include "global.h"
#include "protocol/recordserver.h"

#define DATABASE_PREFIX			"shard."
#define DATABASE_POSTFIX		""
#define TABLE_PREFIX			"table."
#define TABLE_POSTFIX			""

#define DATABASE_NAME(shard)	\
	({ 	char database[64];\
		snprintf(database, sizeof(database), "%s%u%s", DATABASE_PREFIX, shard, DATABASE_POSTFIX);\
		database;})

#define TABLE_NAME(tableid, hashid)	\
	({	char tablename[64];\
		snprintf(tablename, sizeof(tablename), "%s%u.%u%s", TABLE_PREFIX, tableid, hashid, TABLE_POSTFIX);\
		tablename;})


// unordered_map MUST gcc version is above 7
//static std::unordered_map<enum_field_types, Entity::value_type> m2v_signed = {
static std::map<enum_field_types, Entity::value_type> m2v_signed = {
	{ MYSQL_TYPE_TINY, Entity::type_bool },
	{ MYSQL_TYPE_SHORT, Entity::type_integer },
	{ MYSQL_TYPE_LONG, Entity::type_integer },
	{ MYSQL_TYPE_LONGLONG, Entity::type_integer },
	{ MYSQL_TYPE_FLOAT, Entity::type_float },
	{ MYSQL_TYPE_DOUBLE, Entity::type_float },
	{ MYSQL_TYPE_VAR_STRING, Entity::type_string },
	{ MYSQL_TYPE_STRING, Entity::type_string },
	{ MYSQL_TYPE_VARCHAR, Entity::type_string },
	{ MYSQL_TYPE_TINY_BLOB, Entity::type_string },
	{ MYSQL_TYPE_BLOB, Entity::type_string },
	{ MYSQL_TYPE_MEDIUM_BLOB, Entity::type_string },
	{ MYSQL_TYPE_LONG_BLOB, Entity::type_string },
	{ MYSQL_TYPE_DATETIME, Entity::type_string },
};

//static std::unordered_map<enum_field_types, Entity::value_type> m2v_unsigned = {
static std::map<enum_field_types, Entity::value_type> m2v_unsigned = {
	{ MYSQL_TYPE_TINY, Entity::type_bool },
	{ MYSQL_TYPE_SHORT, Entity::type_integer },
	{ MYSQL_TYPE_LONG, Entity::type_integer },
	{ MYSQL_TYPE_LONGLONG, Entity::type_integer },
	{ MYSQL_TYPE_FLOAT, Entity::type_float },
	{ MYSQL_TYPE_DOUBLE, Entity::type_float },		
	{ MYSQL_TYPE_VAR_STRING, Entity::type_string },
	{ MYSQL_TYPE_STRING, Entity::type_string },
	{ MYSQL_TYPE_VARCHAR, Entity::type_string },
	{ MYSQL_TYPE_TINY_BLOB, Entity::type_string },
	{ MYSQL_TYPE_BLOB, Entity::type_string },
	{ MYSQL_TYPE_MEDIUM_BLOB, Entity::type_string },
	{ MYSQL_TYPE_LONG_BLOB, Entity::type_string },
	{ MYSQL_TYPE_DATETIME, Entity::type_string },		
};

//static std::unordered_map<Entity::value_type, std::string> v2m = {
static std::map<Entity::value_type, std::string> v2m = {
	{ Entity::type_integer, "BIGINT SIGNED" },
	{ Entity::type_bool, "TINYINT UNSIGNED" },
	{ Entity::type_float, "FLOAT" },
	{ Entity::type_string, "VARCHAR" }	// max: 21845 utf-8, auto-expand
};


RecordProcess::RecordProcess(u32 shard, const char* name)
	: Entry<u32, std::string>(shard, name) {
}

bool RecordProcess::SlotDatabase::init(std::string conf) {
	this->dbhandler = new MySQL();
	bool rc = this->dbhandler->openDatabase(conf);
	CHECK_RETURN(rc, false, "conf: %s error", conf.c_str());
	
	//load databases
	std::set<std::string> results;
	rc = this->dbhandler->loadDatabase(DATABASE_PREFIX, DATABASE_POSTFIX, results);
	CHECK_RETURN(rc, false, "load database error");
	
	//create database
	std::string database = DATABASE_NAME(this->id);
	if (results.find(database) == results.end()) {
		rc = this->dbhandler->createDatabase(database);
		CHECK_RETURN(rc, false, "create database: %s error", database.c_str());
		System << "create database: " << database << " OK";
	}
	
	//select database
	rc = this->dbhandler->selectDatabase(database);
	CHECK_RETURN(rc, false, "select database: %s error", database.c_str());
	
#if 0
	//load table
	results.clear();
	rc = this->dbhandler->loadTable(TABLE_PREFIX, TABLE_POSTFIX, results);
	CHECK_RETURN(rc, false, "load table: %s error", database.c_str());
	
	//load fields decription
	for (auto& table : results) {
		rc = this->loadField(this->id, table);
		CHECK_RETURN(rc, false, "load field: %s error", table.c_str());
	}
#endif

	//TODO: create thread
	
	return true;
}

#if 0
bool RecordProcess::SlotDatabase::loadField(u32 shard, std::string table) {
	std::unordered_map<std::string, Entity::value_type>& desc_fields = this->_tables[table];
	
	std::string s = "SELECT * FROM `";
	s += table + "` LIMIT 1";
	MySQLResult* result = this->_dbhandler->runQuery(s);
	u32 fieldNumber = result->fieldNumber();
	MYSQL_FIELD* fields = result->fetchField();
	for (u32 i = 0; i < fieldNumber; ++i) {
		const MYSQL_FIELD& field = fields[i];
		CHECK_CONTINUE(desc_fields.find(field.org_name) == desc_fields.end(), "duplicate org_name: %s, table: %s", field.org_name, table.c_str());
		bool is_unsigned = (field.flags & UNSIGNED_FLAG) != 0;			
		if (is_unsigned) {
			CHECK_CONTINUE(m2v_unsigned.find(field.type) != m2v_unsigned.end(), "illegal table:%s, field: %s, type: %d", table.c_str(), field.org_name, field.type);
			desc_fields[field.org_name] = m2v_unsigned[field.type];
		}
		else {
			CHECK_CONTINUE(m2v_signed.find(field.type) != m2v_signed.end(), "illegal table:%s, field: %s, type: %d", table.c_str(), field.org_name, field.type);
			desc_fields[field.org_name] = m2v_signed[field.type];
		}
	}
	SafeDelete(result);

	auto dump = [shard, table, &desc_fields]() {
		Trace << "shard: " << shard << ", table: " << table;
		for (auto& i : desc_fields) {
			Trace << "FIELD: " << i.first << " => " << Entity::ValueTypeName(i.second);
		}
	};
	dump();
	
	return true;
}

#endif

bool RecordProcess::init(const std::vector<std::string>& v) {
	assert(v.size() == 8);
	for (auto& conf : v) {
		SlotDatabase* slotDatabase = new SlotDatabase(this->_slots.size());
		CHECK_RETURN(slotDatabase->init(conf), false, "init conf: %s error", conf.c_str());
		this->_slots.push_back(slotDatabase);
	}
	assert(this->_slots.size() == 8);
	return true;
}


bool RecordProcess::request(u32 shard, u32 tableid, u64 objectid, SOCKET s, Netmessage* netmsg) {
	assert(shard == this->id);
	u32 hashid = objectid % 8;
	assert(hashid < this->_slots.size());
	SlotDatabase* slotDatabase = this->_slots[hashid];
	
	std::lock_guard<std::mutex> guard(slotDatabase->wlocker);
	slotDatabase->wlist.push_back(new Recordmessage(s, netmsg));
	return true;
}

void RecordProcess::update() {
	for (auto& slotDatabase : this->_slots) {
		this->update(slotDatabase);
	}
}

void RecordProcess::update(SlotDatabase* slotDatabase) {
	while (!slotDatabase->rlist.empty() && slotDatabase->rlocker.try_lock()) {
		Recordmessage* response = slotDatabase->rlist.front();
		slotDatabase->rlist.pop_front();
		slotDatabase->rlocker.unlock();
		NetworkInterface* networkInterface = sRecordService.getNetworkInterface(response->s);
		if (networkInterface) {
			networkInterface->sendMessage(response->netmsg);
		}
		SafeDelete(response);
	}
}

void RecordProcess::SlotDatabase::response(SOCKET s, Netmessage* netmsg) {
	std::lock_guard<std::mutex> guard(this->rlocker);
	this->rlist.push_back(new Recordmessage(s, netmsg));
}

void RecordProcess::SlotDatabase::stop() {
	if (!this->isstop) {
		this->isstop = true;
		if (this->threadWorker && this->threadWorker->joinable()) {
			this->threadWorker->join();
		}
		SafeDelete(this->threadWorker);
		SafeDelete(this->dbhandler);
		//TODO: remove Recordmessage
		//TODO: remove entities
	}
}

void RecordProcess::stop() {
	for (auto& slotDatabase : this->_slots) {
		slotDatabase->stop();
		SafeDelete(slotDatabase);
	}	
}

void RecordProcess::SlotDatabase::update() {
	u32 milliseconds = sConfig.get("db.sync.interval", 100);
	while (!this->isstop) {
		if (this->synchronous() == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
		}
	}
	System.cout("recordProcess: %d thread exit", this->id);
}

u32 RecordProcess::SlotDatabase::synchronous() {
	while (!this->wlist.empty() && this->wlocker.try_lock()) {
		Recordmessage* request = this->wlist.front();
		this->wlist.pop_front();
		this->wlocker.unlock();

		switch (request->netmsg->id) {
			case ObjectSerializeRequest::id:
				if (true) {
					//ObjectSerializeRequest* msg = (ObjectSerializeRequest*) request->netmsg;
					//assert(msg->shard == this->id);
#if 0					
					TABLE_NAME(msg->tableid);
					if (!ContainsKey(this->_tables, tablename)) {
					}
					
					Entity* entity = FindOrNull(this->_entities, msg->objectid);
					if (entity) {
					}
					else {
					}
#endif
					//NEW_MSG(ObjectSerializeResponse, sizeof(ObjectSerializeResponse));
					//newmsg->len = sizeof(ObjectSerializeResponse);
					//newmsg->shard = msg->shard;
					//newmsg->tableid = msg->tableid;
					//newmsg->objectid = msg->objectid;
					//this->response(request->s, newmsg, request->opcode);
				}
				break;

			case ObjectUnserializeRequest::id:
				if (true) {
					//ObjectUnserializeRequest* msg = (ObjectUnserializeRequest*) request->netmsg;
					//assert(msg->shard == this->id);
#if 0
					Entity* entity = FindOrNull(this->_entities, msg->objectid);
					if (entity) {
					}
					else {
						TABLE_NAME(msg->tableid);
						if (!ContainsKey(this->_tables, tablename)) {
						}
					}
					
					ObjectSerializeResponse response;
					response.shard = request->shard;
					response.tableid = request->tableid;
					response.objectid = request->objectid;
					this->response(request->s, &response, request->opcode);
#endif					
				}
				break;
				
			default: Error << "unhandle msg: " << request->netmsg->id;
		}

		SafeDelete(request);
	}

	return 0;
}

#if 0
bool RecordProcess::createTable(u32 shard, std::string table, const Entity* entity) {
	ShardDatabase* s = FindOrNull(this->_databases, shard);
	CHECK_RETURN(s, false, "not found shard: %u", shard);

	std::unordered_map<std::string, record::Value::ValueType>& fields = s->tables[table];
	fields.clear();
	
	const google::protobuf::Map<std::string, record::Value>& values = entity->values();
	for (auto& i : values) {
		fields[i.first] = i.second.type;
	}

	TABLE_NAME(table.c_str());

	std::string sql = "CREATE TABLE `"; // IF NOT EXISTS
	sql += tablename;
	sql += "` (";
	sql += "`id` BIGINT UNSIGNED NOT NULL PRIMARY KEY ";
	for (auto& i : fields) {
		CHECK_CONTINUE(v2m.find(i.second) != v2m.end(), "illegal ValueType: %d", i.second);
		sql += ", `";
		sql += i.first + "` ";
		sql += v2m[i.second] + " NOT NULL ";	//TODO: KEY setting
	}
	sql += ") ENGINE=InnoDB DEFAULT CHARSET=utf8 ";

	Trace << "createTable: " << sql;

	return s->handler->runCommand(sql);
}
#endif

