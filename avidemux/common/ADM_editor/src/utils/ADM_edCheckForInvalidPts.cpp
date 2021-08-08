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
#include <map>
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
            goToTimeVideo(from);
            vid->lastSentFrame=0;

            if(false == ADM_verifyDts(hdr,inc))
                return false;
            if(false == setPtsEqualDts(hdr,inc))
                return false;

            std::map<uint64_t,uint32_t> timing;
            std::vector<uint64_t> ListOfPts,ListOfDts;

            for(int i=0;i<totalFrames;i++)
            {
                uint64_t pts,dts;
                hdr->getPtsDts(i,&pts,&dts);
                timing.insert({dts,i});
            }

            EditorCache *cache = vid->_videoCache;
            ADM_assert(cache);
            cache->flush();

            bool interlaced = false; // when true and less than ~ 2/3 of frames produce pics => field-encoded
            uint64_t bfdelay = 0;
            std::map<uint64_t,uint32_t>::iterator it = timing.begin();

            DIA_workingBase *decoding=createWorking(QT_TRANSLATE_NOOP("ADM_Composer","Decoding video..."));

            while(!vid->decoder->endOfStreamReached())
            {
                if(false==decoding->isAlive())
                {
                    vid->dontTrustBFramePts=true;
                    goToTimeVideo(from);
                    delete decoding;
                    return true;
                }
                decoding->update(vid->lastSentFrame,totalFrames);

                ADMCompressedImage img;
                img.data = compBuffer;
                img.cleanup(vid->lastSentFrame);
                img.dataLength = 0;

                if(!vid->decoder->getDrainingState())
                {
                    if(!hdr->getFrame(vid->lastSentFrame,&img))
                    {
                        ADM_warning("getFrame failed for frame %" PRIu32"\n",vid->lastSentFrame);
                        img.dataLength = 0;
                        vid->decoder->setDrainingState(true);
                    }else
                    {
                        vid->lastSentFrame++;
                    }
                }

                ADMImage *pic = cache->getFreeImage(seg->_reference);
                if(!pic)
                {
                    ADM_warning("Cannot find free image in cache, aborting PTS reconstruction.\n");
                    rewind();
                    delete decoding;
                    return false;
                }
                if(false == decompressImage(pic,&img,seg->_reference))
                {
                    if(vid->decoder->keepFeeding())
                    {
                        //printf("[checkForValidPts] No pic for frame %u yet.\n",vid->lastSentFrame);
                        continue;
                    }
                    if(vid->decoder->endOfStreamReached())
                    {
                        //printf("[checkForValidPts] End of stream\n");
                        break;
                    }
                    ADM_warning("Decoding error for frame %d\n",vid->lastSentFrame);
                    continue;
                }
                if(pic->Pts == ADM_NO_PTS)
                {
                    //printf("[checkForValidPts] No PTS out of decoder, last frame: %d\n",vid->lastSentFrame);
                    cache->invalidate(pic);
                    continue;
                }
                uint64_t pts = pic->Pts; // used only to match the decoded frame to its source
                if(pic->flags & AVI_FIELD_STRUCTURE)
                    interlaced = true;
                cache->invalidate(pic);

                ListOfPts.push_back(pts);
                ListOfDts.push_back(pts);
            }
            delete decoding;
            decoding = NULL;

            // invalidate pts in the index
            it = timing.begin();
            hdr->setPtsDts(0,0,it->first);
            it++;
            for(int i=1;i<totalFrames;i++)
            {
                if(it == timing.end())
                    break;
                hdr->setPtsDts(i,ADM_NO_PTS,it->first);
                it++;
            }
            // now set pts for all frames which produced output
            std::sort(ListOfDts.begin(),ListOfDts.end());
            int nbValid = ListOfDts.size();
            //printf("[checkForValidPts] We have %d pics out of %d frames.\n",nbValid,totalFrames);
            if(interlaced && nbValid < totalFrames*2/3) // arbitrary threshold
            {
                ADM_info("Video seems to be field-encoded.\n");
                vid->fieldEncoded = true;
                inc *= 2;
            }
            {
            int frame = 0;
            for(std::vector<uint64_t>::iterator t = ListOfPts.begin(); t != ListOfPts.end(); t++)
            {
                if(frame >= nbValid)
                    break;
                it = timing.find(*t);
                if(it == timing.end())
                {
                    ADM_warning("Timestamp %s not found!\n",ADM_us2plain(*t));
                    continue;
                }
                uint64_t pts = ListOfDts.at(frame);
                uint64_t dts = it->first;
                // calculate B-frame delay
                if(pts != ADM_NO_PTS && dts != ADM_NO_PTS && dts > pts && bfdelay < dts-pts)
                    bfdelay = dts-pts;
#if 0
                printf("adding entry %d for frame %u with PTS %s ",frame,it->second,ADM_us2plain(pts));
                printf("DTS %s\n",ADM_us2plain(dts));
#endif
                hdr->setPtsDts(it->second,pts,dts);
                frame++;
            }
            }

            ADM_info("B-frame delay set to %" PRIu64" ms\n",bfdelay/1000);

            uint64_t maxPts = ADM_NO_PTS;
            for(int i=0;i<totalFrames;i++)
            { // add B-frame delay
                uint64_t pts,dts;
                hdr->getPtsDts(i,&pts,&dts);
                if(pts == ADM_NO_PTS)
                    continue;
                pts += bfdelay;
                if(maxPts == ADM_NO_PTS || maxPts < pts)
                    maxPts = pts;
                hdr->setPtsDts(i,pts,dts);
            }
            // update segment duration
            if(maxPts != ADM_NO_PTS)
            {
                maxPts += inc;
                if(maxPts > seg->_durationUs)
                {
                    printf("[checkForValidPts] Increasing segment duration from %s \n",ADM_us2plain(seg->_durationUs));
                    printf("to %s\n",ADM_us2plain(maxPts));
                    seg->_durationUs = maxPts;
                }
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
