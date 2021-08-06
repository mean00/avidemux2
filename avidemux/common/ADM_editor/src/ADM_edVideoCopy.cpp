/***************************************************************************
    \file  ADM_edVideoCopy.cpp  
    \brief handle direct stream copy for video
    \author mean (c) 2002/2009 fixounet@free.fr

    

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
#include "ADM_edit.hxx"
#include "ADM_vidMisc.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_h264_tag.h"

#if 0
#define aprintf printf
#else
#define aprintf(...) {}// printf
#endif

static bool checkCodec(aviInfo *first,aviInfo *second)
{
    bool match=false;
#define XCHECK(x) if(!match && x(first->fcc) && x(second->fcc)) { ADM_info("Codecs identified by %s() as matching\n",#x); match=true; }
    XCHECK(isH264Compatible)
    XCHECK(isH265Compatible)
    XCHECK(isMpeg4Compatible)
    XCHECK(isMpeg12Compatible)
    XCHECK(isVC1Compatible)
    XCHECK(isVP9Compatible)
    XCHECK(isVP6Compatible)
    XCHECK(isMSMpeg4Compatible)
    XCHECK(isDVCompatible)
    // catch fourcc values not covered by above
    if(!match && first->fcc == second->fcc)
        match=true;
    return match;
}

static bool boundsCheck(vidHeader *hdr, ADMCompressedImage *img, uint32_t frame, uint32_t max)
{
    img->dataLength=0;
    if(!hdr->getFrameSize(frame,&(img->dataLength)) || !img->dataLength)
    {
        ADM_warning("Cannot get valid length for frame %u\n",frame);
        return false;
    }
    if(img->dataLength > max)
    {
        ADM_warning("Length of frame %u out of bounds: %u vs %u\n", frame, img->dataLength, max);
        return false;
    }
    return true;
}

static bool getH264SPSInfo(_VIDEOS *vid,ADM_SPSInfo *sps)
{
    if(!vid)
        return false;
    vidHeader *demuxer=vid->_aviheader;
    if(!demuxer)
        return false;
    if(!sps)
        return false;

    // Do we have a cached copy?
    if(vid->infoCacheSize==sizeof(ADM_SPSInfo) && vid->infoCache)
    {
        memcpy(sps,vid->infoCache,sizeof(ADM_SPSInfo));
        return true;
    }

    bool gotSps=false;
    uint8_t *buffer=NULL;
    uint8_t *extra;
    uint32_t extraLen=0;
    demuxer->getExtraHeaderData(&extraLen,&extra);
    if(!extraLen) // AnnexB
    {
        // Allocate memory to hold compressed frame
        ADMCompressedImage img;
#define MAX_FRAME_LENGTH (1920*1080*3) // ~7 MiB, should be enough even for 4K
        if(!boundsCheck(demuxer,&img,0,MAX_FRAME_LENGTH))
        {
            ADM_warning("First frame of video has invalid length.\n");
            goto _the_end;
        }
        buffer=new uint8_t[MAX_FRAME_LENGTH];
        img.data=buffer;
        if(!demuxer->getFrame(0,&img))
        {
            ADM_warning("Unable to get the first frame of video\n");
            goto _the_end;
        }
#define MAX_NALU_TO_CHECK 4
        static NALU_descriptor desc[MAX_NALU_TO_CHECK];

        int nbNalu=ADM_splitNalu(img.data, img.data+img.dataLength, MAX_NALU_TO_CHECK, desc);
        int spsIndex=ADM_findNalu(NAL_SPS,nbNalu,desc);

        NALU_descriptor *d=desc;
        if(spsIndex<0)
            goto _the_end;
        d+=spsIndex;
        if(extractSPSInfo(d->start, d->size, sps))
        { // Cache raw SPS data
            if(vid->paramCache)
                delete [] vid->paramCache;
            vid->paramCacheSize=d->size+1;
            vid->paramCache=new uint8_t[vid->paramCacheSize];
            uint8_t *p=vid->paramCache;
            p[0]=d->nalu;
            memcpy(p+1, d->start, d->size);
            gotSps=true;
        }
    }else
    {
        gotSps=extractSPSInfo(extra,extraLen,sps);
        if(gotSps && extra[5] == 0xe1 /* We support only a single one SPS */ )
        { // Cache raw SPS data
            vid->paramCacheSize=(extra[6] << 8) + extra[7];
            if(vid->paramCacheSize && vid->paramCacheSize+8<=extraLen) // 8 bytes AVCC header + SPS
            {
                vid->paramCache=new uint8_t[vid->paramCacheSize];
                memcpy(vid->paramCache, extra+8, vid->paramCacheSize);
            }
        }
    }
    if(!gotSps)
        goto _the_end;
    // Store the decoded SPS info in the cache
    vid->infoCacheSize=sizeof(ADM_SPSInfo);
    vid->infoCache=new uint8_t[vid->infoCacheSize]; // will be destroyed with the video
    memcpy(vid->infoCache,sps,sizeof(ADM_SPSInfo));
_the_end:
    if(buffer) delete [] buffer;
    buffer=NULL;
    return gotSps;
}

/**
    \fn getH265SPSInfo
*/
static bool getH265SPSInfo(_VIDEOS *vid, ADM_SPSinfoH265 *sps)
{
    if(!vid)
        return false;
    vidHeader *demuxer=vid->_aviheader;
    if(!demuxer)
        return false;
    if(!sps)
        return false;

    // Do we have a cached copy?
    uint32_t spsSize=sizeof(ADM_SPSinfoH265);
    if(vid->infoCacheSize==spsSize && vid->infoCache)
    {
        memcpy(sps,vid->infoCache,spsSize);
        return true;
    }
    // Nope, determine stream type
    bool gotSps=false;
    uint8_t *buffer=NULL;
    uint8_t *extra;
    uint32_t extraLen=0;
    demuxer->getExtraHeaderData(&extraLen,&extra);
    if(!extraLen) // AnnexB
    {
        // Allocate memory to hold compressed frame
        ADMCompressedImage img;
        if(!boundsCheck(demuxer,&img,0,MAX_FRAME_LENGTH))
        {
            ADM_warning("First frame of video has invalid length.\n");
            goto _the_end_hevc;
        }
        buffer=new uint8_t[MAX_FRAME_LENGTH];
        img.data=buffer;
        if(!demuxer->getFrame(0,&img))
        {
            ADM_warning("Unable to get the first frame of video\n");
            goto _the_end_hevc;
        }
        gotSps=extractSPSInfoH265(img.data,img.dataLength,sps);
    }else
    {
        gotSps=extractSPSInfoH265(extra,extraLen,sps);
    }
    if(!gotSps)
        goto _the_end_hevc;
    // Store the decoded SPS info in the cache
    vid->infoCacheSize=spsSize;
    vid->infoCache=new uint8_t[vid->infoCacheSize]; // will be destroyed with the video
    memcpy(vid->infoCache,sps,vid->infoCacheSize);
_the_end_hevc:
    if(buffer) delete [] buffer;
    buffer=NULL;
    return gotSps;
}

