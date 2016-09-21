/***************************************************************************
    \brief TS indexer, H264 video
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
#include "ADM_tsIndex.h"

static bool decoderSei1(const ADM_SPSInfo &spsInfo,int size, uint8_t *bfer,pictureStructure *pic);
static bool decoderSei6(int size, uint8_t *bfer,uint32_t *recovery);
/**
        \fn decodeSEI
        \brief decode SEI to get short ref I
        @param recoveryLength # of recovery frame
        \return true if recovery found
*/
bool TsIndexer::decodeSEI(uint32_t nalSize, uint8_t *org,uint32_t *recoveryLength,
                pictureStructure *picStruct)
{
    if(nalSize+16>=ADM_NAL_BUFFER_SIZE)
    {
        ADM_warning("SEI size too big, probably corrupted input (%d bytes)\n",nalSize);
        return false;
    }
    uint8_t *payload=payloadBuffer;
    bool r=false;
    nalSize=ADM_unescapeH264(nalSize,org,payload);
    uint8_t *tail=payload+nalSize;
    *picStruct=pictureFrame; // frame
    while( payload<tail-2)
    {
                uint32_t sei_type=0,sei_size=0;
                while(payload[0]==0xff) {sei_type+=0xff;payload++;};
                sei_type+=payload[0];payload++;
                while(payload[0]==0xff) {sei_size+=0xff;payload++;};
                sei_size+=payload[0];payload++;
                aprintf("  [SEI] Type : 0x%x size:%d\n",sei_type,sei_size);
                if(payload+sei_size>=tail)
                {
                        return false;
                }
                switch(sei_type) // Recovery point
                {

                       case 1:
                        {
                            decoderSei1(spsInfo,sei_size,payload,picStruct);
                            payload+=sei_size;
                            break;
                        }
                       case 6:
                        {
                            decoderSei6(sei_size,payload,recoveryLength);
                            payload+=sei_size;
                            aprintf("[SEI] Recovery :%" PRIu32"\n",*recoveryLength);
                            r=true;
                            break;
                        }
                        default:
                            payload+=sei_size;
                            break;
                }
    }
    //if(payload+1<tail) ADM_warning("Bytes left in SEI %d\n",(int)(tail-payload));
    return r;
}

