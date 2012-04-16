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
#include "ADM_audioIdentify.h"
#include "ADM_audioAccessFile.h"
/**
    \fn ctor
*/
ADM_edAudioTrackExternal:: ADM_edAudioTrackExternal(const char *file, WAVHeader *hdr,ADM_audioAccess *cess)
:  ADM_edAudioTrack(ADM_EDAUDIO_EXTERNAL,hdr,cess)
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
// TODO FIXME intialize codec etc..
    return true;
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
    #define EXTERNAL_PROBE_SIZE (1024*1024)
    // Identify file type
    uint8_t buffer[EXTERNAL_PROBE_SIZE];
    FILE *f=ADM_fopen(name,"rb");
    if(!f)
    {
        ADM_warning("Cannot open %s\n",name);
        return NULL;
    }
    fread(buffer,1,EXTERNAL_PROBE_SIZE,f);
    fclose(f);
    WAVHeader hdr;
    if(false==ADM_identifyAudioStream(EXTERNAL_PROBE_SIZE,buffer,hdr))
    {
        ADM_warning("Cannot identify external audio track\n");
        return NULL;
    }
    // Try to create an access for the file...
    switch(hdr.encoding)
    {
        case WAV_PCM:
        case WAV_AC3:
        case WAV_MP3:
                break;
        default:
                ADM_warning("Unsupported external audio tracks \n");
                return NULL;
                break;
    }
    // create access
    ADM_audioAccessFile *access=new ADM_audioAccessFile(name);
    // create ADM_edAudioTrack
    ADM_edAudioTrackExternal *external=new ADM_edAudioTrackExternal(name, &hdr,access);
    if(!external->create())
    {
        delete external;
        external=NULL;
        ADM_warning("Failed to create external track from %s\n",name);
        return NULL;
    }
    // done!
    return external;
}

// EOF
