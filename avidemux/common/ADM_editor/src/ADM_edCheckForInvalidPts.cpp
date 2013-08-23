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
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "A_functions.h"
#include "ADM_audioFilterInterface.h"
#include "audioEncoderApi.h"
#include "ADM_muxerProto.h"
#include "GUI_ui.h"
#include "ADM_coreVideoFilterFunc.h"

#include "fourcc.h"
#include "ADM_edit.hxx"
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
    int totalFrames=vid->_aviheader->getVideoStreamHeader()->dwLength;
    
    if(checkRange>totalFrames) checkRange=totalFrames;
    goToTimeVideo(seg->_startTimeUs);
    
    stats.reset();
    ADM_info("Checking file for broken PTS...\n");
    ADM_info("Checking %d frames out of %d.\n",checkRange,totalFrames);
    DIA_workingBase *working=createWorking("Checking if timestamps are valid..");
    for(int i=0;i<checkRange;i++)
    {
        DecodeNextPicture(seg->_reference);
        working->update(i,checkRange);
    }
    goToTimeVideo(seg->_startTimeUs);
    delete working;
    ADM_info("-------- Stats :----------\n");
#define INFO(x) ADM_info(#x":%d\n",stats.x);
    INFO(  nbBFrames);
    INFO(  nbPFrames);
    INFO(  nbIFrames);
    INFO(  nbNoImage);
    INFO(  nbPtsgoingBack);
    ADM_info("-------- /Stats ----------\n");
    if(stats.nbPtsgoingBack>1)
    {
        if(!GUI_Question(QT_TR_NOOP("Some timing information are incorrect.\nIt happens with some capture software.\n"
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

    }
    goToTimeVideo(seg->_startTimeUs);
    return true;
}
/**
    \fn checkForDoubledFps
    \brief Checks if the DTS increases by half the fps

*/
bool ADM_Composer::checkForDoubledFps(vidHeader *hdr,uint64_t timeIncrementUs)
{       
    int totalFrames=hdr->getVideoStreamHeader()->dwLength;
    int good=0,bad=0,skipped=0;
    uint64_t dtsCeil= (timeIncrementUs*18)/10;
    ADM_info("Checking for doubled FPS.., time increment ceiling = %d\n",(int)dtsCeil);
    for(int i=0;i<totalFrames-1;i++)
    {
          uint64_t pts,dts;
          uint64_t dts2;
          hdr->getPtsDts(i,&pts,&dts);
          hdr->getPtsDts(i+1,&pts,&dts2);
          if(dts==ADM_NO_PTS || dts2==ADM_NO_PTS)
          {
              skipped++;
              continue;
          }
          if((dts2-dts)< dtsCeil)
              bad++;
          else
              good++;
    }
    ADM_info("Out of %d frames, we have :\n",totalFrames);
    ADM_info("Bad     : %d\n",bad);
    ADM_info("Good    : %d\n",good);
    ADM_info("Skipped : %d\n",skipped);
    int total=good+bad+skipped;
    if(!total) return false;
    if(bad==0 && good*100>40*total)
    {
        ADM_info("  Looks like doubled fps\n");
        return true;
              
    }
    return false;
}
// EOF
