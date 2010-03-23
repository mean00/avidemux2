/**
        \file  ADM_filterThread.h
        \brief Queue buffered filter. A dedicated thread is filling the queue. To be put just before encoder
        \author mean, fixounet@free.fr
*/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef ADM_FILTER_THREAD_H
#define ADM_FILTER_THREAD_H
#include "ADM_coreVideoFilter.h"
#include "ADM_threadQueue.h"
#define ADM_THREAD_QUEUE_SIZE 8
/**
 *  \class ADM_videoFilterQueue
 *  \brief 
 */
class ADM_videoFilterQueue : public ADM_coreVideoFilter,public ADM_threadQueue
{
protected:
           
public:
                            ADM_videoFilterQueue(ADM_coreVideoFilter *son,CONFcouple *conf=NULL);
       virtual              ~ADM_videoFilterQueue();

       virtual const char   *getConfiguration(void) {return "NONE";}                    
       virtual bool         getCoupledConf(CONFcouple **couples) {*couples=NULL;return true;} ;   /// Return the current filter configuration
       virtual bool         configure(void) {return true;}             /// Start graphical user interface
/**/
       virtual bool         goToTime(uint64_t usSeek); 
       virtual bool         getNextFrame(uint32_t *frameNumber,ADMImage *image);     
       virtual FilterInfo  *getInfo(void)    ;

protected:
        virtual bool                runAction(void); 
};

#endif
