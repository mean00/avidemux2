/***************************************************************************
    
    copyright            : (C) 2007 by mean
    email                : fixounet@free.fr
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
#include <math.h>
#include <map>

#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"
#include "ADM_mp4.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif


#define MAX_CHUNK_SIZE (4*1024)
#define AUDIO_PACKET_BUFFER_SIZE (64*1024)

static uint32_t sample2byte(WAVHeader *hdr,uint32_t sample);

/**
 * \fn splitAudio
 * \brief Split audio chunks into small enough pieces
 * @param track
 * @param vinfo
 * @return 
 */
bool MP4Header::splitAudio(MP4Track *track,MPsampleinfo *info, uint32_t trackScale)
{
    uint64_t maxChunkSize=(MAX_CHUNK_SIZE>>5)<<5;
    // DTS packet can be up to 1064960 bytes large and cannot be split
    if(track->_rdWav.encoding == WAV_DTS)
        maxChunkSize=AUDIO_PACKET_BUFFER_SIZE;
    if((track->_rdWav.encoding == WAV_PCM || track->_rdWav.encoding == WAV_LPCM) && info->bytePerPacket > 1)
    {
        uint64_t remainder = maxChunkSize % (info->bytePerPacket * track->_rdWav.channels);
        maxChunkSize -= remainder;
        ADM_info("Setting max chunk size to %" PRIu64"\n",maxChunkSize);
    }
    // Probe if it is needed
    uint64_t sizeOfAudio=0;
    uint64_t sz,largestBlockSize=0;
    uint32_t i,extra=0;
    uint32_t nbBlocksToSplit=0;
    uint32_t largestBlockNb=-1;
    for(i=0;i<track->nbIndex;i++)
    {
        sz=track->index[i].size;
        if(track->_rdWav.encoding == WAV_DTS && sz > AUDIO_PACKET_BUFFER_SIZE)
        {
            ADM_warning("DTS packet size %llu too big, rejecting track.\n",sz);
            return false;
        }
        if(sz > largestBlockSize)
        {
            largestBlockNb=i;
            largestBlockSize=sz;
        }
        uint32_t x = sz ? (sz-1)/maxChunkSize : 0;
        if(x) nbBlocksToSplit++;
        extra+=x;
        sizeOfAudio+=sz;
    }
    ADM_info("The largest block is %llu bytes in size at index %d out of %u\n",largestBlockSize,largestBlockNb,track->nbIndex);
    if(!extra)
    {
        ADM_info("No very large blocks found, %llu bytes present over %d blocks\n",sizeOfAudio,track->nbIndex);
        return true;
    }
    ADM_info("%u large blocks found, splitting into %u %llu bytes blocks\n",nbBlocksToSplit,nbBlocksToSplit+extra,maxChunkSize);

    uint32_t newNbCo=track->nbIndex+extra;
    MP4Index *newindex=new MP4Index[newNbCo];
    int w=0;

    for(i=0;i<track->nbIndex;i++)
    {
        sz=track->index[i].size;
        if(sz<=maxChunkSize)
        {
            memcpy(&(newindex[w]),&(track->index[i]),sizeof(MP4Index));
            w++;
            continue;
        }
        // We have to split it...
        int part=0;

        uint64_t offset=track->index[i].offset;
        uint32_t samples=track->index[i].dts;
        uint32_t totalSamples=samples;
        uint64_t originalSize=sz;
        while(sz>maxChunkSize)
        {
            newindex[w].offset=offset+part*maxChunkSize;
            newindex[w].size=maxChunkSize;
            newindex[w].dts=(samples*maxChunkSize)/originalSize;
            newindex[w].pts=ADM_COMPRESSED_NO_PTS; // No seek
            totalSamples-=newindex[w].dts;
            ADM_assert(w<newNbCo);
            w++;
            part++;
            sz-=maxChunkSize;
        }
        // The last one...
        newindex[w].offset=offset+part*maxChunkSize;
        newindex[w].size=sz;
        newindex[w].dts=totalSamples;
        newindex[w].pts=ADM_COMPRESSED_NO_PTS;
        w++;
    }
    delete [] track->index;
    track->index=newindex;
    track->nbIndex=w;
    uint64_t total=0;
    for(i=0;i<track->nbIndex;i++)
        total+=track->index[i].size;
    ADM_info("After split, we have %llu bytes across %d blocks\n",total,w);

    return true;
}
/**
 * \fn processAudio
 * \brief used when all samples have the same size. We make some assumptions here, 
 * might not work with all mp4/mov files.
 * @param track
 * @param trackScale
 * @param info
 * @param outNbChunk
 * @return 
 */
