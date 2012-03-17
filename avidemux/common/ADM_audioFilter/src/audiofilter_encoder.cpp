/***************************************************************************
            \file audiofilter_encoder.cpp
            \brief Generate a access class = to the output of encoder + filterchain
              (c) 2006 Mean , fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ADM_cpp.h"
using std::string;
#include "ADM_default.h"
#include <math.h>

#include "audiofilter_bridge.h"
#include "audiofilter_access.h"
#include "audiofilter_internal.h"
#include "audiofilter_conf.h"
#include "audioencoder.h"
#include "audioEncoderApi.h"
#include "ADM_vidMisc.h"
extern ADM_Composer *video_body;
//
 extern bool ADM_buildFilterChain(VectorOfAudioFilter *vec,ADM_AUDIOFILTER_CONFIG *config);
 extern bool ADM_emptyFilterChain(VectorOfAudioFilter *vec);
/**
    \class ADM_audioStream_autoDelete
*/
class ADM_audioStream_autoDelete: public ADM_audioStream
{
    protected:
                        ADM_audioStream *son;
                        ADM_audioAccess *access;
        public:
                        ADM_audioStream_autoDelete(ADM_audioStream *s,ADM_audioAccess *access)
                        :    ADM_audioStream(s->getInfo(),access)
                        {
                                this->access=access;
                                son=s;
                        }
                        virtual ~ADM_audioStream_autoDelete()
                        {
                                ADM_info("Killing son audioStream\n");
                                delete son;
                                son=NULL;
                                ADM_info("Killing son audioAccess\n");
                                delete access;
                                access=NULL;
                        }
virtual                 WAVHeader                *getInfo(void) {return son->getInfo();}
virtual uint8_t         getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts)
                        {
                                return son->getPacket(buffer,size,sizeMax,nbSample,dts);
                        }
virtual bool            goToTime(uint64_t nbUs) {return son->goToTime(nbUs);};
virtual bool            getExtraData(uint32_t *l, uint8_t **d) {return son->getExtraData(l,d);};
        uint64_t        getDurationInUs(void) {return son->getDurationInUs();}
};
//************************************************
extern ADM_audioAccess *ADM_threadifyAudioAccess(ADM_audioAccess *son);
/**
        \fn createPlaybackFilter
        \brief Create a float output filter for playback
        @param StartTime in us
        @param shift in ms
*/
AUDMAudioFilter *createEncodingFilter(EditableAudioTrack *ed, uint64_t startTime,int32_t shift)
{
    //
    if(!ed) return NULL;
    ADM_info("Creating audio encoding filter with start time %s\n",ADM_us2plain(startTime));
    ed->audioEncodingConfig.startTimeInUs=startTime;
    ed->audioEncodingConfig.shiftInMs=shift;
    //
    ADM_buildFilterChain(&(ed->EncodingVector),&( ed->audioEncodingConfig));
    //
    int last=ed->EncodingVector.size();
    ADM_assert(last);
    return ed->EncodingVector[last-1];
}
/**
        \fn destroyPlaybackFilter
        \brief Destroy a float output filter for playback
*/

bool            destroyEncodingFilter(EditableAudioTrack *ed)
{

    ADM_emptyFilterChain(&(ed->EncodingVector));
    return true;

}
/**
    \fn createEncodingAccess
*/
ADM_audioStream *audioCreateEncodingStream(EditableAudioTrack *ed, bool globalHeader,uint64_t startTime,int32_t shift)
{
    if(!ed)
    {
        return NULL;
    }
    printf("[AccessFilter] Creating access filter, startime %s, globalHeader %d\n",ADM_us2plain(startTime),globalHeader);
    // 1-Create access filter
    AUDMAudioFilter *filter=createEncodingFilter(ed, startTime,shift);
    if(!filter)
    {
        printf("[Access] Cannot create audio filter\n");
        return NULL;
    }

    // 2- spawn encoder
    ADM_AudioEncoder *encoder=audioEncoderCreate(filter,globalHeader);
    if(!encoder) 
    {
        printf("[Access] Cannot create audio encoder\n");
        destroyEncodingFilter(ed);
        return NULL;
    }
    if(true!=encoder->initialize())
    {
        printf("[Access] Encoder initialization failed\n");
        delete encoder;
        destroyEncodingFilter(ed);
        return NULL;
    }
    // 3- Create access
    ADMAudioFilter_Access *access=new ADMAudioFilter_Access(filter,encoder,0);
    if(!access)
    {
        printf("[Access] Cannot create access\n");
        delete encoder;
        destroyEncodingFilter(ed);
        return NULL;
    }
    // 3b create threaded version
    ADM_audioAccess *threaded=ADM_threadifyAudioAccess(access);
    // 4- Create Stream 
    ADM_audioStream *stream=ADM_audioCreateStream(encoder->getInfo(), threaded,false); // No map, it is not seekable
    if(!stream)
    {
        printf("[Access] Cannot create stream\n");
        delete threaded; // Access will destroy filter & encoder
        return NULL;
    }
    ADM_audioStream_autoDelete *autoDelete=new ADM_audioStream_autoDelete(stream, threaded);
    return autoDelete;
}
// EOF
