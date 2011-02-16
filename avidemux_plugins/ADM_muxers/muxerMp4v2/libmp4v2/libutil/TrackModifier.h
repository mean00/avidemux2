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

#ifndef MP4V2_UTIL_TRACKMODIFIER_H
#define MP4V2_UTIL_TRACKMODIFIER_H

namespace mp4v2 { namespace util {

///////////////////////////////////////////////////////////////////////////////

class MP4V2_EXPORT TrackModifier
{
private:
    class Properties
    {
    private:
        TrackModifier& _trackModifier;

    public:
        Properties( TrackModifier& );

        void update();

        MP4Integer24Property&    flags;
        MP4Integer16Property&    layer;
        MP4Integer16Property&    alternateGroup;
        MP4Float32Property&      volume;
        MP4Float32Property&      width;
        MP4Float32Property&      height;
        MP4LanguageCodeProperty& language;
        MP4StringProperty&       handlerType;
        MP4StringProperty&       handlerName;
        MP4BytesProperty*        userDataName;

    private:
        MP4Property& refProperty( const char* );
        MP4Property* findProperty( const char* );
        void updateProperty( const char*, MP4Property** );
    };

    friend class Properties;

private:
    static MP4Atom& refTrackAtom( MP4File&, uint16_t );

private:
    MP4Atom&   _track;
    Properties _props;

    // Track Header
    bool     _enabled;
    bool     _inMovie;
    bool     _inPreview;
    uint16_t _layer;
    uint16_t _alternateGroup;
    float    _volume;
    float    _width;
    float    _height;

    // Media Header
    bmff::LanguageCode _language;

    // Handler Reference
    string _handlerType;
    string _handlerName;

    // User Data name
    string _userDataName;

public:
    MP4File&         file;
    const uint16_t   trackIndex;
    const MP4TrackId trackId;

    const bool&     enabled;
    const bool&     inMovie;
    const bool&     inPreview;
    const uint16_t& layer;
    const uint16_t& alternateGroup;
    const float&    volume;
    const float&    width;
    const float&    height;

    const bmff::LanguageCode& language;

    const string& handlerType;
    const string& handlerName;

    const string& userDataName;

public:
    TrackModifier( MP4FileHandle, uint16_t );
    ~TrackModifier();

    void setEnabled        ( bool );
    void setInMovie        ( bool );
    void setInPreview      ( bool );
    void setLayer          ( uint16_t );
    void setAlternateGroup ( uint16_t );
    void setVolume         ( float );
    void setWidth          ( float );
    void setHeight         ( float );
    void setLanguage       ( bmff::LanguageCode );
    void setHandlerName    ( const string& );
    void setUserDataName   ( const string& );

    // set by string
    void setEnabled        ( const string& );
    void setInMovie        ( const string& );
    void setInPreview      ( const string& );
    void setLayer          ( const string& );
    void setAlternateGroup ( const string& );
    void setVolume         ( const string& );
    void setWidth          ( const string& );
    void setHeight         ( const string& );
    void setLanguage       ( const string& );

    bool hasUserDataName() const;
    void removeUserDataName();

    void dump( ostream&, const string& );

private:
    void fetch();

    static string toString( bool );
    static string toString( float, uint8_t, uint8_t );

    static bool&     fromString( const string&, bool& );
    static float&    fromString( const string&, float& );
    static uint16_t& fromString( const string&, uint16_t& );

    static string toStringTrackType( const string& );
};

///////////////////////////////////////////////////////////////////////////////

}} // namespace mp4v2::util

#endif // MP4V2_UTIL_TRACKMODIFIER_H
