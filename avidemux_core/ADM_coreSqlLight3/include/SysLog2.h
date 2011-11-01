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
