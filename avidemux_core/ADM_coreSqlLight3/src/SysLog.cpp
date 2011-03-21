/*
 **	SysLog.cpp
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
#ifndef WIN32

#include <ADM_sqlite3.h>
#include <syslog.h>
#include <string.h> // MEANX

#include "Database.h"
#include "Query.h"
#include "IError.h"
#include "SysLog.h"


#ifdef SQLITEW_NAMESPACE
namespace SQLITEW_NAMESPACE {
#endif


SysLog::SysLog(const std::string& appname,int option,int facility)
{
static	char blah[100];
	strcpy(blah, appname.c_str());
	openlog(blah, option, facility);
}


SysLog::~SysLog()
{
	closelog();
}


void SysLog::error(Database& db,const std::string& str)
{
	syslog(LOG_ERR, "%s", str.c_str() );
}


void SysLog::error(Database& db,Query& q,const std::string& str)
{
	syslog(LOG_ERR, "%s: %s(%d)", str.c_str(),q.GetError().c_str(),q.GetErrno() );
	syslog(LOG_ERR, "QUERY: \"%s\"", q.GetLastQuery().c_str());
}


#ifdef SQLITEW_NAMESPACE
} // namespace SQLITEW_NAMESPACE {
#endif

#endif // WIN32
