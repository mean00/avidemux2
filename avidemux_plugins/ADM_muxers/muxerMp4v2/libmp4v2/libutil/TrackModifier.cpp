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

TrackModifier::TrackModifier( MP4FileHandle file_, uint16_t trackIndex_ )
    : _track          ( refTrackAtom( *static_cast<MP4File*>(file_), trackIndex_ ))
    , _props          ( *this ) // must come after _track is initialized
    , _enabled        ( false )
    , _inMovie        ( false )
    , _inPreview      ( false )
    , _layer          ( 0 )
    , _alternateGroup ( 0 )
    , _volume         ( 1.0f )
    , _width          ( 0.0f )
    , _height         ( 0.0f )
    , _language       ( bmff::ILC_UND )
    , _handlerType    ( "" )
    , _handlerName    ( "" )
    , _userDataName   ( "" )
    , file            ( *static_cast<MP4File*>(file_) )
    , trackIndex      ( trackIndex_ )
    , trackId         ( MP4FindTrackId( file_, trackIndex_ ))
    , enabled         ( _enabled )
    , inMovie         ( _inMovie )
    , inPreview       ( _inPreview )
    , layer           ( _layer )
    , alternateGroup  ( _alternateGroup )
    , volume          ( _volume )
    , width           ( _width )
    , height          ( _height )
    , language        ( _language )
    , handlerType     ( _handlerType )
    , handlerName     ( _handlerName )
    , userDataName    ( _userDataName )
{
    fetch();
}

///////////////////////////////////////////////////////////////////////////////

