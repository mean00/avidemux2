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
#include <climits>

#include "ADM_default.h"
#include "ADM_Video.h"
//#include "fourcc.h"

#include "ADM_mkv.h"

#include "mkv_tags.h"
#include "ADM_codecType.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_vidMisc.h"
#define VIDEO _tracks[0]

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

/**
    \fn indexLastCluster
    \brief Parse the most recently found cluster.
*/
uint8_t mkvHeader::indexLastCluster(ADM_ebml_file *parser)
{
    ADM_assert(readBuffer);
    ADM_assert(_work);
    ADM_assert(_clusters.size());

    uint64_t len,id,off;
    ADM_MKV_TYPE type;
    const char *ss;

    uint64_t fileSize = parser->getFileSize();
    int lastCluster = _clusters.size() - 1;
    uint8_t res = 1;

    mkvIndex *cdx = &(_clusters[lastCluster]);
    parser->seek(cdx->pos);
    ADM_ebml_file cluster(parser, cdx->size);

    while(!cluster.finished())
    {
        if(!_work->isAlive())
            return ADM_IGN;

        off = cluster.tell();
        _work->update(off >> 10, fileSize >> 10);
        if(!cluster.readElemId(&id,&len))
            break;
        if(!ADM_searchMkvTag((MKV_ELEM_ID)id,&ss,&type))
        {
            ADM_warning("Cluster %d, tag 0x%" PRIx64" at 0x%" PRIx64" not found (len %" PRIu64")\n",lastCluster,id,cluster.tell()-2,len);
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
                indexBlock(parser,len,cdx->Dts);
                break;
            case MKV_BLOCK_GROUP:
            {
                //printf("Block Group\n");
                ADM_ebml_file blockGroup(parser,len);
                while(!blockGroup.finished())
                {
                    if(!blockGroup.readElemId(&id,&len))
                        break;
                    if(!ADM_searchMkvTag((MKV_ELEM_ID)id,&ss,&type))
                    {
                        ADM_warning("Block group in cluster %d, tag 0x%" PRIx64" at 0x%" PRIx64" not found (len %" PRIu64")\n",lastCluster,id,blockGroup.tell()-2,len);
                        blockGroup.skip(len);
                        continue;
                    }
                    //printf("\t\t\t\tFound %s\n",ss);
                    switch(id)
                    {
                        default:
                            blockGroup.skip(len);
                            break;
                        case MKV_BLOCK:
                        case MKV_SIMPLE_BLOCK:
                            indexBlock(&blockGroup,len,cdx->Dts);
                            break;
                    }
                }
            }
            break; // Block Group
        }
    }
    //printf("[MKV] ending cluster at 0x%" PRIx64"\n",parser->tell());
    return res;
}
/**
      \fn indexBlock
      \brief index a block, identify it and update index
*/
uint8_t mkvHeader::indexBlock(ADM_ebml_file *parser,uint32_t len,uint32_t clusterTimeCodeMs)
{
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

    //int lacing=((flags>>1)&3);

    uint32_t entryFlags = (flags & 0x80)? AVI_KEY_FRAME : 0;

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

static bool canRederiveFrameType(uint32_t fcc)
{
    if(isMpeg4Compatible(fcc) || isVC1Compatible(fcc)) return true;
    if(isH264Compatible(fcc)) return true;
    if(isH265Compatible(fcc)) return true;
    if(isMpeg12Compatible(fcc)) return true;
    return false;

}

/**
    \fn addVideoEntry
    \brief add an entry to the video index
    @param timecodeMS PTS of the frame in ms!
*/
uint8_t mkvHeader::addIndexEntry(uint32_t track,ADM_ebml_file *parser,uint64_t where, uint32_t size,uint32_t flags,uint32_t timecodeMS)
{
    mkvTrak *Track=&(_tracks[track]);
    mkvIndex ix;

    ix.pos=where;
    ix.size=size;
    ix.flags=flags;
    ix.Dts=timecodeMS*_timeBase;
    ix.Pts=timecodeMS*_timeBase;
    //printf("Track=%d, timecode=%d timeBase=%d, Pts=%d\n",track,(int)timecodeMS,(int)_timeBase,ix.Pts);
    uint32_t rpt=_tracks[0].headerRepeatSize;
    uint32_t frameNo = Track->index.size();

    // expand buffer if needed
    if(size + rpt > readBufferSize)
    {
        ADM_info("Expanding read buffer for frame %" PRIu32" of size %" PRIu32" in track %" PRIu32" from %" PRIu32" to %" PRIu32" bytes.\n",
            frameNo, size, track, readBufferSize, rpt + size*2);
        delete [] readBuffer;
        readBufferSize = rpt + size*2;
        readBuffer=new uint8_t[readBufferSize];
        memset(readBuffer,0,readBufferSize);
    }
    if(!track)
    {
        aprintf("adding image at 0x%llx , time = %d\n",where,timecodeMS);
        if(Track->_needExtraData > 0 && (flags & AVI_KEY_FRAME))
        {
            if(rpt)
                memcpy(readBuffer,_tracks[0].headerRepeat,rpt);
            parser->readBin(readBuffer+rpt,size-3);
            uint8_t *dest = NULL;
            int l = ADM_extractVideoExtraData(_videostream.fccHandler, size, readBuffer, &dest);
            if(l < 0)
            {
                Track->_needExtraData = l;
            }else if(l > 0)
            {
                Track->extraData = dest;
                Track->extraDataLen = l;
                Track->_needExtraData = 0;
            }
        }
    }
    // since frame type is unreliable for mkv, we scan each frame
    // For the 2 most common cases : mp4 & h264.
    // Hackish, we already read the 3 bytes header
    // But they are already taken into account in the size part
    if(!track && canRederiveFrameType(_videostream.fccHandler)) // Track 0 is video
    {
        if( isMpeg4Compatible(_videostream.fccHandler))
        {
            if(rpt)
                memcpy(readBuffer,_tracks[0].headerRepeat,rpt);
            parser->readBin(readBuffer+rpt,size-3);

            // Search the frame type...
            uint32_t timeIncBits=0;
            const uint32_t u32 = sizeof(uint32_t);

            if(_tracks[0].paramCache && _tracks[0].paramCacheSize == u32)
                memcpy(&timeIncBits,_tracks[0].paramCache,u32);

            ADM_vopS vops[10];
            vops[0].type=AVI_KEY_FRAME;

            if(ADM_searchVop(readBuffer,readBuffer+rpt+size-3,10,vops,&timeIncBits))
            {
                if(!_tracks[0].paramCache)
                {
                    _tracks[0].paramCache = new uint8_t[u32];
                    _tracks[0].paramCacheSize = u32;
                }
                memcpy(_tracks[0].paramCache,&timeIncBits,u32);
            }else
            {
                ADM_warning("No VOP at index entry %u, corrupted data? Size: %d\n",frameNo,rpt+size-3);
            }
            ix.flags=vops[0].type;

        }else if(isH264Compatible(_videostream.fccHandler))
        {
            uint32_t flags=AVI_KEY_FRAME;

            if(rpt)
                memcpy(readBuffer,_tracks[0].headerRepeat,rpt);
            parser->readBin(readBuffer+rpt,size-3);
            // Deal with Matroska files containing Annex-B type of H.264 stream
            bool AnnexB=false;
            if(!_tracks[0].extraDataLen && rpt+size > 3 && !readBuffer[0] && !readBuffer[1])
            {
                uint32_t mark=(readBuffer[2] << 8) + readBuffer[3];
                if(mark == 1 || (mark > 0xFF && mark < 0x200 && rpt+size-3 != mark+4))
                    AnnexB=true;
            }
            if((int)Track->_nalSize == -1)
            {
                Track->_nalSize = 0;
                if(Track->extraDataLen)
                    Track->_nalSize = ADM_getNalSizeH264(Track->extraData, Track->extraDataLen);
                ADM_info("H.264 NAL size set to %" PRIu32"\n",Track->_nalSize);
            }
            ADM_SPSInfo *info=(ADM_SPSInfo *)_tracks[0].infoCache;
            uint8_t *sps=_tracks[0].paramCache;
            uint32_t spsLen=_tracks[0].paramCacheSize;
            // do we have inband SPS?
            uint8_t buf[MAX_H264_SPS_SIZE];
            uint32_t inBandSpsLen=0;
            if(!AnnexB)
                inBandSpsLen=getRawH264SPS(readBuffer, rpt+size-3, Track->_nalSize, buf, MAX_H264_SPS_SIZE);
            else
                inBandSpsLen=getRawH264SPS_startCode(readBuffer, rpt+size-3, buf, MAX_H264_SPS_SIZE);
            bool match=true;
            if(inBandSpsLen > 1) // else likely misdetected Annex-B start code
            {
                if(inBandSpsLen!=spsLen)
                    ADM_warning("SPS length mismatch: %u (old) vs %u (new)\n",spsLen,inBandSpsLen);
                if(spsLen)
                    match=!memcmp(buf, sps, (spsLen>inBandSpsLen)? inBandSpsLen : spsLen);
                else
                    match=false;
            }
            if(!match)
            {
                ADM_warning("SPS mismatch? Checking deeper...\n");
                ADM_SPSInfo info2;
                if(extractSPSInfoFromData(buf,inBandSpsLen,&info2))
                {
                    const uint32_t sz=sizeof(ADM_SPSInfo);
                    if(!info)
                    {
                        _tracks[0].infoCache=new uint8_t[sz];
                        memcpy(_tracks[0].infoCache, &info2, sz);
                        _tracks[0].infoCacheSize=sz;
                    }else
                    {
#define MATCH(x) if(info->x != info2.x) { ADM_warning("%s value does not match.\n",#x); info->x = info2.x; match=false; }
                        match=true;
                        MATCH(width) // FIXME: dimensions mismatch should be fatal
                        MATCH(height)
                        MATCH(CpbDpbToSkip)
                        MATCH(hasPocInfo)
                        MATCH(log2MaxFrameNum)
                        MATCH(log2MaxPocLsb)
                        MATCH(frameMbsOnlyFlag)
                        MATCH(refFrames)
                        if(!match)
                        {
                            ADM_warning("Codec parameters change on the fly at frame %u, expect problems.\n",frameNo);
                            // Nevertheless, update the cached info
                            memcpy(_tracks[0].infoCache, &info2, sz);
                        }
                    }
                    // Update cached raw SPS
                    if(_tracks[0].paramCache)
                        delete [] _tracks[0].paramCache;
                    _tracks[0].paramCache=new uint8_t[inBandSpsLen];
                    memcpy(_tracks[0].paramCache, buf, inBandSpsLen);
                    _tracks[0].paramCacheSize=inBandSpsLen;
                }
            }
            bool r = false;
            if(AnnexB)
                r = extractH264FrameType_startCode(readBuffer, rpt+size-3, &flags, NULL, info, &_H264Recovery);
            else
                r = extractH264FrameType(readBuffer, rpt+size-3, Track->_nalSize, &flags, NULL, info, &_H264Recovery);
            if(r)
            {
                if(flags & AVI_KEY_FRAME)
                {
                    if(flags & AVI_FIELD_STRUCTURE)
                    {
                        if(Track->_secondField)
                        {
                            printf("[MKV/H264] Clearing keyframe flag from second field at index entry %" PRIu32"\n",frameNo);
                            flags &= ~AVI_KEY_FRAME;
                        }
                        Track->_secondField = !Track->_secondField;
                    }
                    setFlag(frameNo,flags);
                    if(flags & AVI_KEY_FRAME)
                        printf("[MKV/H264] Frame %" PRIu32" is a keyframe\n",frameNo);
                }else
                    Track->_secondField = false;
                ix.flags=flags;
                if(Track->index.size()) ix.Dts=ADM_NO_PTS;
            }
        }else if(isH265Compatible(_videostream.fccHandler))
        {
            uint32_t flags=AVI_KEY_FRAME;

            if(rpt)
                memcpy(readBuffer,_tracks[0].headerRepeat,rpt);
            parser->readBin(readBuffer+rpt,size-3);

            // Deal with Matroska files containing Annex-B type of HEVC stream
            bool AnnexB=false;
            if(!_tracks[0].extraDataLen && rpt+size > 3 && !readBuffer[0] && !readBuffer[1])
            {
                uint32_t mark=(readBuffer[2] << 8) + readBuffer[3];
                if(mark == 1 || (mark > 0xFF && mark < 0x200 && rpt+size-3 != mark+4))
                    AnnexB=true;
            }

            if((int)Track->_nalSize == -1)
            {
                Track->_nalSize = 0;
                if(Track->extraDataLen)
                    Track->_nalSize = ADM_getNalSizeH265(Track->extraData, Track->extraDataLen);
                ADM_info("HEVC NAL size set to %" PRIu32"\n",Track->_nalSize);
            }
            ADM_SPSinfoH265 info; // TODO: check for inband VPS/PPS/SPS changing on the fly
            int poc = INT_MIN;

            bool r = false;
            if(AnnexB)
                r = extractH265FrameType_startCode(readBuffer, rpt+size-3, &info, &flags, &poc);
            else
                r = extractH265FrameType(readBuffer, rpt+size-3, Track->_nalSize, &info, &flags, &poc);

            if(r)
            {
                if(flags & AVI_KEY_FRAME)
                {
                    printf("[MKV/H265] Frame %" PRIu32" is a keyframe",(uint32_t)Track->index.size());
                    printf("%s\n",(flags & AVI_IDR_FRAME)? " (IDR)" : " (non-IDR)");
                }
                ix.flags=flags;
                if(Track->index.size()) ix.Dts=ADM_NO_PTS;
            }
        }else if(isMpeg12Compatible(_videostream.fccHandler))
        {
            if(rpt)
                memcpy(readBuffer,_tracks[0].headerRepeat,rpt);
            parser->readBin(readBuffer+rpt,size-3);
            uint8_t *begin=readBuffer;
            uint8_t *end=readBuffer+size-3+rpt;
            bool following=false;
            int picFound=0;
            while(begin<end)
            {
                int code=mkvFindStartCode(begin,end);
                if(!picFound && code==-1)
                {
                    ADM_warning("[Mpg2InMkv]No startcode found\n");
                    break;
                }
                //printf("Startcode found = 0x%x\n",code);
                if(!code) // picture
                {
                    int picType=begin[1]>>3;
                    begin+=4;
                    picFound++;
                    following=true;
                    if(picFound>1)
                        continue;
                    picType&=7;
                    switch(picType)
                    {
                        case 1: ix.flags=AVI_KEY_FRAME;break;
                        case 2: ix.flags=AVI_P_FRAME;break;
                        case 4: ix.flags=AVI_P_FRAME;break;
                        case 3: ix.flags=AVI_B_FRAME;break;
                        default: ADM_warning("[Mpeg2inMkv]Bad pictype : %d\n",picType);
                    }
                }
                if(code==0xB5) // extension
                {
                    int id=(begin[0]<<8)+begin[1];
                    id>>=12;
                    begin+=2;
                    if(id!=8)
                    {
                        following=false;
                        continue;
                    }
                    if(picFound>1)
                    {
                        //printf("Multiple pics in a single buffer, clearing field structure.\n");
                        ix.flags &= ~AVI_STRUCTURE_TYPE_MASK;
                        break;
                    }
                    if(!following)
                    {
                        ADM_warning("Skipping picture coding extension not following picture.\n");
                        begin+=3;
                        continue;
                    }
                    following=false;
                    int picStruct=begin[0]&3;
                    begin+=3; // skip also parity and progressive flags
                    switch(picStruct)
                    {
                        case 1: ix.flags|=AVI_TOP_FIELD+AVI_FIELD_STRUCTURE; break;
                        case 2: ix.flags|=AVI_BOTTOM_FIELD+AVI_FIELD_STRUCTURE; break;
                        default:break;
                    }
                }
            }
        }else if(isVC1Compatible(_videostream.fccHandler))
        {
            if(rpt)
                memcpy(readBuffer,_tracks[0].headerRepeat,rpt);
            parser->readBin(readBuffer+rpt,size-3);
            uint8_t *begin=readBuffer;
            uint8_t *end=readBuffer+size-3+rpt;
            int frameType;
            if(ADM_VC1getFrameType(begin, (int)(end-begin),&frameType))
            {
                ix.flags=frameType;
            }
        }
    }
    if(size>3)
        Track->_sizeInBytes+=size-3;
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
bool mkvHeader::readCue(ADM_ebml_file *parser)
{
    uint64_t len,vlen;
    uint64_t id;
    ADM_MKV_TYPE type;
    const char *ss;
    uint64_t time;

    if(!goBeforeAtomAtPosition(parser, _cuePosition,vlen, MKV_CUES,"MKV_CUES"))
    {
        ADM_warning("Cannot go to the CUES atom\n");
        return false;
    }

    ADM_ebml_file cues(parser,vlen);
    while(!cues.finished())
    {
        if(!cues.readElemId(&id,&len))
            continue;
        if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
        {
            printf("[MKV] Tag 0x%" PRIx64" in CUES not found (len %" PRIu64")\n",id,len);
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
        if(!cue.readElemId(&id,&len))
            continue;
        if(id!=MKV_CUE_TIME)
        {
            ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type);
            printf("Found %s(0x%" PRIx64"), expected CUE_TIME  (0x%x)\n", ss,id,MKV_CUE_TIME);
            cue.skip(cue.remaining());
            continue;
        }
        time=cue.readUnsignedInt(len);

        if(!cue.readElemId(&id,&len))
            continue;
        if(id!=MKV_CUE_TRACK_POSITION)
        {
            ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type);
            printf("Found %s (0x%" PRIx64"), expected MKV_CUE_TRACK_POSITION (0x%x)\n", ss,id,MKV_CUE_TRACK_POSITION);
            cue.skip(cues.remaining());
            continue;
        }
        ADM_ebml_file trackPos(&cue,len);
        uint64_t tid=0;
        uint64_t cluster_position=0;
        uint64_t cue_position=0;
        while(!trackPos.finished())
        {
            if(!trackPos.readElemId(&id,&len))
                continue;
            switch(id)
            {
                case MKV_CUE_TRACK: tid=trackPos.readUnsignedInt(len);break;
                case MKV_CUE_CLUSTER_POSITION: cluster_position=trackPos.readUnsignedInt(len);break;
                case MKV_CUE_RELATIVE_POSITION: cue_position=trackPos.readUnsignedInt(len);break;
                default:
                    ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type);
                    printf("[MKV] in cluster position found tag %s (0x%" PRIx64")\n",ss,id);
                    trackPos.skip(len);
                    continue;
            }
        }
        aprintf("Track %" PRIx64" segmentPos=%" PRIx64" Cluster Position 0x%" PRIx64" Cue position 0x%" PRIx64" Absolute=%" PRIx64" time %" PRIu64"\n",
            tid,_segmentPosition,cluster_position,cue_position,cue_position+cluster_position+_segmentPosition,time);

        if(!searchTrackFromTid(tid)) //only keep video i.e. track zero
        {
            //printf("Adding cue entry\n");
            _cueTime.append(time);
        }
    }
    if(_cueTime.size())
    {
        ADM_info("[MKV] Cues updated\n");
        return true;
    }else
    {
        ADM_info("[MKV] No Cue found\n");
        return false;
    }

}

