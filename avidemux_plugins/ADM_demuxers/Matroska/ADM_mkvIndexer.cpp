/***************************************************************************
    copyright            : (C) 2006 by mean
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
#include "ADM_cpp.h"
#include <math.h>

#include "ADM_default.h"
#include "ADM_Video.h"
//#include "fourcc.h"

#include "ADM_mkv.h"

#include "mkv_tags.h"
#include "DIA_working.h"
#include "ADM_codecType.h"
#include "ADM_videoInfoExtractor.h"
#define VIDEO _tracks[0]
/**
    \fn videoIndexer
    \brief index the video Track
*/
uint8_t mkvHeader::videoIndexer(ADM_ebml_file *parser)
{
  uint64_t fileSize,len,bsize;
  uint32_t alen,vlen;
  uint64_t id;
  ADM_MKV_TYPE type;
  const char *ss;

   parser->seek(0);
   DIA_workingBase *work=createWorking("Matroska Images");
   
   readBufferSize=200*1024;
   readBuffer=new uint8_t[readBufferSize];
    //************
   work->update(0);
#if 0
    int nbCluster=_clusters.size();
   for(int clusters=0;clusters<nbCluster;clusters++)
    {
        printf("[Cluster] %d/%d StartTimecode=%"PRIu64" ms\n",clusters,nbCluster,_clusters[clusters].Dts);
    }
#endif
    int nbClusters=_clusters.size();
   for(int clusters=0;clusters<nbClusters;clusters++)
   {
   parser->seek(_clusters[clusters].pos);
   ADM_ebml_file cluster(parser,_clusters[clusters].size);
   int thiscluster=0;
   while(!cluster.finished())
   {
      work->update(clusters,nbClusters);
      cluster.readElemId(&id,&len);
      if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
      {
        printf("[MKV] Tag 0x%"PRIx64" not found (len %"PRIu64")\n",id,len);
        cluster.skip(len);
        continue;
      }
      //printf("\t\tFound %s\n",ss);
      switch(id)
      {
                default:
                case MKV_TIMECODE:
                  //printf("Skipping %s\n",ss);
                  cluster.skip(len);
                  break;
                case MKV_SIMPLE_BLOCK:
                    {
                      indexBlock(parser,len,_clusters[clusters].Dts);
                    }
                    break;
                case MKV_BLOCK_GROUP:
                {
                        //printf("Block Group\n");
                        ADM_ebml_file blockGroup(parser,len);
                        while(!blockGroup.finished())
                        {
                                blockGroup.readElemId(&id,&len);
                                if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
                                {
                                  printf("[MKV] Tag 0x%"PRIx64" not found (len %"PRIu64")\n",id,len);
                                  blockGroup.skip(len);
                                  continue;
                                }
                                //printf("\t\t\t\tFound %s\n",ss);
                                //printf(">%s\n",ss);
                                switch(id)
                                {
                                  default: blockGroup.skip(len);break;
                                  case MKV_BLOCK :
                                  case MKV_SIMPLE_BLOCK:
                                  {
                                    indexBlock(&blockGroup,len,_clusters[clusters].Dts);
                                  }
                                  break;
                                }
                          }
                          thiscluster++;
            }

            break; // Block Group
       }
     }
   // printf("[MKV] ending cluster at 0x%llx\n",segment.tell());
  }
     printf("Found %"PRIu32" images in this cluster\n",(uint32_t)VIDEO.index.size());
     delete work;
     delete [] readBuffer;
     readBuffer=NULL;
     return 1;
}
/**
      \fn indexBlock
      \brief index a block, identify it and update index
*/
uint8_t mkvHeader::indexBlock(ADM_ebml_file *parser,uint32_t len,uint32_t clusterTimeCodeMs)
{
  int lacing,nbLaces,entryFlags=0;
  //
  uint64_t tail=parser->tell()+len;
  // Read Track id
  uint32_t tid=parser->readEBMCode();
  int track=searchTrackFromTid(tid);

      //printf("Wanted %u got %u\n",_tracks[0].streamIndex,tid);
      if(track==-1) //dont care track
      {
        parser->seek(tail);
        return 1; // we do only video here...
      }
      // Skip timecode
      uint64_t blockBegin=parser->tell();
      int16_t timecode=parser->readSignedInt(2);
      //if(!track) printf("TC: %d\n",timecode);
      uint8_t flags=parser->readu8();

      lacing=((flags>>1)&3);

      addIndexEntry(track,parser,blockBegin,tail-blockBegin,entryFlags,clusterTimeCodeMs+timecode);
      parser->seek(tail);
      return 1;
}
/**
 * 
 * @return 
 */