/**
    \fn findLastFrameBeforeSwitch
    \brief Identify the last frame in stream order and last displayed frame before segment switch
*/
bool ADM_Composer::findLastFrameBeforeSwitch(uint32_t segNo, uint32_t *lastFrame, uint32_t *maxPtsFrame, uint64_t *maxPts)
{
    if(segNo>=_segments.getNbSegments())
    {
        ADM_error("Requested segment number %d out of range!\n",segNo);
        return false;
    }

    _SEGMENT *seg=_segments.getSegment(segNo);
    ADM_assert(seg);
    _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
    ADM_assert(vid);
    vidHeader *demuxer=vid->_aviheader;
    ADM_assert(demuxer);

    // Identify the last frame before segment switch
    int frame=0;
    uint64_t tail = seg->_refStartTimeUs + seg->_durationUs;
    if(false==getFrameNumFromPtsOrBefore(vid,tail-1,frame))
    {
        ADM_warning("Cannot identify the last frame in display order for segment %d\n",segNo);
        return false;
    }
    // Check restrictions imposed by PTS delay and early B-frames in the next segment
    uint64_t limitDts,limitPts;
    limitDts=limitPts=tail;
    if(segNo+1 < _segments.getNbSegments())
    {
        _SEGMENT *nextSeg=_segments.getSegment(segNo+1);
        ADM_assert(nextSeg);
        uint64_t delta=0;
        if(nextSeg->_refStartTimeUs > nextSeg->_refStartDts)
            delta = nextSeg->_refStartTimeUs - nextSeg->_refStartDts;
        if(delta<limitDts)
            limitDts-=delta;
        uint32_t dlay=0;
        if(getOpenGopDelayForSegment(segNo+1,0,&dlay) && (uint64_t)dlay < limitPts)
            limitPts-=dlay;
    }
    int maxFrame=frame;
#define MAX_REF_FRAMES_FIELDS 32 // the highest possible nb of references in a field-encoded H.264 stream
    // FIXME In theory, 32 may be not enough as we may encounter an unknown number of early B-frames
    // while maxFrame may be far past the first frame of the next segment if it uses the same ref video.
    frame = (frame > MAX_REF_FRAMES_FIELDS)? frame - MAX_REF_FRAMES_FIELDS : 0;
    *lastFrame=frame;
    // Now search the frame with max pts before segment switch
    *maxPts=0;
    *maxPtsFrame=-1;
    uint64_t lastPts=0, lastDts=ADM_NO_PTS;
    while(frame<=maxFrame)
    {
        uint64_t pts,dts;
        if(!demuxer->getPtsDts(frame,&pts,&dts))
            break;
        if(pts!=ADM_NO_PTS && pts >= limitPts)
            break;
        if(dts!=ADM_NO_PTS)
        {
            if(lastDts!=ADM_NO_PTS && lastDts>=dts)
                ADM_warning("Frame %d DTS delta %" PRId64" ms\n",frame,
                        ((int64_t)lastDts-(int64_t)dts)/1000); // Should we rather bail out?
            lastDts=dts;
        }
        else if(lastDts!=ADM_NO_PTS)
            lastDts+=vid->timeIncrementInUs;
        if(lastDts!=ADM_NO_PTS && lastDts >= limitDts)
            break;
        if(pts!=ADM_NO_PTS)
            lastPts=pts;
        if(lastPts>*maxPts)
        {
            *maxPts=lastPts;
            *maxPtsFrame=frame;
        }
        *lastFrame=frame;
        frame++;
    }

    return true;
}