bool mkvHeader::loadIndex(const std::string &idxName, uint64_t fileSize)
{
    FILE * f = ADM_fopen(idxName.c_str(), "rb");
    if (!f) return false;

    uint64_t indexSize = ADM_fileSize(idxName.c_str());

    bool ret = false;
    uint8_t * buf;
    uint64_t bufU64, tmpU64;
    int64_t bufI64;
    ADM_assert(strlen(ADM_MKV_INDEX_MAGIC) == 8);
    
    uint64_t bufPos=0;
    buf = new uint8_t[8];

    const char *failmsg = "unknown";
    if (indexSize < 40)
    {
        failmsg = "Index size too small";
        goto loadErr;
    }
    if (ADM_fread(buf,8,1,f)!=1)
    {
        failmsg = "Cannot read index magic";
        goto loadErr;
    }
    if (memcmp(buf,ADM_MKV_INDEX_MAGIC,8))
    {
        failmsg = "Index magic is wrong";
        goto loadErr;
    }
    if (ADM_fread(&bufU64,sizeof(uint64_t),1,f)!=1)
    {
        failmsg = "Cannot read index version";
        goto loadErr;
    }
    if (bufU64 != ADM_MKV_INDEX_VERSION)
    {
        failmsg = "Index version mismatch";
        goto loadErr;
    }
    if (ADM_fread(&bufU64,sizeof(uint64_t),1,f)!=1)
    {
        failmsg = "Cannot read file size";
        goto loadErr;
    }
    if (bufU64 != fileSize)
    {
        failmsg = "File size mismatch";
        goto loadErr;
    }
    indexSize -= 8+2*sizeof(uint64_t);
    if (indexSize < 8)
    {
        failmsg = "Remaning index size too small";
        goto loadErr;
    }
    if (indexSize > ADM_MKV_INDEX_SIZE_LIMIT)
    {
        failmsg = "Index size too large";
        goto loadErr;
    }
    delete [] buf;
    buf = new uint8_t[indexSize+65536];
    
    if (ADM_fread(buf,indexSize,1,f)!=1) goto loadErr;
    
    if (memcmp(ADM_MKV_INDEX_MAGIC,buf+(indexSize - 8),8)) goto loadErr;
    indexSize -= 8;
    
    #define LOAD_U64(X)     memcpy(&bufU64, buf+bufPos, sizeof(uint64_t)); \
                            bufPos+=sizeof(uint64_t); \
                            ADM_assert(bufPos<=indexSize); \
                            X = bufU64
    #define LOAD_I64(X)     memcpy(&bufI64, buf+bufPos, sizeof(int64_t)); \
                            bufPos+=sizeof(int64_t); \
                            ADM_assert(bufPos<=indexSize); \
                            X = bufI64
    #define LOAD_BOOL(X)    memcpy(&bufU64, buf+bufPos, sizeof(uint64_t)); \
                            bufPos+=sizeof(uint64_t); \
                            ADM_assert(bufPos<=indexSize); \
                            X = (bufU64 != 0)
    #define LOAD_TYPE(X,T)  memcpy(&X, buf+bufPos, sizeof(T)); \
                            bufPos+=sizeof(T); \
                            ADM_assert(bufPos<=indexSize)
    #define LOAD_BVECT(X,L) ADM_assert(bufPos+L<=indexSize); \
                            memcpy(X, buf+bufPos, L); \
                            bufPos+=L; \
                            ADM_assert(bufPos<=indexSize)
     // _clusters
    uint64_t clusterCount;
    LOAD_U64(clusterCount);
    _clusters.clear();
    for (uint64_t i=0; i<clusterCount; i++)
    {
        mkvIndex tmp;
        LOAD_TYPE(tmp, mkvIndex);
        _clusters.append(tmp);
    }
    
    LOAD_U64(_nbAudioTrack);
    // _tracks
    for (int t=0; t<1+_nbAudioTrack; t++)
    {
        uint64_t indexCount;
        LOAD_U64(indexCount);
        _tracks[t].index.clear();
        for (uint64_t i=0; i<indexCount; i++)
        {
            mkvIndex tmp;
            LOAD_TYPE(tmp, mkvIndex);
            _tracks[t].index.append(tmp);
        }

        LOAD_U64(_tracks[t].streamIndex);
        LOAD_U64(_tracks[t].duration);
        LOAD_TYPE(_tracks[t].wavHeader, WAVHeader);
        LOAD_U64(_tracks[t].nbPackets);
        LOAD_U64(_tracks[t].nbFrames);
        LOAD_U64(_tracks[t].length);
        LOAD_U64(_tracks[t]._sizeInBytes);
        LOAD_U64(_tracks[t]._defaultFrameDuration);
        LOAD_I64(_tracks[t]._needBuffering);
        LOAD_I64(_tracks[t]._needExtraData);
        LOAD_BOOL(_tracks[t]._secondField);
        LOAD_U64(_tracks[t]._nalSize);

        LOAD_U64(tmpU64);
        if (tmpU64 > 0)
        {
            _tracks[t].extraDataLen = tmpU64;
            if (_tracks[t].extraData) delete [] _tracks[t].extraData;
            _tracks[t].extraData = new uint8_t[_tracks[t].extraDataLen];
            LOAD_BVECT(_tracks[t].extraData, _tracks[t].extraDataLen);
        }

        LOAD_U64(tmpU64);
        if (tmpU64 > 0)
        {
            _tracks[t].infoCacheSize = tmpU64;
            if (_tracks[t].infoCache) delete [] _tracks[t].infoCache;
            _tracks[t].infoCache = new uint8_t[_tracks[t].infoCacheSize];
            LOAD_BVECT(_tracks[t].infoCache, _tracks[t].infoCacheSize);
        }

        LOAD_U64(tmpU64);
        if (tmpU64 > 0)
        {
            _tracks[t].paramCacheSize = tmpU64;
            if (_tracks[t].paramCache) delete [] _tracks[t].paramCache;
            _tracks[t].paramCache = new uint8_t[_tracks[t].paramCacheSize];
            LOAD_BVECT(_tracks[t].paramCache, _tracks[t].paramCacheSize);
        }

        LOAD_U64(_tracks[t].headerRepeatSize);
        ADM_assert(_tracks[t].headerRepeatSize <= MKV_MAX_REPEAT_HEADER_SIZE);
        LOAD_BVECT(_tracks[t].headerRepeat, _tracks[t].headerRepeatSize);
        
        LOAD_U64(tmpU64);
        ADM_assert(tmpU64 == ADM_MKV_INDEX_MAGICMARK);
    }    
    
    #undef LOAD_U64
    #undef LOAD_I64
    #undef LOAD_BOOL
    #undef LOAD_TYPE
    #undef LOAD_BVECT
    
    
    ADM_assert(bufPos==indexSize);
    ret = true;
loadErr:
    delete [] buf;
    ADM_fclose(f);
    if (!ret)
        ADM_warning("Cannot load index, reason: %s.\n", failmsg);
    return ret;
}