static int mkvFindStartCode(uint8_t *& start, uint8_t *end)
{
        uint32_t last=0xffffffff;
        while(start<end)
        {
            last=(last<<8)+*start;
            if((last & 0xFFFFFF00)==0x100)
            {
                int r=start[0];
                start++;
                return r;
            }
            start++;
        }
        return -1;
}
/**
    \fn addVideoEntry
    \brief add an entry to the video index
    @param timecodeMS PTS of the frame in ms!
*/
uint8_t mkvHeader::addIndexEntry(uint32_t track,ADM_ebml_file *parser,uint64_t where, uint32_t size,uint32_t flags,uint32_t timecodeMS)
{
  //
  mkvTrak *Track=&(_tracks[track]);
  mkvIndex ix;

  ix.pos=where;
  ix.size=size;
  ix.flags=AVI_KEY_FRAME;
  ix.Dts=timecodeMS*_timeBase;
  ix.Pts=timecodeMS*_timeBase;
   uint32_t rpt=_tracks[0].headerRepeatSize;
   
   // expand buffer if needed
   if(size>readBufferSize)
   {
       delete [] readBuffer;
       readBufferSize=size*2;
       readBuffer=new uint8_t[readBufferSize];
   }
   
  // since frame type is unreliable for mkv, we scan each frame
  // For the 2 most common cases : mp4 & h264.
  // Hackish, we already read the 3 bytes header
  // But they are already taken into account in the size part 
  if(!track) // Track 0 is video
  {
    if( isMpeg4Compatible(_videostream.fccHandler))
    {
        
            if(rpt)
                memcpy(readBuffer,_tracks[0].headerRepeat,rpt);
            parser->readBin(readBuffer+rpt,size-3);
            // Search the frame type...

             uint32_t nb,vopType,timeinc=16;
             ADM_vopS vops[10];
             vops[0].type=AVI_KEY_FRAME;
             ADM_searchVop(readBuffer,readBuffer+rpt+size-3,&nb,vops, &timeinc);
             ix.flags=vops[0].type;
        
    }else
    if(isH264Compatible(_videostream.fccHandler))
    {
                
                uint32_t flags=AVI_KEY_FRAME;
                
                if(rpt)
                        memcpy(readBuffer,_tracks[0].headerRepeat,rpt);
                parser->readBin(readBuffer+rpt,size-3);
                extractH264FrameType(2,readBuffer,rpt+size-3,&flags); // Nal size is not used in that case
                if(flags & AVI_KEY_FRAME)
                {
                    printf("[MKV/H264] Frame %"PRIu32" is a keyframe\n",(uint32_t)Track->index.size());
                }
                ix.flags=flags;
                if(Track->index.size()) ix.Dts=ADM_NO_PTS;

    }/*else if(isH265Compatible(_videostream.fccHandler))
    {
                
                uint32_t flags=AVI_KEY_FRAME;
                
                if(rpt)
                        memcpy(readBuffer,_tracks[0].headerRepeat,rpt);
                parser->readBin(readBuffer+rpt,size-3);
                extractH265FrameType(2,readBuffer,rpt+size-3,&flags); // Nal size is not used in that case
                if(flags & AVI_KEY_FRAME)
                {
                    printf("[MKV/H265] Frame %"PRIu32" is a keyframe\n",(uint32_t)Track->index.size());
                }
                ix.flags=flags;
                if(Track->index.size()) ix.Dts=ADM_NO_PTS;

    }*/else if(isMpeg12Compatible(_videostream.fccHandler))
    {

                if(rpt)
                        memcpy(readBuffer,_tracks[0].headerRepeat,rpt);
                parser->readBin(readBuffer+rpt,size-3);
                uint8_t *begin=readBuffer;
                uint8_t *end=readBuffer+size-3+rpt;
                uint32_t flags=0;
                while(begin<end)
                {
                    int code=mkvFindStartCode(begin,end);
                    if(code==-1) 
                    {
                        ADM_warning("[Mpg2InMkv]No startcode found\n");
                        break;
                    }
                    //printf("Startcode found = 0x%x\n",code);
                    if(!code) // picture
                    {
                        int picType=begin[1]>>3;
                        begin+=4;
                        picType&=7;
                        switch(picType)
                        {
                            case 1: ix.flags=AVI_KEY_FRAME;break;
                            case 2: ix.flags=AVI_P_FRAME;break;
                            case 4: ix.flags=AVI_P_FRAME;break;
                            case 3: ix.flags=AVI_B_FRAME;break;
                            default: ADM_warning("[Mpeg2inMkv]Bad pictype : %d\n",picType);
                        }
                        break;
                    }
                    
                }             
                
    }
  }
  Track->index.append(ix);

  return 1;
}

/**
  \fn searchTrackFromTid
  \brief Returns our track number for the stream track number. -1 if the stream is not handled.

*/
int mkvHeader::searchTrackFromTid(uint32_t tid)
{
  for(int i=0;i<1+_nbAudioTrack;i++)
  {
    if(tid==_tracks[i].streamIndex) return i;
  }
  return -1;
}

