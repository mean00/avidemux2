/***************************************************************************
    \file stillimage.cpp
    \author eumagga0x2a     @mailbox.org
    \brief Duplicate frame for a given duration
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_vidMisc.h"
#include "ADM_coreVideoFilter.h"
#include "DIA_coreToolkit.h"
#include "DIA_factory.h"

#include "confStillimage.h"
#include "confStillimage_desc.cpp"

#if 1
    #define aprintf(...) {}
#else
    #define aprintf printf
#endif

/**
    \class stillimage
*/
class stillimage : public ADM_coreVideoFilter
{
protected:
    configuration       params;
    uint64_t            from, begin, end;
    uint64_t            timeIncrement;
    uint64_t            freezeDuration, startPts, endPts;
    uint32_t            frameNb, nbStillImages;
    bool                seek;
    bool                capture;
    bool                useTimeBase;
    ADMImage            *stillPicture;

    bool                updateTimingInfo(void);
    bool                checkTimeBase(void);
    void                cleanup(void);

public:
                        stillimage(ADM_coreVideoFilter *in, CONFcouple *conf);
                        ~stillimage();
    virtual const char  *getConfiguration(void); // Return current configuration as a human-readable string
    virtual bool        getNextFrame(uint32_t *fn, ADMImage *image); // Get the next image
    virtual bool        getCoupledConf(CONFcouple **couples); // Get the current filter configuration
    virtual void        setCoupledConf(CONFcouple *couples);
    virtual bool        goToTime(uint64_t usSeek, bool fineSeek = false);
    virtual bool        getTimeRange(uint64_t *startTme, uint64_t *endTme); // Provide an updated time range for the next filter
    virtual bool        configure(void); // Start graphical user interface
};

DECLARE_VIDEO_FILTER(   stillimage,     // Class
                        1,0,0,          // Version
                        ADM_UI_ALL,     // UI
                        VF_TRANSFORM,   // Category
                        "stillimage",   // Internal name (must be unique!)
                        QT_TRANSLATE_NOOP("stillimage", "Still Image"), // Display name
                        QT_TRANSLATE_NOOP("stillimage", "Duplicate frames for a given duration.") // Description
                    );

/**
    \fn getConfiguration
*/
const char *stillimage::getConfiguration(void)
{
    static char buf[256];
    snprintf(buf, 255, "Duplicate frame at %s for %.3f s",
        ADM_us2plain(1000LL*params.start),
        (double)params.duration/1000.);
    return buf;
}
/**
    \fn ctor
*/
stillimage::stillimage( ADM_coreVideoFilter *in, CONFcouple *setup ) : ADM_coreVideoFilter(in, setup)
{
    if(!setup || !ADM_paramLoad(setup, configuration_param, &params))
    {
        // Default values
        params.start=0; // the first frame
        params.duration=10000; // 10 seconds
    }

    from=in->getAbsoluteStartTime();
    timeIncrement=in->getInfo()->frameIncrement;
    updateTimingInfo();
    stillPicture=NULL;

    useTimeBase = checkTimeBase();
    aprintf("[stillimage] Using %s for PTS calculation.\n", useTimeBase ? "time base" : "time increment");
    frameNb=0;
    nbStillImages=0;

    seek = false;
    capture = true;
    startPts = endPts = ADM_NO_PTS;
}

/**
    \fn dtor
*/
stillimage::~stillimage()
{
    cleanup();
}

/**
    \fn checkTimeBase
    \brief Check whether we can avoid accumulation of rounding errors by using a rational
*/
bool stillimage::checkTimeBase(void)
{
    typedef struct {
        uint64_t mn,mx;
        uint32_t n,d;
    } TimeIncrementType;

    TimeIncrementType fpsTable[] = {
        {40000,40000,1000,25000},
        {20000,20000,1000,50000},
        {16661,16671,1000,60000},
        {16678,16688,1001,60000},
        {33360,33371,1001,30000},
        {41700,41710,1001,24000},
        {33328,33338,1000,30000},
        {41660,41670,1000,24000}
    };

    uint32_t i,nb,tbn,tbd = 0;

    nb = sizeof(fpsTable) / sizeof(TimeIncrementType);

    for(i=0; i < nb; i++)
    {
        TimeIncrementType *t=fpsTable+i;
        if(timeIncrement >= t->mn && timeIncrement <= t->mx)
        {
            tbn = t->n;
            tbd = t->d;
            break;
        }
    }
    if(!tbd) return false;
    if(tbn == info.timeBaseNum && tbd == info.timeBaseDen)
        return true;

    uint32_t nmult = 0;

    if(tbn > info.timeBaseNum && !(tbn % info.timeBaseNum))
        nmult = tbn / info.timeBaseNum;
    else if(info.timeBaseNum > tbn && !(info.timeBaseNum % tbn))
        nmult = info.timeBaseNum / tbn;
    if(!nmult) return false;

    uint32_t dmult = 0;

    if(tbd > info.timeBaseDen && !(tbd % info.timeBaseDen))
        dmult = tbd / info.timeBaseDen;
    else if(info.timeBaseDen > tbd && !(info.timeBaseDen % tbd))
        dmult = info.timeBaseDen / tbd;
    if(!dmult) return false;

    return nmult == dmult;
}

/**
    \fn cleanup
*/
void stillimage::cleanup(void)
{
    if(stillPicture)
        delete stillPicture;
    stillPicture=NULL;
}