void mkvHeader::saveIndex(const std::string &idxName, uint64_t fileSize)
{
    if (ADM_fileExist(idxName.c_str())) // nothing to do
        return;

    FILE *f = ADM_fopen(idxName.c_str(), "wb");
    if (!f) return;
    
    uint64_t bufU64;
    int64_t bufI64;
    uint64_t nbWrite = 0;

    #define SAVE_U64(X)         do{ bufU64 = (X); if (1 != ADM_fwrite(&bufU64,sizeof(uint64_t),1,f)) goto saveErr; nbWrite++; } while(0)
    #define SAVE_I64(X)         do{ bufI64 = (X); if (1 != ADM_fwrite(&bufI64,sizeof(int64_t),1,f)) goto saveErr; nbWrite++; } while(0)
    #define SAVE_U64_BRK(X)     bufU64 = (X); if (ADM_fwrite(&bufU64,sizeof(uint64_t),1,f) !=1 ) goto saveErr; nbWrite++;
    #define SAVE_I64_BRK(X)     bufI64 = (X); if (ADM_fwrite(&bufI64,sizeof(int64_t),1,f) !=1 ) goto saveErr; nbWrite++;
    #define SAVE_TYPE_BRK(X,T)  if (ADM_fwrite(X,sizeof(T),1,f) !=1 ) goto saveErr; nbWrite++;
    #define SAVE_BVECT_BRK(X,L) if (ADM_fwrite(X,L,1,f) !=1 ) goto saveErr; nbWrite++;
    
    ADM_fwrite(ADM_MKV_INDEX_MAGIC,8,1,f);
    SAVE_U64(ADM_MKV_INDEX_VERSION);
    SAVE_U64(fileSize);
    
    SAVE_U64(_clusters.size());
    for (uint32_t i=0; i<_clusters.size(); i++)
    {
        mkvIndex *tmp = &(_clusters[i]);
        SAVE_TYPE_BRK(tmp, mkvIndex);
    }
    
    SAVE_U64(_nbAudioTrack);
    for (int t=0; t<1+_nbAudioTrack; t++)
    {
        uint32_t nbIdx = _tracks[t].index.size();
        SAVE_U64(nbIdx);
        uint32_t chunk = (nbIdx < 0x1000) ? nbIdx : 0x1000;
        uint8_t *tmpbuf = (uint8_t *)ADM_alloc(chunk * sizeof(mkvIndex));
        if (tmpbuf == NULL) goto saveErr;
        //ADM_info("Allocated %" PRIu64" bytes for temporary index buffer for track %d\n", chunk * sizeof(mkvIndex), t);
        uint32_t indexOffset = 0;
        while (nbIdx)
        {
            nbIdx -= chunk;
            uint8_t *p = tmpbuf;
            for (uint32_t i=0; i < chunk; i++)
            {
                memcpy(p, &(_tracks[t].index[indexOffset]), sizeof(mkvIndex));
                p += sizeof(mkvIndex);
                indexOffset++;
            }
            if (1 != ADM_fwrite(tmpbuf, chunk * sizeof(mkvIndex), 1, f))
            {
                ADM_dealloc(tmpbuf);
                tmpbuf = NULL;
                goto saveErr;
            }
            nbWrite++;
            //ADM_info("%" PRIu32" index entries for track %d written, remaining: %" PRIu32"\n", chunk, t, nbIdx);
            if (chunk > nbIdx)
                chunk = nbIdx;
        }
        ADM_dealloc(tmpbuf);
        tmpbuf = NULL;

        SAVE_U64_BRK(_tracks[t].streamIndex);
        SAVE_U64_BRK(_tracks[t].duration);
        SAVE_TYPE_BRK(&(_tracks[t].wavHeader), WAVHeader);
        SAVE_U64_BRK(_tracks[t].nbPackets);
        SAVE_U64_BRK(_tracks[t].nbFrames);
        SAVE_U64_BRK(_tracks[t].length);
        SAVE_U64_BRK(_tracks[t]._sizeInBytes);
        SAVE_U64_BRK(_tracks[t]._defaultFrameDuration);
        SAVE_I64_BRK(_tracks[t]._needBuffering);
        SAVE_I64_BRK(_tracks[t]._needExtraData);
        SAVE_U64_BRK(_tracks[t]._secondField ? 1:0);
        SAVE_U64_BRK(_tracks[t]._nalSize);

        SAVE_U64_BRK(_tracks[t].extraDataLen);
        if (_tracks[t].extraDataLen > 0)
        {
            SAVE_BVECT_BRK(_tracks[t].extraData, _tracks[t].extraDataLen);
        }

        SAVE_U64_BRK(_tracks[t].infoCacheSize);
        if (_tracks[t].infoCacheSize > 0)
        {
            SAVE_BVECT_BRK(_tracks[t].infoCache, _tracks[t].infoCacheSize);
        }

        SAVE_U64_BRK(_tracks[t].paramCacheSize);
        if (_tracks[t].paramCacheSize > 0)
        {
            SAVE_BVECT_BRK(_tracks[t].paramCache, _tracks[t].paramCacheSize);
        }

        SAVE_U64_BRK(_tracks[t].headerRepeatSize);
        if (_tracks[t].headerRepeatSize > 0)
        {
            SAVE_BVECT_BRK(_tracks[t].headerRepeat, _tracks[t].headerRepeatSize);
        }

        SAVE_U64_BRK(ADM_MKV_INDEX_MAGICMARK);
    }

    #undef SAVE_U64
    #undef SAVE_I64
    #undef SAVE_U64_BRK
    #undef SAVE_I64_BRK
    #undef SAVE_TYPE_BRK
    #undef SAVE_BVECT_BRK

    if (1 != ADM_fwrite(ADM_MKV_INDEX_MAGIC,8,1,f))
        goto saveErr;
    ADM_fclose(f);
    ADM_info("Index written, %" PRIu64" write-outs.\n", nbWrite);
    return;
saveErr:
    ADM_fclose(f);
    remove(idxName.c_str());
}

