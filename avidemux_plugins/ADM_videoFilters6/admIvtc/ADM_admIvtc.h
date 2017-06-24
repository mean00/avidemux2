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

enum ivtcMatch
{
    IVTC_NO_MATCH=0,
    IVTC_LEFT_MATCH=1,
    IVTC_RIGHT_MATCH=2
};
enum ivtcState
{
    IVTC_SYNCING,
    IVTC_PROCESSING
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
                
                int             incomingNum; // Incoming frame number
                int             currentNum;  // outgoing frame number
                int             phaseStart;  // Frame Number of 1st frame of cycle
                uint64_t        phaseStartPts; // its PTS
                int             dupeOffset;  // offset of the duplicate to drop,  i.e. abs number = phaseStart+dupeOffset
                uint32_t        delta[PERIOD+1]; // List of image difference
                unsigned int    hints[PERIOD+1]; // From D. Graft ivtc if used
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
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool               getCoupledConf(CONFcouple **couples) ;     /// Return the current filter configuration
        virtual void               setCoupledConf(CONFcouple *couples);
        virtual bool               configure(void);                           /// Start graphical user interface
                uint32_t           lumaDiff(bool field,ADMImage *src1,ADMImage *src2,uint32_t noise);
                
protected:                
                ivtcState       state;
                ivtcMatch       mode;
                int             offsetInSequence;
                int             startSequence;
};