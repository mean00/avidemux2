///////////////////////////////////////////////////////////////////////////////
//
//  The contents of this file are subject to the Mozilla Public License
//  Version 1.1 (the "License"); you may not use this file except in
//  compliance with the License. You may obtain a copy of the License at
//  http://www.mozilla.org/MPL/
//
//  Software distributed under the License is distributed on an "AS IS"
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
//  License for the specific language governing rights and limitations
//  under the License.
// 
//  The Original Code is MP4v2.
// 
//  The Initial Developer of the Original Code is Kona Blend.
//  Portions created by Kona Blend are Copyright (C) 2008.
//  All Rights Reserved.
//
//  Contributors:
//      Kona Blend, kona8lend@@gmail.com
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MP4V2_UTIL_DATABASE_H
#define MP4V2_UTIL_DATABASE_H

namespace mp4v2 { namespace util {

///////////////////////////////////////////////////////////////////////////////
///
/// Database class is the base implementation for persistent databases.
///
/// All databases use an ASCII file format:
///     @li leading/trailing spaces on any line are trimmed.
///     @li lines beginning with '#' are considered comments.
///     @li lines of { <NAME><DELIMITER><VALUE><EOL> } form NAME/VALUEs pairs.
///     @li <NAME> cannot contain any <DELIMITER> characters.
///     @li <DELIMITER> is any combination of { ' ', '=', '\t' }.
///     @li <VALUE> continues until <EOL>.
///     @li <EOL> is optional for last line.
///     @li <NAME> of value this._key marks the beginning of a new record.
///     @li <NAME> is case-insensitive.
///     @li subsequent lines of NAME/VALUE pairs are part of the same record.
///
///////////////////////////////////////////////////////////////////////////////
class Database {
public:
    virtual ~Database();

protected:
    /// Constructor.
    ///
    /// @param file specifies filename for IO operations.
    /// @param key specifies the name of primary key.
    ///
    Database( const string& file, const string& key );

    /// Close database file.
    void close();

    /// Open database file.
    ///
    /// @param write <b>true</b> to open file for writing, <b>false</b> for reading.
    /// @param fname filename to open.
    ///     On Windows, this should be a UTF-8 encoded string.
    ///     On other platforms, it should be an 8-bit encoding that is
    ///     appropriate for the platform, locale, file system, etc.
    ///     (prefer to use UTF-8 when possible).
    ///
    /// @return <b>true</b> on error.
    ///
    bool open( bool write, string& fname );

    /// Parse a record-data from open intput stream.
    ///
    /// @param data is populated with NAME/VALUE pairs if any.
    ///
    void parseData( map<string,string>& data );

    ///////////////////////////////////////////////////////////////////////////

    const string  _filename; // filename (basename only) used for database
    const string  _key;      // name of key for record boundries
    fstream       _stream;   // // IO object

private:
    /// parse a name/value pair from open input stream.
    ///
    /// @param name stores the parsed name.
    /// @param value stores the parsed value.
    ///
    /// @return <b>true</b> on error (no name/value pair was parised).
    ///
    bool parsePair( string& name, string& value );

    /*************************************************************************/

    string _currentKeyValue;
};

///////////////////////////////////////////////////////////////////////////////

}} // namespace mp4v2::util

#endif // MP4V2_UTIL_DATABASE_H
