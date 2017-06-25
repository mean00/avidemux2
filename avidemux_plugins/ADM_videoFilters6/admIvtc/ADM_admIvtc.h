/***************************************************************************

 Custom IVTC

    copyright            : (C) 2017 by mean
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
#pragma once


#define PERIOD 4

#define MARK_PROGRESSIVE 'PRGS'
#define MARK_DUPLICATE   'DUPE'


enum ivtcMatch
{
    IVTC_NO_MATCH=0,
    IVTC_TOP_MATCH=1,
    IVTC_BOTTOM_MATCH=2
};
enum ivtcState
{
    IVTC_SYNCING,
    IVTC_PROCESSING,
    IVTC_RESYNCING,
    IVTC_SKIPPING
};
/**
    \class admIvtc
*/
class admIvtc : public  ADM_coreVideoFilterCached
{
public:              
                enum searchMode
                {
                    full=0,
                    fast=1,
                    veryFast=2
                };
                                
                                
protected:
                dupeRemover        configuration;
                ivtcMatch          searchSync(int &offset);
                bool               postProcess(ADMImage *in,ADMImage *out,uint64_t pts);
                ivtcMatch          computeMatch(ADMImage *left,ADMImage *right, int threshold);
public:
                                   admIvtc(ADM_coreVideoFilter *previous,CONFcouple *conf);
                                   ~admIvtc();
                bool               goToTime(uint64_t usSeek);
        virtual const char         *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool               getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image	 
        virtual bool               getCoupledConf(CONFcouple **couples) ;     /// Return the current filter configuration
        virtual void               setCoupledConf(CONFcouple *couples);
        virtual bool               configure(void);                           /// Start graphical user interface
                uint32_t           lumaDiff(bool bottom,ADMImage *src1,ADMImage *src2,uint32_t noise);
                
protected:                
                ivtcState       state; // synced or searching
                ivtcMatch       mode; // top or bottom
                int             offsetInSequence; // Current frame in the IVTC pattern AA / AB / BC / CC / DD
                int             startSequence;  // frame number of AA
                ivtcMatch       matches[PERIOD*2]; //       
                int             skipCount;
protected:
                bool            getNextImageInSequence(uint32_t *fn,ADMImage *image);
                bool            trySimpleFieldMatching();
                bool            tryInterlacingDetection(ADMImage **img);
                bool            verifySamePattern(ADMImage **images, ivtcMatch candidate);
                bool            displayStatus(ADMImage *image,const char *st);
                ADMImage        *spare[2];
};