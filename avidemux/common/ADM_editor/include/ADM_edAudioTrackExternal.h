
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

#include "ADM_edAudioTrack.h"

/**
    \class ADM_edAudioTrackExternal
*/
class ADM_edAudioTrackExternal : public ADM_edAudioTrack
{
protected:
                    std::string  sourceFile;
public:
                                 ADM_edAudioTrackExternal(const char *file, ADM_audioAccess *ccess);
            virtual            ~ADM_edAudioTrackExternal();
                    bool        destroyable() {return true;};
                    bool        create(void);
           virtual bool        destroyable(){return true;}
                    std::string   &getMyName() {return sourceFile; }

                    CHANNEL_TYPE *getChannelMapping(void );
                    uint32_t     getOutputFrequency(void); // sbr
            virtual bool         getPCMPacket(float  *dest, uint32_t sizeMax, uint32_t *samples,uint64_t *odts);
virtual ADM_edAudioTrackExternal *castToTrackExternal(void) {return this;}


};

ADM_edAudioTrackExternal *create_edAudioExternal(const char *name);

#endif
//EOF
