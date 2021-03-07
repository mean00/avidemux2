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
#include "ADM_h265_tag.h"
#include "ADM_vidMisc.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif
/**
 * 
 * @param sc
 * @return 
 */    
static const char *startCodeToString(int sc)
{
    int n= sizeof(nalDesc)/sizeof(NAL_DESC);
    for(int i=0;i<n;i++)
    {
        if(nalDesc[i].value==sc)
            return nalDesc[i].name;
    }
    return "????";
}

/**
        \fn decodeSEI
        \brief decode SEI to get short ref I
        @param recoveryLength # of recovery frame
        \return true if recovery found
*/
bool TsIndexerH265::decodeSEIH265(uint32_t nalSize, uint8_t *org,uint32_t *recoveryLength,
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
    while(true)
    {
      int startCode=pkt->findStartCode();
      if(!pkt->stillOk())
      {
          return false;
      }     
      aprintf("Match %x %d\n",startCode,((startCode>>1)&0x3f));
      startCode=((startCode>>1)&0x3f);  
      
      if(startCode!=match && match) 
          continue;
        dmxPacketInfo packetInfo;
        pkt->getInfo( &packetInfo);
      
      ADM_info("%s found at 0x%x+0x%x\n",name,(int)packetInfo.startAt,packetInfo.offset);
      return true;
    }
    return false;
}

/**
 * \fn findGivenStartCode
 * @param pkt
 * @param match: Startcode to find, zero means any startcode
 * @return 
 */
static uint8_t * findGivenStartCodeInBuffer(uint8_t *start, uint8_t *end,int match, const char *name)
{
    
    while(start+4<end)
    {
        if(!start[0]&&!start[1] && start[2]==0x01)
        {
            uint8_t code=(start[3]>>1)&0x3f;
            aprintf(" Matcho = %d\n",code);
            if(code==match || !match) return start;
        }
        start++;
    }
    ADM_warning("Cannot find %s\n",name);
    return NULL;
}

/**
 * \fn findH264SPS
 * @return 
 */
bool TsIndexerH265::findH265VPS(tsPacketLinearTracker *pkt,TSVideo &video)
{    
    dmxPacketInfo packetInfo;
    uint8_t headerBuffer[512+5]={0,0,0,1,(NAL_H265_VPS<<1)}; // we are forcing some bits to be zero...
    // This is a bit naive...
        
    if(!findGivenStartCode(pkt,NAL_H265_VPS ,"VPS"))
    {
        ADM_warning("Cannot find HEVC VPS\n");
        return false;
    }   
    
    pkt->getInfo( &packetInfo);
    thisUnit.consumedSoFar=0; // Head
    
    //uint64_t startExtraData=packetInfo.startAt-193; // /!\ It may be in the previous packet, very unlikely though
    pkt->read(512,headerBuffer+5);
    uint8_t *pointer=headerBuffer+5;
    uint8_t *end=headerBuffer+512;
    // Rewind
    if(packetInfo.offset>12) // 2 x 5 bytes long start code + 2 bytes AUD
        packetInfo.offset-=12;
    else
        packetInfo.offset=0;
    pkt->seek(packetInfo.startAt,packetInfo.offset);
    pkt->collectStats();

    pointer=findGivenStartCodeInBuffer(pointer,end,NAL_H265_SPS,"SPS");
    if(!pointer)
    {
        ADM_warning("Cannot find HEVC SPS\n");
        return false;
    }
    ADM_info("SPS found at %d\n",(int)(pointer-headerBuffer));
    pointer=findGivenStartCodeInBuffer(pointer,end,NAL_H265_PPS,"PPS");
    if(!pointer)
    {
        ADM_warning("Cannot find HEVC PPS\n");
        return false;
    }
    ADM_info("PPS found at %d\n",(int)(pointer-headerBuffer));
    pointer=findGivenStartCodeInBuffer(pointer+3,end,0,"Any");
    if(!pointer)
    {
        ADM_warning("Cannot find HEVC next marker\n");
        return false;
    }
    ADM_info("Any found at %d\n",(int)(pointer-headerBuffer));
    int extraLen=(int)(pointer-headerBuffer); // should be enough (tm)    
    
    ADM_info("VPS/SPS/PPS lengths = %d bytes \n",extraLen);
    
    if(!extractSPSInfoH265(headerBuffer,extraLen,&info))
    {
        ADM_warning("Cannot extract SPS/VPS/PPS\n");
        return false;
    }
    video.w=info.width;
    video.h=info.height;
    video.fps=info.fps1000;
    writeVideo(&video,ADM_TS_H265);
    writeAudio();
    qfprintf(index,"[Data]");
    
    ADM_info("Found video %d x %d\n",info.width,info.height);
    return true;
    
  
}
/**
 * \fn decodePictureType
 */
