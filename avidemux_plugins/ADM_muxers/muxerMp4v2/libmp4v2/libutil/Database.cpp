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

#include "libutil/impl.h"

namespace mp4v2 { namespace util {

///////////////////////////////////////////////////////////////////////////////

Database::Database( const string& filename, const string& key )
    : _filename ( filename )
    , _key      ( key )
{
}

///////////////////////////////////////////////////////////////////////////////

Database::~Database()
{
}

///////////////////////////////////////////////////////////////////////////////

void
Database::close()
{
    _stream.close();
    _stream.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool
Database::open( bool write, string& fname )
{
    _currentKeyValue.clear();

    _stream.clear();
    _stream.open( fname.c_str(), write ? ios::out : ios::in );
    return _stream.rdstate();
}

///////////////////////////////////////////////////////////////////////////////

void
Database::parseData( map<string,string>& data )
{
    data.clear();

    string name;
    string value;

    if ( _currentKeyValue.length() ) {
        data[ _key ] = _currentKeyValue;
        _currentKeyValue.clear();
    }

    while ( !parsePair( name, value ) ) {
        if ( name == _key ) {
            _currentKeyValue = value;
            break;
        }
        data[ name ] = value;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
Database::parsePair( string& name, string& value )
{
    enum Mode { LTRIM, COMMENT, NAME, DELIM, VALUE };
    Mode mode = LTRIM;
    bool delimVisible = false;

    int visibleLength = 0;

    for ( char c; !_stream.get( c ).rdstate(); ) {
        switch( mode ) {
            case LTRIM:
                switch( c ) {
                    case '\0':  // NULL
                    case '\t':  // TAB
                    case ' ':   // SPACE
                    case '\n':  // NEWLINE
                    case '\r':  // CARRIAGE-RETURN
                        break;

                    case '#':  // COMMENT
                        mode = COMMENT;
                        break;

                    default:
                        mode = NAME;
                        name = tolower( c );
                        break;
                }
                break;

            case COMMENT:
                switch( c ) {
                    case '\n':  // NEWLINE
                    case '\r':  // CARRIAGE-RETURN
                        mode = LTRIM;
                        name.clear();
                        break;

                    default:
                        break;
                }
                break;

            case NAME:
                switch( c ) {
                    case '\0':  // NULL
                        break;

                    case '\n':  // NEWLINE
                    case '\r':  // CARRIAGE-RETURN
                        mode = LTRIM;
                        break;

                    case '\t':  // TAB
                    case ' ':   // SPACE
                    case '=':   // DELIMITER
                        mode = DELIM;
                        delimVisible = false;
                        break;

                    default:
                        name += tolower( c );
                        break;
                }
                break;

            case DELIM:
                switch( c ) {
                    case '\n':  // NEWLINE
                    case '\r':  // CARRIAGE-RETURN
                        mode = LTRIM;
                        name.clear();
                        break;

                    case '\0':  // NULL
                    case '\t':  // TAB
                    case ' ':   // SPACE
                        break;

                    case '=':  // DELIMITER
                        if( delimVisible ) {
                            mode = VALUE;
                            value = c;
                            visibleLength = (uint32_t)value.length();
                        }
                        delimVisible = true;
                        break;

                    default:
                        mode = VALUE;
                        value = c;
                        visibleLength = (uint32_t)value.length();
                        break;
                }
                break;

            case VALUE:
                switch (c) {
                    case '\0':  // NULL
                        break;

                    case '\n':  // NEWLINE
                    case '\r':  // CARRIAGE-RETURN
                        if( visibleLength )
                            value.resize( visibleLength );
                        return false;

                    case '\t':  // TAB
                    case ' ':   // SPACE
                        value += ' ';
                        break;

                    default:
                        value += c;
                        visibleLength = (uint32_t)value.length();
                        break;
                }
                break;

            default:
                break;
        }
    }

    if( mode != VALUE )
        return true;

    if( visibleLength )
        value.resize( visibleLength );

    return false;
}

///////////////////////////////////////////////////////////////////////////////

}} // namespace mp4v2::util
