/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is MPEG4IP.
 *
 * The Initial Developer of the Original Code is Cisco Systems Inc.
 * Portions created by Cisco Systems Inc. are
 * Copyright (C) Cisco Systems Inc. 2001 - 2005.  All Rights Reserved.
 *
 * 3GPP features implementation is based on 3GPP's TS26.234-v5.60,
 * and was contributed by Ximpo Group Ltd.
 *
 * Portions created by Ximpo Group Ltd. are
 * Copyright (C) Ximpo Group Ltd. 2003, 2004.  All Rights Reserved.
 *
 * Contributor(s):
 *      Dave Mackie                dmackie@cisco.com
 *      Alix Marchandise-Franquet  alix@cisco.com
 *      Ximpo Group Ltd.           mp4v2@ximpo.com
 *      Bill May                   wmay@cisco.com
 *      Rouven Wessling            mp4v2@rouvenwessling.de
 */

/*
 * MP4 library API functions
 *
 * These are wrapper functions that provide C linkage conventions
 * to the library, and catch any internal errors, ensuring that
 * a proper return value is given.
 */

#include "src/impl.h"

#define PRINT_ERROR(e) \
    VERBOSE_ERROR(((MP4File*)hFile)->GetVerbosity(), e->Print());

using namespace mp4v2::impl;

