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

Timecode::Timecode( const Timecode& obj )
    : _scale            ( 1.0 )
    , _duration         ( 0 )
    , _format           ( FRAME )
    , _svalue           ( "" )
    , _hours            ( 0 )
    , _minutes          ( 0 )
    , _seconds          ( 0 )
    , _subseconds       ( 0 )
    , scale             ( _scale )
    , duration          ( _duration )
    , format            ( _format )
    , svalue            ( _svalue )
    , hours             ( _hours )
    , minutes           ( _minutes )
    , seconds           ( _seconds )
    , subseconds        ( _subseconds )
{
    operator=( obj );
}

///////////////////////////////////////////////////////////////////////////////

Timecode::Timecode( const string& time_, double scale_ )
    : _scale            ( scale_ < 1.0 ? 1.0 : scale_ )
    , _duration         ( 0 )
    , _format           ( FRAME )
    , _svalue           ( "" )
    , _hours            ( 0 )
    , _minutes          ( 0 )
    , _seconds          ( 0 )
    , _subseconds       ( 0 )
    , scale             ( _scale )
    , duration          ( _duration )
    , format            ( _format )
    , svalue            ( _svalue )
    , hours             ( _hours )
    , minutes           ( _minutes )
    , seconds           ( _seconds )
    , subseconds        ( _subseconds )
{
    parse( time_ );
}

///////////////////////////////////////////////////////////////////////////////

Timecode::Timecode( uint64_t duration_, double scale_ )
    : _scale            ( scale_ < 1.0 ? 1.0 : scale_ )
    , _duration         ( 0 )
    , _format           ( FRAME )
    , _svalue           ( "" )
    , _hours            ( 0 )
    , _minutes          ( 0 )
    , _seconds          ( 0 )
    , _subseconds       ( 0 )
    , scale             ( _scale )
    , duration          ( _duration )
    , format            ( _format )
    , svalue            ( _svalue )
    , hours             ( _hours )
    , minutes           ( _minutes )
    , seconds           ( _seconds )
    , subseconds        ( _subseconds )
{
    setDuration( duration_ );
}

///////////////////////////////////////////////////////////////////////////////

uint64_t
Timecode::convertDuration( const Timecode& obj ) const
{
    if( _scale == obj._scale )
        return obj._duration;

    return static_cast<uint64_t>( ( _scale / obj._scale ) * obj._duration );
}

///////////////////////////////////////////////////////////////////////////////