/**
    \fn indexClusters
    \brief Create a list of all clusters with there position & size, index each one.
*/
uint8_t mkvHeader::indexClusters(ADM_ebml_file *parser)
{
    uint64_t fileSize,len;
    uint64_t alen,vlen;
    uint64_t id;
    ADM_MKV_TYPE type;
    const char *ss;
    uint64_t pos;
    uint8_t res=1;
    bool indexOnDisk = true;

    if (NULL != getenv("ADM_NOINDEX_MKV") && !strncmp(getenv("ADM_NOINDEX_MKV"), "1", 1))
        indexOnDisk = false;

    if (indexOnDisk)
    {
        if (loadIndex(_idxName, parser->getFileSize()))
        {
            printf("[MKV] Video track indexing loaded from \"%s\"\n", _idxName.c_str());
            return ADM_OK;
        }
    }

    mkvIndex tmpCluster;

    readBufferSize = 200*1024;
    readBuffer = new uint8_t[readBufferSize];
    memset(readBuffer,0,readBufferSize);

    // Search segment
    fileSize=parser->getFileSize();
    if(!parser->simplefind(MKV_SEGMENT,&vlen,1))
    {
        ADM_warning("[MKV] cluster indexer, cannot find CLUSTER atom\n");
        return 0;
    }
    // In case the segment is ridiculously small take file size....
    pos=parser->tell();
    ADM_info("FileSize = %" PRIu64", pos=%" PRIu64" size=%" PRIu64",pos+size=%" PRIu64"\n",fileSize,pos,vlen,pos+vlen);
    if(pos+vlen<fileSize)
    {
        ADM_warning("Segment is way too small, trying to guess the right value\n");
        vlen=fileSize-pos;
    }
    ADM_ebml_file segment(parser,vlen);

    _work = createWorking(QT_TRANSLATE_NOOP("matroskademuxer","Indexing Matroska Video Track"));
    while(segment.simplefind(MKV_CLUSTER,&alen,0))
    {
        if(!_work->isAlive())
        {
            res=ADM_IGN;
            break;
        }
        // UI update
        _work->update(segment.tell() >> 10, fileSize >> 10);
        tmpCluster.pos=segment.tell();
        tmpCluster.size=alen;
        _clusters.append(tmpCluster);

     // Normally the timecode is the 1st one following

tryAgain:
        if(!segment.readElemId(&id,&len))
            continue;
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
            ADM_warning("[MKV] Cluster : no time code Found %s(0x%" PRIx64"), expected MKV_TIMECODE  (0x%x)\n",
                ss,id,MKV_TIMECODE);
        }else // timecode
        {
            uint64_t timecode=segment.readUnsignedInt(len);
            _clusters[seekme].Dts=timecode;
        }
        res = indexLastCluster(&segment);
        if(res != 1)
            break;
        segment.seek( _clusters[seekme].pos+ _clusters[seekme].size);
        //printf("Position :%u %u MB\n", _clusters[seekme].pos+ _clusters[seekme].size,( _clusters[seekme].pos+ _clusters[seekme].size)>>20);
    }
    delete _work;
    _work = NULL;
    //ADM_info("[MKV] Found %u clusters\n",(int)_clusters.size());

    if (indexOnDisk)
    {
        if ((res == ADM_OK) && (!!VIDEO.index.size()))
        {
            saveIndex(_idxName, parser->getFileSize());
        }
    }
    
    return (ADM_IGN == res)? res : !!VIDEO.index.size();
}
/**
 * \fn updateFlagsWithCue
 * The position is slightly before the actual image
 * i.e.
 * position
 *     timecode
 *     length
 *     Actual image <= We point here
 * Typically the difference is ~ 16 bytes, so we accept anything between 0 and32
 * @return
 */
