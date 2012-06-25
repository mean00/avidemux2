/*
 **	StderrLog.cpp
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
#include <stdio.h>
#ifdef _WIN32
#pragma warning(disable:4786)
#endif

#include <string>
#include <map>
#include <time.h>
#include <sqlite3.h>

#include "Database.h"
#include "Query.h"
#include "IError.h"
#include "StderrLog.h"


#ifdef SQLITEW_NAMESPACE
namespace SQLITEW_NAMESPACE {
#endif


void StderrLog::error(Database& db,const std::string& str)
{
	time_t t = time(NULL);
	struct tm *tp = localtime(&t);
	fprintf(stderr,"%d-%02d-%02d %02d:%02d:%02d :: Database: %s\n",
		tp -> tm_year + 1900,tp -> tm_mon + 1,tp -> tm_mday,
		tp -> tm_hour,tp -> tm_min, tp -> tm_sec,
		str.c_str());
}


void StderrLog::error(Database& db,Query& q,const std::string& str)
{
	time_t t = time(NULL);
	struct tm *tp = localtime(&t);
	fprintf(stderr,"%d-%02d-%02d %02d:%02d:%02d :: Query: %s: %s(%d)\n",
		tp -> tm_year + 1900,tp -> tm_mon + 1,tp -> tm_mday,
		tp -> tm_hour,tp -> tm_min, tp -> tm_sec,
		str.c_str(),q.GetError().c_str(),q.GetErrno());
	fprintf(stderr," (QUERY: \"%s\")\n",q.GetLastQuery().c_str());
}


#ifdef SQLITEW_NAMESPACE
} // namespace SQLITEW_NAMESPACE {
#endif

