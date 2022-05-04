/***************************************************************************
                    \fn       ADM_vidReverse.h  

    copyright: 2022 szlldm

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
#include "ADM_default.h"
#include "ADM_image.h"
#include "ADM_coreVideoFilter.h"


/**
    \class ADMVideoReverse
*/
class ADMVideoReverse : public  ADM_coreVideoFilter
{
  protected:
    ADMImage *           original;
    reverse              param;
    unsigned int         frameCount;
    bool                 overrun;
    ADMImage *           overrunImg;
    bool                 invalidatedBySeek;
    bool                 internalError;
    bool                 seekToStart;
    uint32_t             _fn;
    unsigned int         bufferCount;
    bool                 validFileBuffer;
    typedef struct
    {
        uint64_t         Pts;
        ADM_colorSpace   _colorSpace;
        ADM_colorRange   _range;
    } buffered_frame_t;
    buffered_frame_t *   frameBuffer;
    
    void                 clean();
    void                 cleanFile();
    void                 wrongImage(ADMImage * img, int Y, int U, int V, const char * msg);

  public:
    ADMVideoReverse(ADM_coreVideoFilter *previous,CONFcouple *conf);
    ~ADMVideoReverse();

    virtual const char * getConfiguration(void);                   /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual FilterInfo * getInfo(void);                             /// Return picture parameters after this filter
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;             /// Start graphical user interface
    virtual bool         goToTime(uint64_t time);
};


