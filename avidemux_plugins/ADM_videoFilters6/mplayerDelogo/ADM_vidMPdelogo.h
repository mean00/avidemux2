/***************************************************************************
    copyright            : Mplayer team/Mean for ADM port
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
/**
 */
class  MPDelogo:public ADM_coreVideoFilter
 {
 protected:
                    delogo          param;

 public:

                            MPDelogo(ADM_coreVideoFilter *in,CONFcouple *couples)   ;
                            MPDelogo(ADM_coreVideoFilter *in,int x,int y)   ;
       virtual              ~MPDelogo();
       virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
       virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
       virtual void         setCoupledConf(CONFcouple *couples);
       virtual bool         configure(void) ;                 /// Start graphical user interface     
       static  bool         doDelogo(ADMImage *img,  int x,  int y, 
                             int w,  int h,int band, int show);

 }     ;

typedef struct MPDELOGO_PARAM
{
        uint32_t xoff;
        uint32_t yoff;
        uint32_t lw;
        uint32_t lh;
        uint32_t band;
        uint32_t show;
}MPDELOGO_PARAM;

