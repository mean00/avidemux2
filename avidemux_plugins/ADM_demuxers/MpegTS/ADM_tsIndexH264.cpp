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
#include <algorithm>

static bool decoderSei1(const ADM_SPSInfo &spsInfo, uint32_t size, uint8_t *bfer, pictureStructure *pic);
static bool decoderSei6(uint32_t size, uint8_t *bfer, uint32_t *recovery);
/**
 *  \fn decodeSEI
 *  \brief decode SEI to get short ref I
 *  @param recoveryLength # of recovery frame, may be NULL
 *  \return 1 if picture structure found, 6 if recovery found, 7 if both, 0 if none.
 */
uint8_t TsIndexerH264::decodeSEI(uint32_t nalSize, uint8_t *org,uint32_t *recoveryLength,
                pictureStructure *picStruct)
{
    if(nalSize+16>=ADM_NAL_BUFFER_SIZE)
    {
        ADM_warning("SEI size too big, probably corrupted input (%u bytes)\n",nalSize);
        return false;
    }
    uint8_t *payload=payloadBuffer;
    uint8_t ret=0;
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
        if(payload+sei_size>tail)
            return ret;

        switch(sei_type)
        {
            case 1:
            {
                decoderSei1(spsInfo,sei_size,payload,picStruct);
                ret+=1;
                break;
            }
            case 6: // Recovery point
            {
                if(!recoveryLength) break;
                decoderSei6(sei_size,payload,recoveryLength);
                aprintf("[SEI] Recovery :%" PRIu32"\n",*recoveryLength);
                ret+=6;
                break;
            }
            default:break;
        }
        payload+=sei_size;
    }
    //if(payload+1<tail) ADM_warning("Bytes left in SEI %d\n",(int)(tail-payload));
    return ret;
}
/**
 * \fn findH264SPS
 * @return 
 */
