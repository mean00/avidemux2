/*
 **	Query.cpp
 **
 **	Published / author: 2005-08-12 / grymse@alhem.net
 **/

/*
Copyright (C) 2001-2006  Anders Hedstrom

This program is made available under the terms of the GNU GPL.

If you would like to use this program in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for this program, please
visit http://www.alhem.net/sqlwrapped/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifdef _WIN32
#pragma warning(disable:4786)
#endif

#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include "Database.h"
#include "Query.h"


#ifdef SQLITEW_NAMESPACE
namespace SQLITEW_NAMESPACE {
#endif


Query::Query(Database& dbin) 
: m_db(dbin)
,odb(dbin.grabdb())
,res(NULL)
,row(false)
,cache_rc(0)
,cache_rc_valid(false)
,m_row_count(0)
,m_num_cols(0)
{
}


Query::Query(Database& dbin,const std::string& sql) 
: m_db(dbin)
,odb(dbin.grabdb())
,res(NULL)
,row(false)
,cache_rc(0)
,cache_rc_valid(false)
,m_row_count(0)
,m_num_cols(0)
{
	execute(sql);
}


Query::~Query()
{
	if (res)
	{
		GetDatabase().error(*this, "sqlite3_finalize in destructor");
		sqlite3_finalize(res);
	}
	if (odb)
	{
		m_db.freedb(odb);
	}
}


Database& Query::GetDatabase() const
{
	return m_db;
}


/*
The sqlite3_finalize() routine deallocates a prepared SQL statement. 
All prepared statements must be finalized before the database can be closed.
*/
bool Query::execute(const std::string& sql)
{
	// query, no result
	m_last_query = sql;
	if (odb && res)
	{
		GetDatabase().error(*this, "execute: query busy");
	}
	if (odb && !res)
	{
		const char *s = NULL;
		int rc = sqlite3_prepare(odb -> db, sql.c_str(), sql.size(), &res, &s);
		if (rc != SQLITE_OK)
		{
			GetDatabase().error(*this, "execute: prepare query failed");
			return false;
		}
		if (!res)
		{
			GetDatabase().error(*this, "execute: query failed");
			return false;
		}
		rc = sqlite3_step(res); // execute
		sqlite3_finalize(res); // deallocate statement
		res = NULL;
		switch (rc)
		{
		case SQLITE_BUSY:
			GetDatabase().error(*this, "execute: database busy");
			return false;
		case SQLITE_DONE:
		case SQLITE_ROW:
			return true;
		case SQLITE_ERROR:
			GetDatabase().error(*this, sqlite3_errmsg(odb -> db));
			return false;
		case SQLITE_MISUSE:
			GetDatabase().error(*this, "execute: database misuse");
			return false;
		}
		GetDatabase().error(*this, "execute: unknown result code");
	}
	return false;
}



// methods using db specific api calls

sqlite3_stmt *Query::get_result(const std::string& sql)
{
	// query, result
	m_last_query = sql;
	if (odb && res)
	{
		GetDatabase().error(*this, "get_result: query busy");
	}
	if (odb && !res)
	{
		const char *s = NULL;
		int rc = sqlite3_prepare(odb -> db, sql.c_str(), sql.size(), &res, &s);
		if (rc != SQLITE_OK)
		{
			GetDatabase().error(*this, "get_result: prepare query failed");
			return NULL;
		}
		if (!res)
		{
			GetDatabase().error(*this, "get_result: query failed");
			return NULL;
		}
		// get column names from result
		{
			int i = 0;
			do
			{
				const char *p = sqlite3_column_name(res, i);
				if (!p)
					break;
				m_nmap[p] = ++i;
			} while (true);
			m_num_cols = i;
		}
		cache_rc = sqlite3_step(res);
		cache_rc_valid = true;
		m_row_count = (cache_rc == SQLITE_ROW) ? 1 : 0;
	}
	return res;
}


void Query::free_result()
{
	if (odb && res)
	{
		sqlite3_finalize(res);
		res = NULL;
		row = false;
		cache_rc_valid = false;
	}
	// clear column names
	while (m_nmap.size())
	{
		std::map<std::string,int>::iterator it = m_nmap.begin();
		m_nmap.erase(it);
	}
}


bool Query::fetch_row()
{
	rowcount = 0;
	row = false;
	if (odb && res)
	{
		int rc = cache_rc_valid ? cache_rc : sqlite3_step(res); // execute
		cache_rc_valid = false;
		switch (rc)
		{
		case SQLITE_BUSY:
			GetDatabase().error(*this, "execute: database busy");
			return false;
		case SQLITE_DONE:
			return false;
		case SQLITE_ROW:
			row = true;
			return true;
		case SQLITE_ERROR:
			GetDatabase().error(*this, sqlite3_errmsg(odb -> db));
			return false;
		case SQLITE_MISUSE:
			GetDatabase().error(*this, "execute: database misuse");
			return false;
		}
		GetDatabase().error(*this, "execute: unknown result code");
	}
	return false;
}


