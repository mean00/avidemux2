/***************************************************************************
    \file  ADM_edAudioTrackExternal
    \brief Manage audio track(s) coming from an external file
    \author (c) 2012 Mean, fixounet@free.Fr 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <string.h>
#include "ADM_cpp.h"
#include "ADM_default.h"
#include <math.h>


#include "fourcc.h"
#include "ADM_edit.hxx"
#include "ADM_edAudioTrackExternal.h"
/**
    \fn ctor
*/
ADM_edAudioTrackExternal:: ADM_edAudioTrackExternal(const char *file, ADM_audioAccess *cess)
:  ADM_edAudioTrack(ADM_EDAUDIO_EXTERNAL,cess)
{
    ADM_info("Creating edAudio from external file %s\n",file);
    sourceFile=std::string(file);
}
/**
    \fn dtor
*/
ADM_edAudioTrackExternal::~ADM_edAudioTrackExternal()
{
    ADM_info("Destroying edAudio from external %s \n",sourceFile.c_str());
    
}
/**
    \fn create
    \brief actually create the track, can fail
*/
bool ADM_edAudioTrackExternal::create(void)
{
    ADM_info("Initializing audio track from external %s \n",sourceFile.c_str());
    return false;
}
/**
    \fn getChannelMapping
*/
CHANNEL_TYPE * ADM_edAudioTrackExternal::getChannelMapping(void )
{
    return NULL;
}
/**
    \fn getPCMPacket
*/
bool         ADM_edAudioTrackExternal::getPCMPacket(float  *dest, uint32_t sizeMax, uint32_t *samples,uint64_t *odts)
{
    return false;
}
/**
    \fn getOutputFrequency
*/          
uint32_t    ADM_edAudioTrackExternal::getOutputFrequency(void)
{
    return wavHeader.frequency;
}
/**
    \fn create_edAudioExternal
*/
ADM_edAudioTrackExternal *create_edAudioExternal(const char *name)
{
    // Identify file type

    // Try to create an access for the file...

    // create ADM_edAudioTrack

    // done!
    return NULL;
}

// EOF
