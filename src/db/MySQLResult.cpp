/*
 * \file: MySQLResult.cpp
 * \brief: Created by hushouguo at 22:43:40 Aug 24 2018
 */

#include "bundle.h"

BEGIN_NAMESPACE_BUNDLE {
	MySQLResult::MySQLResult(MYSQL_RES* res)
		: _res(res)
	{
	}

	MySQLResult::~MySQLResult() {
		this->free();
	}

	void MySQLResult::free() {
		mysql_free_result(this->_res);
	}

	MYSQL_ROW MySQLResult::fetchRow() {
		return mysql_fetch_row(this->_res);
	}

	u64 MySQLResult::rowNumber() {
		return mysql_num_rows(this->_res);
	}

	u32 MySQLResult::fieldNumber() {
		return mysql_num_fields(this->_res);
	}
	
	MYSQL_FIELD* MySQLResult::fetchField() {
		return mysql_fetch_fields(this->_res);
	}
}

