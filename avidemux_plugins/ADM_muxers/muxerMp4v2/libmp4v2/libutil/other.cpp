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
    using namespace mp4v2::impl;

///////////////////////////////////////////////////////////////////////////////

// search atom recursively for any 64-bit characteristics.
// nlargsize indicates number of atoms which use largesize extension.
// nversion1 indicates number of atoms which use version==1 extension.
// nspecial indicates number of special 64-bit atoms;
//   eg: stbl may container one of { stco || co64 } for chunkoffsets.

void
searchFor64bit( MP4Atom& atom, FileSummaryInfo& info )
{
    const uint32_t max = atom.GetNumberOfChildAtoms();
    for( uint32_t i = 0; i < max; i++ ) {
        MP4Atom& child = *atom.GetChildAtom( i );

        if( child.GetLargesizeMode() )
            info.nlargesize++;

        MP4Integer8Property* version;
        if( child.FindProperty( "version", (MP4Property**)&version ) && version->GetValue() == 1 )
            info.nversion1++;

        if( !strcmp( child.GetType(), "co64" ))
            info.nspecial++;

        searchFor64bit( child, info );
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
fileFetchSummaryInfo( MP4FileHandle file, FileSummaryInfo& info )
{
    if( file == MP4_INVALID_FILE_HANDLE )
        return true;
    MP4File& mp4 = *((MP4File*)file);

    MP4Atom* root = mp4.FindAtom( "" );
    if( !root )
        return true;

    MP4FtypAtom* ftyp = (MP4FtypAtom*)root->FindAtom( "ftyp" );
    if( !ftyp )
        return true;

    info.major_brand   = ftyp->majorBrand.GetValue();
    info.minor_version = ftyp->minorVersion.GetValue();

    const uint32_t cbmax = ftyp->compatibleBrands.GetCount();
    for( uint32_t i = 0; i < cbmax; i++ ) {
        string s = ftyp->compatibleBrands.GetValue( i );

        // remove spaces so brand set is presentable
        string stripped;
        const string::size_type max = s.length();
        for( string::size_type pos = 0; pos < max; pos++ ) {
            if( s[pos] != ' ' )
                stripped += s[pos];
        }

        if( stripped.empty() )
            continue;

        info.compatible_brands.insert( stripped );
    }

    info.nlargesize = 0;
    info.nversion1  = 0;
    info.nspecial   = 0;
    searchFor64bit( *root, info );

    return false;
}

///////////////////////////////////////////////////////////////////////////////

}} // namespace mp4v2::util
