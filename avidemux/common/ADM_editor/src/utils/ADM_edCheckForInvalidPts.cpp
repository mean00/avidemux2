/***************************************************************************
                          ADM_edit.cpp  -  description
                             -------------------
    begin                : Thu Feb 28 2002
    copyright            : (C) 2002/2008 by mean
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
#include <algorithm> 
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "A_functions.h"
#include "GUI_ui.h"

#include "fourcc.h"
#include "ADM_edit.hxx"
#include "ADM_edPtsDts.h"
#include "DIA_coreToolkit.h"
#include "prefs.h"

#include "ADM_coreDemuxer.h"
#include "ADM_vidMisc.h"

/**
    \fn ADM_Composer

*/
bool ADM_Composer::checkForValidPts (_SEGMENT *seg)
{       
    int checkRange=100;
    _VIDEOS *vid=_segments.getRefVideo(seg->_reference);
    vidHeader *hdr=vid->_aviheader;
    int totalFrames=hdr->getVideoStreamHeader()->dwLength;
    
    if(checkRange>totalFrames) checkRange=totalFrames;
    uint64_t from=seg->_startTimeUs;
    if(!seg->_refStartTimeUs && vid->firstFramePts)
        from+=vid->firstFramePts;
    if(!goToTimeVideo(from))
    {
        ADM_warning("Cannot navigate, cannot check file for broken PTS.\n");
        return false;
    }
    vid->lastSentFrame=0;
    vid->dontTrustBFramePts=false;
    uint64_t inc=vid->timeIncrementInUs;

    stats.reset();

    bool bFramesPresent=false;
    for(int i=0;i<totalFrames;i++)
    {
        uint32_t flags;
        if(!hdr->getFlags(i,&flags))
            break;
        if(flags & AVI_B_FRAME)
        {
            bFramesPresent=true;
            break;
        }
    }

    /* If the demuxer knows that B-frames are present but cannot provide
    valid pts, we may proceed with pts reconstruction right away. */
    if(!bFramesPresent || hdr->providePts())
    {
        ADM_info("Checking file for broken PTS...\n");
        ADM_info("Checking %d frames out of %d.\n",checkRange,totalFrames);
        DIA_workingBase *working=createWorking(QT_TRANSLATE_NOOP("ADM_Composer","Checking if timestamps are valid.."));
        for(int i=0;i<checkRange;i++)
        {
            DecodeNextPicture(seg->_reference);
            working->update(i,checkRange);
        }
        goToTimeVideo(from);
        delete working;
        ADM_info("-------- Stats :----------\n");
#define INFO(x) ADM_info(#x":%d\n",stats.x);
        INFO(  nbBFrames)
        INFO(  nbPFrames)
        INFO(  nbIFrames)
        INFO(  nbNoImage)
        INFO(  nbPtsgoingBack)
        ADM_info("-------- /Stats ----------\n");

        if(stats.nbBFrames)
            bFramesPresent=true;
    }

    if(!bFramesPresent && hdr->providePts()==false)
    {
        ADM_info("No B-frames and no PTS, setting PTS equal DTS\n");
        return setPtsEqualDts(hdr,inc);
    }
    // check whether DTS are completely missing, ignore the first frame
    bool noDts=true;
    for(int i=1;i<totalFrames;i++)
    {
        uint64_t pts,dts;
        hdr->getPtsDts(i,&pts,&dts);
        if(dts!=ADM_NO_PTS)
        {
            noDts=false;
            break;
        }
    }
    if(!stats.nbBFrames && !stats.nbPtsgoingBack && hdr->providePts() && noDts)
    {
        ADM_info("No B-frames and no DTS, setting DTS equal PTS\n");
        return setPtsEqualDts(hdr,inc);
    }

    if(stats.nbPtsgoingBack>1 || (bFramesPresent && hdr->providePts()==false))
    {
#ifdef WORK_AROUND_BAD_PTS
        if(!GUI_Question(QT_TRANSLATE_NOOP("ADM_Composer",
                "This video contains B-frames, but presentation time stamps (PTS) are either missing or monotonically increasing. "
                "Avidemux can try to reconstruct correct PTS by decoding the entire video. "
                "This may take a lot of time. Proceed?")))
        {
            vid->dontTrustBFramePts=true;
        }else
        {
            std::vector<uint64_t> ListOfDts; // future PTS values
            std::vector<uint64_t> ListOfPts; // identify the frame for a PTS from above
            goToTimeVideo(from);
            vid->lastSentFrame=0;
            uint64_t bfdelay=2*inc; // FIXME B-frame delay
            for(int i=0;i<totalFrames;i++)
            {
                uint64_t pts,dts;
                hdr->getPtsDts(i,&pts,&dts);
                if(dts==ADM_NO_PTS)
                {
                    if(!i)
                        dts=0;
                    else
                    {
                        hdr->getPtsDts(i-1,&pts,&dts);
                        dts+=inc;
                    }
                }
                pts=dts;
                hdr->setPtsDts(i,pts,dts);
            }

            DIA_workingBase *decoding=createWorking(QT_TRANSLATE_NOOP("ADM_Composer","Decoding video..."));
            for(int i=0;i<totalFrames;i++)
            {
                if(false==decoding->isAlive())
                {
                    vid->dontTrustBFramePts=true;
                    goToTimeVideo(from);
                    delete decoding;
                    return true;
                }
                decoding->update(i,totalFrames);

                EditorCache *cache;
                uint64_t pts,dts;
                hdr->getPtsDts(i,&pts,&dts);
                ListOfDts.push_back(dts);
                //printf("adding %" PRIu64" ms to the list of DTS\n",dts/1000);

                cache=vid->_videoCache;
                ADMCompressedImage img;

                img.data=compBuffer;
                img.cleanup(vid->lastSentFrame);

                uint32_t frame=vid->lastSentFrame;
                vid->lastSentFrame++;
                if(!hdr->getFrame(frame,&img))
                {
                    ADM_warning("getFrame failed for frame %" PRIu32"\n",frame);
                    break;
                }

                ADMImage *pic;
                pic=cache->getFreeImage(seg->_reference);
                if(!decompressImage(pic,&img,seg->_reference))
                {
                    ADM_info("Decoding error for frame %" PRIu32"\n",frame);
                }
                if(dts!=ADM_NO_PTS && dts>vid->lastDecodedPts)
                    vid->lastDecodedPts=dts;
                pts=pic->Pts; // used only to match the decoded frame to its source
                if(pts==ADM_NO_PTS)
                    pts=vid->lastDecodedPts;
                cache->invalidate(pic);
                //printf("adding PTS %" PRIu64" ms to the list of reordered PTS\n",pts/1000);
                ListOfPts.push_back(pts);
            }
            delete decoding;

            uint32_t size=ListOfPts.size();
            if(ListOfDts.size()<size)
                size=ListOfDts.size();
            for(uint32_t i=0; i<size; i++)
            { // look up a nearby frame with matching DTS
                uint64_t pts,dts;
                int match=-1;
                uint32_t lower=0;
                if(i>16)
                    lower=i-16;
                uint32_t upper=size;
                if(size-i>16)
                    upper=i+16;
                for(uint32_t j=lower;j<upper;j++)
                {
                    hdr->getPtsDts(j,&pts,&dts);
                    if(dts==ADM_NO_PTS)
                    {
                        //printf("skipping frame %d\n",(int)j);
                        continue;
                    }
                    if(dts==ListOfPts.at(i))
                    {
                        match=j; // found the frame
                        //printf("frame %d matches %d, dts=%" PRIu64" ms\n",(int)i,match,dts/1000);
                        break;
                    }
                }
                if(match==-1)
                {
                    ADM_warning("No match for PTS %" PRIu64" ms\n",ListOfPts.at(i)/1000);
                    continue;
                }
                hdr->getPtsDts(match,&pts,&dts);
                pts=ListOfDts.at(i);
                hdr->setPtsDts(match,pts,dts);
            }

            for(int i=0;i<totalFrames;i++)
            { // add B-frame delay
                uint64_t pts,dts;
                hdr->getPtsDts(i,&pts,&dts);
                if(pts!=ADM_NO_PTS)
                    pts+=bfdelay;
                hdr->setPtsDts(i,pts,dts);
            }

            _segments.updateRefVideo();

            int nbAudTracks=getNumberOfActiveAudioTracks();
            for(int i=0; i<nbAudTracks; i++)
            { // add the corresponding audio delay
                bool enabled;
                int ms=0;
                getAudioShift(i,&enabled,&ms);
                ms+=(int)(bfdelay/1000);
                if(i==0)
                    UI_setTimeShift(true,ms);
                ADM_info("Adding %d ms to audio delay for track %d, delay=%d ms\n",(int)(bfdelay/1000),i,ms);
                setAudioShift(i,true,ms);
            }

            rewind();
            return true;
        }
#else
        if(!GUI_Question(QT_TRANSLATE_NOOP("ADM_Composer","Some timing information are incorrect.\nIt happens with some capture software.\n"
                "If you re encode video we should drop these informations,\n else it will cause dropped frame/jerky video.\n"
                "If you just copy the video without reencoding,\n you should keep them.\n"
                "Drop timing informations ?")))
            return true;
        // lookup min delta between pts & dts => b frame
        uint64_t delta;
        uint64_t minDelta=1000*1000*10;
        for(int i=0;i<totalFrames;i++)
        {
            uint64_t pts,dts;
            vid->_aviheader->getPtsDts(i,&pts,&dts);
            if(pts!=ADM_NO_PTS && dts!=ADM_NO_PTS)
            {
                delta=pts-dts;
                if(delta<minDelta) minDelta=delta;
            }
        }
        ADM_info("Found %d to be the min value for PTS/DTS delta\n",(int)minDelta);
        minDelta+=(minDelta>>2);
        int processed=0;
        for(int i=1;i<totalFrames;i++) // cannot touch first frame
        {
            uint64_t pts,dts;
            vid->_aviheader->getPtsDts(i,&pts,&dts);
            if(pts!=ADM_NO_PTS && dts!=ADM_NO_PTS)
            {
                delta=pts-dts;
                if(delta<=minDelta)
                {
                        vid->_aviheader->setPtsDts(i,ADM_NO_PTS,dts);
                        processed++;
                }

            }
        }
        ADM_info("Cancelled %d pts as unreliable \n",processed);
#endif
    }
    goToTimeVideo(from);
    return true;
}
/**
 * \fn checkTiming
 * @param list
 * @param limit
 * @return 
 */
