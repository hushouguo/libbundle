/*
 * \file: RecordProcess.cpp
 * \brief: Created by hushouguo at 03:15:51 Sep 07 2018
 */

#include "global.h"
#include "protocol/recordserver.h"

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
	{ Entity::type_string, "TEXT" }	// VARCHAR: max: 21845 utf-8, auto-expand
};

static std::map<enum_field_types, std::function<void(Entity::Value&, std::string, bool)>> m2v = {
	{ MYSQL_TYPE_TINY, [](Entity::Value& value, std::string s, bool is_unsigned) {	// bool
		value = std::stoi(s) != 0;
	}},
	{ MYSQL_TYPE_SHORT, [](Entity::Value& value, std::string s, bool is_unsigned) { // integer
		value = std::stoi(s);
	}},
	{ MYSQL_TYPE_LONG, [](Entity::Value& value, std::string s, bool is_unsigned) { 	// integer
		value = is_unsigned ? std::stoul(s) : std::stol(s);
	}},
	{ MYSQL_TYPE_LONGLONG, [](Entity::Value& value, std::string s, bool is_unsigned) { // integer
		value = is_unsigned ? (u64)std::stoull(s) : (s64)std::stoll(s);
	}},
	{ MYSQL_TYPE_FLOAT, [](Entity::Value& value, std::string s, bool is_unsigned) { // float
		value = std::stof(s);
	}},
	{ MYSQL_TYPE_DOUBLE, [](Entity::Value& value, std::string s, bool is_unsigned) { // float
		value = std::stod(s);
	}},		
	{ MYSQL_TYPE_VAR_STRING, [](Entity::Value& value, std::string s, bool is_unsigned) { // string
		value = s;
	}},
	{ MYSQL_TYPE_STRING, [](Entity::Value& value, std::string s, bool is_unsigned) { // string
		value = s;
	}},
	{ MYSQL_TYPE_VARCHAR, [](Entity::Value& value, std::string s, bool is_unsigned) { // string
		value = s;
	}},
	{ MYSQL_TYPE_TINY_BLOB, [](Entity::Value& value, std::string s, bool is_unsigned) { // string
		value = s;
	}},
	{ MYSQL_TYPE_BLOB, [](Entity::Value& value, std::string s, bool is_unsigned) { // string
		value = s;
	}},
	{ MYSQL_TYPE_MEDIUM_BLOB, [](Entity::Value& value, std::string s, bool is_unsigned) { // string
		value = s;
	}},
	{ MYSQL_TYPE_LONG_BLOB, [](Entity::Value& value, std::string s, bool is_unsigned) { // string
		value = s;
	}},
	{ MYSQL_TYPE_DATETIME, [](Entity::Value& value, std::string s, bool is_unsigned) { // string
		value = s;
	}},
};

RecordProcess::RecordProcess(u32 shard, const char* name)
	: Entry<u32, std::string>(shard, name) {
}

bool RecordProcess::SlotDatabase::init(std::string conf) {
	this->dbhandler = new MySQL();
	bool rc = this->dbhandler->openDatabase(conf);
	CHECK_RETURN(rc, false, "conf: %s error", conf.c_str());

	char database[64];
	snprintf(database, sizeof(database), "shard_%u_%u", this->shard, this->id);
	
	//load databases
	std::set<std::string> results;
	rc = this->dbhandler->loadDatabase(database, results);
	CHECK_RETURN(rc, false, "load database error");
	
	//create database
	if (results.find(database) == results.end()) {
		rc = this->dbhandler->createDatabase(database);
		CHECK_RETURN(rc, false, "create database: %s error", database);
		System << "create database: " << database << " OK";
	}
	
	//select database
	rc = this->dbhandler->selectDatabase(database);
	CHECK_RETURN(rc, false, "select database: %s error", database);

	//load table
	results.clear();
	rc = this->dbhandler->loadTable("%", results);
	CHECK_RETURN(rc, false, "load table from database: %s error", database);
	
	//load fields decription
	for (auto& table : results) {
		rc = this->loadField(table);
		CHECK_RETURN(rc, false, "load field: %s error", table.c_str());
	}

	SafeDelete(this->threadWorker);
	this->threadWorker = new std::thread([this]() {
		this->update();
	});
	
	return true;
}

