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
bool TsIndexer::runMpeg2(const char *file,ADM_TS_TRACK *videoTrac)
{
uint32_t temporal_ref,val;
uint8_t buffer[50*1024];
bool seq_found=false;
H264Unit thisUnit;

beginConsuming=0;

TSVideo video;
indexerData  data;    
dmxPacketInfo tmpInfo;

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
        printf("[PsIndex] Cannot create %s\n",indexName.c_str());
        return false;
    }
    
    pkt=new tsPacketLinearTracker(videoTrac->trackPid, audioTracks);
    
    FP_TYPE append=FP_DONT_APPEND;
    if(true==ADM_probeSequencedFile(file))
    {
        if(true==GUI_Question("There are several files with sequential file names. Should they be all loaded ?"))
               bAppend=true;
    }
    if(true==bAppend)
        append=FP_APPEND;
    writeSystem(file,bAppend);
    pkt->open(file,append);
    gui= createProcessing("Indexing",pkt->getSize());
    data.pkt=pkt;
    fullSize=pkt->getSize();
    int startCode;
    decodingImage=false;
    
#define likely(x) x
#define unlikely(x) x
      while(1)
      {
        startCode=pkt->findStartCode();
        if(!pkt->stillOk()) break;

          switch(startCode)
                  {
// B2: User Data
                  case 0xB3: // sequence start
                          if(seq_found)
                          {
                                decodingImage=false;
                                pkt->getInfo(&thisUnit.packetInfo);
                                thisUnit.consumedSoFar=pkt->getConsumed();
                                addUnit(data,unitTypeSps,thisUnit,4);
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
                          addUnit(data,unitTypeSps,thisUnit,4+4+4);
                          continue;
                          break;
#warning FIXME, update pic field info.... It triggers a end-of-pic message as it is
#if 0
                    case 0xB5: //  extension
                                { 
                                    uint8_t id=pkt->readi8()>>4;  // +1
                                    uint8_t two;
                                    switch(id)
                                    {
                                        case 1: // Sequence extension
                                            val=(val>>3)&1; // gop type progressive, unreliable, not used
                                            break;
                                        case 8: // picture coding extension (mpeg2)
                                        {
                                            // skip motion vector
                                            uint8_t picture_structure;
                                            pkt->forward(1); // 4*4 bits   // +1
                                            two=pkt->readi8();             // +1
                                            picture_structure=(two)&3;
                                            
                                            //printf("Picture type %02x struct:%x\n",two,picture_structure);
                                            updatePicStructure(video,picture_structure);
                                            pkt->getInfo(&thisUnit.packetInfo);
                                            thisUnit.consumedSoFar=pkt->getConsumed();
                                            addUnit(data,unitTypePicInfo,thisUnit,4+1+1+1);
                                        }
                                        default:break;
                                    }
                                }
                                break;
#endif
                  case 0xb8: // GOP
                          // Update ui                        
                          if(!seq_found) continue;

                          pkt->getInfo(&thisUnit.packetInfo);
                          thisUnit.consumedSoFar=pkt->getConsumed();
                          addUnit(data,unitTypeSps,thisUnit,4);
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
                          if( type<1 ||  type>3)
                          {
                                  printf("[Indexer]Met illegal pic at %"PRIx64" + %"PRIx32"\n",
                                                  thisUnit.packetInfo.startAt,thisUnit.packetInfo.offset);
                                  continue;
                          }
                          
                          pkt->getInfo(&thisUnit.packetInfo);
                          thisUnit.consumedSoFar=pkt->getConsumed();
                          thisUnit.imageType=type;
                          addUnit(data,unitTypePic,thisUnit,4+2);
                          pkt->invalidatePtsDts();
                          data.nbPics++;
                        }
                          break;
                  default:
                    break;
                  }
      }
    
        printf("\n");
        qfprintf(index,"\n[End]\n");
        qfprintf(index,"\n# Found %"PRIu32" images \n",data.nbPics); // Size
        qfprintf(index,"# Found %"PRIu32" frame pictures\n",video.frameCount); // Size
        qfprintf(index,"# Found %"PRIu32" field pictures\n",video.fieldCount); // Size
        qfclose(index);
        index=NULL;
        audioTracks=NULL;
        delete pkt;
        pkt=NULL;
        return 1; 
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/

//

//EOF
