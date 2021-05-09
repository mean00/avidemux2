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
    uint32_t            frameNb, nbStillImages;
    ADMImage            *stillPicture;

    bool                updateTimingInfo(void);
    void                cleanup(void);

public:
                        stillimage(ADM_coreVideoFilter *in, CONFcouple *conf);
                        ~stillimage();
    virtual const char  *getConfiguration(void); // Return current configuration as a human-readable string
    virtual bool        getNextFrame(uint32_t *fn, ADMImage *image); // Get the next image
    virtual bool        getCoupledConf(CONFcouple **couples); // Get the current filter configuration
    virtual void        setCoupledConf(CONFcouple *couples);
    virtual bool        goToTime(uint64_t usSeek);
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
    frameNb=0;
    nbStillImages=0;
}

/**
    \fn dtor
*/
stillimage::~stillimage()
{
    cleanup();
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
bool stillimage::goToTime(uint64_t usSeek)
{
    cleanup();
    uint64_t time=usSeek;
    if(time >= begin && time <= end)
        time=begin;
    else if(time > end)
        time-=end-begin;
    return previousFilter->goToTime(time);
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
    // we have the image and are within range
    if(stillPicture && stillPicture->Pts < end)
    {
        uint64_t pts=stillPicture->Pts;
        pts+=timeIncrement;
        stillPicture->Pts=pts;
        // output our image instead of requesting a new frame from the previous filter
        image->duplicate(stillPicture);
        frameNb++;
        *fn=frameNb;
        nbStillImages++;
        aprintf("[stillimage] stillPicture PTS = %s, frame %d\n",ADM_us2plain(pts),*fn);
        return true;
    }
    // not in range, get a frame from the previous filter
    if(!previousFilter->getNextFrame(fn,image))
        return false;
    uint64_t pts=image->Pts;
    if(pts==ADM_NO_PTS || pts < begin)
    {
        *fn+=nbStillImages;
        return true;
    }
    aprintf("[stillimage] original image PTS = %" PRIu64" ms\n",pts/1000);
    // now in range, create our image
    if(!stillPicture)
    {
        aprintf("[stillimage] creating stillPicture\n");
        stillPicture=new ADMImageDefault(previousFilter->getInfo()->width, previousFilter->getInfo()->height);
        stillPicture->duplicate(image);
        frameNb=*fn;
        return true;
    }
    // past the end, adjust the PTS and the frame count
    pts+=end-begin;
    image->Pts=pts;
    *fn+=nbStillImages;
    aprintf("[stillimage] final image PTS = %" PRIu64" ms, frame %d\n",pts/1000,*fn);

    return true;
}

/**
    \fn updateTimingInfo
    \brief perform a sanity check and update info with the new totalDuration
*/
bool stillimage::updateTimingInfo(void)
{
    uint64_t old=previousFilter->getInfo()->totalDuration;
    if(1000LL*params.start + timeIncrement > old)
    {
        if(old > timeIncrement)
            params.start=(uint32_t)((old-timeIncrement)/1000);
        else
            params.start=0;
    }

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

    info.totalDuration=old+end-begin;
    if (info.markerA >= begin)
        info.markerA += end-begin;
    if (info.markerB >= begin)
        info.markerB += end-begin;

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
