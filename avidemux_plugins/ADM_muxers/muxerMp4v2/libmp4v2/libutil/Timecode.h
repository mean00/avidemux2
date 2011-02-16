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

#ifndef MP4V2_UTIL_TIMECODE_H
#define MP4V2_UTIL_TIMECODE_H

namespace mp4v2 { namespace util {

///////////////////////////////////////////////////////////////////////////////

/// Class representing SMPTE timecode.
///
/// Objects of this class represent a specific time or duration and can
/// be converted to/from string values, and various epochs.
///
/// The standard pattern for string representation is as follows:
///     @li HH:MM:SS:FF
///     @li HH:MM:SS.DDD
///
/// where:
///     @li <b>HH</b> specifies hours
///     @li <b>MM</b> specifies minutes
///     @li <b>SS</b> specifies seconds
///     @li <b>:</b> specifies normal timecode
///     @li <b>FF</b> specifies the frame number
///     @li <b>.</b> specifies decimal fractions of a second follow
///     @li <b>DDD</b> specifies decimal fractions of a second, rounded down to closest scale
///
class MP4V2_EXPORT Timecode {
public:
    enum Format {
        FRAME,
        DECIMAL,
    };

private:
    double   _scale;
    uint64_t _duration;
    Format   _format;
    string   _svalue;

    uint64_t _hours;
    uint64_t _minutes;
    uint64_t _seconds;
    uint64_t _subseconds;

public:
    const double&   scale;
    const uint64_t& duration;
    const Format&   format;
    const string&   svalue;

    const uint64_t& hours;
    const uint64_t& minutes;
    const uint64_t& seconds;
    const uint64_t& subseconds;

public:
    Timecode( const Timecode& );
    explicit Timecode( const string&, double = 1.0 );
    explicit Timecode( uint64_t = 0, double = 1.0 );

    Timecode& operator=  ( const Timecode& );
    Timecode& operator+= ( const Timecode& );
    Timecode& operator-= ( const Timecode& );

    bool operator<  ( const Timecode& ) const;
    bool operator<= ( const Timecode& ) const;
    bool operator>  ( const Timecode& ) const;
    bool operator>= ( const Timecode& ) const;
    bool operator!= ( const Timecode& ) const;
    bool operator== ( const Timecode& ) const;

    Timecode operator+ ( const Timecode& ) const;
    Timecode operator- ( const Timecode& ) const;

    bool parse( const string&, string* = NULL );

    void reset();

    void setScale    ( double );
    void setDuration ( uint64_t, double = 0.0 );
    void setFormat   ( Format );

    void setHours      ( uint64_t );
    void setMinutes    ( uint64_t );
    void setSeconds    ( uint64_t );
    void setSubseconds ( uint64_t );

private:
    uint64_t convertDuration( const Timecode& ) const;
    void recompute();
};

///////////////////////////////////////////////////////////////////////////////

}} // namespace mp4v2::util

#endif // MP4V2_UTIL_TIMECODE_H