sqlite_int64 Query::insert_id()
{
	if (odb)
	{
		return sqlite3_last_insert_rowid(odb -> db);
	}
	else
	{
		return 0;
	}
}


long Query::num_rows()
{
	return odb && res ? m_row_count : 0;
}


int Query::num_cols()
{
	return m_num_cols;
}


bool Query::is_null(int x)
{
	if (odb && res && row)
	{
		if (sqlite3_column_type(res, x) == SQLITE_NULL)
			return true;
	}
	return false; // ...
}


const char *Query::getstr(const std::string& x)
{
	int index = m_nmap[x] - 1;
	if (index >= 0)
		return getstr(index);
	error("Column name lookup failure: " + x);
	return "";
}


const char *Query::getstr(int x)
{
	if (odb && res && row && x < sqlite3_column_count(res) )
	{
		const unsigned char *tmp = sqlite3_column_text(res, x);
		return tmp ? (const char *)tmp : "";
	}
	return "";
}


const char *Query::getstr()
{
	return getstr(rowcount++);
}


double Query::getnum(const std::string& x)
{
	int index = m_nmap[x] - 1;
	if (index >= 0)
		return getnum(index);
	error("Column name lookup failure: " + x);
	return 0;
}


double Query::getnum(int x)
{
	if (odb && res && row)
	{
		return sqlite3_column_double(res, x);
	}
	return 0;
}


long Query::getval(const std::string& x)
{
	int index = m_nmap[x] - 1;
	if (index >= 0)
		return getval(index);
	error("Column name lookup failure: " + x);
	return 0;
}


long Query::getval(int x)
{
	if (odb && res && row)
	{
		return sqlite3_column_int(res, x);
	}
	return 0;
}


double Query::getnum()
{
	return getnum(rowcount++);
}


long Query::getval()
{
	return getval(rowcount++);
}


unsigned long Query::getuval(const std::string& x)
{
	int index = m_nmap[x] - 1;
	if (index >= 0)
		return getuval(index);
	error("Column name lookup failure: " + x);
	return 0;
}


unsigned long Query::getuval(int x)
{
	unsigned long l = 0;
	if (odb && res && row)
	{
		l = sqlite3_column_int(res, x);
	}
	return l;
}


unsigned long Query::getuval()
{
	return getuval(rowcount++);
}


int64_t Query::getbigint(const std::string& x)
{
	int index = m_nmap[x] - 1;
	if (index >= 0)
		return getbigint(index);
	error("Column name lookup failure: " + x);
	return 0;
}


int64_t Query::getbigint(int x)
{
	if (odb && res && row)
	{
		return sqlite3_column_int64(res, x);
	}
	return 0;
}


int64_t Query::getbigint()
{
	return getbigint(rowcount++);
}


uint64_t Query::getubigint(const std::string& x)
{
	int index = m_nmap[x] - 1;
	if (index >= 0)
		return getubigint(index);
	error("Column name lookup failure: " + x);
	return 0;
}


uint64_t Query::getubigint(int x)
{
	uint64_t l = 0;
	if (odb && res && row)
	{
		l = sqlite3_column_int64(res, x);
	}
	return l;
}


uint64_t Query::getubigint()
{
	return getubigint(rowcount++);
}


double Query::get_num(const std::string& sql)
{
	double l = 0;
	if (get_result(sql))
	{
		if (fetch_row())
		{
			l = getnum();
		}
		free_result();
	}
	return l;
}


long Query::get_count(const std::string& sql)
{
	long l = 0;
	if (get_result(sql))
	{
		if (fetch_row())
			l = getval();
		free_result();
	}
	return l;
}


const char *Query::get_string(const std::string& sql)
{
	bool found = false;
	m_tmpstr = "";
	if (get_result(sql))
	{
		if (fetch_row())
		{
			m_tmpstr = getstr();
			found = true;
		}
		free_result();
	}
	return m_tmpstr.c_str(); // %! changed from 1.0 which didn't return NULL on failed query
}


const std::string& Query::GetLastQuery()
{
	return m_last_query;
}


std::string Query::GetError()
{
	if (odb)
		return sqlite3_errmsg(odb -> db);
	return "";
}


int Query::GetErrno()
{
	if (odb)
		return sqlite3_errcode(odb -> db);
	return 0;
}


bool Query::Connected()
{
	return odb ? true : false;
}




void Query::ViewRes()
{
	if (!res)
	{
		printf("no result stored\n");
		return;
	}
	printf("result column count = %d\n", sqlite3_column_count(res));
	for (int i = 0; i < sqlite3_column_count(res); i++)
	{
		printf(" %2d   type %d   name '%s'", i, sqlite3_column_type(res, i), sqlite3_column_name(res, i));
		printf("  / '%s'", (char *)sqlite3_column_text(res, i));
		printf("  / %d", sqlite3_column_int(res, i));
		printf("  / %f", sqlite3_column_double(res, i));
		printf("\n");
	}
}


void Query::error(const std::string& msg)
{
	GetDatabase().error(*this, msg);
}


#ifdef SQLITEW_NAMESPACE
} // namespace SQLITEW_NAMESPACE {
#endif