/**
    \fn checkSegmentStartsOnIntra
    \brief In copy mode, if the cuts are not on intra we will run into trouble :
            * We include skipped ref frames: we will have DTS going back error
            * We skip them, we have borked video at cut points due to missing ref frames
*/
ADM_cutPointType ADM_Composer::checkSegmentStartsOnIntra(uint32_t segNo)
{
    if(segNo>=_segments.getNbSegments())
    {
        ADM_error("Requested segment number %d out of range!\n",segNo);
        return ADM_EDITOR_CUT_POINT_KEY;
    }

    _SEGMENT *seg=_segments.getSegment(segNo);
    _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
    vidHeader *demuxer=vid->_aviheader;

    uint32_t oldSeg=_currentSegment;
    uint32_t oldFrame=vid->lastSentFrame;
    uint32_t refNo=seg->_reference;
    uint64_t refstart=(seg->_refStartTimeUs > vid->firstFramePts)? seg->_refStartTimeUs : vid->firstFramePts;
    int frame=0;
    uint8_t *buffer=NULL;
    uint8_t *spsBuf1=NULL;
    uint8_t *spsBuf2=NULL;
    ADM_cutPointType cut=ADM_EDITOR_CUT_POINT_UNCHECKED;

#define BOWOUT if(buffer) { delete [] buffer; buffer=NULL; } \
               if(spsBuf1) { delete [] spsBuf1; spsBuf1=NULL; } \
               if(spsBuf2) { delete [] spsBuf2; spsBuf2=NULL; } \
               _currentSegment=oldSeg; vid->lastSentFrame=oldFrame; \
               return cut;

    if(false==switchToSegment(segNo,true))
    {
        BOWOUT
    }
    if(false==getFrameNumFromPtsOrBefore(vid,refstart,frame))
    {
        BOWOUT
    }
    // Cursorily check that codec matches if the ref video is not the first one.
    aviInfo info;
    demuxer->getVideoInfo(&info);
    if(refNo)
    {
        aviInfo inf0;
        getVideoInfo(&inf0);
        if(!checkCodec(&info,&inf0))
        {
            ADM_error("Codec doesn't match the one of the first ref video. This is currently unsupported.\n");
            cut=ADM_EDITOR_CUT_POINT_MISMATCH;
            BOWOUT
        }
    }

    uint32_t flags=0;
    uint64_t pts,dts;
    demuxer->getPtsDts(frame,&pts,&dts);

    // After a seg switch we are at the keyframe before or equal to where we want to go
    // if the dts do not match, it means we went back too much
    // When re-encoding, it's not a problem, it is when copying.
    ADM_info("seg %d: ref %d, refDTS=%" PRIu64"\n",segNo,seg->_reference,seg->_refStartDts);
    ADM_info("seg %d: ref %d, imgDTS=%" PRIu64"\n",segNo,seg->_reference,dts);
    if(!seg->_refStartDts && !seg->_reference)
    {
        ADM_info("Ignoring first seg (unreliable DTS)\n");
    }else if(dts!=ADM_NO_PTS && seg->_refStartDts!=ADM_NO_PTS && dts!=seg->_refStartDts)
    {
        ADM_warning("Segment %d does not start on a known DTS (%" PRIu64" us = %s)\n",segNo,dts,ADM_us2plain(dts));
        ADM_warning("expected: %" PRIu64" us = %s\n",seg->_refStartDts,ADM_us2plain(seg->_refStartDts));
        cut=ADM_EDITOR_CUT_POINT_NON_KEY; // ??
        BOWOUT
    }
    // DTS check passed, now check flags
    demuxer->getFlags(frame,&flags);
    if(!(flags & AVI_KEY_FRAME))
    {
        ADM_warning("Segment %d does not start on a keyframe (time in ref %s)\n",segNo,ADM_us2plain(pts));
        cut=ADM_EDITOR_CUT_POINT_NON_KEY;
        BOWOUT
    }
    // The frame is marked as keyframe
    if(!segNo) // Not a cut point
    {
        cut=ADM_EDITOR_CUT_POINT_KEY;
        BOWOUT
    }

    // It is a cut, perform codec-specific checks

    if(isH264Compatible(info.fcc))
    {
        int poc1,maxpoc1,poc2,minpoc2;
        // It is H.264, check deeper. The keyframe may be a non-IDR random access point.
        // If picture order count after a cut is going back, at least FFmpeg-based players
        // like VLC and mpv may get stuck until POC is greater than before the cut point.
        // Get SPS to be able to decode the slice header
        ADM_SPSInfo sps;
        if(!getH264SPSInfo(vid,&sps))
        {
            ADM_warning("Cannot retrieve SPS info.\n");
            BOWOUT
        }
        // Determine stream type
        bool AnnexB=false;
        uint8_t *extra;
        uint32_t extraLen=0;
        uint32_t nalSize=0;
        demuxer->getExtraHeaderData(&extraLen,&extra);
        if(!extraLen)
            AnnexB=true;
        else
            nalSize=ADM_getNalSizeH264(extra,extraLen);
        // Allocate memory to hold compressed frame
        ADMCompressedImage img;
#define CHECK_FRAME(x) if(!boundsCheck(demuxer,&img,x,MAX_FRAME_LENGTH)) \
        { \
            ADM_warning("Length of frame %d in ref %d, segment %d out of bounds or invalid.\n",x,seg->_reference,segNo); \
            BOWOUT \
        }
        CHECK_FRAME(frame)
        buffer=new uint8_t[MAX_FRAME_LENGTH];
        img.data=buffer;
        // We have SPS, now get the frame we are interested in.
        if(!demuxer->getFrame(frame,&img))
        {
            ADM_warning("Unable to get frame %d in ref %d, segment %d\n",frame,seg->_reference,segNo);
            BOWOUT
        }
        // Do we have in-band SPS?
        ADM_info("Checking for in-band SPS...\n");
        bool match=true;
        uint32_t spsLen1,spsLen2=0;
        // Allocate memory to hold copy of raw SPS data
        if(!spsBuf2)
            spsBuf2=new uint8_t[MAX_H264_SPS_SIZE];
        memset(spsBuf2,0,MAX_H264_SPS_SIZE);
        if(AnnexB)
            spsLen2=getRawH264SPS_startCode(img.data, img.dataLength, spsBuf2, MAX_H264_SPS_SIZE);
        else
            spsLen2=getRawH264SPS(img.data, img.dataLength, nalSize, spsBuf2, MAX_H264_SPS_SIZE);
        // If we have in-band SPS, the global one doesn't matter. The check below is purely informative.
        if(spsLen2)
        {
            //mixDump(vid->paramCache, vid->paramCacheSize);
            //mixDump(spsBuf, spsLen);
            if(memcmp(vid->paramCache, spsBuf2, (spsLen2 > vid->paramCacheSize)? vid->paramCacheSize : spsLen2))
            {
                ADM_warning("SPS mismatch? Checking deeper...\n");
                ADM_SPSInfo tmpinfo;
                if(extractSPSInfoFromData(spsBuf2, spsLen2, &tmpinfo))
                { // Update SPS info
#define MATCH(x) if(sps.x != tmpinfo.x) { ADM_warning("%s value does not match.\n",#x); sps.x = tmpinfo.x; match=false; }
                    MATCH(width)
                    MATCH(height)
                    MATCH(hasPocInfo)
                    MATCH(log2MaxFrameNum)
                    MATCH(log2MaxPocLsb)
                    MATCH(frameMbsOnlyFlag)
                    MATCH(refFrames)
                    if(!match)
                        ADM_warning("Codec parameters at frame %d don't match initial ones, expect problems.\n",frame);
                }
            }
        }

        poc2=-1; // POC of the first frame after the cut
#define NO_RECOVERY_INFO 0xFF
        uint32_t recoveryDistance=NO_RECOVERY_INFO;
        bool outcome=false;
        if(AnnexB)
            outcome=extractH264FrameType_startCode(img.data, img.dataLength, &(img.flags), &poc2, &sps, &recoveryDistance);
        else
            outcome=extractH264FrameType(img.data, img.dataLength, nalSize, &(img.flags), &poc2, &sps, &recoveryDistance);
        if(!outcome)
        {
            ADM_warning("Cannot get H.264 frame type and Picture Order Count Least Significant Bits value.\n");
            BOWOUT
        }
        /* Even if the frame is an IDR, we should check for SPS mismatch in case
        adjacent segments reference different videos. If the result is OK, we
        will skip POC check. */
        bool gotIdr = (img.flags & AVI_KEY_FRAME) && (img.flags & AVI_IDR_FRAME);

        /* Now we need to calculate the minimum POC among the early B-frames
        following the first frame of the segment. While not relevant for
        the effect of being "stuck", a POC smaller than the maximum POC
        before the cut point at an non-IDR frame will result in considerable
        artifacts, dropped frames, stutter, flicker and such. */

        minpoc2=-1;
        uint32_t delay;
        int earliest;
        if(getOpenGopDelayForSegment(segNo,0,&delay,&earliest) && earliest>0)
        {
            CHECK_FRAME(earliest)
            if(demuxer->getFrame(earliest,&img))
            {
                outcome=false;
                if(AnnexB)
                    outcome=extractH264FrameType_startCode(img.data, img.dataLength, &(img.flags), &minpoc2, &sps, NULL);
                else
                    outcome=extractH264FrameType(img.data, img.dataLength, nalSize, &(img.flags), &minpoc2, &sps, NULL);
                if(!outcome)
                    ADM_warning("Cannot get H.264 frame type and Picture Order Count Least Significant Bits value.\n");
            }else
            {
                ADM_warning("Unable to get frame %d in ref %d, segment %d\n",earliest,seg->_reference,segNo);
            }
        }else
        {
            earliest=frame;
        }
        if(minpoc2!=-1)
            ADM_info("Segment %u minimum POC LSB = %d at frame %d\n",segNo,minpoc2,earliest);
        // We are done with this segment, restore the last sent frame
        vid->lastSentFrame=oldFrame;

        uint8_t *nextSegParamCache=vid->paramCache;
        uint32_t nextSegParamCacheSize=vid->paramCacheSize;

        // Check the preceding segment
        ADM_info("Getting previous segment, current segment number: %d\n",segNo);
        ADM_assert(segNo>0);
        segNo--;
        seg=_segments.getSegment(segNo);
        vid=_segments.getRefVideo(seg->_reference);
        demuxer=vid->_aviheader;
        oldFrame=vid->lastSentFrame;

        if(false==switchToSegment(segNo,true))
        {
            ADM_warning("Cannot check the previous segment %d\n",segNo);
            BOWOUT
        }
        ADM_info("Switched to segment %d\n",segNo);
        // Same ref video?
        if(seg->_reference==refNo)
        {
            // Marked as IDR? Ignore recovery.
            if((img.flags & AVI_KEY_FRAME) && (img.flags & AVI_IDR_FRAME))
            {
                ADM_info("IDR verified and same ref video, the cut point is fine.\n");
                cut=ADM_EDITOR_CUT_POINT_KEY;
                BOWOUT
            }
            // Recovery point and no POC? Don't bother to perform further checks.
            if(!recoveryDistance && poc2==-1)
            {
                ADM_warning("No POC to compare with, only POC explicitely set in the slice header is supported.\n");
                cut=ADM_EDITOR_CUT_POINT_KEY; // ??
                BOWOUT
            }
            // Check for in-band SPS
            // Identify the last keyframe of the segment. If in-band SPS is present, it will be there.
            if(false==getFrameNumFromPtsOrBefore(vid, seg->_refStartTimeUs+seg->_durationUs-1, frame))
            {
                ADM_warning("Cannot identify the last frame in display order for segment %d\n",segNo);
                BOWOUT
            }
            flags=0;
            demuxer->getFlags(frame,&flags);
            while(frame>0)
            {
                if(flags & AVI_KEY_FRAME) break;
                demuxer->getFlags(--frame,&flags);
            }
            // Retrieve this keyframe
            CHECK_FRAME(frame)
            if(!demuxer->getFrame(frame,&img))
            {
                ADM_warning("Unable to get frame %d in ref %d, segment %d\n",frame,seg->_reference,segNo);
                BOWOUT
            }
            spsLen1=0;
            // Allocate another buffer to hold copy of raw SPS data
            if(!spsBuf1)
                spsBuf1=new uint8_t[MAX_H264_SPS_SIZE];
            memset(spsBuf1,0,MAX_H264_SPS_SIZE);
            if(AnnexB)
                spsLen1=getRawH264SPS_startCode(img.data, img.dataLength, spsBuf1, MAX_H264_SPS_SIZE);
            else
                spsLen1=getRawH264SPS(img.data, img.dataLength, nalSize, spsBuf1, MAX_H264_SPS_SIZE);
            if(spsLen1)
            {   /* If the next segment doesn't have in-band SPS but this one does,
                we are in trouble as we can't be sure that we parsed the slice header
                using a valid SPS info. The least harmful thing to do is to check that
                in-band SPS matches the global one and to bail out if it does not. */
                uint8_t *sbuf=vid->paramCache;
                uint32_t slen=vid->paramCacheSize;
                if(spsLen2 && spsBuf2)
                {
                    sbuf=spsBuf2;
                    slen=spsLen2;
                }
                if(memcmp(spsBuf1, sbuf, (spsLen1 > slen)? slen : spsLen1))
                {
                    ADM_warning("In-band SPS does not match the global one? Checking deeper...\n");
                    ADM_SPSInfo tmpinfo;
                    if(extractSPSInfoFromData(spsBuf1, spsLen1, &tmpinfo))
                    { // Update SPS info used to parse slice header later
                        match=true;
                        MATCH(width)
                        MATCH(height)
                        MATCH(hasPocInfo)
                        MATCH(log2MaxFrameNum)
                        MATCH(log2MaxPocLsb)
                        MATCH(frameMbsOnlyFlag)
                        MATCH(refFrames)
                        if(!match && !spsLen2)
                        {
                            ADM_warning("Appended incompatible streams possible, aborting check.\n");
                            BOWOUT
                        }
                    }
                }
            }
        }else
        {
            // Not the same video. Does the codec match?
            demuxer->getVideoInfo(&info);
            if(!isH264Compatible(info.fcc))
            {
                ADM_error("Codec mismatch!\n");
                cut=ADM_EDITOR_CUT_POINT_MISMATCH;
                BOWOUT
            }
            // Check that stream types match
            extraLen=0;
            demuxer->getExtraHeaderData(&extraLen,&extra);
            if(!extraLen != AnnexB) // FIXME stream filter setup should be per segment
            {
                ADM_warning("Combining AVCC and AnnexB type H.264 streams is currently not supported.\n");
                cut=ADM_EDITOR_CUT_POINT_MISMATCH;
                BOWOUT
            }
            // For AVCC, check that NALU length is coded on the same number of bytes
            if(extraLen)
            {
                uint32_t nalSize2=ADM_getNalSizeH264(extra,extraLen);
                if(nalSize2 != nalSize)
                {
                    ADM_warning("NAL unit length size mismatch: %u vs %u\n",nalSize2,nalSize);
                    cut=ADM_EDITOR_CUT_POINT_MISMATCH;
                    BOWOUT
                }
            }
            // Get SPS info
            // Check for in-band SPS first
            // Identify the last keyframe of the segment. If in-band SPS is present, it will be there.
            if(false==getFrameNumFromPtsOrBefore(vid, seg->_refStartTimeUs+seg->_durationUs-1, frame))
            {
                ADM_warning("Cannot identify the last frame in display order for segment %d\n",segNo);
                BOWOUT
            }
            flags=0;
            demuxer->getFlags(frame,&flags);
            while(frame>0)
            {
                if(flags & AVI_KEY_FRAME) break;
                demuxer->getFlags(--frame,&flags);
            }
            // Retrieve this keyframe
            CHECK_FRAME(frame)
            if(!demuxer->getFrame(frame,&img))
            {
                ADM_warning("Unable to get frame %d in ref %d, segment %d\n",frame,seg->_reference,segNo);
                BOWOUT
            }
            spsLen1=0;
            // Allocate buffer to hold copy of raw SPS data
            if(!spsBuf1)
                spsBuf1=new uint8_t[MAX_H264_SPS_SIZE];
            memset(spsBuf1,0,MAX_H264_SPS_SIZE);
            if(AnnexB)
                spsLen1=getRawH264SPS_startCode(img.data, img.dataLength, spsBuf1, MAX_H264_SPS_SIZE);
            else
                spsLen1=getRawH264SPS(img.data, img.dataLength, nalSize, spsBuf1, MAX_H264_SPS_SIZE);
            if(spsLen1)
            {   /* If the next segment's ref video doesn't have in-band SPS
                but this one does, we must check that the SPS matches
                the global header. */
                if(memcmp(spsBuf1, nextSegParamCache, (spsLen1 > nextSegParamCacheSize)? nextSegParamCacheSize : spsLen1))
                {
                    ADM_warning("In-band SPS before and after the cut point does not match? Checking deeper...\n");
                    ADM_SPSInfo tmpinfo;
                    if(!extractSPSInfoFromData(spsBuf1, spsLen1, &tmpinfo) || !getH264SPSInfo(vid,&tmpinfo))
                    {
                        ADM_warning("Cannot retrieve H.264 SPS info for segment %u\n",segNo);
                        BOWOUT
                    }
                    // Update SPS info used to parse slice header later
                    match=true;
                    MATCH(width)
                    MATCH(height)
                    MATCH(hasPocInfo)
                    MATCH(log2MaxFrameNum)
                    MATCH(log2MaxPocLsb)
                    MATCH(frameMbsOnlyFlag)
                    MATCH(refFrames)
                    if(!match && !spsLen2)
                    {
                        ADM_warning("SPS mismatch, saved video will be broken.\n");
                        cut=ADM_EDITOR_CUT_POINT_MISMATCH;
                        BOWOUT
                    }
#undef MATCH
                }
            }
            if(gotIdr) // If the first frame after the cut is IDR, we can ignore POC.
            {
                ADM_info("IDR verified, ref videos are different but probably compatible.\n");
                cut=ADM_EDITOR_CUT_POINT_KEY;
                BOWOUT
            }
        }

        ADM_info("Recovery distance: %u\n",recoveryDistance);
        if(!recoveryDistance) // recovery point, check POC
        {
            if(poc2==-1)
            {
                ADM_warning("No POC to compare, only POC explicitely set in the slice header is supported.\n");
                cut=ADM_EDITOR_CUT_POINT_KEY; // ??
                BOWOUT
            }
            /* Get the POC of the last frame in stream order before the cut,
            this is not necessarily the last displayed frame of the segment.
            We need to start earlier and identify the cut point. */
            uint32_t maxFrame,maxPtsFrame;
            uint64_t maxPts;
            if(false==findLastFrameBeforeSwitch(segNo,&maxFrame,&maxPtsFrame,&maxPts))
            {
                ADM_warning("Cannot identify the last frame before segment switch for segment %d\n",segNo);
                BOWOUT
            }
            // maxFrame is now the last frame before segment switch.
            // We need poc_lsb of this frame to get poc_msb right.
            CHECK_FRAME(maxFrame)
            if(!demuxer->getFrame(maxFrame,&img))
            {
                ADM_warning("Unable to get frame %d in ref %d, segment %d\n",maxFrame,seg->_reference,segNo);
                BOWOUT
            }
            // Try to get POC, recovery distance doesn't matter here
            poc1 = -1; // POC of the last frame before the cut
            outcome=false;
            if(AnnexB)
                outcome=extractH264FrameType_startCode(img.data, img.dataLength, &(img.flags), &poc1, &sps, NULL);
            else
                outcome=extractH264FrameType(img.data, img.dataLength, nalSize, &(img.flags), &poc1, &sps, NULL);
            if(!outcome)
            {
                ADM_warning("Cannot get H.264 frame type and Picture Order Count Least Significant Bits value.\n");
                BOWOUT
            }
            if(poc1==-1)
            {
                ADM_warning("Cannot get POC, only POC explicitely set in the slice header is supported.\n");
                BOWOUT
            }
            /* Calculate poc_msb, the discontinuity of poc_lsb at the cut point
            may easily result in poc_msb incrementing or decrementing across the cut
            due to poc_lsb wrapping around. */
            const int maxPocLsb = 1 << sps.log2MaxPocLsb;
            int pocMsb = 0;
            if(poc1 > poc2 && poc1 - poc2 >= maxPocLsb/2)
                pocMsb += maxPocLsb;
            else if(poc2 > poc1 && poc2 - poc1 > maxPocLsb/2)
                pocMsb -= maxPocLsb;

            ADM_info("POC of the last frame %u in stream order of the previous seg: %d + %d = %d\n",
                    maxFrame, pocMsb, poc1, pocMsb + poc1);

            int delta = poc1 - pocMsb - poc2;

            /* If delta is positive, the cut is bad and we don't need to check POC of other frames.
            If delta is negative (POC increasese across the cut), playback won't get stuck, but we
            need to check whether POC ranges overlap which may result in pictures from both segments
            displayed interleaved. */
            if(delta>0)
            {
                ADM_warning("Saved video won't be smoothly playable in FFmpeg-based players (POC going back by %d)\n",delta);
                cut=ADM_EDITOR_CUT_POINT_BAD_POC;
                BOWOUT
            }

            cut=ADM_EDITOR_CUT_POINT_KEY;

            if(!maxPts || maxPtsFrame==-1)
            {
                ADM_warning("Cannot identify the last frame in display order before the cut for segment %d\n",segNo);
                BOWOUT
            }
            CHECK_FRAME(maxPtsFrame)
            if(!demuxer->getFrame(maxPtsFrame,&img))
            {
                ADM_warning("Unable to get frame %d in ref %d, segment %d\n",maxPtsFrame,seg->_reference,segNo);
                BOWOUT
            }
            // Got our frame, get poc_lsb of the slice
            maxpoc1=-1;
            outcome=false;
            if(AnnexB)
                outcome=extractH264FrameType_startCode(img.data, img.dataLength, &(img.flags), &maxpoc1, &sps, NULL);
            else
                outcome=extractH264FrameType(img.data, img.dataLength, nalSize, &(img.flags), &maxpoc1, &sps, NULL);
            if(!outcome)
            {
                ADM_warning("Cannot get H.264 frame type and Picture Order Count Least Significant Bits value.\n");
                BOWOUT
            }

            if(maxpoc1==-1)
            {
                ADM_warning("Cannot get POC, only POC explicitely set in the slice header is supported.\n");
                BOWOUT // or should the check fail instead?
            }

            /* Check that POC doesn't go back, i.e. max POC before the cut is less
            than min POC after segment switch. First check that poc_msb is still valid. */
            int prevPocMsb=0;
            if(maxpoc1 > poc1 && maxpoc1 - poc1 >= maxPocLsb/2)
                prevPocMsb += maxPocLsb;
            else if(poc1 > maxpoc1 && poc1 - maxpoc1 > maxPocLsb/2)
                prevPocMsb -= maxPocLsb;

            ADM_info("POC of the last frame %u in display order of the previous seg: %d + %d = %d\n",
                    maxPtsFrame, prevPocMsb, maxpoc1, prevPocMsb + maxpoc1);

            poc1 += prevPocMsb;

            if(poc2 > minpoc2 && poc2 - minpoc2 >= maxPocLsb/2)
                pocMsb += maxPocLsb;
            else if(minpoc2 > poc2 && minpoc2 - poc2 > maxPocLsb/2)
                pocMsb -= maxPocLsb;

            ADM_info("POC of the earliest frame %d in display order of the current seg: %d + %d = %d\n",
                    earliest, pocMsb, poc2, pocMsb + poc2);

            poc2 += pocMsb;

            delta=poc1-poc2;
            if(delta>0)
            {
                ADM_warning("Saved video will exhibit flicker at cut point in FFmpeg-based players (POC overlap by %d)\n",delta);
                cut=ADM_EDITOR_CUT_POINT_BAD_POC;
                BOWOUT
            }
        }
    }

    if(isH265Compatible(info.fcc))
    {
        int poc1,maxpoc1,poc2,minpoc2;
        // In HEVC, we check only for POC going back.
        // Get SPS to be able to decode the slice header
        ADM_SPSinfoH265 sps;
        if(!getH265SPSInfo(vid,&sps))
        {
            ADM_warning("Cannot retrieve SPS info.\n");
            BOWOUT
        }
        // Determine stream type
        bool AnnexB=false;
        uint8_t *extra;
        uint32_t extraLen=0;
        uint32_t nalSize=0;
        demuxer->getExtraHeaderData(&extraLen,&extra);
        if(!extraLen)
            AnnexB=true;
        else
            nalSize=ADM_getNalSizeH265(extra,extraLen);
        // Allocate memory to hold compressed frame
        ADMCompressedImage img;
        buffer=new uint8_t[MAX_FRAME_LENGTH];
        img.data=buffer;
        // With HEVC we need to check the preceding segment first
        ADM_info("Getting previous segment, current segment number: %d\n",segNo);
        ADM_assert(segNo>0);
        segNo--;
        seg=_segments.getSegment(segNo);
        vid=_segments.getRefVideo(seg->_reference);
        demuxer=vid->_aviheader;
        oldFrame=vid->lastSentFrame;
        uint32_t rememberCurrent=frame;
        uint32_t rememberLastSent=oldFrame;

        if(false==switchToSegment(segNo,true))
        {
            ADM_warning("Cannot check the previous segment %d\n",segNo);
            BOWOUT
        }
        // Not the same ref video?
        if(seg->_reference!=refNo)
        {
            // Does the codec match?
            demuxer->getVideoInfo(&info);
            if(!isH265Compatible(info.fcc))
            {
                ADM_error("Codec mismatch!\n");
                cut=ADM_EDITOR_CUT_POINT_MISMATCH;
                BOWOUT
            }
            // Check that stream types match
            extraLen=0;
            demuxer->getExtraHeaderData(&extraLen,&extra);
            if(!extraLen != AnnexB) // FIXME stream filter setup should be per segment
            {
                ADM_warning("Combining AVCC and AnnexB type HEVC streams is currently not supported.\n");
                cut=ADM_EDITOR_CUT_POINT_MISMATCH;
                BOWOUT
            }
            // For HVCC, check that NALU length is coded on the same number of bytes
            if(extraLen)
            {
                uint32_t nalSize2=ADM_getNalSizeH265(extra,extraLen);
                if(nalSize2 != nalSize)
                {
                    ADM_warning("NAL unit length size mismatch: %u vs %u\n",nalSize2,nalSize);
                    cut=ADM_EDITOR_CUT_POINT_MISMATCH;
                    BOWOUT
                }
            }
            // Get SPS info
            ADM_SPSinfoH265 sps2;
            if(!getH265SPSInfo(vid,&sps2))
            {
                ADM_warning("Cannot retrieve SPS info for HEVC ref video in segment %d\n",segNo);
                BOWOUT
            }
            // We have SPS, does it match the one from the next segment?
            // Width and height match for sure, check remaining fields.
#define MATCH(x) if(sps.x != sps2.x) { ADM_warning("%s value does not match.\n",#x); match=false; }
            bool match=true;
            MATCH(fps1000)
            MATCH(log2_max_poc_lsb)
            MATCH(separate_colour_plane_flag)
            MATCH(num_extra_slice_header_bits)
            MATCH(dependent_slice_segments_enabled_flag)
            MATCH(output_flag_present_flag)
            MATCH(field_info_present)
            MATCH(address_coding_length)
            if(!match)
            {
                ADM_warning("SPS mismatch, saved video will be broken.\n");
                cut=ADM_EDITOR_CUT_POINT_MISMATCH;
                BOWOUT
            }
        }

        // Get the POC of the last frame in stream order before the cut,
        uint32_t maxFrame,maxPtsFrame;
        uint64_t maxPts=0;
        if(false==findLastFrameBeforeSwitch(segNo,&maxFrame,&maxPtsFrame,&maxPts) ||
            (int)maxFrame==-1 || (int)maxPtsFrame==-1 || !maxPts)
        {
            ADM_warning("Cannot identify the last frame before segment switch for segment %d\n",segNo);
            BOWOUT
        }
        // maxFrame is now the last frame before segment switch.
        // If maxPts happens earlier, we must start there.
        if(maxFrame>maxPtsFrame && maxFrame-maxPtsFrame>16)
            maxPtsFrame=maxFrame; // invalidate maxPtsFrame
        frame=(maxPtsFrame < maxFrame && maxFrame-maxPtsFrame < 16)? maxPtsFrame : maxFrame;

        // Try to get POC, we start either at the last frame in display order
        // or in stream order before the cut, whichever comes first.
        poc1 = INT_MIN;
        maxpoc1 = poc1;
        bool outcome=false;
        for(int i=frame; i<=maxFrame; i++)
        {
            CHECK_FRAME(i)
            if(!demuxer->getFrame(i,&img))
            {
                ADM_warning("Unable to get frame %d in ref %d, segment %d\n",i,seg->_reference,segNo);
                BOWOUT
            }
            if(AnnexB)
                outcome=extractH265FrameType_startCode(img.data, img.dataLength, &sps, &(img.flags), &poc1);
            else
                outcome=extractH265FrameType(img.data, img.dataLength, nalSize, &sps, &(img.flags), &poc1);
            if(!outcome)
            {
                ADM_warning("Cannot get HEVC frame type and Picture Order Count value.\n");
                BOWOUT
            }
            if(poc1 > maxpoc1)
                maxpoc1 = poc1;
        }
        // We are done with this segment, restore the last sent frame
        vid->lastSentFrame=oldFrame;
        // Switch back to the segment after the cutpoint
        segNo++;
        if(false==switchToSegment(segNo,true))
        {
            ADM_warning("Cannot return to the current segment %d\n",segNo);
            BOWOUT
        }
        // Restore
        frame=rememberCurrent;
        vid->lastSentFrame=rememberLastSent;
        // Get the frame we are interested in.
        CHECK_FRAME(frame)
        if(!demuxer->getFrame(frame,&img))
        {
            ADM_warning("Unable to get frame %d in ref %d, segment %d\n",frame,seg->_reference,segNo);
            BOWOUT
        }
        poc2=poc1; // POC of the first frame after the cut
        outcome=false;
        if(AnnexB)
            outcome=extractH265FrameType_startCode(img.data, img.dataLength, &sps, &(img.flags), &poc2);
        else
            outcome=extractH265FrameType(img.data, img.dataLength, nalSize, &sps, &(img.flags), &poc2);
        if(!outcome)
        {
            ADM_warning("Cannot get HEVC frame type and Picture Order Count value.\n");
            BOWOUT
        }

        if((img.flags & AVI_KEY_FRAME) && (img.flags &AVI_IDR_FRAME)) // IDR verified, POC doesn't matter
        {
            cut=ADM_EDITOR_CUT_POINT_KEY;
            BOWOUT
        }

        int delta = poc1 - poc2;
        if(delta>0)
        {
            ADM_warning("Saved video won't be smoothly playable in FFmpeg-based players (POC going back by %d)\n",delta);
            cut=ADM_EDITOR_CUT_POINT_BAD_POC;
            BOWOUT
        }

        /* Calculate the minimum POC among the early B-frames following
        the first frame of the segment. Overlapping POC ranges may result
        in dropped pictures, considerable artifacts and stutter. */

        minpoc2 = INT_MAX;
        uint32_t delay;
        int earliest;

        if(getOpenGopDelayForSegment(segNo,0,&delay,&earliest) && earliest>frame)
        {
            for(int i=frame+1; i<=earliest; i++)
            {
                CHECK_FRAME(i)
                if(!demuxer->getFrame(i,&img))
                {
                    ADM_warning("Unable to get frame %d in ref %d, segment %d\n",i,seg->_reference,segNo);
                    BOWOUT
                }
                if(AnnexB)
                    outcome=extractH265FrameType_startCode(img.data, img.dataLength, &sps, &(img.flags), &poc2);
                else
                    outcome=extractH265FrameType(img.data, img.dataLength, nalSize, &sps, &(img.flags), &poc2);
                if(!outcome)
                {
                    ADM_warning("Cannot get HEVC frame type and Picture Order Count value.\n");
                    BOWOUT
                }
                if(poc2 < minpoc2)
                    minpoc2 = poc2;
            }
        }
        delta=0;
        if(minpoc2 != INT_MAX)
            delta = maxpoc1 - minpoc2;
        if(delta>0)
        {
            ADM_warning("Saved video will exhibit flicker at cut point in FFmpeg-based players (POC overlap by %d)\n",delta);
            cut=ADM_EDITOR_CUT_POINT_BAD_POC;
            BOWOUT
        }
    }
    cut=ADM_EDITOR_CUT_POINT_KEY;
    BOWOUT
}
/**
    \fn checkCutsAreOnIntra
    \brief Check all segments
*/
ADM_cutPointType ADM_Composer::checkCutsAreOnIntra(void)
{
    ADM_cutPointType success=ADM_EDITOR_CUT_POINT_UNCHECKED;
    int nbSeg=_segments.getNbSegments();
    ADM_info("Checking cuts start on keyframe..\n");
    for(int i=0;i<nbSeg;i++)
    {
        success=checkSegmentStartsOnIntra(i);
        if(success!=ADM_EDITOR_CUT_POINT_KEY)
            break;
    }
    return success;
}