bool RecordProcess::SlotDatabase::loadField(std::string table) {
	std::unordered_map<std::string, Entity::value_type>& desc_fields = this->tables[table];

	char sql[64];
	snprintf(sql, sizeof(sql), "SELECT * FROM `%s` LIMIT 1", table.c_str());
	
	MySQLResult* result = this->dbhandler->runQuery(sql);
	assert(result);
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

	auto dump = [&desc_fields](u32 shard, const std::string& table) {
		Trace << "shard: " << shard << ", table: " << table;
		for (auto& i : desc_fields) {
			Trace << "FIELD: " << i.first << " => " << Entity::ValueTypeName(i.second);
		}
	};
	dump(this->shard, table);
	
	return true;
}

bool RecordProcess::init(const std::vector<std::string>& v) {
	assert(v.size() == 8);
	for (auto& conf : v) {
		SlotDatabase* slotDatabase = new SlotDatabase(this->id, this->_slots.size());
		CHECK_RETURN(slotDatabase->init(conf), false, "init conf: %s error", conf.c_str());
		this->_slots.push_back(slotDatabase);
	}
	assert(this->_slots.size() == 8);
	return true;
}


bool RecordProcess::request(u32 shard, u64 objectid, SOCKET s, const Netmessage* netmsg) {
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
		SafeFree(response->netmsg);
		SafeDelete(response);
	}
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
	auto sendSerializeResponse = [this](SOCKET s, u32 shard, const char* table, u64 objectid, u32 retval) {
		ObjectSerializeResponse* newmsg = Constructor((ObjectSerializeResponse *)(::malloc(sizeof(ObjectSerializeResponse))));
		Recordmessage* response = new Recordmessage(s, newmsg);
		newmsg->shard = shard;
		strncpy(newmsg->table, table, sizeof(newmsg->table));
		newmsg->objectid = objectid;
		newmsg->retval = retval;
		newmsg->len = newmsg->size();
		
		std::lock_guard<std::mutex> guard(this->rlocker);
		this->rlist.push_back(response);
	};

	auto sendUnserializeResponse = [this](SOCKET s, u32 shard, const char* table, u64 objectid, u32 retval, u32 datalen, const char* data) {
		ObjectUnserializeResponse* newmsg = Constructor((ObjectUnserializeResponse *)(::malloc(sizeof(ObjectUnserializeResponse) + datalen)));
		Recordmessage* response = new Recordmessage(s, newmsg);
		newmsg->shard = shard;
		strncpy(newmsg->table, table, sizeof(newmsg->table));
		newmsg->objectid = objectid;
		newmsg->retval = retval;
		newmsg->datalen = datalen;
		if (data) {
			assert(datalen > 0);
			memcpy(newmsg->data, data, datalen);
		}
		newmsg->len = newmsg->size();
		
		std::lock_guard<std::mutex> guard(this->rlocker);
		this->rlist.push_back(response);
	};
	
	while (!this->wlist.empty() && this->wlocker.try_lock()) {
		Recordmessage* request = this->wlist.front();
		this->wlist.pop_front();
		this->wlocker.unlock();

		switch (request->netmsg->id) {
			case ObjectSerializeRequest::id:
				if (true) {
					ObjectSerializeRequest* msg = (ObjectSerializeRequest*) request->netmsg;					
					assert(msg->shard == this->shard);

					Entity* entity = FindOrNull(this->entities, msg->objectid);
					if (!entity) {
						entity = new Entity(msg->objectid);
					}
					bool rc = entity->ParseFromString(msg->data, msg->datalen);
					if (!rc) {
						sendSerializeResponse(request->s, msg->shard, msg->table, msg->objectid, RECORD_ILLEGAL_JSON_STRING);
						break;
					}
					
					rc = this->serialize(msg->table, entity);
					if (!rc) {
						SafeDelete(entity);
						this->removeEntity(msg->objectid);
						sendSerializeResponse(request->s, msg->shard, msg->table, msg->objectid, RECORD_SERIALIZE_ERROR);
						break;
					}

					sendSerializeResponse(request->s, msg->shard, msg->table, msg->objectid, RECORD_OK);
				}
				break;

			case ObjectUnserializeRequest::id:
				if (true) {
					ObjectUnserializeRequest* msg = (ObjectUnserializeRequest*) request->netmsg;
					assert(msg->shard == this->shard);

					Entity* entity = FindOrNull(this->entities, msg->objectid);
					if (!entity) {
						entity = this->unserialize(msg->table, msg->objectid);						
					}
					
					if (!entity) {
						sendUnserializeResponse(request->s, msg->shard, msg->table, msg->objectid, RECORD_NOT_FOUND_OBJECT, 0, nullptr);
						break;
					}

					std::ostringstream o;
					bool rc = entity->SerializeToString(o, true);
					if (!rc) {
						SafeDelete(entity);
						sendUnserializeResponse(request->s, msg->shard, msg->table, msg->objectid, RECORD_ILLEGAL_OBJECT, 0, nullptr);
						break;
					}
					this->entities.insert(std::make_pair(msg->objectid, entity));

					const std::string& js = o.str();
					sendUnserializeResponse(request->s, msg->shard, msg->table, msg->objectid, RECORD_OK, js.length(), js.data());
				}
				break;
				
			default: Error << "unhandle msg: " << request->netmsg->id; break;
		}

		sRecordService.releaseMessage(request->netmsg);
		SafeDelete(request);
	}

	return 0;
}