Timecode&
Timecode::operator=( const Timecode& rhs )
{
    _scale    = rhs._scale;
    _duration = rhs._duration;
    _format   = FRAME;
    _svalue   = rhs._svalue;

    _hours      = rhs._hours;
    _minutes    = rhs._minutes;
    _seconds    = rhs._seconds;
    _subseconds = rhs._subseconds;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Timecode&
Timecode::operator+=( const Timecode& rhs )
{
    uint64_t dur = _duration + convertDuration( rhs );
    // overflow check
    if( dur < _duration )
        dur = numeric_limits<long long>::max();

    setDuration( dur );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Timecode&
Timecode::operator-=( const Timecode& rhs )
{
    uint64_t dur = _duration - convertDuration( rhs );
    // underflow check
    if( dur > _duration )
        dur = 0;

    setDuration( dur );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

bool
Timecode::operator<( const Timecode& obj ) const
{
    return _duration < convertDuration( obj );
}

///////////////////////////////////////////////////////////////////////////////

bool
Timecode::operator<=( const Timecode& obj ) const
{
    return _duration <= convertDuration( obj );
}

///////////////////////////////////////////////////////////////////////////////

bool
Timecode::operator>( const Timecode& obj ) const
{
    return _duration < convertDuration( obj );
}

///////////////////////////////////////////////////////////////////////////////

bool
Timecode::operator>=( const Timecode& obj ) const
{
    return _duration < convertDuration( obj );
}

///////////////////////////////////////////////////////////////////////////////

bool
Timecode::operator!=( const Timecode& obj ) const
{
    return _duration != convertDuration( obj );
}

///////////////////////////////////////////////////////////////////////////////

bool
Timecode::operator==( const Timecode& obj ) const
{
    return _duration == convertDuration( obj );
}

///////////////////////////////////////////////////////////////////////////////

Timecode
Timecode::operator+( const Timecode& obj ) const
{
    Timecode t( *this );
    t += obj;
    return t;
}

///////////////////////////////////////////////////////////////////////////////

Timecode
Timecode::operator-( const Timecode& obj ) const
{
    Timecode t( *this );
    t -= obj;
    return t;
}

///////////////////////////////////////////////////////////////////////////////

bool
Timecode::parse( const string& time, string* outError )
{
    string outErrorPlacebo;
    string& error = outError ? *outError : outErrorPlacebo;
    error.clear();

    _format     = FRAME;
    _hours      = 0;
    _minutes    = 0;
    _seconds    = 0;
    _subseconds = 0;

    // bail if empty
    if( time.empty() ) {
        recompute();
        return false;
    }

    // count number of ':'
    int nsect = 0;
    int nsemi = 0;
    int ndot  = 0;

    const string::size_type max = time.length();
    for( string::size_type i = 0; i < max; i++ ) {
        switch( time[i] ) {
            case ':':
                nsect++;
                break;

            case ';':
                if( nsemi++ ) {
                    error = "too many semicolons";
                    return true;
                }
                nsect++;
                break;

            case '.':
                if( ndot++ ) {
                    error = "too many periods";
                    return true;
                }
                nsect++;
                break;

            default:
                break;
        }
    }

    // bail if impossible number of sections
    if( nsect > 3 ) {
        recompute();
        error = "too many sections";
        return true;
    }

    enum Target {
        HOURS,
        MINUTES,
        SECONDS,
        SUBSECONDS,
    };

    // setup target before parsing
    Target target;
    uint64_t* tvalue;
    switch( nsect ) {
        default:
        case 0:
            target = SUBSECONDS;
            tvalue = &_subseconds;
            break;

        case 1:
            target = SECONDS;
            tvalue = &_seconds;
            break;

        case 2:
            target = MINUTES;
            tvalue = &_minutes;
            break;

        case 3:
            target = HOURS;
            tvalue = &_hours;
            break;
    }

    istringstream convert;
    string tbuffer;
    for( string::size_type i = 0; i < max; i++ ) {
        const char c = time[i];
        switch( c ) {
            case ':':
                switch( target ) {
                    case HOURS:
                        convert.clear();
                        convert.str( tbuffer );
                        if( !tbuffer.empty() && !(convert >> *tvalue) ) {
                            error = "failed to convert integer";
                            return true;
                        }
                        tbuffer.clear();
                        target = MINUTES;
                        tvalue = &_minutes;
                        break;

                    case MINUTES:
                        convert.clear();
                        convert.str( tbuffer );
                        if( !tbuffer.empty() && !(convert >> *tvalue) ) {
                            error = "failed to convert integer";
                            return true;
                        }
                        tbuffer.clear();
                        target = SECONDS;
                        tvalue = &_seconds;
                        break;

                    case SECONDS:
                        convert.clear();
                        convert.str( tbuffer );
                        if( !tbuffer.empty() && !(convert >> *tvalue) ) {
                            error = "failed to convert integer";
                            return true;
                        }
                        tbuffer.clear();
                        target = SUBSECONDS;
                        tvalue = &_subseconds;
                        break;

                    default:
                    case SUBSECONDS:
                        error = "unexpected char ':'";
                        return true;
                }
                break;

            case '.':
            {
                if( target != SECONDS ) {
                    error = "unexpected char '.'";
                    return true;
                }
                _format = DECIMAL;
                convert.clear();
                convert.str( tbuffer );
                if( !tbuffer.empty() && !(convert >> *tvalue) ) {
                    error = "failed to convert integer";
                    return true;
                }
                tbuffer.clear();
                target = SUBSECONDS;
                tvalue = &_subseconds;
                break;
            }

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                tbuffer += c;
                if( tbuffer.length() > 16 ) {
                    error = "overflow";
                    return true;
                }
                break;

            default:
                error = "unexpected char '";
                error += c;
                error += "'";
                return true;
        }
    }

    // apply final section
    if( !tbuffer.empty() ) {
        convert.clear();
        convert.str( tbuffer );
        if( !tbuffer.empty() && !(convert >> *tvalue) ) {
            error = "failed to convert integer";
            return true;
        }
    }

    // special post processing
    switch( _format ) {
        case FRAME:
        default:
            break;

        case DECIMAL:
        {
            double div = std::pow( 10.0, static_cast<double>(tbuffer.length()) );
            if( div < 1.0 )
                div = 1.0;
            *tvalue = static_cast<uint64_t>( static_cast<double>(*tvalue) / div * std::ceil( _scale ));
            break;
        }
    }

    recompute();
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void
Timecode::recompute()
{
    // case: 29.97 becomes 30.0
    // case: 30.0  becomes 30.0
    const uint64_t iscale = uint64_t( std::ceil( _scale ));

    if( _subseconds > iscale - 1 ) {
        uint64_t n = _subseconds / iscale;
        _seconds += n;
        _subseconds -= n * iscale;
    }

    if( _seconds > 59 ) {
        uint64_t n = _seconds / 60;
        _minutes += n;
        _seconds -= n * 60;
    }

    if( _minutes > 59 ) {
        uint64_t n = _minutes / 60;
        _hours += n;
        _minutes -= n * 60;
    }

    _duration = _subseconds + (iscale * _seconds) + (iscale * _minutes * 60) + (iscale * _hours * 3600);

    ostringstream oss;
    oss << setfill('0') << right
        << setw(2) << _hours
        << ':'
        << setw(2) << _minutes
        << ':'
        << setw(2) << _seconds;

    switch( _format ) {
        case FRAME:
            oss << ':' << setw(2) << setfill( '0' ) << _subseconds;
            break;

        case DECIMAL:
        {
            oss << '.' << setw(3) << setfill( '0' ) << static_cast<uint64_t>(_subseconds / _scale * 1000.0 + 0.5);
            break;
        }
    }

    _svalue = oss.str();
}

///////////////////////////////////////////////////////////////////////////////

void
Timecode::reset()
{
    setDuration( 0 );
}

///////////////////////////////////////////////////////////////////////////////

void
Timecode::setDuration( uint64_t duration_, double scale_ )
{
    if( scale_ != 0.0 ) {
        _scale = scale_;
        if( _scale < 1.0 )
            _scale = 1.0;
    }

    _duration = duration_;

    const uint64_t iscale = uint64_t( std::ceil( _scale ));
    uint64_t i = _duration;

    _hours = i / (iscale * 3600);
    i -= (iscale * 3600 * _hours);

    _minutes = i / (iscale * 60);
    i -= (iscale * 60 * _minutes);

    _seconds = i / iscale;
    i -= (iscale * _seconds);

    _subseconds = i;

    recompute();
}

///////////////////////////////////////////////////////////////////////////////

void
Timecode::setFormat( Format format_ )
{
    _format = format_;
    recompute();
}

///////////////////////////////////////////////////////////////////////////////

void
Timecode::setHours( uint64_t hours_ )
{
    _hours = hours_;
    recompute();
}

///////////////////////////////////////////////////////////////////////////////

void
Timecode::setMinutes( uint64_t minutes_ )
{
    _minutes = minutes_;
    recompute();
}

///////////////////////////////////////////////////////////////////////////////

void
Timecode::setScale( double scale_ )
{
    const double oldscale = _scale;
    _scale = scale_;
    if( _scale < 1.0 )
        _scale = 1.0;

    _subseconds = static_cast<uint64_t>( (_scale / oldscale) * _subseconds );
    recompute();
}

///////////////////////////////////////////////////////////////////////////////

void
Timecode::setSeconds( uint64_t seconds_ )
{
    _seconds = seconds_;
    recompute();
}

///////////////////////////////////////////////////////////////////////////////

void
Timecode::setSubseconds( uint64_t subseconds_ )
{
    _subseconds = subseconds_;
    recompute();
}

///////////////////////////////////////////////////////////////////////////////

}} // namespace mp4v2::util
