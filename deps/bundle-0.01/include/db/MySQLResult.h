/*
 * \file: MySQLResult.h
 * \brief: Created by hushouguo at 22:42:52 Aug 24 2018
 */
 
#ifndef __MYSQLRESULT_H__
#define __MYSQLRESULT_H__

BEGIN_NAMESPACE_BUNDLE {
	class MySQLResult {
		public:
			MySQLResult(MYSQL_RES* res);
			~MySQLResult();

		public:
			void free();
			MYSQL_ROW fetchRow();
			u64 rowNumber();
			u32 fieldNumber();
			MYSQL_FIELD* fetchField();

		private:
			MYSQL_RES* _res = nullptr;
	};
}

#endif