/**
    \fn checkCutsAreOnIntra
    \brief Check a span
*/
ADM_cutPointType ADM_Composer::checkCutsAreOnIntra(uint64_t startTime,uint64_t endTime)
{
    ADM_cutPointType success=ADM_EDITOR_CUT_POINT_UNCHECKED;
    int nbSeg=_segments.getNbSegments();
    ADM_info("Checking cuts start on keyframe..\n");
    uint32_t segNo;
    uint64_t segTime;

    if(false==_segments.convertLinearTimeToSeg(startTime,&segNo,&segTime))
        return ADM_EDITOR_CUT_POINT_UNCHECKED; // we can't do anything meaningful if we fail to convert time to segment

    ADM_assert(segNo<nbSeg);
    int startSeg=segNo;
    // end time greater or equal video duration means the last segment
    if(endTime >= getVideoDuration())
        segNo = nbSeg-1;
    else if(false==_segments.convertLinearTimeToSeg(endTime,&segNo,&segTime))
        return ADM_EDITOR_CUT_POINT_UNCHECKED;

    if(segNo==startSeg) // within one and the same segment, check only that codec matches
    {
        aviInfo here,first;
        getVideoInfo(&first);
        _SEGMENT *seg=_segments.getSegment(segNo);
        _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
        vid->_aviheader->getVideoInfo(&here);
        if(!checkCodec(&here,&first))
            return ADM_EDITOR_CUT_POINT_MISMATCH;
        return ADM_EDITOR_CUT_POINT_KEY;
    }
    for(int i=startSeg;i<segNo;i++)
    {
        success=checkSegmentStartsOnIntra(i+1);
        if(success!=ADM_EDITOR_CUT_POINT_KEY)
            break;
    }
    return success;
}

