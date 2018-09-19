/*
 * \file: MySQL.cpp
 * \brief: Created by hushouguo at 16:35:53 Jan 21 2018
 */

#include "bundle.h"

#define MYSQL_CONNECT_TIMEOUT		10
#define MYSQL_WAIT_TIMEOUT			604800
#define MYSQL_INTERACTIVE_TIMEOUT	604800

BEGIN_NAMESPACE_BUNDLE {
	MySQL::MySQL() {
		mysql_init(&this->_mysqlhandle);
	}

	MySQL::~MySQL() {
		mysql_close(&this->_mysqlhandle);
	}

	bool MySQL::openDatabase(const char *host, const char *user, const char *passwd, const char *db, int port) {
		this->_mysqlconf.host = host;
		this->_mysqlconf.port = port;
		this->_mysqlconf.user = user;
		this->_mysqlconf.passwd = passwd;
		this->_mysqlconf.database = db;
		return this->openDatabase();
	}

	void MySQL::closeDatabase() {
		mysql_close(&this->_mysqlhandle);
		mysql_init(&this->_mysqlhandle);
	}

	bool MySQL::runCommand(std::string s) {
		int rc = mysql_real_query(&this->_mysqlhandle, s.c_str(), s.length());
		CHECK_RETURN(rc == 0, false, "runCommand: %s", mysql_error(&this->_mysqlhandle));
		return true;
	}

	MySQLResult* MySQL::runQuery(std::string s) {
		int rc = mysql_real_query(&this->_mysqlhandle, s.c_str(), s.length());
		CHECK_RETURN(rc == 0, nullptr, "runQueue: %s", mysql_error(&this->_mysqlhandle));
		MYSQL_RES* res = mysql_store_result(&this->_mysqlhandle);
		CHECK_RETURN(res, nullptr, "store result: %s", mysql_error(&this->_mysqlhandle));
		return new MySQLResult(res);
	}

	bool MySQL::resetDatabase() {
		this->closeDatabase();
		bool rc = openDatabase();
		CHECK_RETURN(rc, false, "resetDatabase rc failure");
		return rc;
	}

	bool MySQL::selectDatabase(std::string database) {
		int rc = mysql_select_db(&this->_mysqlhandle, database.c_str());
		CHECK_RETURN(rc == 0, false, "selectDatabase: %s", mysql_error(&this->_mysqlhandle));
		this->_mysqlconf.database = database;
		return true;
	}

	bool MySQL::createDatabase(std::string database) {
		std::ostringstream sql;
		sql << "CREATE DATABASE IF NOT EXISTS `" << database << "` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci";
		return this->runCommand(sql.str());
	}

	bool MySQL::loadDatabase(std::string where, std::set<std::string>& results) {
		MySQLStatement stmt(this);
		std::string s = "SHOW DATABASES LIKE '" + where + "'";
				
		if (!stmt.prepare(s)) { return false; }
		if (!stmt.exec()) { return false; }
		
		char database[64];
		unsigned long lengths[1] = {0};
		
		MYSQL_BIND result[1];
		memset(result, 0, sizeof(result));
		
		result[0].buffer_type = MYSQL_TYPE_VAR_STRING;
		result[0].buffer = (void*) database;
		result[0].buffer_length = sizeof(database);
		result[0].length = &lengths[0];
		
		if (!stmt.bindResult(result)) { return false; }
		while (stmt.fetch()) {
			results.insert(database);
		}
		
		return stmt.freeResult();
	}

	bool MySQL::loadTable(std::string where, std::set<std::string>& results) {
		MySQLStatement stmt(this);
		std::string s = "SHOW TABLES LIKE '" + where + "'";
				
		if (!stmt.prepare(s)) { return false; }
		if (!stmt.exec()) { return false; }
		
		char table[64];
		unsigned long lengths[1] = {0};
		
		MYSQL_BIND result[1];
		memset(result, 0, sizeof(result));
		
		result[0].buffer_type = MYSQL_TYPE_VAR_STRING;
		result[0].buffer = (void*) table;
		result[0].buffer_length = sizeof(table);
		result[0].length = &lengths[0];
		
		if (!stmt.bindResult(result)) { return false; }
		while (stmt.fetch()) {
			results.insert(table);
		}
		
		return stmt.freeResult();
	}

	u64 MySQL::insertId() {
		return mysql_insert_id(&this->_mysqlhandle);
	}

	bool MySQL::openDatabase(std::string conf) {
		if (!this->_mysqlconf.parsefrom(conf)) {
			return false;
		}
		return this->openDatabase();
	}

	bool MySQLConfig::parsefrom(std::string conf) {
		std::vector<std::string> v;
		bool rc = splitString(conf.c_str(), ':', v);
		CHECK_RETURN(rc, false, "illegal conf: %s", conf.c_str());
		CHECK_RETURN(v.size() == 5, false, "illegal conf: %s", conf.c_str());
		this->host = v[0];
		//int value = strtol(v[1].c_str(), (char**)NULL, 10);
		int value_port = -1;
		try {
			value_port = std::stol(v[1].c_str());
		}
		catch(std::exception& e) {
			CHECK_RETURN(false, false, "illegal port: %s, errno: %s", v[1].c_str(), e.what());
		}
		//CHECK_RETURN(errno == 0, false, "illegal port: %s, errno: %d, %s", v[1].c_str(), errno, strerror(errno));
		this->port = value_port;
		this->user = v[2];
		this->passwd = v[3];
		this->database = v[4];
		return true;
	}

	bool MySQL::openDatabase() {
		if (true) { // connect timeout
			u32 seconds = MYSQL_CONNECT_TIMEOUT;
			if (mysql_options(&this->_mysqlhandle, MYSQL_OPT_CONNECT_TIMEOUT, (const char *) &seconds)) {
				CHECK_GOTO(false, except_exit, "error: %s", mysql_error(&this->_mysqlhandle));
			}
		}

		if (true) { // enable reconnect
			my_bool reconnect = true;
			if (mysql_options(&this->_mysqlhandle, MYSQL_OPT_RECONNECT, &reconnect)) {
				CHECK_GOTO(false, except_exit, "error: %s", mysql_error(&this->_mysqlhandle));
			}
		}

		if (true) { // wait timeout, default 8 hours
			u32 seconds = MYSQL_WAIT_TIMEOUT;
			char sql[1024];
			snprintf(sql, sizeof(sql), "set wait_timeout=%d", seconds);
			if (mysql_options(&this->_mysqlhandle, MYSQL_INIT_COMMAND, sql)) {
				CHECK_GOTO(false, except_exit, "error: %s", mysql_error(&this->_mysqlhandle));
			}
		}

		if (true) { // interactive timeout, default 8 hours
			u32 seconds = MYSQL_INTERACTIVE_TIMEOUT;
			char sql[1024];
			snprintf(sql, sizeof(sql), "set interactive_timeout=%d", seconds);
			if (mysql_options(&this->_mysqlhandle, MYSQL_INIT_COMMAND, sql)) {
				CHECK_GOTO(false, except_exit, "error: %s", mysql_error(&this->_mysqlhandle));
			}
		}

		if (true) { // report data truncation
			my_bool report = true;			
			if (mysql_options(&this->_mysqlhandle, MYSQL_REPORT_DATA_TRUNCATION, (const char *) &report)) {
				CHECK_GOTO(false, except_exit, "error: %s", mysql_error(&this->_mysqlhandle));
			}
		}

		//TODO: SET NAMES UTF8

		// CLIENT_INTERACTIVE & set interactive_timeout
		if (!mysql_real_connect(
					&this->_mysqlhandle, 
					this->_mysqlconf.host.c_str(), 
					this->_mysqlconf.user.c_str(), 
					this->_mysqlconf.passwd.c_str(), 
					this->_mysqlconf.database.length() > 0 ? this->_mysqlconf.database.c_str() : nullptr, 
					this->_mysqlconf.port, nullptr, 0)) {
			CHECK_GOTO(false, except_exit, "error: %s", mysql_error(&this->_mysqlhandle));
		}

		return true;

except_exit:
		this->closeDatabase();
		return false;
	}
}
