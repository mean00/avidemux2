/*
 **	Database.cpp
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
#include <stdio.h>
#ifdef _WIN32
#pragma warning(disable:4786)
#endif

#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ADM_sqlite3.h>
#include <stdarg.h>

#include "IError.h"
#include "Database.h"


#ifdef SQLITEW_NAMESPACE
namespace SQLITEW_NAMESPACE {
#endif


Database::Database(const std::string& d,IError *e)
:database(d)
,m_errhandler(e)
,m_embedded(true)
,m_mutex(m_mutex)
,m_b_use_mutex(false)
{
}


Database::Database(Mutex& m,const std::string& d,IError *e)
:database(d)
,m_errhandler(e)
,m_embedded(true)
,m_mutex(m)
,m_b_use_mutex(true)
{
}


Database::~Database()
{
	for (opendb_v::iterator it = m_opendbs.begin(); it != m_opendbs.end(); it++)
	{
		OPENDB *p = *it;
		sqlite3_close(p -> db);
	}
	while (m_opendbs.size())
	{
		opendb_v::iterator it = m_opendbs.begin();
		OPENDB *p = *it;
		if (p -> busy)
		{
			error("destroying Database object before Query object");
		}
		delete p;
		m_opendbs.erase(it);
	}
}


void Database::RegErrHandler(IError *p)
{
	m_errhandler = p;
}


Database::OPENDB *Database::grabdb()
{
	Lock lck(m_mutex, m_b_use_mutex);
	OPENDB *odb = NULL;

	for (opendb_v::iterator it = m_opendbs.begin(); it != m_opendbs.end(); it++)
	{
		odb = *it;
		if (!odb -> busy)
		{
			break;
		}
		else
		{
			odb = NULL;
		}
	}
	if (!odb)
	{
		odb = new OPENDB;
		if (!odb)
		{
			error("grabdb: OPENDB struct couldn't be created");
			return NULL;
		}
		int rc = sqlite3_open(database.c_str(), &odb -> db);
		if (rc)
		{
			error("Can't open database: %s\n", sqlite3_errmsg(odb -> db));
			sqlite3_close(odb -> db);
			delete odb;
			return NULL;
		}
		odb -> busy = true;
		m_opendbs.push_back(odb);
	}
	else
	{
		odb -> busy = true;
	}
	return odb;
}


void Database::freedb(Database::OPENDB *odb)
{
	Lock lck(m_mutex, m_b_use_mutex);
	if (odb)
	{
		odb -> busy = false;
	}
}


void Database::error(const char *format, ...)
{
	if (m_errhandler)
	{
		va_list ap;
		char errstr[5000];
		va_start(ap, format);
#ifdef WIN32
		vsprintf(errstr, format, ap);
#else
		vsnprintf(errstr, 5000, format, ap);
#endif
		va_end(ap);
		m_errhandler -> error(*this, errstr);
	}
}


void Database::error(Query& q,const char *format, ...)
{
	if (m_errhandler)
	{
		va_list ap;
		char errstr[5000];
		va_start(ap, format);
#ifdef WIN32
		vsprintf(errstr, format, ap);
#else
		vsnprintf(errstr, 5000, format, ap);
#endif
		va_end(ap);
		m_errhandler -> error(*this, q, errstr);
	}
}


void Database::error(Query& q,const std::string& msg)
{
	if (m_errhandler)
	{
		m_errhandler -> error(*this, q, msg);
	}
}


bool Database::Connected()
{
	OPENDB *odb = grabdb();
	if (!odb)
	{
		return false;
	}
	freedb(odb);
	return true;
}


Database::Lock::Lock(Mutex& mutex,bool use) : m_mutex(mutex),m_b_use(use) 
{
	if (m_b_use) 
	{
		m_mutex.Lock();
	}
}


Database::Lock::~Lock() 
{
	if (m_b_use) 
	{
		m_mutex.Unlock();
	}
}


Database::Mutex::Mutex()
{
#ifdef _WIN32
	m_mutex = ::CreateMutex(NULL, FALSE, NULL);
#else
	pthread_mutex_init(&m_mutex, NULL);
#endif
}


Database::Mutex::~Mutex()
{
#ifdef _WIN32
	::CloseHandle(m_mutex);
#else
	pthread_mutex_destroy(&m_mutex);
#endif
}


void Database::Mutex::Lock()
{
#ifdef _WIN32
	DWORD d = WaitForSingleObject(m_mutex, INFINITE);
	// %! check 'd' for result
#else
	pthread_mutex_lock(&m_mutex);
#endif
}


void Database::Mutex::Unlock()
{
#ifdef _WIN32
	::ReleaseMutex(m_mutex);
#else
	pthread_mutex_unlock(&m_mutex);
#endif
}


std::string Database::safestr(const std::string& str)
{
	std::string str2;
	for (size_t i = 0; i < str.size(); i++)
	{
		switch (str[i])
		{
		case '\'':
		case '\\':
		case 34:
			str2 += '\'';
		default:
			str2 += str[i];
		}
	}
	return str2;
}


std::string Database::xmlsafestr(const std::string& str)
{
	std::string str2;
	for (size_t i = 0; i < str.size(); i++)
	{
		switch (str[i])
		{
		case '&':
			str2 += "&amp;";
			break;
		case '<':
			str2 += "&lt;";
			break;
		case '>':
			str2 += "&gt;";
			break;
		case '"':
			str2 += "&quot;";
			break;
		case '\'':
			str2 += "&apos;";
			break;
		default:
			str2 += str[i];
		}
	}
	return str2;
}


int64_t Database::a2bigint(const std::string& str)
{
	int64_t val = 0;
	bool sign = false;
	size_t i = 0;
	if (str[i] == '-')
	{
		sign = true;
		i++;
	}
	for (; i < str.size(); i++)
	{
		val = val * 10 + (str[i] - 48);
	}
	return sign ? -val : val;
}


uint64_t Database::a2ubigint(const std::string& str)
{
	uint64_t val = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		val = val * 10 + (str[i] - 48);
	}
	return val;
}


#ifdef SQLITEW_NAMESPACE
} // namespace SQLITEW_NAMESPACE {
#endif

