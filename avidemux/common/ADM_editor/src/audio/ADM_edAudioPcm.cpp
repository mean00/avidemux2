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
#include "ADM_edit.hxx"
#include "ADM_edAudioTrackFromVideo.h"
#include "ADM_vidMisc.h"

#ifdef _MSC_VER
#define abs(x) _abs64(x)
#endif

#if 1
#define vprintf(...) {}
#else
#define vprintf printf
#endif
/**
    \fn     getOutputFrequency
    \brief  Get output fq, 2*wavheader->fq for sbr

*/

uint32_t        ADM_edAudioTrackFromVideo::getOutputFrequency(void)
{
    ADM_audioStreamTrack *trk=getCurrentTrack();
    if(!trk) 
    {
        ADM_warning("No audio track\n");
        return 0;
    }
    if(!trk->codec || trk->codec->configurationChanged())
        return trk->wavheader.frequency;
    return trk->codec->getOutputFrequency();
}

/**
    \fn getOutputChannels
    \brief get nb of channels as seen by codec
*/
uint32_t ADM_edAudioTrackFromVideo::getOutputChannels(void)
{
    ADM_audioStreamTrack *trk=getCurrentTrack();
    if(!trk)
    {
        ADM_warning("No audio track\n");
        return 0;
    }
    if(!trk->codec || trk->codec->configurationChanged())
        return trk->wavheader.channels;
    return trk->codec->getOutputChannels();
}

/**
    \fn     getPCMPacket
    \brief  Get audio packet

*/

bool ADM_edAudioTrackFromVideo::getPCMPacket(float  *dest, uint32_t sizeMax, uint32_t *samples,uint64_t *odts)
{
uint32_t fillerSample=0;   // FIXME : Store & fix the DTS error correctly!!!!
bool drop=false;
bool checkDts;
static bool fail=false;
uint32_t outFrequency,outChannels;

 vprintf("[PCMPacket]  request TRK %d:%x\n",myTrackNumber,(long int)getCurrentTrack());
again:
    *samples=0;
    ADM_audioStreamTrack *trk=getCurrentTrack();
    if(!trk) return false;
    if(trk->codec->isDummy()) return false;

    outChannels=getOutputChannels();
    if(!outChannels) return false;

    outFrequency=getOutputFrequency();
    if(!outFrequency) return false;

    checkDts=(trk->stream->constantSamplesPerPacket() && !trk->isbr);
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
    vprintf("[PCMPacket]  TRK %d Got %d samples, time code %08" PRIu64"  lastDts=%08" PRIu64" delta =%" PRId64"\n",
                myTrackNumber,packetBufferSamples,packetBufferDts,lastDts,packetBufferDts-lastDts);
    fail=false;

    // Check if the Dts matches
    if(checkDts && lastDts!=ADM_AUDIO_NO_DTS &&packetBufferDts!=ADM_AUDIO_NO_DTS)
    {
        if(labs((int64_t)lastDts-(int64_t)packetBufferDts)>ADM_ALLOWED_DRIFT_US)
        {
            printf("[Composer::getPCMPacket] Track %d, %p ", (int)myTrackNumber,trk);
            printf(": drift %d, computed : %s", (int)(lastDts-packetBufferDts),ADM_us2plain(lastDts));
            printf(" got %s\n", ADM_us2plain(packetBufferDts));
            if(packetBufferDts<lastDts)
            {
                printf("[Composer::getPCMPacket] Track %d:%" PRIx64" : Dropping packet %" PRIu32" last =%" PRIu32"\n",myTrackNumber,(uint64_t)trk,(uint32_t)(lastDts/1000),(uint32_t)(packetBufferDts/1000));
                drop=true;
            }else 
            {
                // There is a "hole" in audio
                // Let's add some filler
                // Compute filler size
                *odts=lastDts;
                float f=packetBufferDts-lastDts; // in us
                f*=outFrequency;
                f/=1000000.;
                // in samples!
                uint32_t fillerSample=(uint32_t )(f+0.49);
                uint32_t mx=sizeMax/outChannels;
                
                if(mx<fillerSample) fillerSample=mx;
                // arbitrary cap, max 4kSample in one go
                // about 100 ms
                if(fillerSample>4*1024) 
                {
                    fillerSample=4*1024;
                }
                uint32_t start=fillerSample*sizeof(float)*outChannels;
                memset(dest,0,start);

                advanceDtsByCustomSample(fillerSample,outFrequency);
                dest+=fillerSample*outChannels;
                *samples=fillerSample;
                vprintf("[Composer::getPCMPacket] Track %d:%x  Adding %u padding samples, dts is now %lu\n",
                            myTrackNumber,(long  int)trk,fillerSample,lastDts);
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
            ADM_warning("Track %d at 0x%p : decoding failed\n",myTrackNumber,trk);
            return false;
    }
    packetBufferSize=0; // consume

    // Compute how much decoded sample to compare with what demuxer said
    uint32_t decodedSample=nbOut;
    decodedSample/=outChannels;
    if(!decodedSample) goto again;
#define ADM_MAX_JITTER 5000  // in samples, due to clock accuracy, it can be +er, -er, + er, -er etc etc
    if(checkDts && labs((int64_t)decodedSample-(int64_t)packetBufferSamples)>ADM_MAX_JITTER)
    {
        ADM_warning("[Composer::getPCMPacket] Track %d:%x Demuxer was wrong %d vs %d samples!\n",
                    myTrackNumber,trk,packetBufferSamples,decodedSample);
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
    advanceDtsByCustomSample(decodedSample,outFrequency);
    vprintf("[Composer::getPCMPacket] Track %d:%p Adding %u decoded, Adding %u filler sample, dts is now %" PRIu64", fq: %d\n",
                    myTrackNumber,trk,decodedSample,fillerSample,lastDts,outFrequency);
    ADM_assert(sizeMax>=(fillerSample+decodedSample)*outChannels);
    return true;
}


//EOF

