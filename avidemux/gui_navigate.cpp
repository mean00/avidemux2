/***************************************************************************
                          gui_navigate.cpp  -  description
                             -------------------

            GUI Part of get next frame, previous key frame, any frame etc...

    
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


#include "avi_vars.h"

#include <math.h>

#include "DIA_fileSel.h"
#include "ADM_assert.h"
#include "prototype.h"
#include "audio_out.h"
#include "ADM_audio/aviaudio.hxx"
#include "ADM_audiofilter/audioprocess.hxx"
#include "gui_action.hxx"
#include "gtkgui.h"
#include "DIA_coreToolkit.h"
#include "ADM_userInterfaces/ADM_render/GUI_render.h"
#include "DIA_working.h"
#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"
#include "ADM_userInterfaces/ADM_commonUI/DIA_busy.h"
#include "ADM_userInterfaces/ADM_commonUI/GUI_ui.h"

#include "ADM_video/ADM_vidMisc.h"
#include "ADM_preview.h"

extern void    UI_purge(void );

/**
    \fn GUI_NextFrame
    \brief next frame
*/
void GUI_NextFrame(uint32_t frameCount)
{
//    uint8_t *ptr;
uint32_t flags;
    if (playing)	return;
    if (!avifileinfo) return;

      admPreview::nextPicture( ) ;
      GUI_setCurrentFrameAndTime();
      UI_purge();
}


/**
    \fn GUI_NextKeyFrame
    \brief Go to the next keyframe
    
*/
void GUI_NextKeyFrame(void)
{
 
    if (playing)	return;
    if (!avifileinfo) return;

    if(!admPreview::nextKeyFrame())
    {
        GUI_Error_HIG(QT_TR_NOOP("Error"),QT_TR_NOOP("Cannot go to next keyframe"));
        return;
    }
    GUI_setCurrentFrameAndTime();
    UI_purge();
}

/**
    \fn GUI_GoToKFrame
    \brief Go to the nearest previous keyframe
*/
void GUI_GoToKFrame(uint32_t frame)
{

    if (playing)            return;
    if(!avifileinfo) return;
    uint32_t f;
    video_body->getFlags(0,&f);
    if(!frame && (f&AVI_KEY_FRAME))
    {

    }else
    {
        if(!video_body->getPKFrame(&frame))
        {
            printf("[GUI_GoToKFrame] failed for frame %u\n",frame);
            return;
        }
    }
    admPreview::seekToIntra(frame);
    admPreview::samePicture();
    GUI_setCurrentFrameAndTime();
    UI_purge();
}
/**
    \fn GUI_GoToFrame
    \brief go to a given frame. Half broken, do not use.
*/
int GUI_GoToFrame(uint32_t frame)
{
uint32_t flags;

      if (playing)              return 0;
      if (!avifileinfo)         return 0;
      if(frame>=avifileinfo->nb_frames) return 0;
    
     if(! video_body->setCurrentFrame(frame)) return 0;
     return admPreview::samePicture();
    
}

/**
    \fn GUI_PreviousKeyFrame
    \brief Go to previous keyframe
*/

void GUI_PreviousKeyFrame(void)
{

 uint32_t f;
 uint32_t flags;


    if (playing)		return;
    if (!avifileinfo)   return;


    if(!admPreview::previousKeyFrame())
    {
        GUI_Error_HIG(QT_TR_NOOP("Error"),QT_TR_NOOP("Cannot go to next keyframe"));
        return;
    }
    GUI_setCurrentFrameAndTime();
    UI_purge();

};

uint8_t A_rebuildKeyFrame(void)
{

        return video_body->rebuildFrameType();
}
/**
    \fn GUI_PrevFrame
    \brief Go to current frame -1
*/
void        GUI_PrevFrame(uint32_t frameCount)
{
      return; 
}
/**
      \fn A_jogRead
      \brief read an average value of jog
*/
#define NB_JOG_READ           3
#define JOG_READ_PERIOD_US    5*1000 // 5ms
#define JOG_THRESH1           40
#define JOG_THRESH2           80
#define JOG_THRESH1_PERIOD    100*1000 //us
#define JOG_THRESH2_PERIOD    40*1000
#define JOG_THRESH3_PERIOD    500
/**
    \fn A_jogRead
    \brief Read an average value of jog
*/
uint32_t A_jogRead(void)
{
  int32_t sum=0,v;
    for(int i=0;i<NB_JOG_READ;i++)
    {
        v=UI_readJog();
        if(abs(v)<10) v=0;
        sum+=v;
        ADM_usleep(JOG_READ_PERIOD_US);
    } 
    return sum/NB_JOG_READ;
}
#define REFRESH 10000
/**
      \fn     A_jog
      \brief  Handle jogshuttle widget
*/
void A_jog(void)
{
  int32_t r;
  uint32_t a;
  uint32_t slip;
  static int jog=0;
  if(jog) return;
  jog++;
  while(r=A_jogRead())
  {
      a=abs(r);
      printf("%d \n",r);
      if(a<JOG_THRESH1) slip=JOG_THRESH1_PERIOD;
        else if(a<JOG_THRESH2) slip=JOG_THRESH2_PERIOD;
          else slip=JOG_THRESH3_PERIOD;
    
      if(r>0) GUI_NextKeyFrame();
      else GUI_PreviousKeyFrame();       
      UI_purge();
      for(int i=0;i<slip/REFRESH;i++)
      {
        UI_purge();
        ADM_usleep(REFRESH);
        UI_purge();
      }
  }
  jog--;
}
/**
    \fn     GUI_setAllFrameAndTime
    \brief  Update all  informations : current frame # and current time, total frame ...

*/
void  GUI_setAllFrameAndTime(void)
{
    char text[80];
    double len;
//    int val;

    //    if(!guiReady) return ;
    text[0] = 0;
 
	UI_updateFrameCount(video_body->getCurrentFrame());
    UI_setCurrentTime(admPreview::getCurrentPts());
	UI_setTotalTime(video_body->getVideoDuration());
    
    // progress bar
    len = 100;
    if(avifileinfo->nb_frames>1)
    	len=len / (double) (avifileinfo->nb_frames-1);
    len *= (double) video_body->getCurrentFrame();

   

     UI_setScale(len);
   	
}

/**
    \fn GUI_setCurrentFrameAndTime
    \brief Update some informations : current frame # and current time
*/
void GUI_setCurrentFrameAndTime(void)
{
    char text[80];
    double len;
//    int val;

    //    if(!guiReady) return ;
    text[0] = 0;
 
	UI_setFrameCount( video_body->getCurrentFrame(), avifileinfo->nb_frames);
	UI_setCurrentTime(admPreview::getCurrentPts());
	
    // progress bar
    len = 100;
    if(avifileinfo->nb_frames>1)
    	len=len / (double) (avifileinfo->nb_frames-1);
    len *= (double) video_body->getCurrentFrame();



     UI_setScale(len);
}
//EOF
