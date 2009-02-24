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

#include "ADM_default.h"
#include "avi_vars.h"

#include <math.h>
#include "prototype.h"
#include "gui_action.hxx"
#include "gtkgui.h"

#include "DIA_coreToolkit.h"
#include "ADM_userInterfaces/ADM_commonUI/DIA_busy.h"
#include "ADM_userInterfaces/ADM_commonUI/GUI_ui.h"
#include "DIA_enter.h"

#include "ADM_video/ADM_vidMisc.h"
#include "ADM_preview.h"

extern void UI_purge(void);
extern uint8_t DIA_gotoTime(uint16_t *hh, uint16_t *mm, uint16_t *ss);
extern bool SliderIsShifted;

uint8_t A_jumpToTime(uint32_t hh,uint32_t mm,uint32_t ss,uint32_t ms);
/**
    \fn HandleAction_Navigate

*/
void HandleAction_Navigate(Action action)
{
static int ignore_change=0;
    switch (action)
      {
      case ACT_Scale:
        if (!ignore_change)
        {
          uint32_t nf;
          ignore_change++;
          nf = GUI_GetScale ();
          if (!SliderIsShifted) GUI_GoToKFrame (nf);
          else GUI_GoToFrame (nf);
          ignore_change--;

        }
        break;
      case ACT_GotoMarkA:
      case ACT_GotoMarkB:
            {
                uint32_t nf;
                  if (action == ACT_GotoMarkA)
                      nf = frameStart;
                  else
                      nf = frameEnd;
                  GUI_GoToFrame(nf);
            }
	  break;
      case ACT_Goto:
	  uint32_t fn;
	  fn = video_body->getCurrentFrame();
	  if (DIA_GetIntegerValue
	      ((int *) &fn, 0, avifileinfo->nb_frames,
	       QT_TR_NOOP("Go to Frame"), QT_TR_NOOP("_Go to frame:")))
	    {
		if (fn < avifileinfo->nb_frames)
		    GUI_GoToFrame(fn);
		else
		    GUI_Error_HIG(QT_TR_NOOP("Out of bounds"), NULL);
	    }
	  break;
      case ACT_Back25Frames:
          if (video_body->getCurrentFrame() >= 25)
          {
              DIA_StartBusy();
              GUI_GoToFrame (video_body->getCurrentFrame() - 25);
              DIA_StopBusy();
          }
	  break;

      case ACT_PreviousKFrame:
        GUI_PreviousKeyFrame();
	  break;
      case ACT_PreviousFrame:
        GUI_PrevFrame();
	  break;
      case ACT_Forward100Frames:
        //GUI_GoToKFrame (curframe + (avifileinfo->fps1000 / 1000 * 4));
	  break;

      case ACT_Back100Frames:
        //GUI_GoToKFrame (curframe - (avifileinfo->fps1000 / 1000 * 4));
	  break;


      case ACT_Forward50Frames:
	  // GUI_GoToFrame (curframe + 50);
	  break;

      case ACT_Forward25Frames:
	  // GUI_GoToFrame (curframe + 25);
	  break;

      case ACT_Back50Frames:
	  // if (curframe >= 50)
	  {
	      DIA_StartBusy();
	      // GUI_GoToFrame (curframe - 50);
	      DIA_StopBusy();
	  }
	  break;
      case ACT_NextFrame:
        GUI_NextFrame();
	  break;
      case ACT_NextKFrame:
        GUI_NextKeyFrame();
	  break;
      case ACT_NextBlackFrame:
        GUI_NextPrevBlackFrame(1);
	  break;
      case ACT_PrevBlackFrame:
        GUI_NextPrevBlackFrame(-1);
	  break;
      case ACT_End:
            {
                uint32_t nf = avifileinfo->nb_frames;
                          GUI_GoToFrame(nf - 1);
            }
	  break;
      case ACT_Begin:
        GUI_GoToKFrame(0);
	  break;
      case ACT_JumpToFrame:
        {
              // read value
              uint32_t nf = UI_readCurFrame();
              if (nf < avifileinfo->nb_frames)
                  GUI_GoToFrame(nf);
              UI_JumpDone();
        }
	  break;
      case ACT_JumpToTime:
	  {
	      uint16_t hh, mm, ss, ms;

	      if (UI_readCurTime(hh, mm, ss, ms))
		  A_jumpToTime(hh, mm, ss, ms);
	  }
	  break;
      case ACT_GotoTime:
	  {
           // Get current time
            uint64_t pts=video_body->estimatePts(video_body->getCurrentFrame());

	      uint16_t mm, hh, ss, ms;
            ms2time((uint32_t)(pts/1000),&hh,&mm,&ss,&ms);
	      if (DIA_gotoTime(&hh, &mm, &ss))
          {
		    A_jumpToTime(hh, mm, ss, 0);
          }
	  }
	  break;
      default:
	  ADM_assert(0);
	  break;
      }
}
/**
    \fn GUI_NextFrame
    \brief next frame
*/
void GUI_NextFrame(uint32_t frameCount)
{
    // uint8_t *ptr;
    uint32_t flags;
    if (playing)
	return;
    if (!avifileinfo)
	return;

    admPreview::nextPicture();
    GUI_setCurrentFrameAndTime();
    UI_purge();
}