/**
    \fn getCoupledConf
*/
bool stillimage::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, configuration_param, &params);
}

/**
    \fn setCoupledConf
*/
void stillimage::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, configuration_param, &params);
}

/**
    \fn goToTime
*/
bool stillimage::goToTime(uint64_t usSeek, bool fineSeek)
{
    cleanup();
    uint64_t time=usSeek;
    if(time >= begin && time <= end)
        time=begin;
    else if(time > end)
        time-=end-begin;
    if(previousFilter->goToTime(time,fineSeek))
    {
        seek = true;
        capture = true;
        return true;
    }
    return false;
}

/**
    \fn getTimeRange
*/
bool stillimage::getTimeRange(uint64_t *startTme, uint64_t *endTme)
{
    *startTme=0;
    *endTme=info.totalDuration;
    return true;
}

/**
    \fn getNextFrame
*/
bool stillimage::getNextFrame(uint32_t *fn, ADMImage *image)
{
    uint64_t pts = ADM_NO_PTS;
    // we have the image and are within range
    if(stillPicture && stillPicture->Pts < end)
    {
        pts = stillPicture->Pts;
        // increment PTS
        if(useTimeBase)
        {
            double d = info.timeBaseNum;
            d *= nbStillImages + 1;
            d *= 1000000;
            d /= info.timeBaseDen;
            pts = startPts + (uint64_t)(d + 0.49);
        }else
        {
            pts += timeIncrement;
        }

        stillPicture->Pts = pts;

        if(pts > end)
        {
            aprintf("[stillimage] Leaving the range at %s after %u frames.\n", ADM_us2plain(pts), nbStillImages);
            aprintf("[stillimage] Updating freeze duration from %s ", ADM_us2plain(freezeDuration));
            freezeDuration = endPts - startPts;
            aprintf("to %s\n", ADM_us2plain(freezeDuration));
        }else
        {
            // output our image instead of requesting a new frame from the previous filter
            image->duplicate(stillPicture);
            frameNb++;
            *fn = frameNb;
            aprintf("[stillimage] Outputting stillPicture with PTS = %s, frame %u\n", ADM_us2plain(stillPicture->Pts), frameNb);
            nbStillImages++;
            endPts = pts;
            // update effect duration to keep frame duration of the last still picture the same
            seek = false;
            return true;
        }
    }
    // not in range or after a seek, get a frame from the previous filter
    if(!previousFilter->getNextFrame(fn,image))
        return false;

    pts = image->Pts;
    aprintf("[stillimage] original image PTS = %s\n", ADM_us2plain(pts));

    if(pts == ADM_NO_PTS || pts < begin)
    {
        seek = false;
        return true;
    }
    if(seek && (pts > begin + 999))
    {
        aprintf("[stillimage] After seek, skipping image capture\n");
        capture = false;
    }
    if(!stillPicture && capture)
    {
        aprintf("[stillimage] creating stillPicture\n");
        stillPicture=new ADMImageDefault(previousFilter->getInfo()->width, previousFilter->getInfo()->height);
        stillPicture->duplicate(image);
        frameNb=*fn;
        nbStillImages = 0;
        startPts = stillPicture->Pts;
        aprintf("[stillimage] Our image PTS = %s, frame %u\n", ADM_us2plain(image->Pts), frameNb);
        seek = false;

        return true;
    }
    // past the end, adjust the PTS and the frame count
    pts += freezeDuration;
    image->Pts=pts;
    *fn+=nbStillImages;
    aprintf("[stillimage] final image PTS = %s, frame %u\n", ADM_us2plain(pts), *fn);
    seek = false;

    return true;
}

/**
    \fn updateTimingInfo
    \brief Update total duration and markers.
*/
bool stillimage::updateTimingInfo(void)
{
    uint64_t old=previousFilter->getInfo()->totalDuration;
    begin=1000LL*params.start;
    end=begin+1000LL*params.duration;
    if(from < begin)
    {
        begin-=from;
        end-=from;
    }else if(from < end)
    {
        begin=0;
        end-=from;
    }else
    {
        begin=end=0;
    }
    freezeDuration = end - begin;
    aprintf("[stillimage::updateTimingInfo] Freeze duration set to %s\n", ADM_us2plain(freezeDuration));

    info.totalDuration = old + freezeDuration;
    info.markerA = previousFilter->getInfo()->markerA;
    info.markerB = previousFilter->getInfo()->markerB;
    if (info.markerA > from + begin)
        info.markerA += freezeDuration;
    if (info.markerB > from + begin)
        info.markerB += freezeDuration;

    return true;
}

/**
    \fn configure
*/
bool stillimage::configure(void)
{
    uint32_t dur=(uint32_t)(previousFilter->getInfo()->totalDuration/1000LL);
    uint32_t max9h=9*3600*1000;

    diaElemTimeStamp start(&params.start,QT_TRANSLATE_NOOP("stillimage","_Start time:"),0,dur);
    diaElemTimeStamp duration(&params.duration,QT_TRANSLATE_NOOP("stillimage","_Duration:"),0,max9h);

    diaElem *elems[2]={&start,&duration};
    bool r=diaFactoryRun(QT_TRANSLATE_NOOP("stillimage","Still Image"),2,elems);
    updateTimingInfo();
    return r;
}

//EOF