bool TsIndexerH264::findH264SPS(tsPacketLinearTracker *pkt,TSVideo &video)
{
    dmxPacketInfo tmpInfo;
    uint64_t rewindStart=0;
    uint32_t rewindOffset=0;
    bool sps_found=false;
    bool sei_found=false;
    bool no_timing=false;
    std::vector <uint64_t> listOfPts;
    TS_PESpacket SEI_nal(0);
    while(true)
    {
        uint32_t startCode=pkt->findStartCode();

        if(!pkt->stillOk()) break;
        if(startCode&0x80) continue; // Marker missing
        startCode&=0x1f;
        if(sps_found && no_timing)
        {
            dmxPacketInfo tme;
            pkt->getInfo(&tme);
            if(tme.pts != ADM_NO_PTS)
            {
                if(listOfPts.empty() || (listOfPts.size() && listOfPts.back() != tme.pts))
                    listOfPts.push_back(tme.pts);
            }
        }
        if(!sps_found && startCode!=NAL_SPS)
            continue;
        if(startCode!=NAL_SPS && startCode!=NAL_SEI)
            continue;

        // Got SPS!
        if(startCode==NAL_SPS && sps_found) // most likely the next keyframe
        {
            if(!sei_found)
                ADM_warning("No picture timing SEI found within the first GOP, can't check interlacing.\n");
            break;
        }
        // Get info
        pkt->getInfo(&tmpInfo);
        // Read just enough...
        SEI_nal.empty();
        uint32_t code=0xffff+0xffff0000;
        while(((code&0xffffff)!=1) && pkt->stillOk())
        {
            uint8_t r=pkt->readi8();
            code=(code<<8)+r;
            SEI_nal.pushByte(r);
        }
        if(!pkt->stillOk()) break;
        if(startCode==NAL_SPS &&!sps_found && extractSPSInfo(SEI_nal.payload, SEI_nal.payloadSize-3, &spsInfo))
        {
            ADM_info("[TsIndexer] Found video %" PRIu32"x%" PRIu32", fps=%" PRIu32"\n",video.w,video.h,video.fps);
            ADM_info("[TsIndexer] SPS says %" PRIu32"x%" PRIu32"\n",spsInfo.width,spsInfo.height);
            sps_found=true;
            rewindStart=tmpInfo.startAt;
            rewindOffset=tmpInfo.offset-5;
            video.w=spsInfo.width;
            video.h=spsInfo.height;
            video.fps=spsInfo.fps1000;
            if(!video.fps || video.fps > 1000000 /* arbitrary limit, tbc obviously doesn't match fps */ )
                no_timing=true;
            spsLen=SEI_nal.payloadSize-3;
            if(spsLen>ADM_NAL_BUFFER_SIZE)
                spsLen=ADM_NAL_BUFFER_SIZE;
            memcpy(spsCache,SEI_nal.payload,spsLen);
            continue; // we miss the next NALU, should be harmless
        }
        if(startCode==NAL_SEI && sps_found && !sei_found && SEI_nal.payloadSize>=7)
        {
            pictureStructure p=pictureFrame;
            if(decodeSEI(SEI_nal.payloadSize-4, SEI_nal.payload, NULL, &p) & 1)
            {
                video.interlaced=(p!=pictureFrame);
                sei_found=true;
                // keep collecting pts if we cannot use codec time base for fps
                if(!no_timing)
                    break;
            }
            startCode=pkt->readi8(); // peek now, findStartCode will miss it
            if((startCode&0x1f)==NAL_SPS) break;
        }
    }
    // Try to derive fps from a sorted list of pts from the first GOP.
    if(no_timing && listOfPts.size() > 1)
    {
        std::sort(listOfPts.begin(), listOfPts.end());
        uint64_t delta = ADM_NO_PTS;
        uint64_t previous = listOfPts.front();
        std::vector<uint64_t>::iterator it;
        for(it = listOfPts.begin(); it != listOfPts.end(); it++)
        {
            if(*it == previous)
                continue;
            uint64_t min = *it - previous;
            if(min < delta)
                delta = min;
        }
        if(delta != ADM_NO_PTS)
        {
            ADM_info("Minimum PTS delta in 90 kHz ticks: %" PRIu64"\n",delta);
            double d = delta;
            d /= 9.;
            d *= 100.;
            d += 0.49;
            delta = (uint64_t)d; // in microseconds;
            if(delta < 10000 || delta > 42000)
            {
                ADM_warning("Minimum delta %" PRIu64" out of range, probably bogus, hardcoding fps1000 to 25000\n",delta);
                video.fps = 25000;
            }else
            {
                video.fps = ADM_Fps1000FromUs(delta);
                ADM_info("Setting fps1000 to %u\n",video.fps);
            }
        }else
        {
            ADM_warning("No usable timing information found, hardcoding fps1000 to 25000\n");
            video.fps = 25000;
        }
    }
    if(sps_found)
    {
        pkt->seek(rewindStart,rewindOffset);
        pkt->collectStats();
        writeVideo(&video,ADM_TS_H264);
        writeAudio();
        qfprintf(index,"[Data]");
    }
    return sps_found;
}
/**
    \fn run
    \brief Index H264 stream
*/
uint8_t TsIndexerH264::run(const char *file, ADM_TS_TRACK *videoTrac)
{
    bool seq_found=false;
    bool firstSps=true;
    TS_PESpacket SEI_nal(0);
    TSVideo video;
    indexerData data;

    uint8_t result=0;

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
        ADM_error("[TsIndexerH264] Cannot create %s\n",indexName.c_str());
        return 0;
    }

    uint64_t lastAudOffset=0;
    int audCount=0;
    int audStartCodeLen=5;
    int lastRefIdc=0;
    bool keepRunning=true;
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
        case ADM_TS_H264 :
            seq_found=findH264SPS(pkt,video);
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
            printf("[Ts] Nal Marker missing: 0x%x\n",startCode);
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
                pkt->getInfo(&packetInfo,startCodeLength);
                lastAudOffset=pkt->getConsumed();
                audStartCodeLen=startCodeLength;
                audCount++;
            }
                break;
            case NAL_SEI:
            {
                if(!audCount)
                {
                    pkt->getInfo(&packetInfo,startCodeLength);
                    thisUnit.consumedSoFar=pkt->getConsumed();
                }else
                {
                    thisUnit.consumedSoFar=lastAudOffset;
                    startCodeLength=audStartCodeLen;
                }
                thisUnit.packetInfo=packetInfo;
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
                bool gotPictStructInfo=false;
                if(SEI_nal.payloadSize>=7)
                {
                    if(decodeSEI(SEI_nal.payloadSize-4,
                              SEI_nal.payload,
                              &(thisUnit.recoveryCount),
                              &(thisUnit.imageStructure)) & 1)
                        gotPictStructInfo=true;
                }else
                    printf("[SEI] Too short size+4=%d\n",*(SEI_nal.payload));
                startCode=pkt->readi8();

                decodingImage=false;
                if(gotPictStructInfo && !addUnit(data,unitTypeSei,thisUnit,startCodeLength))
                {
                    result=ADM_IGN;
                    goto the_end;
                }
                fourBytes=true;
                goto resume;
            }
                break;

            case NAL_SPS:
            {
                decodingImage=false;
                if(!audCount)
                    pkt->getInfo(&packetInfo,startCodeLength);
                thisUnit.packetInfo=packetInfo;
                if(firstSps)
                {
                    uint64_t pos=0;
                    if(audCount) // Currently always false because we don't rewind after findH264SPS()
                    {
                        pos+=pkt->getConsumed();
                        pos-=lastAudOffset;
                        startCodeLength=audStartCodeLen;
                    }
                    pos+=startCodeLength;
                    pkt->setConsumed(pos); // reset consume counter
                    thisUnit.consumedSoFar=pos;
                    firstSps=false;
                }else
                {
                    if(audCount)
                    {
                        thisUnit.consumedSoFar=lastAudOffset;
                        startCodeLength=audStartCodeLen;
                    }else
                    {
                        thisUnit.consumedSoFar=pkt->getConsumed();
                    }
                    SEI_nal.empty();
                    uint32_t code=0xffff+0xffff0000;
                    while(((0xffffff&code)!=1) && pkt->stillOk())
                    {
                        uint8_t r=pkt->readi8();
                        code=(code<<8)+r;
                        SEI_nal.pushByte(r);
                    }
                    if(!pkt->stillOk()) goto resume;
                    uint32_t tmpLen=SEI_nal.payloadSize;
                    aprintf("[SPS] Nal size :%d\n",tmpLen);
                    if(tmpLen>=7 && tmpLen<=ADM_NAL_BUFFER_SIZE+3)
                    {
                        tmpLen-=3;
                        bool match=!memcmp(spsCache, SEI_nal.payload, (spsLen > tmpLen)? tmpLen : spsLen);
                        if(!match)
                        {
                            ADM_warning("SPS changing midstream? Checking deeper...\n");
                            ADM_SPSInfo tmpInfo;
                            if(extractSPSInfo(SEI_nal.payload, tmpLen, &tmpInfo))
                            {
                                if(spsInfo.width!=tmpInfo.width || spsInfo.height!=tmpInfo.height)
                                {
                                    GUI_Info_HIG(ADM_LOG_IMPORTANT, QT_TRANSLATE_NOOP("tsdemuxer","Size Change"),
                                                 QT_TRANSLATE_NOOP("tsdemuxer","The size of the video changes at frame %u from %ux%u to %ux%u. "
                                                                   "This is unsupported, stopping here."),
                                                 data.nbPics, spsInfo.width, spsInfo.height, tmpInfo.width, tmpInfo.height);
                                    result=1;
                                    goto the_end;
                                }
                                match=true;
#define MATCH(x) if(spsInfo.x != tmpInfo.x) { ADM_warning("%s value does not match.\n",#x); spsInfo.x = tmpInfo.x; match=false; }

                                MATCH(CpbDpbToSkip)
                                MATCH(hasPocInfo)
                                MATCH(log2MaxFrameNum)
                                MATCH(log2MaxPocLsb)
                                MATCH(frameMbsOnlyFlag)
                                MATCH(refFrames)

                                if(!match)
                                {
                                    ADM_warning("Codec parameters change on the fly at frame %u, expect problems.\n",data.nbPics);
                                    // Nevertheless, update the cached SPS
                                    memcpy(spsCache, SEI_nal.payload, tmpLen);
                                    spsLen=tmpLen;
                                }
                            }
                        }
                    }
                }
                if(!addUnit(data,unitTypeSps,thisUnit,startCodeLength))
                {
                    result=ADM_IGN;
                    goto the_end;
                }
            }
                break;

            case NAL_IDR:
            case NAL_NON_IDR:
            {
                if(!audCount)
                {
                    pkt->getInfo(&packetInfo,startCodeLength);
                    thisUnit.consumedSoFar=pkt->getConsumed();
                }else
                {
                    thisUnit.consumedSoFar=lastAudOffset;
                    startCodeLength=audStartCodeLen;
                }
                thisUnit.packetInfo=packetInfo;
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
                    default: thisUnit.imageType=2;break; // SP/SI
                }
                if(startCode==NAL_IDR) thisUnit.imageType=4; // IDR
                aprintf("[>>>>>>>>] Pic Type %" PRIu32" Recovery %" PRIu32"\n",thisUnit.imageType,recoveryCount);
                if(thisUnit.imageType==1 && !thisUnit.recoveryCount)
                    thisUnit.imageType=4; //I  + Recovery=0 = IDR!

                data.nbPics++;
                decodingImage=true;
                audCount=0;

                if(!addUnit(data,unitTypePic,thisUnit,startCodeLength))
                {
                    result=ADM_IGN;
                    goto the_end;
                }
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


