/***************************************************************************
   \fn  ADMedAVIAUD.cpp
   \brief Interface to audio track(s) from editor

    Handle switching from pieces of movie
    Also fix the gap/overlap in audio to offer a strictly continuous audio stream
    
    copyright            : (C) 2008/2009 by mean
    email                : fixounet@free.fr

Todo:
-----
        x Fix handling of overlay/gap, it is wrong.


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
#include "ADM_editor/ADM_edit.hxx"

#include "ADM_debugID.h"
#define MODULE_NAME MODULE_AUDIO_EDITOR
#include "ADM_debug.h"

#include "ADM_vidMisc.h"

#define AUDIOSEG 	_segments[_audioseg]._reference
#define SEG 		_segments[seg]._reference

#define ADM_ALLOWED_DRIFT_US 40000 // Allow 4b0 ms jitter on audio

#if 1
#define vprintf(...) {}
#else
#define vprintf printf
#endif


/**
    \fn     getPCMPacket
    \brief  Get audio packet

*/

bool ADM_Composer::getPCMPacket(float  *dest, uint32_t sizeMax, uint32_t *samples,uint64_t *odts)
{
uint32_t fillerSample=0;   // FIXME : Store & fix the DTS error correctly!!!!
uint32_t inSize;
bool drop=false;
static bool fail=false;

again:
    *samples=0;
    ADM_audioStreamTrack *trk=getTrack(0);
    if(!trk) return false;
    // Do we already have a packet ?
    if(!packetBufferSize)
    {
        if(!refillPacketBuffer())
        {
            if(fail==false)
              ADM_warning("[Editor] Cannot refill audio\n");
            fail=true;
            return false;
        }
    }
    // We do now
    vprintf("[PCMPacket]  Got %d samples, time code %08lu  lastDts=%08lu delta =%08ld\n",packetBufferSamples,packetBufferDts,lastDts,packetBufferDts-lastDts);
    fail=false;

    // Check if the Dts matches
    if(lastDts!=ADM_AUDIO_NO_DTS &&packetBufferDts!=ADM_AUDIO_NO_DTS)
    {
        if(abs(lastDts-packetBufferDts)>ADM_ALLOWED_DRIFT_US)
        {
            printf("[Composer::getPCMPacket] drift %d, computed :%"LLU" got %"LLU"\n",(int)(lastDts-packetBufferDts),lastDts,packetBufferDts);
            if(packetBufferDts<lastDts)
            {
                printf("[Composer::getPCMPacket] Dropping packet %"LU" last =%"LU"\n",(uint32_t)(lastDts/1000),(uint32_t)(packetBufferDts/1000));
                drop=true;
            }else 
            {
                // There is a "hole" in audio
                // Let's add some filler
                // Compute filler size
                float f=packetBufferDts-lastDts; // in us
                f*=wavHeader.frequency;
                f/=1000000.;
                // in samples!
                uint32_t fillerSample=(uint32_t )(f+0.49);
                uint32_t mx=sizeMax/trk->wavheader.channels;
                
                if(mx<fillerSample) fillerSample=mx;
                // arbitrary cap, max 4kSample in one go
                // about 100 ms
                if(fillerSample>4*1024) 
                {
                    fillerSample=4*1024;
                }
                uint32_t start=fillerSample*sizeof(float)*trk->wavheader.channels;
                memset(dest,0,start);

                advanceDtsBySample(fillerSample);
                dest+=fillerSample*trk->wavheader.channels;
                *samples=fillerSample;
                vprintf("[Composer::getPCMPacket] Adding %u padding samples, dts is now %lu\n",fillerSample,lastDts);
                return true;
       }
      }
    }
    // If lastDts is not initialized....
    if(lastDts==ADM_AUDIO_NO_DTS) lastDts=packetBufferDts;
    
    //
    //  The packet is ok, decode it...
    //
    uint32_t nbOut=0; // Nb sample as seen by codec
    if(!trk->codec->run(packetBuffer, packetBufferSize, dest, &nbOut))
    {
            packetBufferSize=0; // consume
            adm_printf(ADM_PRINT_ERROR,"[Composer::getPCMPacket] codec failed failed\n");
            return false;
    }
    packetBufferSize=0; // consume

    // Compute how much decoded sample to compare with what demuxer said
    uint32_t decodedSample=nbOut;
    decodedSample/=trk->wavheader.channels;
    if(!decodedSample) goto again;
#define ADM_MAX_JITTER 5000  // in samples, due to clock accuracy, it can be +er, -er, + er, -er etc etc
    if(abs(decodedSample-packetBufferSamples)>ADM_MAX_JITTER)
    {
        printf("[Composer::getPCMPacket] Demuxer was wrong %d vs %d samples!\n",packetBufferSamples,decodedSample);
    }
    
    // This packet has been dropped (too early packt), try the next one
    if(drop==true)
    {
        // TODO Check if the packet somehow overlaps, i.e. starts too early but finish ok
        goto again;
    }
    // Update infos
    *samples=(decodedSample);
    *odts=lastDts;
    advanceDtsBySample(decodedSample);
    vprintf("[Composer::getPCMPacket] Adding %u decodedSample, dts is not %lu\n",fillerSample,lastDts);
    ADM_assert(sizeMax>=(fillerSample+decodedSample)*trk->wavheader.channels);
    return true;
}


//EOF