/**
    \fn checkCutIsOnIntra
    \brief Allow to check if a particular delete operation results in a cut being not on an intra
*/
ADM_cutPointType ADM_Composer::checkCutIsOnIntra(uint64_t time)
{
    uint32_t segNo;
    uint64_t segTime;
    if(false==_segments.convertLinearTimeToSeg(time,&segNo,&segTime))
        return ADM_EDITOR_CUT_POINT_UNCHECKED; // we can't do anything meaningful if we fail to convert time to segment

    ADM_info("Checking whether cut at %s is on a keyframe...\n",ADM_us2plain(time));

    return checkSegmentStartsOnIntra(segNo);
}

/**
     \fn bFrameDroppable
*/
static bool bFrameDroppable(uint32_t fcc,_VIDEOS *vid,ADMCompressedImage *img=NULL)
{
    if(isH264Compatible(fcc))
    {
        if(!img)
            return false;
        if(img->flags & (AVI_STRUCTURE_TYPE_MASK+AVI_FIELD_STRUCTURE))
            return false; // don't drop fields
        if(img->flags & AVI_NON_REF_FRAME)
            return true; // the demuxer has done the job for us
        uint32_t maxSize=img->dataLength;
        maxSize+=512; // arbitrary safety margin
        uint32_t size=maxSize;
        uint32_t nalSize=0;
        notStackAllocator buf(maxSize);
        bool AnnexB=false;
        if(vid && vid->_aviheader)
        {
            uint8_t *extra;
            uint32_t extraLen=0;
            vid->_aviheader->getExtraHeaderData(&extraLen,&extra);
            if(!extraLen)
                AnnexB=true;
            else
                nalSize=ADM_getNalSizeH264(extra,extraLen);
        }
        if(AnnexB)
            size=ADM_convertFromAnnexBToMP4(img->data,img->dataLength,buf.data,maxSize);
        else
            memcpy(buf.data,img->data,img->dataLength);
        if(size)
        {
            ADM_SPSInfo sps;
            bool gotSps=getH264SPSInfo(vid,&sps);
            if(!gotSps)
                ADM_warning("No SPS info available to check fields.\n");
            if(extractH264FrameType(buf.data, size, nalSize, &(img->flags), NULL, gotSps? &sps : NULL,NULL) &&
                    (img->flags & AVI_NON_REF_FRAME) &&
                    !(img->flags & (AVI_STRUCTURE_TYPE_MASK+AVI_FIELD_STRUCTURE)))
                return true;
        }
        return false;
    }
    if(isH265Compatible(fcc)) 
      return false;    
    return true;
}