int  TsIndexerH265::decodePictureTypeH265(int nalType,getBits &bits)
{
    //
    //  1-6-1 Forbidden + nal unit + 1:nuh_layer_id <= Already consumed
    //  5: nuh_layer_id +3 nugh temporal id <= Still there
    bits.skip(8);  // Leftover layer ID + temporal plus 1
    
    int slice=2;
    bool firstSliceInPic=bits.get(1);
    
    if(!firstSliceInPic) return -1;
    
    bool no_output_of_prior_pics_flag=false;
    bool segmentFlag=false;
    if(nalType  >= NAL_H265_BLA_W_LP && nalType <= NAL_H265_IRAP_VCL23) // 7.3.6 IRAP_VCL23
    {
        no_output_of_prior_pics_flag=bits.get(1); ;
    }
    bits.getUEG(); // PPS
    if(!firstSliceInPic)
    {
        if(info.dependent_slice_segments_enabled_flag)
        {
            segmentFlag=bits.get(1);
        }
        int address=bits.get(info.address_coding_length); //  log2 ( width*height/64*64)
        aprintf("Adr=%d / %d\n",address,64*34);
    }
    if(segmentFlag)
    {
        aprintf("Nope\n");
        return -1; 
    }
    
    if(info.num_extra_slice_header_bits)
        bits.skip(info.num_extra_slice_header_bits); // not sure..
    int sliceType=bits.getUEG();
    switch(sliceType)
    {
        case 0: slice=3;  // B
                break;
        case 1: slice=2; // P
                break; 
        case 2: slice=1;     // I        
                if(( nalType==NAL_H265_IDR_W_RADL ) || (nalType==NAL_H265_IDR_N_LP )) // IDR ?
                    slice=4;
                break; 
        default:
                slice=-1;
                ADM_warning("Unknown slice type %d \n",sliceType);
                break;
    }
    aprintf("SliceType==> %d xxx\n",slice);
    return slice;
}
/**
    \fn run
    \brief Index H265 stream
*/
uint8_t TsIndexerH265::run(const char *file,ADM_TS_TRACK *videoTrac)
{
    TSVideo video;
    indexerData data;

    listOfUnits.clear();

    printf("Starting H265 indexer\n");
    if(!videoTrac) return false;
    if(videoTrac[0].trackType!=ADM_TS_H265)
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
        printf("[TsIndexerH265] Cannot create %s\n",indexName.c_str());
        return false;
    }

    uint8_t result=0;
    bool seq_found=false;
    bool firstSps=true;
    uint64_t lastAudOffset=0;
    int audCount=0;
    int audStartCodeLen=5;
    int lastRefIdc=0;
    dmxPacketInfo packetInfo;

    int append=0;
#ifdef ASK_APPEND_SEQUENCED
    append=1;
    {
    int nbFollowUps=ADM_probeSequencedFile(file,&append);
    if(nbFollowUps<0)
    {
        qfclose(index);
        index=NULL;
        return 0;
    }
    if(!nbFollowUps || false==GUI_Question(QT_TRANSLATE_NOOP("tsdemuxer","There are several files with sequential file names. Should they be all loaded ?")))
        append=0;
    }