extern "C" {

///////////////////////////////////////////////////////////////////////////////

bool MP4GetMetadataByIndex(MP4FileHandle hFile, uint32_t index,
                           char** ppName,
                           uint8_t** ppValue, uint32_t* pValueSize)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataByIndex(
                       index, ppName, ppValue, pValueSize);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4MetadataDelete(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->MetadataDelete();
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataName(MP4FileHandle hFile,
                        const char* value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataString("\251nam", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataName(MP4FileHandle hFile,
                        char** value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataString("\251nam", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataName(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("\251nam");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataWriter(MP4FileHandle hFile,
                          const char* value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataString("\251wrt", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataWriter(MP4FileHandle hFile,
                          char** value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataString("\251wrt", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataWriter(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("\251wrt");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataAlbum(MP4FileHandle hFile,
                         const char* value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataString("\251alb", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataAlbum(MP4FileHandle hFile,
                         char** value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataString("\251alb", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataAlbum(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("\251alb");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataArtist(MP4FileHandle hFile,
                          const char* value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataString("\251ART", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataArtist(MP4FileHandle hFile,
                          char** value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataString("\251ART", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataArtist(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("\251ART");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataTool(MP4FileHandle hFile,
                        const char* value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataString("\251too", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataTool(MP4FileHandle hFile,
                        char** value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataString("\251too", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataTool(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("\251too");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataComment(MP4FileHandle hFile,
                           const char* value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataString("\251cmt", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataComment(MP4FileHandle hFile,
                           char** value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataString("\251cmt", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataComment(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("\251cmt");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataYear(MP4FileHandle hFile,
                        const char* value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataString("\251day", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataYear(MP4FileHandle hFile,
                        char** value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataString("\251day", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataYear(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("\251day");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataTrack(MP4FileHandle hFile,
                         uint16_t track, uint16_t totalTracks)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataTrack(track, totalTracks);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataTrack(MP4FileHandle hFile,
                         uint16_t* track, uint16_t* totalTracks)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataTrack(track, totalTracks);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataTrack(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("trkn");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataDisk(MP4FileHandle hFile,
                        uint16_t disk, uint16_t totalDisks)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataDisk(disk, totalDisks);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataDisk(MP4FileHandle hFile,
                        uint16_t* disk, uint16_t* totalDisks)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataDisk(disk, totalDisks);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataDisk(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("disk");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataGenre(MP4FileHandle hFile, const char *genre)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataGenre(genre);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataGenre(MP4FileHandle hFile, char **genre)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataGenre(genre);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataGenre(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataGenre();
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataGrouping(MP4FileHandle hFile, const char *grouping)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataString("\251grp", grouping);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataGrouping(MP4FileHandle hFile, char **grouping)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataString("\251grp", grouping);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataGrouping(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("\251grp");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataTempo(MP4FileHandle hFile, uint16_t tempo)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataUint16("tmpo", tempo);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataTempo(MP4FileHandle hFile, uint16_t* tempo)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataUint16("tmpo", tempo);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataTempo(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("tmpo");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataCompilation(MP4FileHandle hFile, uint8_t cpl)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataUint8("cpil", cpl & 0x1);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataCompilation(MP4FileHandle hFile, uint8_t* cpl)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataUint8("cpil", cpl);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataCompilation(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("cpil");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataPartOfGaplessAlbum (MP4FileHandle hFile,
                                       uint8_t pgap)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataUint8("pgap", pgap & 0x1);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataPartOfGaplessAlbum (MP4FileHandle hFile,
                                       uint8_t* pgap)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataUint8("pgap", pgap);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataPartOfGaplessAlbum (MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("pgap");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataCoverArt(MP4FileHandle hFile,
                            uint8_t *coverArt, uint32_t size)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataCoverArt(coverArt, size);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataCoverArt(MP4FileHandle hFile,
                            uint8_t **coverArt, uint32_t* size,
                            uint32_t index)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataCoverArt(coverArt, size, index);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

uint32_t MP4GetMetadataCoverArtCount(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataCoverArtCount();
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataCoverArt(MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("covr");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataAlbumArtist (MP4FileHandle hFile,
                                const char* value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataString("aART", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataAlbumArtist (MP4FileHandle hFile,
                                char** value)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataString("aART", value);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataAlbumArtist (MP4FileHandle hFile)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataAtom("aART");
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4SetMetadataFreeForm(MP4FileHandle hFile,
                            const char *name,
                            const uint8_t* pValue,
                            uint32_t valueSize,
                            const char *owner)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->SetMetadataFreeForm(name, pValue, valueSize, owner);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4GetMetadataFreeForm(MP4FileHandle hFile, const char *name,
                            uint8_t** pValue, uint32_t* valueSize, const char *owner)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->GetMetadataFreeForm(name, pValue, valueSize, owner);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

bool MP4DeleteMetadataFreeForm(MP4FileHandle hFile, const char *name, const char *owner)
{
    if (MP4_IS_VALID_FILE_HANDLE(hFile)) {
        try {
            return ((MP4File*)hFile)->DeleteMetadataFreeForm(name, owner);
        }
        catch (MP4Error* e) {
            PRINT_ERROR(e);
            delete e;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void
MP4TagsAddArtwork( const MP4Tags* tags, MP4TagArtwork* artwork )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(tags->__handle);
    MP4Tags* c = const_cast<MP4Tags*>(tags);
    cpp.c_addArtwork( c, *artwork );
}

///////////////////////////////////////////////////////////////////////////////

const MP4Tags*
MP4TagsAlloc()
{
    MP4Tags* result = NULL;
    itmf::Tags& m = *new itmf::Tags();
    m.c_alloc( result );
    return result;
}

///////////////////////////////////////////////////////////////////////////////

void
MP4TagsFree( const MP4Tags* tags )
{
    itmf::Tags* cpp = static_cast<itmf::Tags*>(tags->__handle);
    MP4Tags* c = const_cast<MP4Tags*>(tags);
    cpp->c_free( c );
    delete cpp;
}

///////////////////////////////////////////////////////////////////////////////

void
MP4TagsFetch( const MP4Tags* tags, MP4FileHandle hFile )
{
    if( !MP4_IS_VALID_FILE_HANDLE( hFile ))
        return;

    itmf::Tags* cpp = static_cast<itmf::Tags*>(tags->__handle);
    MP4Tags* c = const_cast<MP4Tags*>(tags);

    try {
        cpp->c_fetch( c, hFile );
    }
    catch( MP4Error* e ) {
        VERBOSE_ERROR( static_cast<MP4File*>(hFile)->GetVerbosity(), e->Print() );
        delete e;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
MP4TagsRemoveArtwork( const MP4Tags* tags, uint32_t index )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(tags->__handle);
    MP4Tags* c = const_cast<MP4Tags*>(tags);
    cpp.c_removeArtwork( c, index );
}

///////////////////////////////////////////////////////////////////////////////

void
MP4TagsSetArtwork( const MP4Tags* tags, uint32_t index, MP4TagArtwork* artwork )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(tags->__handle);
    MP4Tags* c = const_cast<MP4Tags*>(tags);
    cpp.c_setArtwork( c, index, *artwork );
}

///////////////////////////////////////////////////////////////////////////////

void
MP4TagsStore( const MP4Tags* tags, MP4FileHandle hFile )
{
    itmf::Tags* cpp = static_cast<itmf::Tags*>(tags->__handle);
    MP4Tags* c = const_cast<MP4Tags*>(tags);

    try {
        cpp->c_store( c, hFile );
    }
    catch( MP4Error* e ) {
        VERBOSE_ERROR( static_cast<MP4File*>(hFile)->GetVerbosity(), e->Print() );
        delete e;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
MP4TagsSetName( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.name, c.name );
}

void
MP4TagsSetArtist( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.artist, c.artist );
}

void
MP4TagsSetAlbumArtist( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.albumArtist, c.albumArtist );
}

void
MP4TagsSetAlbum( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.album, c.album );
}

void
MP4TagsSetGrouping( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.grouping, c.grouping );
}

void
MP4TagsSetComposer( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.composer, c.composer );
}

void
MP4TagsSetComments( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.comments, c.comments );
}

void
MP4TagsSetGenre( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.genre, c.genre );
}

void
MP4TagsSetGenreType( const MP4Tags* m, const uint16_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.genreType, c.genreType );
}

void
MP4TagsSetReleaseDate( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.releaseDate, c.releaseDate );
}

void
MP4TagsSetTrack( const MP4Tags* m, const MP4TagTrack* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setTrack( value, cpp.track, c.track );
}

void
MP4TagsSetDisk( const MP4Tags* m, const MP4TagDisk* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setDisk( value, cpp.disk, c.disk );
}

void
MP4TagsSetTempo( const MP4Tags* m, const uint16_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.tempo, c.tempo );
}

void
MP4TagsSetCompilation( const MP4Tags* m, const uint8_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.compilation, c.compilation );
}

void
MP4TagsSetTVShow( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.tvShow, c.tvShow );
}

void
MP4TagsSetTVNetwork( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.tvNetwork, c.tvNetwork );
}

void
MP4TagsSetTVEpisodeID( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.tvEpisodeID, c.tvEpisodeID );
}

void
MP4TagsSetTVSeason( const MP4Tags* m, const uint32_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.tvSeason, c.tvSeason );
}

void
MP4TagsSetTVEpisode( const MP4Tags* m, const uint32_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.tvEpisode, c.tvEpisode );
}

void
MP4TagsSetSortName( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.sortName, c.sortName );
}

void
MP4TagsSetSortArtist( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.sortArtist, c.sortArtist );
}

void
MP4TagsSetSortAlbumArtist( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.sortAlbumArtist, c.sortAlbumArtist );
}

void
MP4TagsSetSortAlbum( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.sortAlbum, c.sortAlbum );
}

void
MP4TagsSetSortComposer( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.sortComposer, c.sortComposer );
}

void
MP4TagsSetSortTVShow( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.sortTVShow, c.sortTVShow );
}

void
MP4TagsSetDescription( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.description, c.description );
}

void
MP4TagsSetLongDescription( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.longDescription, c.longDescription );
}

void
MP4TagsSetLyrics( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.lyrics, c.lyrics );
}

void
MP4TagsSetCopyright( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.copyright, c.copyright );
}

void
MP4TagsSetEncodingTool( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.encodingTool, c.encodingTool );
}

void
MP4TagsSetEncodedBy( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.encodedBy, c.encodedBy );
}

void
MP4TagsSetPurchaseDate( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.purchaseDate, c.purchaseDate );
}

void
MP4TagsSetPodcast( const MP4Tags* m, const uint8_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.podcast, c.podcast );
}

void
MP4TagsSetKeywords( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.keywords, c.keywords );
}

void
MP4TagsSetCategory( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.category, c.category );
}

void
MP4TagsSetHDVideo( const MP4Tags* m, const uint8_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.hdVideo, c.hdVideo );
}

void
MP4TagsSetMediaType( const MP4Tags* m, const uint8_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.mediaType, c.mediaType );
}

void
MP4TagsSetContentRating( const MP4Tags* m, const uint8_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.contentRating, c.contentRating );
}

void
MP4TagsSetGapless( const MP4Tags* m, const uint8_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.gapless, c.gapless );
}

void
MP4TagsSetITunesAccount( const MP4Tags* m, const char* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setString( value, cpp.iTunesAccount, c.iTunesAccount );
}

void
MP4TagsSetITunesAccountType( const MP4Tags* m, const uint8_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.iTunesAccountType, c.iTunesAccountType );
}

void
MP4TagsSetITunesCountry( const MP4Tags* m, const uint32_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.iTunesCountry, c.iTunesCountry );
}

void
MP4TagsSetCNID( const MP4Tags* m, const uint32_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.cnID, c.cnID );
}

void
MP4TagsSetATID( const MP4Tags* m, const uint32_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.atID, c.atID );
}

void
MP4TagsSetPLID( const MP4Tags* m, const uint64_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.plID, c.plID );
}

void
MP4TagsSetGEID( const MP4Tags* m, const uint32_t* value )
{
    itmf::Tags& cpp = *static_cast<itmf::Tags*>(m->__handle);
    MP4Tags& c = *const_cast<MP4Tags*>(m);
    cpp.c_setInteger( value, cpp.geID, c.geID );
}

///////////////////////////////////////////////////////////////////////////////

MP4ItmfItem*
MP4ItmfItemAlloc( const char* code, uint32_t numData )
{
    return itmf::genericItemAlloc( code, numData );
}

///////////////////////////////////////////////////////////////////////////////

void
MP4ItmfItemFree( MP4ItmfItem* item )
{
    itmf::genericItemFree( item );
}

///////////////////////////////////////////////////////////////////////////////

void
MP4ItmfItemListFree( MP4ItmfItemList* list )
{
    itmf::genericItemListFree( list );
}

///////////////////////////////////////////////////////////////////////////////

MP4ItmfItemList*
MP4ItmfGetItems( MP4FileHandle hFile )
{
    if( !MP4_IS_VALID_FILE_HANDLE( hFile ))
        return NULL;

    try {
        return itmf::genericGetItems( *(MP4File*)hFile );
    }
    catch( MP4Error* e ) {
        PRINT_ERROR( e );
        delete e;
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

MP4ItmfItemList*
MP4ItmfGetItemsByCode( MP4FileHandle hFile, const char* code )
{
    if( !MP4_IS_VALID_FILE_HANDLE( hFile ))
        return NULL;

    try {
        return itmf::genericGetItemsByCode( *(MP4File*)hFile, code );
    }   
    catch( MP4Error* e ) {
        PRINT_ERROR( e );
        delete e;
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

MP4ItmfItemList*
MP4ItmfGetItemsByMeaning( MP4FileHandle hFile, const char* meaning, const char* name )
{
    if( !MP4_IS_VALID_FILE_HANDLE( hFile ))
        return NULL;

    try {
        return itmf::genericGetItemsByMeaning( *(MP4File*)hFile, meaning, name ? name : "" );
    }
    catch( MP4Error* e ) {
        PRINT_ERROR( e );
        delete e;
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

bool
MP4ItmfAddItem( MP4FileHandle hFile, const MP4ItmfItem* item )
{
    if( !MP4_IS_VALID_FILE_HANDLE( hFile ))
        return NULL;

    try {
        return itmf::genericAddItem( *(MP4File*)hFile, item );
    }
    catch( MP4Error* e ) {
        PRINT_ERROR( e );
        delete e;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
MP4ItmfSetItem( MP4FileHandle hFile, const MP4ItmfItem* item )
{
    if( !MP4_IS_VALID_FILE_HANDLE( hFile ))
        return NULL;

    try {
        return itmf::genericSetItem( *(MP4File*)hFile, item );
    }
    catch( MP4Error* e ) {
        PRINT_ERROR( e );
        delete e;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
MP4ItmfRemoveItem( MP4FileHandle hFile, const MP4ItmfItem* item )
{
    if( !MP4_IS_VALID_FILE_HANDLE( hFile ))
        return NULL;

    try {
        return itmf::genericRemoveItem( *(MP4File*)hFile, item );
    }
    catch( MP4Error* e ) {
        PRINT_ERROR( e );
        delete e;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

} // extern "C"