/**
 *  \fn getOpenGopDelayForSegment
 */
bool ADM_Composer::getOpenGopDelayForSegment(uint32_t segNo, uint64_t segTime, uint32_t *delay, int *frameNo)
{
    _SEGMENT *seg=_segments.getSegment(segNo);
    ADM_assert(seg);
    _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
    ADM_assert(vid);
    vidHeader *demuxer=vid->_aviheader;
    ADM_assert(demuxer);
    uint64_t pts,dts;

    // Look ahead to see if we have some late bframe in the past
    int found=-1;
    int nb=demuxer->getMainHeader()->dwTotalFrames;
    uint64_t refTime=segTime+seg->_refStartTimeUs;
    if(!refTime)
    {
        found=0;
        demuxer->getPtsDts(0,&pts,&dts);
    }else
    {
        for(int i=0;i<nb;i++)
        {
            demuxer->getPtsDts(i,&pts,&dts);
            if(pts==refTime)
            {
                found=i;
                break;
            }
        }
    }
    if(found==-1)
    {
        ADM_warning("Cannot find the frame for segment %" PRIu32"\n",segNo);
        return false;
    }

    *delay=0;

    bool trustDemuxer=false;
    int frame=-1;
    aviInfo info;
    demuxer->getVideoInfo(&info);

    for(int i=found+1; i < found + MAX_REF_FRAMES_FIELDS; i++)
    {
        uint32_t flags;
        demuxer->getFlags(i,&flags);
        if(!trustDemuxer && (flags & AVI_B_FRAME))
            trustDemuxer=true;
        if((trustDemuxer && !(flags & AVI_B_FRAME)) || (flags & AVI_KEY_FRAME))
        {
            ADM_info("Not a bframe, stopping (%d)\n",i-found);
            break;
        }
        demuxer->getPtsDts(i,&pts,&dts);
        if(pts==ADM_NO_PTS) continue;
        if(pts<refTime)
        {
            ADM_info("frame %d is early \n",i);
            ADMCompressedImage img;
#define ROUNDUP(x,y) (x+y-1)&~(y-1)
            uint32_t len=ROUNDUP(info.width,16);
            len*=ROUNDUP(info.height,16);
            len*=3;
            notStackAllocator buf(len);
            img.flags=0;
            img.data=buf.data;
            if(!demuxer->getFrameSize(i,&(img.dataLength)))
                img.dataLength=0;
            if(img.dataLength && img.dataLength <= len && demuxer->getFrame(i,&img))
            {
                if(bFrameDroppable(info.fcc,vid,&img))
                    continue; // this frame will be dropped, no need to add delay
            }
            uint32_t delta=refTime-pts;
            if(delta>*delay)
            {
                *delay=delta;
                frame=i;
            }
        }else
        {
            ADM_info("Pts delta = %d\n",(int)(pts-refTime));
        }
    }
    if(frameNo)
        *frameNo=frame;
    if(!segTime && refTime>*delay) // irrelevant for the first segment in range
        seg->_refMinimumPts=refTime-*delay;
 /* printf("Segment %u, start in ref at %s ",segNo,ADM_us2plain(seg->_refStartTimeUs));
    printf("with offset %s ",ADM_us2plain(segTime));
    printf("minimum PTS %s\n",(seg->_refMinimumPts!=ADM_NO_PTS)? ADM_us2plain(seg->_refMinimumPts) : "unset"); */
    return true;
}