/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/

// Workaround win64 bug
// Put that in a separate function
bool decoderSei1(const ADM_SPSInfo &spsInfo, uint32_t size, uint8_t *bfer, pictureStructure *pic)
{
    if(spsInfo.hasStructInfo)
    {
      uint32_t paddedSize=size+64; // AV_INPUT_BUFFER_PADDING_SIZE, guards against lavcodec overread
      uint8_t *padded=(uint8_t *)ADM_alloc(paddedSize);
      if(!padded) return false;

      memcpy(padded,bfer,size);
      memset(padded+size,0,64);

      getBits bits(size,padded);
      if(spsInfo.CpbDpbToSkip)
      {
              bits.get(spsInfo.CpbDpbToSkip);
      }
      //printf("Consumed: %d,\n",bits.getConsumedBits());
      int pic4=bits.get(4);

      ADM_dealloc(padded);
      padded=NULL;

      aprintf("Pic struct: %d,\n",pic4);
      switch(pic4)
      {
          case 0: *pic=pictureFrame; break;
          case 1: *pic=pictureFieldTop;break; // field-encoded, top field
          case 3: *pic=pictureTopFirst;break; // frame-encoded, displayed as fields, top first
          case 2: *pic=pictureFieldBottom;break; // field-encoded, bottom field
          case 4: *pic=pictureBottomFirst;break; // frame-encoded, displayed as fields, bottom first
          default:*pic=pictureFrame;break;
      }
    }
    return true;
}
//
// 2nd one
bool decoderSei6(uint32_t size, uint8_t *bfer, uint32_t *recovery)
{
    uint32_t paddedSize=size+64; // AV_INPUT_BUFFER_PADDING_SIZE, guards against lavcodec overread
    uint8_t *padded=(uint8_t *)ADM_alloc(paddedSize);
    if(!padded) return false;

    memcpy(padded,bfer,size);
    memset(padded+size,0,64);

    getBits bits(size,padded);
    *recovery=bits.getUEG();

    ADM_dealloc(padded);
    padded=NULL;
    return true;
}


//

//EOF
