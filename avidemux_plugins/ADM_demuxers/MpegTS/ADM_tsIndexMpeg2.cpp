/***************************************************************************
    \brief TS indexer, Mpeg2 video
    \author mean fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_tsIndex.h"
#include "DIA_coreToolkit.h"

static const uint32_t FPS[16]={
                0,                      // 0
                23976,          // 1 (23.976 fps) - FILM
                24000,          // 2 (24.000 fps)
                25000,          // 3 (25.000 fps) - PAL
                29970,          // 4 (29.970 fps) - NTSC
                30000,          // 5 (30.000 fps)
                50000,          // 6 (50.000 fps) - PAL noninterlaced
                59940,          // 7 (59.940 fps) - NTSC noninterlaced
                60000,          // 8 (60.000 fps)
                0,                      // 9
                0,                      // 10
                0,                      // 11
                0,                      // 12
                0,                      // 13
                0,                      // 14
                0                       // 15
        };


/**
    \fn runMpeg2
*/  
uint8_t TsIndexerMpeg2::run(const char *file,ADM_TS_TRACK *videoTrac)
{
uint32_t temporal_ref,val;
uint8_t buffer[50*1024];
bool seq_found=false;
H264Unit thisUnit;

beginConsuming=0;

TSVideo video;
indexerData  data;    
dmxPacketInfo tmpInfo;

uint8_t result=1;
bool bAppend=false;

    listOfUnits.clear();

    if(!videoTrac) return false;
    if(videoTrac[0].trackType!=ADM_TS_MPEG2)
    {
        printf("[Ts Indexer] Only Mpeg2 video supported\n");
        return false;
    }
    video.pid=videoTrac[0].trackPid;

    memset(&data,0,sizeof(data));

    string indexName=string(file);
    indexName=indexName+string(".idx2");
    index=qfopen(indexName,"wt");

    if(!index)
    {
        printf("[TsIndexerMpeg2] Cannot create %s\n",indexName.c_str());
        return 0;
    }
    
    pkt=new tsPacketLinearTracker(videoTrac->trackPid, audioTracks);
    
    FP_TYPE append=FP_DONT_APPEND;
#ifdef ASK_APPEND_SEQUENCED
    if(true==ADM_probeSequencedFile(file))
    {
        if(true==GUI_Question(QT_TRANSLATE_NOOP("tsdemuxer","There are several files with sequential file names. Should they be all loaded ?")))
               bAppend=true;
    }
    if(true==bAppend)
        append=FP_APPEND;
#endif
    writeSystem(file,bAppend);
    pkt->open(file,append);
    gui= createProcessing(QT_TRANSLATE_NOOP("tsdemuxer","Indexing"),pkt->getSize());
    data.pkt=pkt;
    fullSize=pkt->getSize();
    int startCode;
    decodingImage=false;
#define likely(x) x
#define unlikely(x) x
    int lastStartCode=0xb3;

#define REMEMBER() { lastStartCode=startCode;}   
      while(true)
      {
        startCode=pkt->findStartCode();
        if(!pkt->stillOk()) break;

          switch(startCode)
                  {
// B2: User Data
                  case 0xB3: // sequence start
                          REMEMBER()
                          if(seq_found)
                          {
                                decodingImage=false;
                                pkt->getInfo(&thisUnit.packetInfo);
                                thisUnit.consumedSoFar=pkt->getConsumed();
                                if(!addUnit(data,unitTypeSps,thisUnit,4))
                                {
                                    result=ADM_IGN;
                                    goto the_end;
                                }
                                pkt->forward(8);  // Ignore
                                continue;
                          }
                          pkt->setConsumed(4); // reset consumed counter
                          //
                          seq_found=1;
                          val=pkt->readi32();                    //+4
                          video.w=val>>20;
                          video.w=((video.w+15)&~15);
                          //video.h= (((val>>8) & 0xfff)+15)& ~15;
                          video.h= ((val>>8) & 0xfff);

                          video.ar = (val >> 4) & 0xf;
                          video.fps= FPS[val & 0xf];
                          pkt->forward(4);                      //+4
                          writeVideo(&video,ADM_TS_MPEG2);
                          writeAudio();
                          qfprintf(index,"[Data]");

                          decodingImage=false;
                          pkt->getInfo(&thisUnit.packetInfo);
                          thisUnit.consumedSoFar=pkt->getConsumed();
                          if(!addUnit(data,unitTypeSps,thisUnit,4+4+4))
                          {
                              result=ADM_IGN;
                              goto the_end;
                          }
                          continue;
                          break;
//#warning FIXME, update pic field info.... It triggers a end-of-pic message as it is

                    case 0xB5: //  extension
                        {                             
                            uint8_t id=pkt->readi16()>>12;  // +1
                            switch(id)
                            {
                                case 1: // Sequence extension
                                    REMEMBER()
                                    val=(val>>3)&1; // gop type progressive, unreliable, not used
                                    break;
                                case 8: // picture coding extension (mpeg2)
                                {
                                    if(lastStartCode!=0)
                                    {
                                        ADM_warning("Picture coding extension not following picture (%x)\n",lastStartCode);
                                        continue;
                                    }
                                    REMEMBER()
                                    int one, two,three;
                                    one=pkt->readi8();             // +1
                                    two=pkt->readi8();             // +1
                                    three=pkt->readi8();
                                    
                                    int picture_structure=one&3;// 1=TOP, 2=BOTTOM, 3=FRAME
                                    bool tff=!!(two&0x80);;
                                    bool progressive_frame=!!(three&0x80);;
                                    updateLastUnitStructure(picture_structure);
#if 0                                                                                
                                    printf("structure %d progressive=%d tff=%d (%x:%x:%x)\n",picture_structure,progressive_frame,tff,one,two,three);

                                    //printf("Picture type %02x struct:%x\n",two,picture_structure);
                                    updatePicStructure(video,picture_structure);
                                    pkt->getInfo(&thisUnit.packetInfo);
                                    thisUnit.consumedSoFar=pkt->getConsumed();
                                    addUnit(data,unitTypePicInfo,thisUnit,4+1+1+1);
#endif                                            
                                }
                                default:break;
                            }
                        }
                        break;
                  case 0xb8: // GOP
                        REMEMBER()
                          // Update ui                        
                          if(!seq_found) continue;

                          pkt->getInfo(&thisUnit.packetInfo);
                          thisUnit.consumedSoFar=pkt->getConsumed();
                          if(!addUnit(data,unitTypeSps,thisUnit,4))
                          {
                              result=ADM_IGN;
                              goto the_end;
                          }
                          break;                                            
                  case 0x00 : // picture
                        {
                          int type;
                          if(!seq_found)
                          { 
                                  printf("[TsIndexer]No sequence start yet, skipping..\n");
                                  continue;
                          }
                          
                          val=pkt->readi16();             // +2
                          
                          temporal_ref=val>>6;
                          type=7 & (val>>3);
                          REMEMBER()
                          if( type<1 ||  type>3)
                          {
                                  printf("[Indexer]Met illegal pic at %" PRIx64" + %" PRIx32"\n",
                                                  thisUnit.packetInfo.startAt,thisUnit.packetInfo.offset);
                                  continue;
                          }
                          
                          pkt->getInfo(&thisUnit.packetInfo);
                          thisUnit.consumedSoFar=pkt->getConsumed();
                          thisUnit.imageType=type;
                          if(!addUnit(data,unitTypePic,thisUnit,4+2))
                          {
                              result=ADM_IGN;
                              goto the_end;
                          }
                          pkt->invalidatePtsDts();
                          data.nbPics++;
                        }
                          break;
                  default:
                    break;
                  }
      }
the_end:
        printf("\n");
        qfprintf(index,"\n[End]\n");
        qfprintf(index,"\n# Found %" PRIu32" images \n",data.nbPics); // Size
        qfprintf(index,"# Found %" PRIu32" frame pictures\n",video.frameCount); // Size
        qfprintf(index,"# Found %" PRIu32" field pictures\n",video.fieldCount); // Size
        qfclose(index);
        index=NULL;
        audioTracks=NULL;
        delete pkt;
        pkt=NULL;
        return result;
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/

//

//EOF
