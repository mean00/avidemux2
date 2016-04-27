
/***************************************************************************
    \file  ADM_edAudioTrackExternal
    \brief Add an external audio track to the pool

    (c) 2012 Mean, fixounet@free.Fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_EDAUDIOTRACKEXTERNAL
#define ADM_EDAUDIOTRACKEXTERNAL

#include <string>
#include "ADM_edAudioTrack.h"

/**
    \class ADM_edAudioTrackExternal
*/
class ADM_edAudioTrackExternal : public ADM_edAudioTrack
{
protected:
                    ADM_audioStream  *internalAudioStream;
                    ADM_audioAccess  *internalAccess;
                    std::string  sourceFile;
public:
                                 ADM_edAudioTrackExternal(const char *file, WAVHeader *hdr,ADM_audioAccess *ccess);
            virtual            ~ADM_edAudioTrackExternal();
                    bool        destroyable() {return true;};
                    bool        create(uint32_t extraLen, uint8_t *extraData);

                    std::string   &getMyName() {return sourceFile; }
// We need the codec etc for external audio tracks
                    ADM_Audiocodec   *codec;
                    bool             vbr;
                    uint64_t        duration;
                    uint64_t        size;
                    bool             refillPacketBuffer(void);
//
                    CHANNEL_TYPE    *getChannelMapping(void );
                    uint32_t        getOutputFrequency(void); // sbr
            virtual bool            getPCMPacket(float  *dest, uint32_t sizeMax, uint32_t *samples,uint64_t *odts);

virtual ADM_edAudioTrackExternal *castToExternal(void) {return this;}

virtual uint8_t         getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts)
                          {
                                return internalAudioStream->getPacket(buffer,size,sizeMax,nbSample,dts);
                            }
virtual bool            goToTime(uint64_t nbUs);
virtual bool            getExtraData(uint32_t *l, uint8_t **d){return internalAudioStream->getExtraData(l,d);}
         uint64_t       getDurationInUs(void) {return internalAudioStream->getDurationInUs();}

};
/// spawn
ADM_edAudioTrackExternal *create_edAudioExternal(const char *name);

#endif
//EOF
