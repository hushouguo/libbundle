/*
 * \file: MySQLStatement.cpp
 * \brief: Created by hushouguo at 22:46:07 Aug 24 2018
 */

#include "bundle.h"

BEGIN_NAMESPACE_BUNDLE {
	MySQLStatement::MySQLStatement(MySQL* owner)
		: _owner(owner)
	{
		this->_stmt = mysql_stmt_init(&this->_owner->_mysqlhandle);
		if (!this->_stmt) {
			Error << "mysql_stmt_init failure, out of memory";
		}
	}

	MySQLStatement::~MySQLStatement() {
		int rc = mysql_stmt_close(this->_stmt);
		if (rc != 0) {
			Error.cout("error: %s", mysql_stmt_error(this->_stmt));
		}
	}

	bool MySQLStatement::prepare(std::string s) {
		int rc = mysql_stmt_prepare(this->_stmt, s.c_str(), s.length());
		CHECK_RETURN(rc == 0, false, "prepare: %s", mysql_stmt_error(this->_stmt));
		return true;
	}

	bool MySQLStatement::bindParam(MYSQL_BIND* b) {
		int rc = mysql_stmt_bind_param(this->_stmt, b);
		CHECK_RETURN(rc == 0, false, "bindParam: %s", mysql_stmt_error(this->_stmt));
		return true;
	}

	bool MySQLStatement::exec() {
		int rc = mysql_stmt_execute(this->_stmt);
		CHECK_RETURN(rc == 0, false, "exec: %s", mysql_stmt_error(this->_stmt));
		return true;
	}

	int MySQLStatement::rowCount() {
		int rc = mysql_stmt_num_rows(this->_stmt);
		return rc;
	}

	bool MySQLStatement::bindResult(MYSQL_BIND* b) {
		int rc = mysql_stmt_bind_result(this->_stmt, b);
		CHECK_RETURN(rc == 0, false, "bindResult: %s", mysql_stmt_error(this->_stmt));
		rc = mysql_stmt_store_result(this->_stmt);
		CHECK_RETURN(rc == 0, false, "store result: %s", mysql_stmt_error(this->_stmt));
		return true;
	}

	bool MySQLStatement::fetch() {
		int rc = mysql_stmt_fetch(this->_stmt);
		if (rc != 0) {
			if (rc == MYSQL_NO_DATA) {
				return false;
			}
			CHECK_RETURN(false, false, "fetch: %s", mysql_stmt_error(this->_stmt));			
		}
		return true;
	}

	bool MySQLStatement::fetchColumn(MYSQL_BIND* b, unsigned int column, unsigned long offset) {
		int rc = mysql_stmt_fetch_column(this->_stmt, b, column, offset);
		CHECK_RETURN(rc == 0, false, "fetchColumn: %s", mysql_stmt_error(this->_stmt));
		return true;
	}

	bool MySQLStatement::freeResult() {
		int rc = mysql_stmt_free_result(this->_stmt);
		CHECK_RETURN(rc == 0, false, "free result: %s", mysql_stmt_error(this->_stmt));
		return true;
	}

	unsigned int MySQLStatement::lastError() {
		return mysql_stmt_errno(this->_stmt);
	}
}