TrackModifier::~TrackModifier()
{
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::dump( ostream& out, const string& xind )
{
    const uint32_t w = 14;
    const string eq = " = ";
    const string ind = "  ";

    out << left << xind << "track[" << trackIndex << "] id=" << trackId
        << '\n' << xind << ind << setw( w ) << "type" << eq << toStringTrackType( handlerType )
        << '\n' << xind << ind << setw( w ) << "enabled" << eq << toString( enabled )
        << '\n' << xind << ind << setw( w ) << "inMovie"  << eq << toString( inMovie )
        << '\n' << xind << ind << setw( w ) << "inPreview"  << eq << toString( inPreview )
        << '\n' << xind << ind << setw( w ) << "layer"  << eq << layer
        << '\n' << xind << ind << setw( w ) << "alternateGroup"  << eq << alternateGroup
        << '\n' << xind << ind << setw( w ) << "volume"  << eq << toString( volume, 8, 8 )
        << '\n' << xind << ind << setw( w ) << "width"  << eq << toString( width, 16, 16 )
        << '\n' << xind << ind << setw( w ) << "height"  << eq << toString( height, 16, 16 )
        << '\n' << xind << ind << setw( w ) << "language"  << eq << bmff::enumLanguageCode.toString( language, true )
        << '\n' << xind << ind << setw( w ) << "handlerName"  << eq << handlerName;

    out << '\n' << xind << ind << setw( w ) << "userDataName"  << eq
        << ( _props.userDataName ? userDataName : "<absent>" );

    out << '\n';
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::fetch()
{
    _props.update();

    const uint32_t flags = _props.flags.GetValue();
    _enabled   = flags & 0x01;
    _inMovie   = flags & 0x02;
    _inPreview = flags & 0x04;

    _layer          = _props.layer.GetValue();
    _alternateGroup = _props.alternateGroup.GetValue();
    _volume         = _props.volume.GetValue();
    _width          = _props.width.GetValue();
    _height         = _props.height.GetValue();

    _language     = _props.language.GetValue();
    _handlerType  = _props.handlerType.GetValue();
    _handlerName  = _props.handlerName.GetValue();

    if( _props.userDataName ) {
        uint8_t* buffer;
        uint32_t size;
        _props.userDataName->GetValue( &buffer, &size );
        _userDataName = string( reinterpret_cast<char*>(buffer), size );
    }
    else {
        _userDataName.clear();
    }
}

///////////////////////////////////////////////////////////////////////////////

bool&
TrackModifier::fromString( const string& src, bool& dst )
{
    if( src == "true" )
        dst = true;
    else if ( src == "false" )
        dst = false;
    else {
        istringstream iss( src );
        iss >> dst;
        if( iss.rdstate() != ios::eofbit ) {
            ostringstream oss;
            oss << "invalid value: " << src;
            throw new Exception( oss.str(), __FILE__, __LINE__, __FUNCTION__ );
        }
    }

    return dst;
}

///////////////////////////////////////////////////////////////////////////////

float&
TrackModifier::fromString( const string& src, float& dst )
{
    istringstream iss( src );
    iss >> dst;
    if( iss.rdstate() != ios::eofbit ) {
        ostringstream oss;
        oss << "invalid value: " << src;
        throw new Exception( oss.str(), __FILE__, __LINE__, __FUNCTION__ );
    }

    return dst;
}

///////////////////////////////////////////////////////////////////////////////

uint16_t&
TrackModifier::fromString( const string& src, uint16_t& dst )
{
    istringstream iss( src );
    iss >> dst;
    if( iss.rdstate() != ios::eofbit ) { 
        ostringstream oss;
        oss << "invalid value: " << src;
        throw new Exception( oss.str(), __FILE__, __LINE__, __FUNCTION__ );
    }   

    return dst;
}

///////////////////////////////////////////////////////////////////////////////

bool
TrackModifier::hasUserDataName() const
{
    return _props.userDataName != NULL;
}

///////////////////////////////////////////////////////////////////////////////

MP4Atom&
TrackModifier::refTrackAtom( MP4File& file, uint16_t index )
{
    MP4Atom& root = *file.FindAtom( NULL );

    ostringstream oss;
    oss << "moov.trak[" << index << "]";
    MP4Atom* trak = root.FindAtom( oss.str().c_str() );
    if( !trak ) {
        oss.str( "" );
        oss << "trackIndex " << index << " not found";
        throw new Exception( oss.str(), __FILE__, __LINE__, __FUNCTION__ );
    }

    return *trak;
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::removeUserDataName()
{
    MP4Atom* name = _track.FindAtom( "trak.udta.name" );
    if( name )
        name->GetParentAtom()->DeleteChildAtom( name );

    MP4Atom* udta = _track.FindAtom( "trak.udta" );
    if( udta && !udta->GetNumberOfChildAtoms() )
        udta->GetParentAtom()->DeleteChildAtom( udta );
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::setAlternateGroup( uint16_t value )
{
    _props.alternateGroup.SetValue( value );
    fetch();
}

void
TrackModifier::setAlternateGroup( const string& value )
{
    uint16_t tmp;
    setAlternateGroup( fromString( value, tmp ));
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::setEnabled( bool value )
{
    _enabled = value;
    _props.flags.SetValue( (_enabled ? 0x01 : 0) | (_inMovie ? 0x02 : 0) | (_inPreview ? 0x04 : 0) );
    fetch();
}

void
TrackModifier::setEnabled( const string& value )
{
    bool tmp;
    setEnabled( fromString( value, tmp ));
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::setHandlerName( const string& value )
{
    _props.handlerName.SetValue( value.c_str() );
    fetch();
}
///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::setHeight( float value )
{
    _props.height.SetValue( value );
    fetch();
}

void
TrackModifier::setHeight( const string& value )
{
    float tmp;
    setHeight( fromString( value, tmp ));
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::setInMovie( bool value )
{
    _inMovie = value;
    _props.flags.SetValue( (_enabled ? 0x01 : 0) | (_inMovie ? 0x02 : 0) | (_inPreview ? 0x04 : 0) );
    fetch();
}

void
TrackModifier::setInMovie( const string& value )
{
    bool tmp;
    setInMovie( fromString( value, tmp ));
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::setInPreview( bool value )
{
    _inPreview = value;
    _props.flags.SetValue( (_enabled ? 0x01 : 0) | (_inMovie ? 0x02 : 0) | (_inPreview ? 0x04 : 0) );
    fetch();
}

void
TrackModifier::setInPreview( const string& value )
{
    bool tmp;
    setInPreview( fromString( value, tmp ));
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::setLanguage( bmff::LanguageCode value )
{
    _props.language.SetValue( value );
    fetch();
}

void
TrackModifier::setLanguage( const string& value )
{
    setLanguage( bmff::enumLanguageCode.toType( value ));
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::setLayer( uint16_t value )
{
    _props.layer.SetValue( value );
    fetch();
}

void
TrackModifier::setLayer( const string& value )
{
    uint16_t tmp;
    setLayer( fromString( value, tmp ));
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::setUserDataName( const string& value )
{
    if( !_props.userDataName ) {
        ostringstream oss;
        oss << "moov.trak[" << trackIndex << "]";
        file.AddDescendantAtoms( oss.str().c_str(), "udta.name" );
        _props.update();
    }

    _props.userDataName->SetValue( reinterpret_cast<const uint8_t*>(value.c_str()), (uint32_t)value.size() );
    fetch();
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::setVolume( float value )
{
    _props.volume.SetValue( value );
    fetch();
}

void
TrackModifier::setVolume( const string& value )
{
    float tmp;
    setVolume( fromString( value, tmp ));
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::setWidth( float value )
{
    _props.width.SetValue( value );
    fetch();
}

void
TrackModifier::setWidth( const string& value )
{
    float tmp;
    setWidth( fromString( value, tmp ));
}

///////////////////////////////////////////////////////////////////////////////

string
TrackModifier::toString( bool value )
{
    ostringstream oss;
    oss << (value ? "true" : "false");
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

string
TrackModifier::toString( float value, uint8_t i, uint8_t f )
{
    ostringstream oss;
    oss << fixed << setprecision(f <= 8 ? 4 : 8) << value;
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

string
TrackModifier::toStringTrackType( const string& code )
{
    if( !code.compare( "vide" ))    // 14496-12
        return "video";
    if( !code.compare( "soun" ))    // 14496-12
        return "audio";
    if( !code.compare( "hint" ))    // 14496-12
        return "hint";

    if( !code.compare( "text" ))    // QTFF
        return "text";
    if( !code.compare( "tmcd" ))    // QTFF
        return "timecode";

    if( !code.compare( "subt" ))    // QTFF
        return "subtitle";

    return string( "(" ) + code + ")";
}

///////////////////////////////////////////////////////////////////////////////

TrackModifier::Properties::Properties( TrackModifier& trackModifier_ )
    : _trackModifier ( trackModifier_ )
    , flags          ( static_cast<MP4Integer24Property&>   ( refProperty(  "trak.tkhd.flags" )))
    , layer          ( static_cast<MP4Integer16Property&>   ( refProperty(  "trak.tkhd.layer" )))
    , alternateGroup ( static_cast<MP4Integer16Property&>   ( refProperty(  "trak.tkhd.alternate_group" )))
    , volume         ( static_cast<MP4Float32Property&>     ( refProperty(  "trak.tkhd.volume" )))
    , width          ( static_cast<MP4Float32Property&>     ( refProperty(  "trak.tkhd.width" )))
    , height         ( static_cast<MP4Float32Property&>     ( refProperty(  "trak.tkhd.height" )))
    , language       ( static_cast<MP4LanguageCodeProperty&>( refProperty(  "trak.mdia.mdhd.language" )))
    , handlerType    ( static_cast<MP4StringProperty&>      ( refProperty(  "trak.mdia.hdlr.handlerType" )))
    , handlerName    ( static_cast<MP4StringProperty&>      ( refProperty(  "trak.mdia.hdlr.name" )))
    , userDataName   ( static_cast<MP4BytesProperty*>       ( findProperty( "trak.udta.name.value" )))
{
}

///////////////////////////////////////////////////////////////////////////////

MP4Property*
TrackModifier::Properties::findProperty( const char* name )
{
    MP4Property* property;
    if( !_trackModifier._track.FindProperty( name, &property ))
        return NULL;

    return property;
}

///////////////////////////////////////////////////////////////////////////////

MP4Property&
TrackModifier::Properties::refProperty( const char* name )
{
    MP4Property* property;
    if( !_trackModifier._track.FindProperty( name, &property )) {
        ostringstream oss;
        oss << "trackId " << _trackModifier.trackId << " property '" << name << "' not found";
        throw new Exception( oss.str(), __FILE__, __LINE__, __FUNCTION__ );
    }

    return *property;
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::Properties::update()
{
    // update optional properties
    updateProperty( "trak.udta.name.value", reinterpret_cast<MP4Property**>( &userDataName ));
}

///////////////////////////////////////////////////////////////////////////////

void
TrackModifier::Properties::updateProperty( const char* name, MP4Property** pp )
{
    *pp = NULL;
    _trackModifier._track.FindProperty( name, pp );
}

///////////////////////////////////////////////////////////////////////////////

}} // namespace mp4v2::util