/**
 * 
 * @param time
 * @param delay
 * \brief for x264/x265 we dont want to drop the non closed gop bframe(s)
 * They will be decoded after our first intra but shown before. 
 * They are needed because they might be used as reference for later frames.
 * 
 * For older codec (mpeg4 ASP, Mpeg1,...) these bframes can be just dropped.
 * 
 * This function tries to find out the maximum delay we must add
 * so that the overall PTS of these frames dont get negative.
 * @return 
 */
bool        ADM_Composer::getNonClosedGopDelay(uint64_t time,uint32_t *delay)
{
    aviInfo info;
    uint32_t startSegNo;
    uint64_t segTime;
    *delay=0;
    if(!_segments.convertLinearTimeToSeg(time,&startSegNo,&segTime))
    {
        ADM_warning("Cannot navigate to get nonclosedgop\n");
        return false;
    }

    _SEGMENT *seg=_segments.getSegment(startSegNo);
    ADM_assert(seg);
    _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
    ADM_assert(vid);
    vidHeader *demuxer=vid->_aviheader;
    ADM_assert(demuxer);
    demuxer->getVideoInfo(&info);
    if(bFrameDroppable(info.fcc,NULL))
        return true; // no need to add extra delay

    bool found=false;

    for(uint32_t segNo=startSegNo; segNo < _segments.getNbSegments(); segNo++)
    {
        if(segNo > startSegNo)
            segTime=0;
        uint32_t segDelay=0;
        if(false == getOpenGopDelayForSegment(segNo,segTime,&segDelay))
            continue;
        if(segDelay>*delay)
            *delay=segDelay;
        found=true;
    }
    if(!found)
        return false;
    ADM_info("Found maximum non closed gop delay = %d\n",*delay);
    return true;
}

/**
        \fn getCompressedPicture
        \brief bypass decoder and directly get the source image

    The dropBframe is as follow :
            0 : Dont drop b frame
            1 : Follow the next bframes
            2 : Drop


*/
bool        ADM_Composer::getCompressedPicture(uint64_t start,uint64_t videoDelay,ADMCompressedImage *img)
{
    uint64_t tail;
    //
    int64_t signedPts;
    int64_t signedDts;

againGet:
    static uint32_t fn=0;
    fn++;
    _SEGMENT *seg=_segments.getSegment(_currentSegment);
    ADM_assert(seg);
    _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
    ADM_assert(vid);
    vidHeader *demuxer=vid->_aviheader;
    ADM_assert(demuxer);

    uint32_t segNo=0;
    uint64_t segTme,startFromPtsInRef=ADM_NO_PTS;
    if(_segments.convertLinearTimeToSeg(start,&segNo,&segTme))
    {
        startFromPtsInRef=segTme+seg->_refStartTimeUs;
    }
    // Get next pic?
    if(false==demuxer->getFrame (vid->lastSentFrame,img))
    {
        ADM_info("Failed to get next frame for ref %" PRIu32"\n",seg->_reference);
        goto nextSeg;
    }
    aprintf("Got frame %d, flags=0x%04x PTS=%s ",vid->lastSentFrame,img->flags,ADM_us2plain(img->demuxerPts));
    aprintf("DTS=%s\n",ADM_us2plain(img->demuxerDts));
    vid->lastSentFrame++;
    //
    aviInfo    info;
    vid->_aviheader->getVideoInfo (&info);
    //
    // after a segment switch, we may have some frames from "the past"
    // if the cut point is not a keyframe, drop them
    if(bFrameDroppable(info.fcc,NULL))
    {
        if(img->flags & AVI_B_FRAME)
        {
            if(seg->_dropBframes==_SEGMENT::ADM_DROPPING) 
            {
                ADM_warning("%" PRIu32" Dropping bframes\n",fn);
                goto againGet;
            }
        }else
        { // not a bframe
            switch(seg->_dropBframes)
            {
                case _SEGMENT::ADM_NO_DROP                : break;
                case _SEGMENT::ADM_DROPPING               : seg->_dropBframes=_SEGMENT::ADM_NO_DROP;break;
                case _SEGMENT::ADM_DROP_MAYBE_AFER_SWITCH : seg->_dropBframes=_SEGMENT::ADM_DROPPING;break;
                default: break;
            }
        }
    }
    if(img->demuxerDts!=ADM_NO_PTS)
    {
        if(img->demuxerDts<seg->_refStartDts)
        {
            printf("[getCompressedPicture] Frame %d DTS is in the past vs segment start: %s /",vid->lastSentFrame,ADM_us2plain(img->demuxerDts));
            printf(" %s\n",ADM_us2plain(seg->_refStartDts));
            goto againGet;
        }
    }
    if(img->demuxerPts!=ADM_NO_PTS)
    {
        // Seeking is not accurate when cutting on non intra
        // we might have some frames that are clearly too early , even in seg0
        uint64_t reftime=(seg->_refStartTimeUs > vid->firstFramePts)? seg->_refStartTimeUs : vid->firstFramePts;
        if(startFromPtsInRef!=ADM_NO_PTS && segNo==_currentSegment && startFromPtsInRef>reftime)
            reftime=startFromPtsInRef;
        if(img->demuxerPts<reftime)
        {
            printf("[getCompressedPicture] Frame %d PTS is in the past vs segment start: %s /",vid->lastSentFrame,ADM_us2plain(img->demuxerPts));
            printf(" %s\n",ADM_us2plain(reftime));
            if(bFrameDroppable(info.fcc,vid,img))
            {
                ADM_warning("Dropping frame %d\n",vid->lastSentFrame);
                goto againGet;
            }
        }
    }

    // Need to switch seg ?
    tail=seg->_refStartTimeUs+seg->_durationUs;
    // Guess DTS
    //
//**
   // ADM_info("Frame : Flags :%X, DTS:%"PRId64" PTS=%"PRId64" nextDts=%"PRId64" tail=%"PRId64"\n",img->flags,img->demuxerDts/1000,img->demuxerPts/1000,_nextFrameDts,tail);
    if(img->demuxerDts!= ADM_NO_PTS && img->demuxerDts>=tail) 
    {
        std::string tailAsString=std::string(ADM_us2plain(tail));
        ADM_info("DTS is too late, switching segment (%s vs %s)\n",ADM_us2plain(img->demuxerDts),tailAsString.c_str());
        goto nextSeg;
    }
    if(img->demuxerPts!= ADM_NO_PTS && img->demuxerPts>=tail) 
    {
        std::string tailAsString=std::string(ADM_us2plain(tail));
        ADM_info("PTS is too late, switching segment (%s vs %s)\n",ADM_us2plain(img->demuxerPts),tailAsString.c_str());
        goto nextSeg;
    }
    
    // Since we rely on PTS for seeking, frame 0 might be at PTS 0, in that case the matching dts would be <0
    // so the caller can delay everything but recalibrate will clamp the value
    // so we use correctedDts so that the value is ok
    if(img->demuxerDts==ADM_NO_PTS)
    {
            aprintf("DTS unset for frame %" PRIu32"\n",vid->lastSentFrame-1);
            signedDts=ADM_NO_PTS;
    }else
    {
            signedDts=(int64_t)img->demuxerDts;
            recalibrateSigned(&(signedDts),seg);
            aprintf("Signed Dts=%s ",ADM_us2plain(signedDts));
    }

    if(img->demuxerPts==ADM_NO_PTS)
    {
            aprintf("PTS unset for frame %" PRIu32"\n",vid->lastSentFrame-1);
            signedPts=ADM_NO_PTS;
    }else
    {
            signedPts=(int64_t)img->demuxerPts;
            recalibrateSigned(&(signedPts),seg);
            aprintf("Signed Pts=%s\n",ADM_us2plain(signedPts));
    }
    // From here we are in linear time, guess DTS if missing...
    if(signedDts==ADM_NO_PTS)
    {
        // border case due to rounding we can have dts slighly above pts
        if(_nextFrameDts!=ADM_NO_PTS)
        {
            signedDts=_nextFrameDts;
            if(signedPts != ADM_NO_PTS && signedDts>signedPts)
            {
                ADM_warning("Guessed DTS > PTS by %" PRId64" us for frame %" PRIu32", bumping PTS to %s (%" PRId64" us)\n",
                              signedDts - signedPts, vid->lastSentFrame-1, ADM_us2plain(signedDts), signedDts);
                signedPts=signedDts;
            }
        }
    }else
    {
        _nextFrameDts=signedDts;
    }
    // Increase for next one
    if(ADM_NO_PTS!=_nextFrameDts)
        _nextFrameDts+=vid->timeIncrementInUs;
    if(_currentSegment+1<_segments.getNbSegments())
    {
        _SEGMENT *nextSeg=_segments.getSegment(_currentSegment+1);
        ADM_assert(nextSeg);
        // Check the DTS is not too late compared to next seg beginning...
        if(signedDts!=ADM_NO_PTS)
        {
            int64_t nextDts=nextSeg->_startTimeUs+nextSeg->_refStartDts;
            if(nextDts<nextSeg->_refStartTimeUs)
            {
                ADM_warning("Frame %" PRIu32", next DTS is negative %" PRIu64" %" PRIu64" us\n",fn,nextDts,nextSeg->_refStartTimeUs);
            }else
            {
                nextDts-=nextSeg->_refStartTimeUs;
                if(signedDts>=nextDts)
                {
                    ADM_warning("Frame %" PRIu32", have to switch segment, DTS limit reached %" PRIu64" %" PRIu64" ms\n",fn,signedDts/1000,nextDts/1000);
                    goto nextSeg;
                }
            }
        }
        // Check that PTS does not collide with early B-frames,
        // _refMinimumPts must be recalculated on every segment layout change!
        // Skip if _refMinimumPts is not set, i.e. is equal zero.
        if(signedPts!=ADM_NO_PTS && nextSeg->_refMinimumPts)
        {
            int64_t nextPts=nextSeg->_startTimeUs+nextSeg->_refMinimumPts;
            if(nextPts<nextSeg->_refStartTimeUs)
            {
                ADM_warning("Frame %" PRIu32", next PTS is negative %" PRIu64" %" PRIu64" us\n",fn,nextPts,nextSeg->_refStartTimeUs);
            }else
            {
                nextPts-=nextSeg->_refStartTimeUs;
                if(signedPts>=nextPts)
                {
                    ADM_warning("Frame %" PRIu32", have to switch segment, PTS limit reached %" PRIu64" %" PRIu64" ms\n",fn,signedPts/1000,nextPts/1000);
                    goto nextSeg;
                }
            }
        }
    }
// **
   // ADM_info("Frame after RECAL: Flags :%X, DTS:%"PRId64" PTS=%"PRId64" tail=%"PRId64"\n",img->flags,img->demuxerDts/1000,img->demuxerPts/1000,tail);
    {
    int64_t finalDts=ADM_NO_PTS;
    if(signedDts!=ADM_NO_PTS)
        finalDts=signedDts+(int64_t)videoDelay;
    if(finalDts!=ADM_NO_PTS && finalDts<0)
    {
        ADM_warning("Final DTS <0, dropping it \n");
        finalDts=0;
        goto againGet;
    }
    img->demuxerDts=finalDts;
    }
    if(signedPts!=ADM_NO_PTS)
        img->demuxerPts=signedPts+(int64_t)videoDelay;
    else
        img->demuxerPts=ADM_NO_PTS;
    aprintf("Final Pts=%s ",ADM_us2plain(img->demuxerPts));
    aprintf("Final Dts=%s ",ADM_us2plain(img->demuxerDts));    
    aprintf("Delay=%s\n",ADM_us2plain(videoDelay));
#if 0
    {
    std::string ptsAsString,dtsAsString,frameTypeAsString;
    ptsAsString=std::string(ADM_us2plain(img->demuxerPts));
    dtsAsString=std::string(ADM_us2plain(img->demuxerDts));
    frameTypeAsString="P";
    if(img->flags & AVI_KEY_FRAME) frameTypeAsString="I";
    if(img->flags & AVI_B_FRAME) frameTypeAsString="B";
    printf("Frame %" PRIu32", DTS=%s, PTS=%s, type: %s\n",vid->lastSentFrame-1,dtsAsString.c_str(),ptsAsString.c_str(),frameTypeAsString.c_str());
    }
#endif
    return true;

nextSeg:
    if(false==switchToNextSegment(true))
    {
        ADM_warning("Cannot update to new segment\n");
        return false;
    }
    // Mark it as drop b frames...
    _SEGMENT *thisseg=_segments.getSegment(_currentSegment);
    thisseg->_dropBframes=_SEGMENT::ADM_DROP_MAYBE_AFER_SWITCH;
    ADM_info("Retrying for next segment\n");
    return getCompressedPicture(start,videoDelay,img);
}