#endif
    writeSystem(file,append);

    pkt=new tsPacketLinearTracker(videoTrac->trackPid, audioTracks);

    if(!pkt->open(file,append))
        goto the_end;
    data.pkt=pkt;
    fullSize=pkt->getSize();
    gui=createProcessing(QT_TRANSLATE_NOOP("tsdemuxer","Indexing"),fullSize);

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
    
     decodingImage=false;
    //******************
    // 2 Index
    //******************
    bool fourBytes;
    while(true)
    {
        fourBytes=false;
        int startCode=pkt->findStartCode2(fourBytes);
        if(!pkt->stillOk()) break;

        int startCodeLength=4;
        if(fourBytes==true) startCodeLength++;

        startCode=((startCode>>1)&0x3f);   

#if 0
        pkt->getInfo(&packetInfo);
        aprintf("Startcode =%d:%s, decoding image=%d,%s\n",startCode,startCodeToString(startCode),decodingImage,ADM_us2plain(packetInfo.dts));
#endif
#define NON_IDR_PRE_READ 32 

        switch(startCode)
        {
            case NAL_H265_AUD:
            {
                aprintf("AU DELIMITER\n");
                decodingImage = false;
                pkt->getInfo(&packetInfo,startCodeLength);
                lastAudOffset=pkt->getConsumed();
                audStartCodeLen = startCodeLength;
                audCount++;
                break;
            }
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
            {
                uint8_t buffer[NON_IDR_PRE_READ],header[NON_IDR_PRE_READ];
                int preRead=NON_IDR_PRE_READ;
                if(!audCount)
                {
                    pkt->getInfo(&packetInfo,startCodeLength);
                    thisUnit.consumedSoFar=pkt->getConsumed();
                }else
                {
                    thisUnit.consumedSoFar=lastAudOffset;
                    startCodeLength = audStartCodeLen;
                }
                // Read the beginning of the picture to get its type...
                pkt->read(preRead,buffer);
                ADM_unescapeH264(preRead,buffer,header);
                getBits bits(preRead,header);
                // Try to see if we have a valid beginning of image
                int picType=decodePictureTypeH265(startCode,bits);
                if(picType!=-1)
                {
                    data.nbPics++;
                    decodingImage=true;
                    thisUnit.packetInfo=packetInfo;
                    thisUnit.imageType=picType;
                    thisUnit.unitType=unitTypePic;
                    if(!addUnit(data,unitTypePic,thisUnit,startCodeLength))
                    {
                        result=ADM_IGN;
                        goto the_end;
                    }
                    // reset to default
                    thisUnit.imageStructure=pictureFrame;
                    thisUnit.recoveryCount=0xff;
                    pkt->invalidatePtsDts();
                    audCount=0;
                }
                break;
            }
            case NAL_H265_VPS:
            {
                decodingImage=false;
                if(!audCount)
                    pkt->getInfo(&packetInfo,startCodeLength);
                thisUnit.packetInfo=packetInfo;
                if(firstSps)
                {
                    uint64_t pos=startCodeLength;
                    if(audCount)
                    {
                        pos+=pkt->getConsumed();
                        pos-=lastAudOffset;
                        startCodeLength = audStartCodeLen;
                    }
                    pkt->setConsumed(pos); // reset consume counter
                    thisUnit.consumedSoFar=pos;
                    firstSps=false;
                }else
                {
                    if(audCount)
                    {
                        thisUnit.consumedSoFar = lastAudOffset;
                        startCodeLength = audStartCodeLen;
                    }else
                    {
                        thisUnit.consumedSoFar = pkt->getConsumed();
                    }
                }
                if(!addUnit(data,unitTypeSps,thisUnit,startCodeLength))
                {
                    result=ADM_IGN;
                    goto the_end;
                }
                break;
            }
            default:
                break;
        }
    } // End while
    result=1;
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