/**
    \fn GUI_NextKeyFrame
    \brief Go to the next keyframe
    
*/
void GUI_NextKeyFrame(void)
{

    if (playing)
	return;
    if (!avifileinfo)
	return;

    if (!admPreview::nextKeyFrame())
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

    if (playing)
	return;
    if (!avifileinfo)
	return;
    uint32_t f;
    video_body->getFlags(0, &f);
    if (!frame && (f & AVI_KEY_FRAME))
      {

    } else
      {
	  if (!video_body->getPKFrame(&frame))
	    {
		printf("[GUI_GoToKFrame] failed for frame %u\n", frame);
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

    if (playing)
        return 0;
    if (!avifileinfo)
        return 0;
    if (frame >= avifileinfo->nb_frames)
        return 0;

    if (!video_body->setCurrentFrame(frame))
        return 0;    

    if(!admPreview::samePicture()) return 0;
    GUI_setAllFrameAndTime();
    return 1;
}

/**
    \fn GUI_PreviousKeyFrame
    \brief Go to previous keyframe
*/

void GUI_PreviousKeyFrame(void)
{

    uint32_t f;
    uint32_t flags;


    if (playing)
	return;
    if (!avifileinfo)
	return;


    if (!admPreview::previousKeyFrame())
      {
	  GUI_Error_HIG(QT_TR_NOOP("Error"),
			QT_TR_NOOP("Cannot go to next keyframe"));
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
void GUI_PrevFrame(uint32_t frameCount)
{
    return;
}
/**
      \fn A_jogRead
      \brief read an average value of jog
*/
#define NB_JOG_READ           3
#define JOG_READ_PERIOD_US    5*1000	// 5ms
#define JOG_THRESH1           40
#define JOG_THRESH2           80
#define JOG_THRESH1_PERIOD    100*1000	// us
#define JOG_THRESH2_PERIOD    40*1000
#define JOG_THRESH3_PERIOD    500
/**
    \fn A_jogRead
    \brief Read an average value of jog
*/
uint32_t A_jogRead(void)
{
    int32_t sum = 0, v;
    for (int i = 0; i < NB_JOG_READ; i++)
      {
	  v = UI_readJog();
	  if (abs(v) < 10)
	      v = 0;
	  sum += v;
	  ADM_usleep(JOG_READ_PERIOD_US);
      }
    return sum / NB_JOG_READ;
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
    static int jog = 0;
    if (jog)
	return;
    jog++;
    while (r = A_jogRead())
      {
	  a = abs(r);
	  printf("%d \n", r);
	  if (a < JOG_THRESH1)
	      slip = JOG_THRESH1_PERIOD;
	  else if (a < JOG_THRESH2)
	      slip = JOG_THRESH2_PERIOD;
	  else
	      slip = JOG_THRESH3_PERIOD;

	  if (r > 0)
	      GUI_NextKeyFrame();
	  else
	      GUI_PreviousKeyFrame();
	  UI_purge();
	  for (int i = 0; i < slip / REFRESH; i++)
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
void GUI_setAllFrameAndTime(void)
{
    char text[80];
    double len;
    // int val;

    // if(!guiReady) return ;
    text[0] = 0;

    UI_updateFrameCount(video_body->getCurrentFrame());
    UI_setCurrentTime(admPreview::getCurrentPts());
    UI_setTotalTime(video_body->getVideoDuration());

    // progress bar
    len = 100;
    if (avifileinfo->nb_frames > 1)
	len = len / (double) (avifileinfo->nb_frames - 1);
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
    // int val;

    // if(!guiReady) return ;
    text[0] = 0;

    UI_setFrameCount(video_body->getCurrentFrame(),
		     avifileinfo->nb_frames);
    UI_setCurrentTime(admPreview::getCurrentPts());

    // progress bar
    len = 100;
    if (avifileinfo->nb_frames > 1)
	len = len / (double) (avifileinfo->nb_frames - 1);
    len *= (double) video_body->getCurrentFrame();



    UI_setScale(len);
}
/**
    \fn A_jumpToTime
    \brief Jump to a given time
*/
uint8_t A_jumpToTime(uint32_t hh,uint32_t mm,uint32_t ss,uint32_t ms)
{
uint64_t pts;
        pts=hh*3600+mm*60+ss;
        pts*=1000;
        pts+=ms;
        pts*=1000;
        uint32_t frame=video_body->searchFrameBefore(pts);
        return GUI_GoToFrame(frame);

}

// EOF