/**
        \fn     getDirectImage
        \brief  For DEBUG DO NOT USE!

*/
bool        ADM_Composer::getDirectImageForDebug(uint32_t frameNum,ADMCompressedImage *img)
{
  
    _SEGMENT *seg=_segments.getSegment(0);
    ADM_assert(seg);
    _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
    ADM_assert(vid);
    vidHeader *demuxer=vid->_aviheader;
    ADM_assert(demuxer);

    // Get next pic?
    if(false==demuxer->getFrame (frameNum,img))
    {
        ADM_info("Failed to get next frame for ref %" PRIu32"\n",seg->_reference);
        return false;
    }
   return true;
}

/**
    \fn getDirectKeyFrameImageAtPts
    \brief Find and retrieve compressed keyframe image at or just before given time,
           needed to create global header from in-band parameter sets.
*/
bool ADM_Composer::getDirectKeyFrameImageAtPts(uint64_t time, ADMCompressedImage *img)
{
    uint32_t segNo;
    uint64_t segTime;

    if(false==_segments.convertLinearTimeToSeg(time,&segNo,&segTime))
        return false;
    _SEGMENT *seg=_segments.getSegment(segNo);
    ADM_assert(seg);
    _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
    ADM_assert(vid);
    vidHeader *demuxer=vid->_aviheader;
    ADM_assert(demuxer);

    uint64_t refTime=segTime+seg->_refStartTimeUs;
    if(refTime<=vid->firstFramePts)
        return demuxer->getFrame(0,img);

    int frame=0;
    if(false==getFrameNumFromPtsOrBefore(vid,refTime,frame))
        return false;
    uint32_t flags=0;
    demuxer->getFlags(frame,&flags);
    while(frame>0 && !(flags & AVI_KEY_FRAME))
        demuxer->getFlags(--frame,&flags);
    return demuxer->getFrame(frame,img);
}

/**
    \fn getUserDataUnregistered
    \brief For H.264, libavcodec parses SEI messages of type user data unregistered to retrieve x264
           version number to apply version-specific quirks. This function copies the NALU containing
           this SEI message to buffer allocated by the caller.
*/
bool ADM_Composer::getUserDataUnregistered(uint64_t start, uint8_t *buffer, uint32_t max, uint32_t *length)
{
    uint32_t segNo;
    uint64_t segTime;

    if(false==_segments.convertLinearTimeToSeg(start,&segNo,&segTime))
        return false;
    _SEGMENT *seg=_segments.getSegment(segNo);
    _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
    vidHeader *demuxer=vid->_aviheader;
    aviInfo info;
    demuxer->getVideoInfo(&info);
    if(!isH264Compatible(info.fcc))
        return false;
    uint8_t *extra;
    uint32_t extraLen=0;
    demuxer->getExtraHeaderData(&extraLen,&extra);
    if(!extraLen) // AnnexB
        return false;
    uint32_t nalSize=ADM_getNalSizeH264(extra,extraLen);
    ADMCompressedImage img;
    uint8_t *space=new uint8_t[MAX_FRAME_LENGTH];
    img.data=space;
    if(!demuxer->getFrame(0,&img))
        return false;

    bool r=extractH264SEI(img.data,img.dataLength,nalSize,buffer,max,length);

    delete [] space;
    space=NULL;
    return r;
}
// EOF