bool  mkvHeader::updateFlagsWithCue(void)
{
    int nbImages=_tracks[0].index.size();
    int nbCues=_cueTime.size();

    int lastImage=0;
    ADM_info("Updating Flags with Cue\n");
    for(int curCue=0;curCue<nbCues;curCue++)
    {
        uint64_t tim=_cueTime[curCue];
        for(int curImage=lastImage;curImage<nbImages;curImage++)
        {
            uint64_t thisTime=_tracks[0].index[curImage].Pts/_timeBase;
            if( tim== thisTime)
            {
                // match!
                _tracks[0].index[curImage].flags |=AVI_KEY_FRAME;
                aprintf("Mapped cue entries %d to image %d (size=%d)\n",curCue,curImage,_tracks[0].index[curImage].size);
                lastImage=curImage+1;
                break;
            }
        }
    }
    ADM_info("Updating Flags with Cue done\n");
    return true;
}
/**
 *
 * @param mx
 * @return
 */
bool mkvHeader::dumpVideoIndex(int mx)
{
    int n=_tracks[0].index.size();
    if(n>mx) n=mx;
    for(int i=0;i<n;i++)
    {
        ADM_info("[%d] Position 0x%llx, size=%d, time=%s, Flags=%x\n",i,
               _tracks[0].index[i].pos,_tracks[0].index[i].size,ADM_us2plain(_tracks[0].index[i].Pts),_tracks[0].index[i].flags);
    }
    return true;
}
//EOF