bool MP4Header::processAudio( MP4Track *track, uint32_t trackScale, MPsampleinfo *info, uint32_t *nbOut)
{
    uint64_t totalBytes=info->SzIndentical*info->nbSz;
    uint32_t totalSamples=0;
    double   skewFactor=1;
    ADM_info("All the same size: %u (total size %" PRIu64" bytes)\n",info->SzIndentical,totalBytes);
    ADM_info("Byte per frame =%d\n",(int)info->bytePerFrame);
    ADM_info("SttsC[0] = %d, sttsN[0]=%d\n",info->SttsC[0],info->SttsN[0]);

    track->totalDataSize=totalBytes;

    if(info->nbStts!=1) 
    {
        ADM_info("WARNING: Same size, different duration\n");
        return 1;
    }

    if(info->SttsC[0]!=1)
    {
        ADM_warning("Not regular (time increment is not 1=%d)\n",(int)info->SttsC[0]);
        return true;
    }

    // Each chunk contains N samples=N bytes
    int *samplePerChunk=(int *)malloc(info->nbCo*sizeof(int));
    memset(samplePerChunk,0,info->nbCo*sizeof(int));
    int total=0;
    for(int i=0;i<info->nbSc;i++)
    {
        for(int j=info->Sc[i]-1;j<info->nbCo;j++)
        {
              aprintf("For chunk %lu, %lu \n",j,info->Sn[i] );
              samplePerChunk[j]=info->Sn[i];
        }
    }

    for(int i=0;i<info->nbCo;i++)
    {
        aprintf("Chunk %d Samples=%d\n",i,samplePerChunk[i]);
        total+=samplePerChunk[i];
    }

    ADM_info("Total size in sample : %u\n",total);
    ADM_info("Sample size          : %u\n",info->SzIndentical);

    if(info->SttsN[0]!=total)
    {
        ADM_warning("Not regular (Nb sequential samples (%d)!= total samples (%d))\n",info->SttsN[0],total);
        //free(samplePerChunk);
        //return 1;
    }

    track->index=new MP4Index[info->nbCo];
    memset(track->index,0,info->nbCo*sizeof(MP4Index));
    track->nbIndex=info->nbCo;;

    totalBytes=0;
    totalSamples=0;
#if 0   
#define ADM_PER info->bytePerPacket
#else
#define ADM_PER info->bytePerFrame    
#endif
    for(int i=0;i<info->nbCo;i++)
    {
        uint32_t sz;

        track->index[i].offset=info->Co[i];
        sz=samplePerChunk[i];
        sz=sz/info->samplePerPacket;
        sz*=ADM_PER; //*track->_rdWav.channels;;
        
        track->index[i].size=sz;
        track->index[i].dts=samplePerChunk[i]; // No seek
        track->index[i].pts=ADM_NO_PTS; // No seek

        totalBytes+=track->index[i].size;
        totalSamples+=samplePerChunk[i];
        aprintf("Block %d, size=%llu, total=%d, samples=%d, total samples =%d\n",i,track->index[i].size,totalBytes,samplePerChunk[i],totalSamples);
    }
    free(samplePerChunk);
    if(info->nbCo)
        track->index[0].pts=0;
    ADM_info("Found %u bytes, spread over %d blocks\n",totalBytes,info->nbCo);
    track->totalDataSize=totalBytes;

    // split large chunk into smaller ones if needed
    if(false==splitAudio(track, info, trackScale))
        return false; // cleanup will be done by parseStbl


    // Now time to update the time...
    // Normally they have all the same duration with a time increment of 
    // 1 per sample
    // so we have so far all samples with a +1 time increment
    uint32_t samplesSoFar=0;
    double scale=trackScale*track->_rdWav.channels;
    switch(track->_rdWav.encoding)
    {
        default:break;
        case WAV_PCM: // wtf ?
        case WAV_LPCM: // wtf ?
        case WAV_ULAW: // Wtf ?
        case WAV_IMAADPCM:
        case WAV_MSADPCM:
                scale/=track->_rdWav.channels;
                break;
    }
    if(info->bytePerPacket!=info->samplePerPacket)
    {
        ADM_info("xx Byte per packet =%d\n",info->bytePerPacket);
        ADM_info("xx Sample per packet =%d\n",info->samplePerPacket);
    }
    for(int i=0;i< track->nbIndex;i++)
    {
         uint32_t thisSample=track->index[i].dts;
         double v=samplesSoFar; // convert offset in sample to regular time (us)
         v=(v)/(scale);
         v*=1000LL*1000LL;
#if 1
         track->index[i].dts=track->index[i].pts=(uint64_t)v;
#else
         track->index[i].dts=track->index[i].pts=ADM_NO_PTS;
#endif
         samplesSoFar+=thisSample;
         aprintf("Block %d, size=%d, dts=%d\n",i,track->index[i].size,track->index[i].dts);
    }
    // track->index[0].dts=0;
    ADM_info("Index done (sample same size)\n");
    return 1;
}
/**
    \fn indexify
    \brief build the index from the stxx atoms
*/
uint8_t MP4Header::indexify( MP4Track *track,
                             uint32_t trackScale,
                             MPsampleinfo *info,
                             uint32_t isAudio,
                             uint32_t *outNbChunk)
{
    uint32_t i,j,cur;

    ADM_info("Build Track index, track timescale: %u\n",trackScale);
    *outNbChunk=0;
    aprintf("+_+_+_+_+_+\n");
    aprintf("co : %lu sz: %lu sc: %lu co[0] %" PRIu64"\n",info->nbCo,info->nbSz,info->nbSc,info->Co[0]);
    aprintf("+_+_+_+_+_+\n");

    ADM_assert(info->Sc);
    ADM_assert(info->Sn);
    ADM_assert(info->Co);
    if(!info->SzIndentical)
    {
        ADM_assert(info->Sz);
    }

    // Audio with all samples of the same size and regular
    if(info->SzIndentical && isAudio && info->nbStts==1 && info->SttsC[0]==1)
        return processAudio(track,trackScale,info,outNbChunk);

    // Audio with variable sample size or video
    track->index=new MP4Index[info->nbSz];
    memset(track->index,0,info->nbSz*sizeof(MP4Index));

    if(info->SzIndentical) // Video, all same size (DV ?)
    {
        aprintf("\t size for all %u frames : %u\n",info->nbSz,info->SzIndentical);
        for(i=0;i<info->nbSz;i++)
            track->index[i].size=info->SzIndentical;

        track->totalDataSize+=info->nbSz*info->SzIndentical;
    }else // Different size
    {
        for(i=0;i<info->nbSz;i++)
        {
            track->index[i].size=info->Sz[i];
            aprintf("\t size : %d : %u\n",i,info->Sz[i]);
            track->totalDataSize+=info->Sz[i];
        }
    }
    // if no sample to chunk we map directly
    // first build the # of sample per chunk table
    uint32_t totalchunk=0;

    // Search the maximum
    if(info->nbSc)
    {
        for(i=0;i<info->nbSc-1;i++)
            totalchunk+=(info->Sc[i+1]-info->Sc[i])*info->Sn[i];

        totalchunk+=(info->nbCo-info->Sc[info->nbSc-1]+1)*info->Sn[info->nbSc-1];
    }
    aprintf("# of chunks %d, max # of samples %d\n",info->nbCo, totalchunk);

    uint32_t *chunkCount = new uint32_t[totalchunk+1];
#if 0
	for(i=0;i<info->nbSc;i++)
	{
		for(j=info->Sc[i]-1;j<info->nbCo;j++)
		{
			chunkCount[j]=info->Sn[i];
                        ADM_assert(j<=totalchunk);
		}
		aprintf("(%d) sc: %lu sn:%lu\n",i,info->Sc[i],info->Sn[i]);
	}
#else
    if(info->nbSc)
    {
        for(i=0;i<info->nbSc-1;i++)
        {
            int mn=info->Sc[i]-1;
            int mx=info->Sc[i+1]-1;
            if(mn<0 || mx<0 || mn>totalchunk || mx > totalchunk || mx<mn)
            {
                ADM_warning("Corrupted file\n");
                return false;
            }
            for(j=mn;j<mx;j++)
            {
                chunkCount[j]=info->Sn[i];
                ADM_assert(j<=totalchunk);
            }
            aprintf("(%d) sc: %lu sn:%lu\n",i,info->Sc[i],info->Sn[i]);
        }
        // Last one
        for(j=info->Sc[info->nbSc-1]-1;j<info->nbCo;j++)
        {
            chunkCount[j]=info->Sn[i];
            ADM_assert(j<=totalchunk);
        }
    }
#endif

    // now we have for each chunk the number of sample in it
    cur=0;
    for(j=0;j<info->nbCo;j++)
    {
        uint64_t tail=0;
        aprintf("--starting at %lu , %lu to go\n",info->Co[j],chunkCount[j]);
        for(uint32_t k=0;k<chunkCount[j];k++)
        {
            track->index[cur].offset=info->Co[j]+tail;
            tail+=track->index[cur].size;
            aprintf(" sample : %d offset : %lu\n",cur,track->index[cur].offset);
            aprintf("Tail : %lu\n",tail);
            cur++;
        }
    }
    delete [] chunkCount;
    track->nbIndex=cur;

    // Now deal with duration
    // the unit is us FIXME, probably said in header
    // we put each sample duration in the time entry
    // then sum them up to get the absolute time position
    if(!info->nbStts)
    {
        ADM_warning("No time-to-sample table (stts) found.\n");
        return 0;
    }

    uint32_t nbChunk=track->nbIndex;
    uint32_t start=0;
    if(info->nbStts>1 || info->SttsC[0]!=1)
    {
        for(uint32_t i=0;i<info->nbStts;i++)
        {
            for(uint32_t j=0;j<info->SttsN[i];j++)
            {
                track->index[start].dts=(uint64_t)info->SttsC[i];
                track->index[start].pts=ADM_COMPRESSED_NO_PTS;
                start++;
                ADM_assert(start<=nbChunk);
            }
        }
    }else // All same duration
    {
        for(uint32_t i=0;i<nbChunk;i++)
        {
            track->index[i].dts=(uint64_t)info->SttsC[0]; // this is not an error!
            track->index[i].pts=ADM_COMPRESSED_NO_PTS;
        }
    }

    if(isAudio)
    {
        if(false==splitAudio(track,info, trackScale))
            return 0; // cleanup will be done by parseStbl
        nbChunk=track->nbIndex;
    }
    // now collapse
    uint64_t total=0;
    double   ftot;
    uint32_t thisone,previous=0;
    uint32_t step=0xFFFFFFFF;
    bool constantFps=true;

    // try to correct jitter from rounding errors first
    if(!isAudio)
    {
        std::map <uint32_t, uint32_t> hist;
        for(uint32_t i=0;i+1<nbChunk;i++)
        {
            thisone=track->index[i].dts;
            if(!thisone) continue;
            if(thisone<step) step=thisone;
            if(hist.find(thisone)==hist.end())
                hist.insert({thisone,1});
            else
                hist[thisone]++;
        }
        ADM_info("Histogram map has %u elements.\n",hist.size());
        std::map <uint32_t, uint32_t>::iterator it;
        for(it=hist.begin(); it!=hist.end(); it++)
        {
            printf("Frame duration %u count: %u\n",it->first,it->second);
        }
        it=hist.begin();
#define JITTER_FILTER_MIN_DURATION 99 // an arbitrary lower bound to avoid false positives
        if(hist.size()==3 && it->first >= JITTER_FILTER_MIN_DURATION) // we look for pattern x-1, x, x+1
        {
            ADM_info("Checking whether we need to fix jitter from rounding errors...\n");
            uint32_t a,b,c;
            uint32_t acount,ccount;
            a=it->first;
            acount=it->second;
            it++;
            b=it->first;
            bool restored=false;
            if(b==a+1)
            {
                it++;
                c=it->first;
                ccount=it->second;
                if(c==b+1 && ccount+2>acount && acount+2>ccount)
                {
                    for(uint32_t i=0;i<nbChunk;i++)
                        track->index[i].dts=b;
                    ADM_info("Yes, enforcing CFR, frame duration %u ticks.\n",b);
                    step=b;
                    restored=true;
                }
            }
            if(!restored)
                ADM_info("No, nothing we can do.\n");
        }
    }
    if(step==0xFFFFFFFF) step=1;

    for(uint32_t i=0;i<nbChunk;i++)
    {
        thisone=track->index[i].dts;
        if(!isAudio && i+1<nbChunk)
        {
            while(thisone%step)
                step=thisone%step;
            if(constantFps && i && thisone!=previous && thisone && previous)
                constantFps=false;
            previous=thisone;
        }
        ftot=total;
        ftot*=1000.*1000.;
        ftot/=trackScale;
        track->index[i].dts=(uint64_t)floor(ftot);
        track->index[i].pts=ADM_COMPRESSED_NO_PTS;
        total+=thisone;
        if(isAudio)
            aprintf("Audio chunk : %u time :%llu\n",i,track->index[i].dts);
    }
    if(isAudio)
    {
        ADM_info("Audio index done.\n");
        return true;
    }
    if(!nbChunk)
    {
        ADM_warning("Empty index!\n");
        return false;
    }
    // Time is now built, it is in us
    ADM_info("Video index done.\n");
    _videoFound++;
    ADM_info("Setting video timebase to %u / %u\n",step,_videoScale);
    _videostream.dwScale=step;
    if(constantFps)
    {
        _mainaviheader.dwMicroSecPerFrame=0; // force usage of fraction for fps
        return true;
    }
    ftot=total;
    ftot/=nbChunk;
    ftot*=1000.*1000.;
    ftot/=trackScale;
    ftot+=0.49;
    /* If the frame increment calculated from the time base is close to the average,
    the stream may be a constant fps stream with a mixture of frames and fields or
    simply have holes. The average is meaningless then. */
    if(step && _videoScale)
    {
        double ti=1000.*1000.;
        ti/=_videoScale;
        ti*=step;
        ti+=0.49;
        if(ftot<ti*2)
        {
            _mainaviheader.dwMicroSecPerFrame=(int32_t)ti;
            ADM_info("Using time base for frame increment %d us instead of average %d\n",(int32_t)ti,(int32_t)ftot);
            return true;
        }
    }
    _mainaviheader.dwMicroSecPerFrame=(int32_t)ftot;
    ADM_info("Variable frame rate, %d us per frame on average.\n",_mainaviheader.dwMicroSecPerFrame);

    return true;
}
/**
      \fn sample2byte
      \brief Convert the # of samples into the # of bytes needed
*/
uint32_t sample2byte(WAVHeader *hdr,uint32_t sample)
{
  float f;
        f=hdr->frequency; // 1 sec worth of data
        f=sample/f;       // in seconds
        f*=hdr->byterate; // in byte
    return (uint32_t)floor(f);
}
// EOF
