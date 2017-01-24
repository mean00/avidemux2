/***************************************************************************
    \brief TS indexer, H265 video
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

#define LIST_OF_NAL_TYPE\
    NAME(NAL_H265_TRAIL_N,      0)    , \
    NAME(NAL_H265_TRAIL_R,      1)    , \
    NAME(NAL_H265_TSA_N      , 2),\
    NAME(NAL_H265_TSA_R      , 3),\
    NAME(NAL_H265_STSA_N     , 4),\
    NAME(NAL_H265_STSA_R     , 5),\
    NAME(NAL_H265_RADL_N     , 6),\
    NAME(NAL_H265_RADL_R     , 7),\
    NAME(NAL_H265_RASL_N     , 8),\
    NAME(NAL_H265_RASL_R     , 9),\
    NAME(NAL_H265_BLA_W_LP   , 16),\
    NAME(NAL_H265_BLA_W_RADL , 17),\
    NAME(NAL_H265_BLA_N_LP   , 18),\
    NAME(NAL_H265_IDR_W_RADL , 19),\
    NAME(NAL_H265_IDR_N_LP,     20) , \
    NAME(NAL_H265_CRA_NUT    ,  21),\
    NAME(NAL_H265_VPS  ,        32)    ,\
    NAME(NAL_H265_SPS  ,        33)    ,\
    NAME(NAL_H265_PPS  ,        34)    ,\
    NAME(NAL_H265_AUD  ,        35)    ,\
    NAME(NAL_H265_SEI_PREFIX,   39),\
    NAME(NAL_H265_SEI_SUFFIX,   40),\


#define NAME(x,y) x= y

enum{
LIST_OF_NAL_TYPE
};
#undef NAME
#define NAME(x,y) {y,#x}

typedef struct NAL_DESC{int value; const char *name;}NAL_DESC;

NAL_DESC nalDesc[]={
    LIST_OF_NAL_TYPE
};
        
/**
        \fn decodeSEI
        \brief decode SEI to get short ref I
        @param recoveryLength # of recovery frame
        \return true if recovery found
*/
bool TsIndexer::decodeSEIH265(uint32_t nalSize, uint8_t *org,uint32_t *recoveryLength,
                pictureStructure *picStruct)
{
#if 0
    if(nalSize+16>=ADM_NAL_BUFFER_SIZE)
    {
        ADM_warning("SEI size too big, probably corrupted input (%u bytes)\n",nalSize);
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
                aprintf("  [SEI] Type: 0x%x, size: %u\n",sei_type,sei_size);
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
#endif
   return false;
}

/**
 * \fn findGivenStartCode
 * @param pkt
 * @param match
 * @return 
 */
static bool findGivenStartCode(tsPacketLinearTracker *pkt,int match, const char *name)
{
    bool keepRunning=true;    
    while(keepRunning)
    {
      int startCode=pkt->findStartCode();
      if(!pkt->stillOk())
      {
          return false;
      }     
      startCode=((startCode>>1)&0x3f);  
      printf("Match %d\n",startCode);
      if(startCode!=match) 
          continue;
      ADM_info("%s found at 0x%x\n",name,(int)pkt->getPos());
      return true;
    }
    return false;
}
/**
 * 
 * @param pkt
 * @return 
 */
#define SPS_HEADER_LENGTH 21
bool TsIndexer::decodeH265SPS(tsPacketLinearTracker *pkt)
{
    uint8_t buffer[SPS_HEADER_LENGTH];
    pkt->read(SPS_HEADER_LENGTH,buffer);
    
    getBits bits(SPS_HEADER_LENGTH,buffer);
    
    bits.skip(4); //videp parameter ID
    bits.skip(3); // max layer minus 1
    bits.skip(1); // nesting flag
    
    // Profile / Tier / level
    bits.get(2); // profile space
    bits.get(1); // tier flag
    int profile=bits.get(5); // profile flag
    for(int j=0;j<32;j++)
        bits.get(1);
    bits.get(1); // progressive
    bits.get(1); // interlaced
    bits.get(1); // non packed
    bits.get(1); // frame only
    bits.get(16);
    bits.get(16);
    bits.get(11); // reserved 43 bits
    bits.get(1); // reserved 1 bits
    int idec=bits.get(8); // general level idc
    bits.get(1); // sub layer
    bits.get(1); // sub layer
    bits.get(2); // reserved
    
    printf("Profile=%d\n",profile);
    bits.get(8); // General level idc
    bits.get(16);
    bits.get(16);
    bits.get(11); // reserved 43 bits
    bits.get(1); // reserved 1 bits
    // sub layer level idc 8
    //
    bits.getUEG(); // SPS Seq ID
    int chroma=bits.getUEG(); // Chroma
    
    int width=bits.getUEG(); 
    int height=bits.getUEG(); 
    
    printf("SPS : Dimension = Chroma =%d, %d x %d\n",chroma,width,height);
    return true;
    
}
/**
 * \fn findH264SPS
 * @return 
 */
bool TsIndexer::findH265VPS(tsPacketLinearTracker *pkt,TSVideo &video)
{    
    bool keepRunning=true;
    // This is a bit naive...
    if(!findGivenStartCode(pkt,NAL_H265_VPS ,"VPS"))
    {
        ADM_warning("Cannot find HEVC VPS\n");
        return false;
    }   
    uint64_t startExtraData=pkt->getPos()-5;
    if(!findGivenStartCode(pkt,NAL_H265_SPS ,"SPS"))
    {
        ADM_warning("Cannot find HEVC SPS\n");
        return false;
    }

    if(!findGivenStartCode(pkt,NAL_H265_PPS ,"PPS"))
    {
        ADM_warning("Cannot find HEVC PPS\n");
        return false;
    }
    pkt->findStartCode();
    uint64_t endExtraData=pkt->getPos()-4; // should be enough
    
    pkt->setPos(startExtraData);
    
    int extraLen=188+(int)(endExtraData-startExtraData);
    
    uint8_t *extra=(uint8_t *)admAlloca(extraLen);
    pkt->read(extraLen,extra);
    pkt->setPos(endExtraData);
    
    ADM_info("VPS/SPS/PPS lengths = %d bytes \n",extraLen);
    ADM_SPSInfo info;
    if(!extractSPSInfoH265(extra,extraLen,&info))
    {
        ADM_warning("Cannot extract SPS/VPS/PPS\n");
        return false;
    }
    video.w=info.width;
    video.h=info.height;
    ADM_info("Found video %d x %d\n",info.width,info.height);
    return true;
    
  
}
/**
    \fn runH264
    \brief Index H264 stream
*/
bool TsIndexer::runH265(const char *file,ADM_TS_TRACK *videoTrac)
{

bool    seq_found=false;
bool    firstSps=true;
TS_PESpacket SEI_nal(0);
TSVideo video;
indexerData  data;

bool result=false;
bool bAppend=false;

    beginConsuming=0;
    listOfUnits.clear();

    printf("Starting H264 indexer\n");
    if(!videoTrac) return false;
    if(videoTrac[0].trackType!=ADM_TS_H265 
       )
    {
        printf("[Ts Indexer] Only H265 video supported\n");
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
    switch(videoTrac[0].trackType)
    {
        case ADM_TS_H265 :
            seq_found=findH265VPS(pkt,video);
            break;
        default:
            break;
    }    
    if(!seq_found) goto the_end;

    video.w=1980*2;
    video.h=1080*2;
    video.fps=23976;
    writeVideo(&video,ADM_TS_H265);
    writeAudio();
    qfprintf(index,"[Data]");

    
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

        startCode=((startCode>>1)&0x3f);   
        printf("Startcode =%d\n",startCode);
          // Ignore multiple chunk of the same pic
          if((startCode==NAL_NON_IDR || startCode==NAL_IDR)&&decodingImage )
          {
            aprintf("Still capturing, ignore\n");
            continue;
          }
#define NON_IDR_PRE_READ 8 // FIXME

          switch(startCode)
                  {
                  case NAL_H265_AUD:
                        {
                          aprintf("AU DELIMITER\n");
                          decodingImage = false;
                        }
                          break;
                case NAL_H265_TRAIL_R:
                case NAL_H265_TRAIL_N:
                case NAL_H265_TSA_N:
                case NAL_H265_TSA_R:
                case NAL_H265_STSA_N:
                case NAL_H265_STSA_R:
                case NAL_H265_BLA_W_LP:
                case NAL_H265_BLA_W_RADL:
                case NAL_H265_BLA_N_LP:
                case NAL_H265_IDR_W_RADL:
                case NAL_H265_IDR_N_LP:
                case NAL_H265_CRA_NUT:
                case NAL_H265_RADL_N:
                case NAL_H265_RADL_R:
                case NAL_H265_RASL_N:
                case NAL_H265_RASL_R:
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
                    break;
                  case NAL_H265_SPS:
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

#if 0                        
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
#endif                      
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


//

//EOF