/**
  \fn readCue
  \brief Update index with cue content

*/
uint8_t                 mkvHeader::readCue(ADM_ebml_file *parser)
{
  uint64_t fileSize,len,bsize;
  uint64_t alen,vlen;
  uint64_t id;
  ADM_MKV_TYPE type;
  const char *ss;
  uint64_t time;
  uint64_t segmentPos;

   parser->seek(0);

   if(!parser->simplefind(MKV_SEGMENT,&vlen,1))
   {
     printf("[MKV] Cannot find CLUSTER atom\n");
     return 0;
   }
   ADM_ebml_file segment(parser,vlen);
   segmentPos=segment.tell();

   while(segment.simplefind(MKV_CUES,&alen,0))
  {
   ADM_ebml_file cues(&segment,alen);
   while(!cues.finished())
   {
      cues.readElemId(&id,&len);
      if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
      {
        printf("[MKV] Tag 0x%"PRIx64" not found (len %"PRIu64")\n",id,len);
        cues.skip(len);
        continue;
      }
      if(id!=MKV_CUE_POINT)
      {
        printf("Found %s in CUES, ignored \n",ss);
        cues.skip(len);
        continue;
      }
      ADM_ebml_file cue(&cues,len);
      // Cue TIME normally
       cue.readElemId(&id,&len);
       if(id!=MKV_CUE_TIME)
       {
          ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type);
          printf("Found %s(0x%"PRIx64"), expected CUE_TIME  (0x%x)\n", ss,id,MKV_CUE_TIME);
          cue.skip(cue.remaining());
          continue;
       }
       time=cue.readUnsignedInt(len);


       cue.readElemId(&id,&len);
       if(id!=MKV_CUE_TRACK_POSITION)
       {
          ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type);
          printf("Found %s (0x%"PRIx64"), expected MKV_CUE_TRACK_POSITION (0x%x)\n", ss,id,MKV_CUE_TRACK_POSITION);
          cue.skip(cues.remaining());
          continue;
       }
       ADM_ebml_file trackPos(&cue,len);
       uint64_t tid=0;
       uint64_t cluster_position=0;
       while(!trackPos.finished())
       {
         trackPos.readElemId(&id,&len);
         switch(id)
         {
           case MKV_CUE_TRACK: tid=trackPos.readUnsignedInt(len);break;
           case MKV_CUE_CLUSTER_POSITION: cluster_position=trackPos.readUnsignedInt(len);break;
           default:
                 ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type);
                 printf("[MKV] in cluster position found tag %s (0x%"PRIx64")\n",ss,id);
                 trackPos.skip(len);
                 continue;
         }
       }
       printf("Track %"PRIx64" Position 0x%"PRIx64" time %"PRIu64" final pos:%"PRIx64" \n",tid,cluster_position,time,
             cluster_position+segmentPos );
     }
   }
   printf("[MKV] Cues updated\n");
   return 1;
}
/**
        \fn indexClusters
        \brief make a list of all clusters with there position & size
*/
uint8_t   mkvHeader::indexClusters(ADM_ebml_file *parser)
{
  uint64_t fileSize,len,bsize;
  uint64_t alen,vlen;
  uint64_t id;
  ADM_MKV_TYPE type;
  const char *ss;
  uint64_t time;
  uint64_t pos;

    mkvIndex tmpCluster;

  // Search segment
   fileSize=parser->getFileSize();
   if(!parser->simplefind(MKV_SEGMENT,&vlen,1))
   {
     ADM_warning("[MKV] cluster indexer, cannot find CLUSTER atom\n");
     return 0;
   }
    // In case the segment is ridiculously small take file size....
   pos=parser->tell();
   ADM_info("FileSize = %"PRIu64", pos=%"PRIu64" size=%"PRIu64",pos+size=%"PRIu64"\n",fileSize,pos,vlen,pos+vlen);
   if(pos+vlen<fileSize)
    {
        ADM_warning("Segment is way too small, trying to guess the right value\n");
        vlen=fileSize-pos;
    }
   ADM_ebml_file segment(parser,vlen);
   
   DIA_workingBase *work=createWorking("Matroska clusters");
   while(segment.simplefind(MKV_CLUSTER,&alen,0))
   {

     // UI update
     work->update(segment.tell()>>10,fileSize>>10);
     tmpCluster.pos=segment.tell();
     tmpCluster.size=alen;
     _clusters.append(tmpCluster);

     // Normally the timecode is the 1st one following

tryAgain:
       segment.readElemId(&id,&len);
       switch(id)
        {
            case MKV_CRC32:
            case MKV_PREV_SIZE:
            case MKV_POSITION:
                segment.skip(len);
                goto tryAgain;
            default:break;
        }
       int seekme=_clusters.size()-1; // last element...
       if(id!=MKV_TIMECODE)
       {
          ss=NULL;
          ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type);
          ADM_warning("[MKV] Cluster : no time code Found %s(0x%"PRIx64"), expected MKV_TIMECODE  (0x%x)\n",
                  ss,id,MKV_TIMECODE);
       }
       else // timecode
       {
           uint64_t timecode=segment.readUnsignedInt(len);
           _clusters[seekme].Dts=timecode;
       }
       segment.seek( _clusters[seekme].pos+ _clusters[seekme].size);
       //printf("Position :%u %u MB\n", _clusters[seekme].pos+ _clusters[seekme].size,( _clusters[seekme].pos+ _clusters[seekme].size)>>20);
   }
   delete work;
   ADM_info("[MKV] Found %u clusters\n",(int)_clusters.size());
   return 1;
}
//EOF