bool RecordProcess::SlotDatabase::serialize(const char* table, const Entity* entity) {
	std::unordered_map<std::string, Entity::value_type>& desc_fields = this->tables[table];

	if (!ContainsKey(this->tables, table)) {
		bool rc = this->createTable(table, entity);
		CHECK_RETURN(rc, false, "create table: %s error", table);
		rc = this->loadField(table);
		CHECK_RETURN(rc, false, "load field from table: %s error", table);
	}

	const std::unordered_map<std::string, Entity::Value>& values = entity->values();

	std::ostringstream sql_fields, sql_insert, sql_update;
	for (auto& iterator : values) {
		if (!ContainsKey(desc_fields, iterator.first)) {
			bool rc = this->addField(table, iterator.first, iterator.second.type);
			CHECK_RETURN(rc, false, "add new field: %s, type: %d error", iterator.first.c_str(), iterator.second.type);
			desc_fields[iterator.first] = iterator.second.type;
			Trace << "table: " << table << ", add new field: " << iterator.first << ", type: " << Entity::ValueTypeName(iterator.second.type);
		}
		
		if (iterator.second.type != desc_fields[iterator.first].type) {
			bool rc = this->alterField(table, iterator.first, iterator.second.type);
			CHECK_RETURN(rc, false, "modify field: %s to new type: %d error", iterator.first.c_str(), iterator.second.type);
			Trace << "table: " << table << ", modify field: " << iterator.first << ", from type: " << Entity::ValueTypeName(desc_fields[iterator.first]) << " to new type: " << Entity::ValueTypeName(iterator.second.type);
			desc_fields[iterator.first] = iterator.second.type;
		}
	
		if (sql_fields.tellp() > 0) { sql_fields << ","; }
		if (sql_insert.tellp() > 0) { sql_insert << ","; }
		if (sql_update.tellp() > 0) { sql_update << ","; }

		sql_fields << "`" << iterator.first << "`";

		switch (iterator.second.type) {
			case Entity::type_integer: 
				sql_insert << iterator.second.value_integer;
				sql_update << "`" << iterator.first << "`=" << iterator.second.value_integer;
				break;

			case Entity::type_float:
				sql_insert << iterator.second.value_float;
				sql_update << "`" << iterator.first << "`=" << iterator.second.value_float;
				break;

			case Entity::type_bool:
				sql_insert << iterator.second.value_bool;
				sql_update << "`" << iterator.first << "`=" << iterator.second.value_bool;
				break;

			case Entity::type_string:
				sql_insert << "'" << iterator.second.value_string << "'";
				sql_update << "`" << iterator.first << "`=" << "'" << iterator.second.value_string << "'";
				break;

			default: CHECK_RETURN(false, false, "illegal value type: %d", iterator.second.type); break;
		}
	}
	
	std::string sql = "INSERT INTO `";
	sql += table;
	sql += "` (";
	sql += sql_fields.str() + ") VALUES (";
	sql += sql_insert.str() + ") ON DUPLICATE KEY UPDATE ";
	sql += sql_update.str();

	Trace << "serialize sql: " << sql;

	return this->dbhandler->runCommand(sql);
}