static bool checkTiming(std::vector<uint64_t> &list, uint64_t limit)
{
    int n=list.size();
    int good=0,bad=0;
    for(int i=0;i<n-1;i++)
    {
        if((list[i+1]-list[i])<limit) bad++;
        else good ++;
    }
    ADM_info("\tGood : %d\n",good);
    ADM_info("\tBad  : %d\n",bad);
    if(!bad) return true;
    return false;
}

/**
    \fn checkForDoubledFps
    \brief Checks if the DTS increases by half the fps

*/
bool ADM_Composer::checkForDoubledFps(vidHeader *hdr,uint64_t timeIncrementUs)
{       
    int totalFrames=hdr->getVideoStreamHeader()->dwLength;
    uint64_t dtsCeil= (timeIncrementUs*18)/10;
    std::vector<uint64_t> dtsList,ptsList;
    uint64_t lastPts=ADM_NO_PTS;
    uint64_t lastDts=ADM_NO_PTS;
    uint64_t maxPts=0;
    uint64_t maxDts=0;
    bool oooPts,oooDts,skipPts,skipDts;
    oooPts=oooDts=skipPts=skipDts=false;
    ADM_info("Checking for doubled FPS.., time increment ceiling = %d\n",(int)dtsCeil);
    for(int i=0;i<totalFrames;i++)
    {
        uint64_t pts,dts;
        hdr->getPtsDts(i,&pts,&dts);
        if(dts!=ADM_NO_PTS && lastDts!=ADM_NO_PTS)
        { /* We've got two consecutive frames with valid DTS. We need to add both,
            the previous one now and the current one will be added to the list in
            the next iteration. */
            skipDts=false;
            if(!oooDts)
            {
                if(lastDts>maxDts) maxDts=lastDts;
                if(maxDts>dts) oooDts=true;
            }
        }
        if(!skipDts && lastDts!=ADM_NO_PTS)
        {
            dtsList.push_back(lastDts);
            if(i==totalFrames-1 && dts!=ADM_NO_PTS)
                dtsList.push_back(dts); // add the last one
        }else
        { /* Stop adding further DTS until we have encountered two consecutive
            frames with valid DTS again. */
            skipDts=true;
        }
        lastDts=dts;

        if(pts!=ADM_NO_PTS && lastPts!=ADM_NO_PTS)
        { /* We've got two consecutive frames with valid PTS. We need to add both,
            the previous one now and the current one will be added to the list in
            the next iteration. */
            skipPts=false;
            if(!oooPts)
            {
                if(lastPts>maxPts) maxPts=lastPts;
                if(maxPts>pts) oooPts=true;
            }
        }
        if(!skipPts && lastPts!=ADM_NO_PTS)
        {
            ptsList.push_back(lastPts);
            if(i==totalFrames-1 && pts!=ADM_NO_PTS)
                ptsList.push_back(pts); // add the last one
        }else
        { /* Stop adding further PTS until we have encountered two consecutive
            frames with valid PTS again. */
            skipPts=true;
        }
        lastPts=pts;
    }
    if(dtsList.size()<2)
    {
        ADM_info("No consecutive frames with valid DTS found, can't safely halve FPS\n");
        return false;
    }
    if(ptsList.size()<2)
    {
        ADM_info("No consecutive frames with valid PTS found, can't safely halve FPS\n");
        return false;
    }
    if(oooDts)
        std::sort (dtsList.begin(), dtsList.end());
    if(oooPts)
        std::sort (ptsList.begin(), ptsList.end());
    ADM_info("Checking %sDTS...\n",oooDts? "sorted " : "");
    bool okDts=checkTiming(dtsList,dtsCeil);
    ADM_info("Checking %sPTS...\n",oooPts? "sorted " : "");
    bool okPts=checkTiming(ptsList,dtsCeil);

    if(okDts && okPts)
    {
        ADM_info("We can safely halve fps\n");
    }else
    {
        ADM_info("Cannot halve fps\n");
    }
    return okDts && okPts;
}
// EOF
