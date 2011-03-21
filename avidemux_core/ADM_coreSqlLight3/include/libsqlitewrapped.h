/*
 **	IError.h
 **
 **	Published / author: 2004-06-11 / grymse@alhem.net
 **/

/*
Copyright (C) 2004,2005,2006  Anders Hedstrom

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

#ifndef _IERROR_H_SQLITE
#define _IERROR_H_SQLITE

#include <string>


#ifdef SQLITEW_NAMESPACE
namespace SQLITEW_NAMESPACE {
#endif


class Database;
class Query;


/** Log class interface. */
class IError
{
public:
	virtual void error(Database&,const std::string&) = 0;
	virtual void error(Database&,Query&,const std::string&) = 0;
};


#ifdef SQLITEW_NAMESPACE
} // namespace SQLITEW_NAMESPACE {
#endif

#endif // _IERROR_H
/*
 **	StderrLog.h
 **
 **	Published / author: 2004-08-18 / grymse@alhem.net
 **/

/*
Copyright (C) 2004,2005,2006  Anders Hedstrom

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
#ifndef _STDERRLOG_H_SQLITE
#define _STDERRLOG_H_SQLITE


#ifdef SQLITEW_NAMESPACE
namespace SQLITEW_NAMESPACE {
#endif


/** Log class writing to standard error. */
class StderrLog : public IError
{
public:
	void error(Database&,const std::string&);
	void error(Database&,Query&,const std::string&);

};



#ifdef SQLITEW_NAMESPACE
} // namespace SQLITEW_NAMESPACE {
#endif

#endif // _STDERRLOG_H
/*
 **	SysLog.h
 **
 **	Published / author: 2004-08-18 / grymse@alhem.net
 **/

/*
Copyright (C) 2004,2005,2006  Anders Hedstrom

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
#ifndef _SYSLOG_H_SQLITE
#define _SYSLOG_H_SQLITE
#ifndef WIN32

#include <syslog.h>


#ifdef SQLITEW_NAMESPACE
namespace SQLITEW_NAMESPACE {
#endif


/** Log class writing to syslog. */
class SysLog : public IError
{
public:
	SysLog(const std::string& = "database", int = LOG_PID, int = LOG_USER);
	virtual ~SysLog();

	void error(Database&,const std::string&);
	void error(Database&,Query&,const std::string&);

};



#ifdef SQLITEW_NAMESPACE
} // namespace SQLITEW_NAMESPACE {
#endif

#endif // WIN32
#endif // _SYSLOG_H
/*
 **	Database.h
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

#ifndef _DATABASE_H_SQLITE
#define _DATABASE_H_SQLITE

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <string>
#include <list>
#ifdef WIN32
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;
#else
#include <stdint.h>
#endif

#ifdef SQLITEW_NAMESPACE
namespace SQLITEW_NAMESPACE {
#endif


class IError;
class Query;
class Mutex;


/** Connection information and pool. */
class Database 
{
public:
	/** Mutex container class, used by Lock. 
		\ingroup threading */
	class Mutex {
	public:
		Mutex();
		~Mutex();
		void Lock();
		void Unlock();
	private:
#ifdef _WIN32
		HANDLE m_mutex;
#else
		pthread_mutex_t m_mutex;
#endif
	};
private:
	/** Mutex helper class. */
	class Lock {
	public:
		Lock(Mutex& mutex,bool use);
		~Lock();
	private:
		Mutex& m_mutex;
		bool m_b_use;
	};
public:
	/** Connection pool struct. */
	struct OPENDB {
		OPENDB() : busy(false) {}
		sqlite3 *db;
		bool busy;
	};
	typedef std::list<OPENDB *> opendb_v;

public:
	/** Use file */
	Database(const std::string& database,
		IError * = NULL);

	/** Use file + thread safe */
	Database(Mutex& ,const std::string& database,
		IError * = NULL);

	virtual ~Database();

	/** try to establish connection with given host */
	bool Connected();

	void RegErrHandler(IError *);
	void error(Query&,const char *format, ...);
	void error(Query&,const std::string& );

	/** Request a database connection.
The "grabdb" method is used by the Query class, so that each object instance of Query gets a unique
database connection. I will reimplement your connection check logic in the Query class, as that's where
the database connection is really used.
It should be used something like this.
{
		Query q(db);
		if (!q.Connected())
			 return false;
		q.execute("delete * from user"); // well maybe not
}

When the Query object is deleted, then "freedb" is called - the database connection stays open in the
m_opendbs vector. New Query objects can then reuse old connections.
	*/
	OPENDB *grabdb();
	void freedb(OPENDB *odb);

	/** Escape string - change all ' to ''. */
	std::string safestr(const std::string& );
	/** Make string xml safe. */
	std::string xmlsafestr(const std::string& );

	/** Convert string to 64-bit integer. */
	int64_t a2bigint(const std::string& );
	/** Convert string to unsigned 64-bit integer. */
	uint64_t a2ubigint(const std::string& );

private:
	Database(const Database& ) : m_mutex(m_mutex) {}
	Database& operator=(const Database& ) { return *this; }
	void error(const char *format, ...);
	//
	std::string database;
	opendb_v m_opendbs;
	IError *m_errhandler;
	bool m_embedded;
	Mutex& m_mutex;
	bool m_b_use_mutex;
};


#ifdef SQLITEW_NAMESPACE
} // namespace SQLITEW_NAMESPACE {
#endif

#endif // _DATABASE_H
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
class Query 
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