Entity* RecordProcess::SlotDatabase::unserialize(const char* table, u64 objectid) {
	std::unordered_map<std::string, Entity::value_type>& desc_fields = this->tables[table];

	char sql[64];
	snprintf(sql, sizeof(sql), "SELECT * FROM `%s` WHERE id = %ld", table, objectid);

	MySQLResult* result = this->dbhandler->runQuery(sql);
	if (!result) {
		return nullptr;
	}

	u32 rowNumber = result->rowNumber();
	if (rowNumber == 0) {
		SafeDelete(result);
		return nullptr;
	}
	
	u32 fieldNumber = result->fieldNumber();
	if (fieldNumber != desc_fields.size()) {
		Alarm.cout("fieldNumber: %u, fields: %ld", fieldNumber, desc_fields.size());
	}
	MYSQL_FIELD* fields = result->fetchField();	
	MYSQL_ROW row = result->fetchRow();
	assert(row);

	Entity* entity = new Entity(objectid);
	
	for (u32 i = 0; i < fieldNumber; ++i) {
		const MYSQL_FIELD& field = fields[i];
		CHECK_CONTINUE(desc_fields.find(field.org_name) != desc_fields.end(), "org_name: %s not exist, table: %s", field.org_name, table);
		CHECK_CONTINUE(ContainsKey(m2v, field.type), "unhandle field.type: %d", field.type);
		bool is_unsigned = (field.flags & UNSIGNED_FLAG) != 0;
		try {
			m2v[field.type](entity->GetValue(field.org_name), row[i], is_unsigned);
		} catch(std::exception& e) {
			Error << "field: " << field.org_name << ", type: " << field.type << " convert error: " << e.what();
		}
	}
	SafeDelete(result);

	return entity;
}

bool RecordProcess::SlotDatabase::addField(const char* table, const std::string& field_name, Entity::value_type type) {
	std::ostringstream sql;
	sql << "ALTER TABLE `" << table << "` ADD `" << field_name << "` " << v2m[type];
	Trace << "alter table: " << sql.str();
	return this->dbhandler->runCommand(sql.str());
}

bool RecordProcess::SlotDatabase::alterField(const char* table, const std::string& field_name, Entity::value_type type) {
	std::ostringstream sql;
	sql << "ALTER TABLE `" << table << "` MODIFY `" << field_name << "` " << v2m[type] << " NOT NULL ";
	Trace << "alter table: " << sql.str();
	return this->dbhandler->runCommand(sql.str());
}

bool RecordProcess::SlotDatabase::createTable(const char* table, const Entity* entity) {
	const std::unordered_map<std::string, Entity::Value>& values = entity->values();
	std::string sql = "CREATE TABLE `"; // IF NOT EXISTS
	sql += table;
	sql += "` (`id` BIGINT UNSIGNED NOT NULL PRIMARY KEY ";	// entity->id
	for (auto& iterator : values) {
		if (iterator.first == "id") { continue; }
		CHECK_CONTINUE(v2m.find(iterator.second.type) != v2m.end(), "illegal ValueType: %d", iterator.second.type);
		sql += ", `";
		sql += iterator.first + "` ";
		sql += v2m[iterator.second.type] + " NOT NULL ";	//TODO: KEY setting		
	}
	sql += ") ENGINE=InnoDB DEFAULT CHARSET=utf8 ";

	Trace << "createTable: " << sql;

	return this->dbhandler->runCommand(sql);
}