/**
    \fn runH264
    \brief Index H264 stream
*/
bool TsIndexer::runH264(const char *file,ADM_TS_TRACK *videoTrac)
{

bool    seq_found=false;
bool    firstSps=true;

TSVideo video;
indexerData  data;
dmxPacketInfo tmpInfo;
TS_PESpacket SEI_nal(0);
bool result=false;
bool bAppend=false;

    beginConsuming=0;
    listOfUnits.clear();

    printf("Starting H264 indexer\n");
    if(!videoTrac) return false;
    if(videoTrac[0].trackType!=ADM_TS_H264)
    {
        printf("[Ts Indexer] Only H264 video supported\n");
        return false;
    }
    video.pid=videoTrac[0].trackPid;

    memset(&data,0,sizeof(data));
    data.picStructure=pictureFrame;
    string indexName=string(file);
    indexName=indexName+string(".idx2");
    index=qfopen(indexName,(const char*)"wt");

    if(!index)
    {
        printf("[PsIndex] Cannot create %s\n",indexName.c_str());
        return false;
    }


    pkt=new tsPacketLinearTracker(videoTrac->trackPid, audioTracks);

    FP_TYPE append=FP_DONT_APPEND;
    if(true==ADM_probeSequencedFile(file))
    {
        if(true==GUI_Question(QT_TRANSLATE_NOOP("tsdemuxer","There are several files with sequential file names. Should they be all loaded ?")))
                bAppend=true;
    }
    if(bAppend==true)
        append=FP_APPEND;
    writeSystem(file,bAppend);
    pkt->open(file,append);
    data.pkt=pkt;
    fullSize=pkt->getSize();
    gui=createProcessing(QT_TRANSLATE_NOOP("tsdemuxer","Indexing"),pkt->getSize());
    int lastRefIdc=0;
    bool keepRunning=true;
    //******************
    // 1 search SPS
    //******************
      while(keepRunning)
      {
        int startCode=pkt->findStartCode();

        if(!pkt->stillOk())
        {
            keepRunning=false;
            continue;
        }
        if(startCode&0x80) continue; // Marker missing
        startCode&=0x1f;
        if(startCode!=NAL_SPS) 
            continue;

          // Got SPS!

          uint32_t xA,xR;
          // Get info
          pkt->getInfo(&tmpInfo);
          // Read just enough...
          {
            SEI_nal.empty();
            uint32_t code=0xffff+0xffff0000;
            while(((code&0xffffff)!=1) && pkt->stillOk())
            {
                    uint8_t r=pkt->readi8();
                    code=(code<<8)+r;
                    SEI_nal.pushByte(r);
            }
            if(!pkt->stillOk()) break;;
            pkt->seek(tmpInfo.startAt,tmpInfo.offset-5);
            if (extractSPSInfo(SEI_nal.payload, SEI_nal.payloadSize-4,&spsInfo))
            {
              ADM_info("[TsIndexer] Found video %" PRIu32"x%" PRIu32", fps=%" PRIu32"\n",video.w,video.h,video.fps);
              ADM_info("[TsIndexer] SPS says %" PRIu32"x%" PRIu32"\n",spsInfo.width,spsInfo.height);
              seq_found=1;
              video.w=spsInfo.width;
              video.h=spsInfo.height;
              video.fps=spsInfo.fps1000;
              xA=spsInfo.darNum;
              xR=spsInfo.darDen;
              writeVideo(&video,ADM_TS_H264);
              writeAudio();
              qfprintf(index,"[Data]");
              // Rewind

              break;
          };
        }
      }

        if(!seq_found) goto the_end;

        decodingImage=false;
    //******************
    // 2 Index
    //******************
        bool fourBytes;
      while(keepRunning)
      {
          fourBytes=false;
        int startCode=pkt->findStartCode2(fourBytes);
resume:
        if(!pkt->stillOk()) break;

        int startCodeLength=4;
        if(fourBytes==true) startCodeLength++;

//  1:0 2:Nal ref idc 5:Nal Type
        if(startCode&0x80)
        {
            printf("[Ts] Nal Marker missing:%x\n",startCode);
            continue; // Marker missing
        }
        int fullStartCode=startCode;
        int ref=(startCode>>5)&3;

        startCode&=0x1f; // Ignore nal ref IDR

        aprintf("[%02x] Nal :0x%x,ref=%d,lastRef=%d at : %d \n",fullStartCode,startCode,ref,lastRefIdc,pkt->getConsumed()-beginConsuming);

          // Ignore multiple chunk of the same pic
          if((startCode==NAL_NON_IDR || startCode==NAL_IDR)&&decodingImage )
          {
            aprintf("Still capturing, ignore\n");
            continue;
          }

          switch(startCode)
                  {
                  case NAL_AU_DELIMITER:
                        {
                          aprintf("AU DELIMITER\n");
                          decodingImage = false;
                        }
                          break;
                  case NAL_SEI:
                        {
                        // Load the whole NAL
                            SEI_nal.empty();
                            uint32_t code=0xffff+0xffff0000;
                            while(((0xffffff&code)!=1) && pkt->stillOk())
                            {
                                uint8_t r=pkt->readi8();
                                code=(code<<8)+r;
                                SEI_nal.pushByte(r);
                            }
                            if(!pkt->stillOk()) goto resume;
                            aprintf("[SEI] Nal size :%d\n",SEI_nal.payloadSize);
                            if(SEI_nal.payloadSize>=7)
                                decodeSEI(SEI_nal.payloadSize-4,
                                    SEI_nal.payload,&(thisUnit.recoveryCount),&(thisUnit.imageStructure));
                            else
                                    printf("[SEI] Too short size+4=%d\n",*(SEI_nal.payload));
                            startCode=pkt->readi8();

                            decodingImage=false;
                            pkt->getInfo(&thisUnit.packetInfo);
                            thisUnit.consumedSoFar=pkt->getConsumed();
                            if(!addUnit(data,unitTypeSei,thisUnit,startCodeLength+SEI_nal.payloadSize+1))
                                keepRunning=false;
                            fourBytes=true;
                            goto resume;
                            }
                            break;

                  case NAL_SPS:
                                decodingImage=false;
                                pkt->getInfo(&thisUnit.packetInfo);
                                if(firstSps)
                                {
                                    pkt->setConsumed(startCodeLength); // reset consume counter
                                    firstSps=false;
                                }
                                thisUnit.consumedSoFar=pkt->getConsumed();
                                if(!addUnit(data,unitTypeSps,thisUnit,startCodeLength))
                                    keepRunning=false;
                          break;

                  case NAL_IDR:
                  case NAL_NON_IDR:
                    {
#define NON_IDR_PRE_READ 8
                      aprintf("Pic start last ref:%d cur ref:%d nb=%d\n",lastRefIdc,ref,data.nbPics);
                      lastRefIdc=ref;

                      uint8_t bufr[NON_IDR_PRE_READ+4];
                      uint8_t header[NON_IDR_PRE_READ+4];


                        pkt->read(NON_IDR_PRE_READ,bufr);
                        // unescape...
                        ADM_unescapeH264(NON_IDR_PRE_READ,bufr,header);
                        //
                        getBits bits(NON_IDR_PRE_READ,header);
                        int first_mb_in_slice,slice_type;

                        first_mb_in_slice= bits.getUEG();
                        slice_type= bits.getUEG31();
                        if(slice_type>9)
                        {
                            printf("[TsIndexer] Bad slice type\n");
                        }
                        if(slice_type>4) slice_type-=5;
                        switch(slice_type)
                        {

                            case 0 : thisUnit.imageType=2;break; // P
                            case 1 : thisUnit.imageType=3;break; // B
                            case 2 : thisUnit.imageType=1;break; // I
                            default : thisUnit.imageType=2;break; // SP/SI
                        }
                      if(startCode==NAL_IDR) thisUnit.imageType=4; // IDR
                      aprintf("[>>>>>>>>] Pic Type %" PRIu32" Recovery %" PRIu32"\n",thisUnit.imageType,recoveryCount);
                      if(thisUnit.imageType==1 && !thisUnit.recoveryCount)
                                thisUnit.imageType=4; //I  + Recovery=0 = IDR!

                      data.nbPics++;



                      decodingImage=true;
                      pkt->getInfo(&thisUnit.packetInfo);
                      thisUnit.consumedSoFar=pkt->getConsumed();

                      if(!addUnit(data,unitTypePic,thisUnit,startCodeLength+NON_IDR_PRE_READ))
                          keepRunning=false;
                        // reset to default
                      thisUnit.imageStructure=pictureFrame;
                      thisUnit.recoveryCount=0xff;
                      pkt->invalidatePtsDts();
                    }

                    break;
                  default:
                      break;
          }
      } // End while
      result=true;
the_end:
        printf("\n");
        qfprintf(index,"\n[End]\n");
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

// Workaround win64 bug
// Put that in a separate function
bool decoderSei1(const ADM_SPSInfo &spsInfo,int size, uint8_t *bfer,pictureStructure *pic)
{
    if(spsInfo.hasStructInfo)
    {
      getBits bits(size,bfer);
      if(spsInfo.CpbDpbToSkip)
      {
              bits.get(spsInfo.CpbDpbToSkip);
      }
      //printf("Consumed: %d,\n",bits.getConsumedBits());
      int pic4=bits.get(4);
      aprintf("Pic struct: %d,\n",pic4);
      switch(pic4)
      {
          case 0: *pic=pictureFrame; break;
          case 3:
          case 4: *pic=pictureFrame;
          case 1: *pic=pictureTopField;break;
          case 2: *pic=pictureBottomField;break;
          default:*pic=pictureFrame;break;
      }
    }
    return true;
}
//
// 2nd one
bool decoderSei6(int size, uint8_t *bfer,uint32_t *recovery)
{
     getBits bits(size,bfer);
     *recovery=bits.getUEG();
     return true;
}


//

//EOF
