#ifdef _WIN32
#pragma warning(disable:4786)
#endif
/*
 **	Query.h
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

#ifndef _QUERY_H_SQLITE
#define _QUERY_H_SQLITE

#include "ADM_coreSqlLight3_export.h"

#include <string>
#include <map>
#ifdef WIN32
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;
#else
#include <stdint.h>
#endif


#ifdef SQLITEW_NAMESPACE
namespace SQLITEW_NAMESPACE {
#endif


/** SQL Statement execute / result. */
class ADM_CORESQLLIGHT3_EXPORT Query 
{
public:
	/** Constructor accepting reference to database object. */
	Query(Database& dbin);
	/** Constructor accepting reference to database object
		and query string to execute. */
	Query(Database& dbin,const std::string& sql);
	~Query();

	/** Check if database object is connectable. */
	bool Connected();
	/** Return reference to database object. */
	Database& GetDatabase() const;
	/** Return string containing last query executed. */
	const std::string& GetLastQuery();

	/** execute() returns true if query is successful,
		does not store result. */
	bool execute(const std::string& sql);

	/** Execute query and store result. */
	sqlite3_stmt *get_result(const std::string& sql);
	/** Free stored result, must be called after get_result() before calling 
		execute()/get_result() again. */
	void free_result();
	/** Fetch next result row.
		\return false if there was no row to fetch (end of rows) */
	bool fetch_row();
	/** Get id of last insert. */
	sqlite_int64 insert_id();
	/** Returns 0 if there are no rows to fetch. */
	long num_rows();
	/** Number of columns in current result. */
	int num_cols();
	/** Last error string. */
	std::string GetError();
	/** Last error code. */
	int GetErrno();

	/** Check if column x in current row is null. */
	bool is_null(int x);

	/** Execute query and return first result as a string. */
	const char *get_string(const std::string& sql);
	/** Execute query and return first result as a long integer. */
	long get_count(const std::string& sql);
	/** Execute query and return first result as a double. */
	double get_num(const std::string& sql);

	/** Return column named x as a string value. */
	const char *getstr(const std::string& x);
	/** Return column x as a string value. */
	const char *getstr(int x);
	/** Return next column as a string value - see rowcount. */
	const char *getstr();

	/** Return column named x as a long integer. */
	long getval(const std::string& x);
	/** Return column x as a long integer. */
	long getval(int x);
	/** Return next column as a long integer - see rowcount. */
	long getval();

	/** Return column named x as an unsigned long integer. */
	unsigned long getuval(const std::string& x);
	/** Return column x as an unsigned long integer. */
	unsigned long getuval(int x);
	/** Return next column as an unsigned long integer. */
	unsigned long getuval();

	/** Return column named x as a 64-bit integer value. */
	int64_t getbigint(const std::string& x);
	/** Return column x as a 64-bit integer value. */
	int64_t getbigint(int x);
	/** Return next column as a 64-bit integer value. */
	int64_t getbigint();

	/** Return column named x as an unsigned 64-bit integer value. */
	uint64_t getubigint(const std::string& x);
	/** Return column x as an unsigned 64-bit integer value. */
	uint64_t getubigint(int x);
	/** Return next column as an unsigned 64-bit integer value. */
	uint64_t getubigint();

	/** Return column named x as a double. */
	double getnum(const std::string& x);
	/** Return column x as a double. */
	double getnum(int x);
	/** Return next column as a double. */
	double getnum();

private:
	/** Hide the copy constructor. */
	Query(const Query& q) : m_db(q.GetDatabase()) {}
	/** Hide the assignment operator. */
	Query& operator=(const Query& ) { return *this; }
	/** Print current result to stdout. */
	void ViewRes();
	/** Print error to debug class. */
	void error(const std::string& );
	Database& m_db; ///< Reference to database object
	Database::OPENDB *odb; ///< Connection pool handle
	sqlite3_stmt *res; ///< Stored result
	bool row; ///< true if fetch_row succeeded
	short rowcount; ///< Current column pointer in result
	std::string m_tmpstr; ///< Used to store result in get_string() call
	std::string m_last_query; ///< Last query executed
	int cache_rc; ///< Cached result after call to get_result()
	bool cache_rc_valid; ///< Indicates cache_rc is valid
	int m_row_count; ///< 0 if get_result() returned no rows
	//
	std::map<std::string,int> m_nmap; ///< map translating column names to index
	int m_num_cols; ///< number of columns in result
};


#ifdef SQLITEW_NAMESPACE
} // namespace SQLITEW_NAMESPACE {
#endif

#endif // _QUERY_H
