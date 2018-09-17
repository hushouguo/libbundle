/*
 * \file: MySQL.h
 * \brief: Created by hushouguo at 16:25:15 Jan 21 2018
 */
 
#ifndef __MYSQL_H__
#define __MYSQL_H__

BEGIN_NAMESPACE_BUNDLE {
	struct MySQLConfig {
		std::string host;
		std::string user;
		std::string passwd;
		std::string database;
		int port = -1;
		bool parsefrom(std::string conf);	// host:port:user:password:db
	};

	class MySQLResult;	
	class MySQL {
		public:
			MySQL();
			~MySQL();

		public:
			bool openDatabase(std::string conf);
			bool openDatabase(const char *host, const char *user, const char *passwd, const char *db, int port = 3306);
			void closeDatabase();

		public:
			bool runCommand(std::string s);
			MySQLResult* runQuery(std::string s);

		public:
			bool resetDatabase();
			bool selectDatabase(std::string database);
			bool createDatabase(std::string database);
			bool loadDatabase(std::string where, std::set<std::string>& results);
			bool loadTable(std::string where, std::set<std::string>& results);

		public:
			u64 insertId();

		private:
			friend class MySQLStatement;
			MYSQL _mysqlhandle;
			MySQLConfig _mysqlconf;
			bool openDatabase();
	};
}

#endif
