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

#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"
#include "ADM_mp4.h"
#include "DIA_coreToolkit.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif


#define QT_TR_NOOP(x) x
#define MAX_CHUNK_SIZE (4*1024)
uint32_t sample2byte(WAVHeader *hdr,uint32_t sample);
/**
 * \fn splitAudio
 * \brief Split audio chunks into small enough pieces
 * @param track
 * @param vinfo
 * @return 
 */
bool MP4Header::splitAudio(MP4Track *track,MPsampleinfo *info, uint32_t trackScale,int extraBlocks )
{
        
        uint64_t audioClock=0;
        uint32_t newNbCo=track->nbIndex+extraBlocks*2; // *2 is enough, should be.
        uint32_t w=0;
        uint32_t one_go;

       one_go=MAX_CHUNK_SIZE/info->SzIndentical;
       one_go=one_go*info->SzIndentical;
       printf("Splitting large audio blocks : One go :%u\n",one_go);
       MP4Index *newindex=new MP4Index[newNbCo];

      int64_t time_increment; // Nb sample*duration of one sample
      printf("Time increment = %d\n",(int)time_increment);
      for(int i=0;i<track->nbIndex;i++)
      {
        uint32_t sz;
            sz=track->index[i].size;
            if(sz<MAX_CHUNK_SIZE)
            {
                memcpy(&(newindex[w]),&(track->index[i]),sizeof(MP4Index));
                w++;
                continue;
            }
            // We have to split it...
            int part=0;
            time_increment=track->index[i].dts/(1+(sz/MAX_CHUNK_SIZE));
            while(sz>one_go)
            {
                  newindex[w].offset=track->index[i].offset+part*one_go;
                  newindex[w].size=one_go;
                  newindex[w].dts=time_increment*part;
                  newindex[w].pts=ADM_COMPRESSED_NO_PTS; // No seek
                  ADM_assert(w<newNbCo);
                  w++;
                  part++;
                  sz-=one_go;
            }
            // The last one...
                  newindex[w].offset=track->index[i].offset+part*one_go;
                  newindex[w].size=sz;
                  newindex[w].dts=track->index[i].dts+part*time_increment; 
                  newindex[w].pts=ADM_COMPRESSED_NO_PTS;
                  w++;
      }
      delete [] track->index;
      track->index=newindex;
      track->nbIndex=w;
      uint32_t total=0;
      for(int i=0;i<track->nbIndex;i++)
          total+=track->index[i].size;
      printf("After split, we have %u bytes\n",total);
      
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
uint8_t	MP4Header::processAudio( MP4Track *track,  uint32_t trackScale,  MPsampleinfo *info,uint32_t *outNbChunk)
{
    uint64_t audioClock=0;
    
    uint32_t totalBytes=info->SzIndentical*info->nbSz;
    printf("All the same size: %u (total size %u bytes)\n",info->SzIndentical,totalBytes);
    printf("SttsC[0] = %d, sttsN[0]=%d\n",info->SttsC[0],info->SttsN[0]);
    
    if(info->nbStts!=1) 
    {
        printf("WARNING: Same size, different duration\n");
        return 1;
    }
    
      if(info->SttsC[0]!=1)
      {
          ADM_warning("Not regular (time increment is not 1)\n");
          return 1;
      }
    //
    // Each chunk contains N samples=N bytes
    int *samplePerChunk=(int *)alloca(info->nbCo*sizeof(int));
    memset(samplePerChunk,0,info->nbCo*sizeof(int));
    int total=0;
    for(int i=0;i<info->nbSc;i++)
    {
        for(int j=info->Sc[i]-1;j<info->nbCo;j++)
        {
              aprintf("For chunk %lu, %lu samples\n",j,info->Sn[i]);
              samplePerChunk[j]=info->Sn[i];
        }
    }
    /**/
    for(int i=0;i<info->nbCo;i++)
      total+=samplePerChunk[i];

    printf("Total size in sample : %u\n",total);
    printf("Sample size          : %u\n",info->SzIndentical);
    
      if(info->SttsN[0]!=total)
      {
          ADM_warning("Not regular (Nb sequential samples (%d)!= total samples (%d))\n",info->SttsN[0],total);
          return 1;
      }
    
    track->index=new MP4Index[info->nbCo];
    memset(track->index,0,info->nbCo*sizeof(MP4Index));
    track->nbIndex=info->nbCo;;
    
    totalBytes=0;
    
    // all the same size & duration...
    
    for(int i=0;i<info->nbCo;i++)
    {
        uint32_t sz;
#define PACK_SIZE info->bytePerFrame // perPacket ??

        track->index[i].offset=info->Co[i];
        sz=samplePerChunk[i];
        /* Sz is in sample, convert it to bytes */
        sz/=info->samplePerPacket;
        if(sz*info->samplePerPacket!=samplePerChunk[i])
        {
          printf("Warning sample per packet not divider of sample per chunk (per packet :%u , chunk :%u)\n",
                    info->samplePerPacket, samplePerChunk[i]); 
        }
        sz*=PACK_SIZE;
        /* */
        track->index[i].size=sz*info->SzIndentical;
        track->index[i].dts=ADM_NO_PTS; // No seek
        track->index[i].pts=ADM_NO_PTS; // No seek
        /*
        if(sz>MAX_CHUNK_SIZE)
        {
            max+=sz/MAX_CHUNK_SIZE;
        }
         */

        totalBytes+=track->index[i].size;
        aprintf("Block %d , size=%d,total=%d\n",i,track->index[i].size,totalBytes);
    }

    if(info->nbCo)
        track->index[0].dts=track->index[0].pts=0;
    printf("Found %u bytes, spred over %d blocks\n",totalBytes,info->nbCo);
    // Now time to update the time...
    // Normally they have all the same duration with a time increment of 
    // 1 per sample
    // so we have so far all samples with a +1 time increment
      
      double sampleDuration,totalDuration=0;
      // Set Dts & Pts accordingly
      uint64_t totalSize=0;
      for(int i=0;i<info->nbCo;i++)
      {     
            double v=totalSize; // convert offset in sample to regular time (us)
            v=v/(trackScale*info->SzIndentical);
            v*=1000LL*1000LL;
            track->index[i].dts=track->index[i].pts=(uint64_t)v;
            totalSize+=track->index[i].size;
      }
   
    printf("Index done (sample same size)\n");
    return 1;
}
/**
        \fn indexify
        \brief build the index from the stxx atoms
*/
uint8_t	MP4Header::indexify(
                          MP4Track *track,   
                          uint32_t trackScale,
                         MPsampleinfo *info,
                         uint32_t isAudio,
                         uint32_t *outNbChunk)

{

uint32_t i,j,cur;

        ADM_info("Build Track index\n");
	*outNbChunk=0;
	aprintf("+_+_+_+_+_+\n");
	aprintf("co : %lu sz: %lu sc: %lu co[0] %"PRIu64"\n",info->nbCo,info->nbSz,info->nbSc,info->Co[0]);
	aprintf("+_+_+_+_+_+\n");

	ADM_assert(info->Sc);
	ADM_assert(info->Sn);
	ADM_assert(info->Co);
	if(!info->SzIndentical)
        {
          ADM_assert(info->Sz);
        }
        //*********************************************************
	// in that case they are all the same size, i.e.audio
        //*********************************************************
	if(info->SzIndentical && isAudio)// in that case they are all the same size, i.e.audio
	{
           return  processAudio(track,trackScale,info,outNbChunk);
        }
	// We have different packet size
	// Probably video
        track->index=new MP4Index[info->nbSz];
        memset(track->index,0,info->nbSz*sizeof(MP4Index));

        if(info->SzIndentical) // Video, all same size (DV ?)
        {
            aprintf("\t size for all %u frames : %u\n",info->nbSz,info->SzIndentical);
            for(i=0;i<info->nbSz;i++)
            {
                    track->index[i].size=info->SzIndentical;
                    
            }
          }
          else // Different size
          {
            for(i=0;i<info->nbSz;i++)
            {
                    track->index[i].size=info->Sz[i];
                    aprintf("\t size : %d : %u\n",i,info->Sz[i]);
            }
          }
	// if no sample to chunk we map directly
	// first build the # of sample per chunk table
        uint32_t totalchunk=0;

        // Search the maximum
        for(i=0;i<info->nbSc-1;i++)
        {
                totalchunk+=(info->Sc[i+1]-info->Sc[i])*info->Sn[i];
        }
        totalchunk+=(info->nbCo-info->Sc[info->nbSc-1]+1)*info->Sn[info->nbSc-1];

        aprintf("# of chunks %d, max # of samples %d\n",info->nbCo, totalchunk);

        uint32_t *chunkCount = new uint32_t[totalchunk+1];
	for(i=0;i<info->nbSc;i++)
	{
		for(j=info->Sc[i]-1;j<info->nbCo;j++)
		{
			chunkCount[j]=info->Sn[i];
                        ADM_assert(j<=totalchunk);
		}
		aprintf("(%d) sc: %lu sn:%lu\n",i,info->Sc[i],info->Sn[i]);
	}
/*			for(j=0;j<nbSc;j++)
			{
				aprintf("\n count number : %d - %lu\n",j,Sn[j]);
			}*/
	// now we have for each chunk the number of sample in it
	cur=0;
	for(j=0;j<info->nbCo;j++)
	{
		int tail=0;
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
        
        
        track->nbIndex=cur;;
	
	
	// Now deal with duration
	// the unit is us FIXME, probably said in header
	// we put each sample duration in the time entry
	// then sum them up to get the absolute time position

        uint32_t nbChunk=track->nbIndex;
	if(info->nbStts)		//uint32_t nbStts,	uint32_t *SttsN,uint32_t SttsC,
	{
		uint32_t start=0;
		if(info->nbStts>1)
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
		}
		else
		{
			// All same duration
			for(uint32_t i=0;i<nbChunk;i++)
            {
                track->index[i].dts=(uint64_t)info->SttsC[0]; // this is not an error!
                track->index[i].pts=ADM_COMPRESSED_NO_PTS;
            }
		
		}
		// now collapse
		uint64_t total=0;
		float    ftot;
		uint32_t thisone;
		
		for(uint32_t i=0;i<nbChunk;i++)
		{
                        thisone=track->index[i].dts;
			ftot=total;
			ftot*=1000.*1000.;
			ftot/=trackScale;
                        track->index[i].dts=(uint64_t)floor(ftot);
                        track->index[i].pts=ADM_COMPRESSED_NO_PTS;
			total+=thisone;
                        aprintf("Audio chunk : %lu time :%lu\n",i,track->index[i].dts);
		}
		// Time is now built, it is in us
	
	
	}
	else // there is not ssts
	{
          GUI_Error_HIG(QT_TR_NOOP("No stts table"), NULL);
		ADM_assert(0);	
	}
        printf("Index done\n");
	return 1;
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
