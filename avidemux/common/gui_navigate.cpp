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
#include "ADM_cpp.h"

#include "ADM_default.h"
#include "avi_vars.h"

#include <math.h>
#include "prototype.h"
#include "gui_action.hxx"
#include "gtkgui.h"

#include "DIA_coreToolkit.h"
#include "ADM_commonUI/DIA_busy.h"
#include "ADM_commonUI/GUI_ui.h"
#include "DIA_enter.h"
#include "DIA_coreToolkit.h"
#include "ADM_vidMisc.h"
#include "ADM_preview.h"

#include "prefs.h"

static ADMCountdown  NaggingCountDown(5000); // Wait 5 sec before nagging again for cannot seek
static void A_timedError(bool *first, const char *s);

extern uint8_t DIA_gotoTime(uint32_t *hh, uint32_t *mm, uint32_t *ss,uint32_t *ms, const char *title);
extern uint8_t DIA_gotoTime(uint32_t *hh, uint32_t *mm, uint32_t *ss,uint32_t *ms);
extern void A_setHDRConfig(void);
extern void A_setPostproc(void);

static uint32_t jumpTarget[4] = {0};
static uint32_t jumpMarkerA[4] = {0};
static uint32_t jumpMarkerB[4] = {0};

bool   GUI_infiniteForward(uint64_t pts);
bool   GUI_lastFrameBeforePts(uint64_t pts);
bool   GUI_SeekByTime(int64_t time);
bool   GUI_CoarseSeekByTime(int64_t time);
void  GUI_PrevCutPoint();
void  GUI_NextCutPoint();
bool A_jumpToTime(uint32_t hh,uint32_t mm,uint32_t ss,uint32_t ms);

/**
    \fn HandleAction_Staged
*/
void HandleAction_Staged(Action action)
{
    switch(action)
    {
        case ACT_SetHDRConfig:
            A_setHDRConfig();
            break;
        case ACT_SetPostProcessing:
            A_setPostproc();
            break;
        case ACT_GetTime:
            {
                stagedActionSuccess = 0;
                // Read the time set in the UI, not the real PTS
                uint32_t *t = jumpTarget;
                if(UI_getCurrentTime(t,t+1,t+2,t+3))
                    stagedActionSuccess = 1;
            }
            break;
        case ACT_SelectTime:
            {
                stagedActionSuccess = 0;
                // Get current time
                uint64_t pts = admPreview::getCurrentPts();
                uint32_t *t = jumpTarget;
                ms2time((uint32_t)(pts/1000),t,t+1,t+2,t+3);
                if (DIA_gotoTime(t,t+1,t+2,t+3))
                    stagedActionSuccess = 1;
            }
            break;
        case ACT_GetMarkerA:
            {
                stagedActionSuccess = 0;
                // Read the time set in the UI, not the real PTS
                uint32_t *t = jumpMarkerA;
                if(UI_getMarkerA(t,t+1,t+2,t+3))
                {
                    // Never leave the other marker uninitialized
                    uint64_t pts = video_body->getMarkerBPts();
                    t = jumpMarkerB;
                    ms2time((uint32_t)(pts/1000),t,t+1,t+2,t+3);
                    stagedActionSuccess = 1;
                }
            }
            break;
        case ACT_GetMarkerB:
            {
                stagedActionSuccess = 0;
                // Read the time set in the UI, not the real PTS
                uint32_t *t = jumpMarkerB;
                if(UI_getMarkerB(t,t+1,t+2,t+3))
                {
                    // Never leave the other marker uninitialized
                    uint64_t pts = video_body->getMarkerAPts();
                    t = jumpMarkerA;
                    ms2time((uint32_t)(pts/1000),t,t+1,t+2,t+3);
                    stagedActionSuccess = 1;
                }
            }
            break;
        case ACT_SelectMarkerA:
        case ACT_SelectMarkerB:
            {
                uint64_t pts;
                uint32_t *t;
                const char *DIA_title = "Set Marker A Time";
                stagedActionSuccess = 0;
                // Get current marker A time
                pts = video_body->getMarkerAPts();
                t = jumpMarkerA;
                ms2time((uint32_t)(pts/1000),t,t+1,t+2,t+3);
                // Get current marker B time
                pts = video_body->getMarkerBPts();
                t = jumpMarkerB;
                ms2time((uint32_t)(pts/1000),t,t+1,t+2,t+3);
                // Always initialize both, but select only one
                if(action == ACT_SelectMarkerA)
                    t = jumpMarkerA;
                else
                    DIA_title = "Set Marker B Time";
                if (DIA_gotoTime(t,t+1,t+2,t+3,DIA_title))
                    stagedActionSuccess = 1;
            }
            break;
        case ACT_GetSelection:
            {
                stagedActionSuccess = 0;
                // Read the time set in the UI, not the real PTS
                uint32_t time[4] = {0}, *t = time;
                if(UI_getSelectionTime(t,t+1,t+2,t+3))
                {
                    // Never leave the markers uninitialized
                    uint64_t ptsA = video_body->getMarkerAPts();
                    uint64_t ptsB, delta = ((t[0]*3600+t[1]*60+t[2])*1000+t[3])*1000;
                    t = jumpMarkerA;
                    ms2time((uint32_t)(ptsA/1000),t,t+1,t+2,t+3);
                    t = jumpMarkerB;
                    ptsB = ptsA + delta;
                    ms2time((uint32_t)(ptsB/1000),t,t+1,t+2,t+3);
                    stagedActionSuccess = 1;
                }
            }
            break;
        default:break;
    }
}
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
          ADM_info("Scale :%" PRIu32"\n",nf);
          double tme=nf;
          double totalDuration=(double) video_body->getVideoDuration();
          tme*=totalDuration;
          tme/=ADM_SCALE_SIZE;
          uint64_t pts=5000+(uint64_t)tme; // aim a bit higher to avoid double "search previous" events
          if(pts>totalDuration) pts=totalDuration;
          
          ADM_info("Scale Time:%" PRIu64" ms (total=%" PRIu64" ms)\n",pts/1000,video_body->getVideoDuration()/1000);
          ADM_info("Scale Time:%s ms \n",ADM_us2plain(pts));
           if(false==video_body->getPKFramePTS(&pts))
            {
                if(false==video_body->getNKFramePTS(&pts))
                {
                    ADM_warning("Cannot seek to %" PRIu64" ms\n",pts/1000);
                    ignore_change--;
                    break;
                }
            }
             ADM_info("Seeking to  Time:%s ms \n",ADM_us2plain(pts));
            if(true!=admPreview::seekToIntraPts(pts))
            {
                ADM_warning("Scale: Seeking to intra at %" PRIu64" ms failed\n",pts/1000);
            }
            UI_setCurrentTime(pts);
            UI_purge();
            ignore_change--;
        }
        break;
      case ACT_FineScale:
        if(playing) break;
        if (!ignore_change)
        {
            uint32_t nf;
            ignore_change++;
            nf = GUI_GetScale ();
            ADM_info("Fine Scale :%" PRIu32"\n",nf);
            double tme=nf;
            double totalDuration=(double) video_body->getVideoDuration();
            tme*=totalDuration;
            tme/=ADM_SCALE_SIZE;
            uint64_t pts=5000+(uint64_t)tme; // aim a bit higher to avoid double "search previous" events
            if(pts>totalDuration) pts=totalDuration;

            ADM_info("Fine Scale Time:%" PRIu64" ms (total=%" PRIu64" ms)\n",pts/1000,video_body->getVideoDuration()/1000);
            ADM_info("Fine Scale Time:%s ms \n",ADM_us2plain(pts));

            uint64_t lastpts=pts;
            if(false==video_body->getNKFramePTS(&lastpts)) // at the end of the video, be careful
            {
                if(false==video_body->getPKFramePTS(&lastpts))
                {
                    ignore_change--;
                    break;
                }
                GUI_infiniteForward(lastpts);
                lastpts=admPreview::getCurrentPts();
                if(pts>=lastpts)
                    pts=lastpts;
            }
            if(false==GUI_lastFrameBeforePts(pts)) // we are probably at the beginning of the video,
                video_body->rewind(); // go to the first frame then
            admPreview::samePicture();
            UI_setCurrentTime(pts);
            UI_purge();
            ignore_change--;
        }
        break;      
      case ACT_GotoMarkA:
      case ACT_GotoMarkB:
        {
            uint64_t pts = ADM_NO_PTS;
            uint64_t rescue = admPreview::getCurrentPts();
            if(action==ACT_GotoMarkA)
                pts = video_body->getMarkerAPts();
            else
                pts = video_body->getMarkerBPts();
            if(pts == ADM_NO_PTS) return;
            // handle default values
            if(!pts)
            {
                video_body->rewind();
            }else if(pts == video_body->getVideoDuration())
            {
                pts = video_body->getLastKeyFramePts();
                if(pts == ADM_NO_PTS) return;
                GUI_infiniteForward(pts);
            }else if(false == video_body->goToTimeVideo(pts))
            {
                ADM_warning("Marker %s PTS %s doesn't match a frame exactly or decoding failure, making something up.\n",
                    (action == ACT_GotoMarkA)? "A" : "B",
                    ADM_us2plain(pts));
                uint64_t start = pts;
                // Can we seek to previous keyframe and approach the marker from there?
                if(false == video_body->getPKFramePTS(&start) || start == ADM_NO_PTS)
                { // Nope, try to seek to the next keyframe instead.
                    start = pts;
                    if(false == video_body->getNKFramePTS(&start) || start == ADM_NO_PTS)
                    {
                        ADM_warning("Seek to marker %s at %s failed.\n", (action == ACT_GotoMarkA)? "A" : "B", ADM_us2plain(pts));
                        admPreview::seekToTime(rescue);
                        return;
                    }
                    if(false == admPreview::seekToIntraPts(start))
                    { // try to recover
                        admPreview::seekToTime(rescue);
                        return;
                    }
                    GUI_setCurrentFrameAndTime();
                    return;
                }
                admPreview::deferDisplay(true);
                if(false == admPreview::seekToIntraPts(start))
                {
                    admPreview::seekToTime(rescue);
                    admPreview::deferDisplay(false);
                    return;
                }
                while(pts > admPreview::getCurrentPts() && admPreview::nextPicture())
                {
                }
                admPreview::deferDisplay(false);
            }
            admPreview::samePicture();
            GUI_setCurrentFrameAndTime();
        }
        break;

      case ACT_PreviousKFrame:
        GUI_PreviousKeyFrame();
        break;
      case ACT_PreviousFrame:
        GUI_PrevFrame();
        break;
    case ACT_Forward1Mn:
          GUI_SeekByTime(60*1000LL*1000LL);
          break;
        case ACT_Back1Mn:
          GUI_SeekByTime(-60*1000LL*1000LL);
          break;        
      case ACT_Forward4Seconds:
        GUI_SeekByTime(4000000LL);
        break;

      case ACT_Forward2Seconds:
        GUI_SeekByTime(2000000LL);
    break;

      case ACT_Forward1Second:
        GUI_SeekByTime(1000000LL);
        break;

      case ACT_Back4Seconds:
        GUI_SeekByTime(-4000000LL);
        break;

      case ACT_Back2Seconds:
        GUI_SeekByTime(-2000000LL);
        break;

      case ACT_Back1Second:
        GUI_SeekByTime(-1000000LL);
    break;

      case ACT_NextFrame:
        GUI_NextFrame();
      break;
      case ACT_NextKFrame:
        GUI_NextKeyFrame();
      break;
      case ACT_PrevCutPoint:
        GUI_PrevCutPoint();
      break;
      case ACT_NextCutPoint:
        GUI_NextCutPoint();
      break;
      case ACT_NextBlackFrame:
        GUI_NextBlackFrame();
      break;
      case ACT_PrevBlackFrame:
        GUI_PrevBlackFrame();
      break;
      case ACT_End:
        {
            uint64_t pts=video_body->getLastKeyFramePts();
            if(pts==ADM_NO_PTS) break;    
            GUI_infiniteForward(pts);
            admPreview::samePicture();
            GUI_setCurrentFrameAndTime();
        }
            break;
      case ACT_Begin:
            video_body->rewind();
            admPreview::samePicture(); // Ugly FIXME TODO
             GUI_setCurrentFrameAndTime();
            //GUI_GoToKFrameTime(0);
            break;
      case ACT_GotoTime:
      {
            if(playing) break;
            if(!stagedActionSuccess) break;
            uint32_t *t = jumpTarget;
            uint32_t hh,mm,ss,ms;
            hh = *t++;
            mm = *t++;
            ss = *t++;
            ms = *t;
            A_jumpToTime(hh,mm,ss,ms);
            stagedActionSuccess = 0;
      }
      break;
      case ACT_ChangeMarkerA:
      case ACT_ChangeMarkerB:
        if(!playing && stagedActionSuccess)
        {
            // Selected marker A/B time
            uint32_t *t = jumpMarkerA;
            uint32_t *tB = jumpMarkerB;
            uint32_t hh,mm,ss,ms;
            uint64_t ptsA, ptsB;

            // Selected marker A/B time chooser
            uint64_t *sel = &ptsA;

            // Total time (beyond the last frame)
            uint64_t tot = video_body->getVideoDuration();

            // Current marker A/B to verify changes
            uint64_t preA = ptsA = video_body->getMarkerAPts();
            uint64_t preB = ptsB = video_body->getMarkerBPts();

            // Backup the current time to restore it later
            uint32_t time[4] = {0};
            uint64_t pts = admPreview::getCurrentPts();
            ms2time((uint32_t)(pts/1000),time,time+1,time+2,time+3);

            // Jump to time to find the correct marker A/B time, then
            // restore the previous current time after procesing.
            while(t)
            {
                hh = t[0];
                mm = t[1];
                ss = t[2];
                ms = t[3];

                // Check if outside boundaries when jumping to A/B
                if(sel)
                {
                    uint64_t x = ((hh*3600+mm*60+ss)*1000+ms)*1000;

                    if(x == 0)
                        *sel = 0;
                    else if(x >= tot)
                        *sel = tot;
                    else if(x != *sel)
                        sel = NULL;
                }

                // Inside boundaries or restoring previous time
                if(!sel)
                {
                    // FIXME: Don't jump to find the correct marker
                    // A/B time, rather find the closest frame then
                    // frame2time.
                    A_jumpToTime(hh,mm,ss,ms);
                }

                // Exit the loop after restoring previous time
                if(!tB)
                {
                    break;
                }
                else if(t != tB)
                {
                    // When inside boundaries find marker A time
                    if(!sel)
                        ptsA = admPreview::getCurrentPts();

                    // Switch to marker B
                    t = tB;
                    sel = &ptsB;
                    continue;
                }
                else
                {
                    // When inside boundaries find marker B time
                    if(!sel)
                        ptsB = admPreview::getCurrentPts();

                    // Swap marker A and B when necessary
                    if(ptsA > ptsB)
                    {
                        bool swapit = false;

                        if(!prefs->get(FEATURES_SWAP_IF_A_GREATER_THAN_B, &swapit))
                            swapit = true;

                        if(swapit) // auto swap
                        {
                            uint64_t y = ptsA;
                            ptsA = ptsB;
                            ptsB = y;
                        }
                        else
                        {
                            if(action == ACT_ChangeMarkerA)
                                ptsB = tot; // reset B
                            else
                                ptsA = 0; // reset A
                        }
                    }

                    // Restore the previous time breaking the loop
                    t = time;
                    tB = NULL;
                    sel = NULL;
                }
            }

            // Check if any marker A/B has been changed
            if(preA != ptsA || preB != ptsB)
            {
                video_body->addToUndoQueue();
                video_body->setMarkerAPts(ptsA);
                video_body->setMarkerBPts(ptsB);
                UI_setMarkers(ptsA, ptsB);
            }

            stagedActionSuccess = 0;
        }
        break;
      case ACT_Refresh:
        { // Flush cache and seek to the current picture
            if(playing) break;
            if(!stagedActionSuccess) break;
            ADMImage *pic = admPreview::getBuffer();
            if(!pic) break;
            uint64_t time = admPreview::getCurrentPts();
            admPreview::deferDisplay(true);
            if(pic->flags & AVI_KEY_FRAME)
            {
                admPreview::nextPicture();
                admPreview::previousKeyFrame();
            }else
            {
                admPreview::previousKeyFrame(); // flush cache
                if(false == admPreview::seekToTime(time))
                    video_body->rewind(); // avoid crash
            }
            admPreview::deferDisplay(false);
            admPreview::samePicture();
            GUI_setCurrentFrameAndTime();
            stagedActionSuccess = 0;
            break;
        }
      case ACT_SeekBackward:
            GUI_CoarseSeekByTime(-4000000LL);
            break;
      case ACT_SeekForward:
            GUI_CoarseSeekByTime(4000000LL);
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
bool GUI_NextFrame(void)
{
    if (playing)
        return false;
    if (!avifileinfo)
        return false;

    if(!admPreview::nextPicture())
        return false;
    GUI_setCurrentFrameAndTime();
    UI_purge();
    return true;
}


/**
    \fn GUI_NextKeyFrame
    \brief Go to the next keyframe
*/
bool GUI_NextKeyFrame(void)
{
    static bool firstError = true;

    if (playing)
        return false;
    if (!avifileinfo)
        return false;

    if (!admPreview::nextKeyFrame())
      {
        UI_notifyError(QT_TRANSLATE_NOOP("navigate","Cannot go to next keyframe"),2500);
        //A_timedError(&firstError, QT_TRANSLATE_NOOP("navigate","Cannot go to next keyframe"));
        return false;
      }
    GUI_setCurrentFrameAndTime();
    UI_purge();
    return true;
}

/**
    \fn GUI_GoToKFrameTime
    \brief Go to keyframe at given exact time
*/
bool GUI_GoToKFrameTime(uint64_t exactTime)
{
    if (playing)
        return false;
    if (!avifileinfo)
        return false;

    if(!admPreview::seekToIntraPts(exactTime))
        return false;
    //admPreview::samePicture(); // why a second time?
    GUI_setCurrentFrameAndTime();
    UI_purge();
    return true;
}
/**
    \fn GUI_GoToFrame
    \brief go to a given frame. Half broken, do not use.
*/
bool GUI_GoToFrame(uint32_t frame)
{
#if 0
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
#endif
    return 1;
}

/**
    \fn GUI_PreviousKeyFrame
    \brief Go to previous keyframe
*/
bool GUI_PreviousKeyFrame(void)
{
    static bool firstError = true;

    if (playing)
        return false;
    if (!avifileinfo)
        return false;

    if (!admPreview::previousKeyFrame())
      {
        UI_notifyError(QT_TRANSLATE_NOOP("navigate","Cannot go to previous keyframe"),2500);
        //A_timedError(&firstError, QT_TRANSLATE_NOOP("navigate","Cannot go to previous keyframe"));
        return false;
      }
    GUI_setCurrentFrameAndTime();
    UI_purge();
    return true;
}

uint8_t A_rebuildKeyFrame(void)
{
//    return video_body->rebuildFrameType();
    return 1;
}
/**
    \fn GUI_PrevFrame
    \brief Go to current frame -1
*/
bool GUI_PrevFrame(void)
{
    if (playing)
        return false;
    if (!avifileinfo)
        return false;

    if (!admPreview::previousPicture())
      {
//        We're probably at the beginning of the file ...
//            GUI_Error_HIG(QT_TRANSLATE_NOOP("navigate","Error"),    QT_TRANSLATE_NOOP("navigate","Cannot go to previous frame"));
        return false;
      }
    GUI_setCurrentFrameAndTime();
    UI_purge();
    return true;
}
/**
      \fn A_jogRead
      \brief read an average value of jog
*/
#define NB_JOG_READ           3
#define JOG_READ_PERIOD_US    5*1000    // 5ms
#define JOG_THRESH1           40
#define JOG_THRESH2           80
#define JOG_THRESH1_PERIOD    100*1000    // us
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
    while ((r = A_jogRead()))
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
    UI_setTotalTime(video_body->getVideoDuration());
    uint32_t numOfSegs = video_body->getNbSegment();
    uint64_t * segPts = new uint64_t[numOfSegs];
    for(int i=0; i<numOfSegs; i++)
    {
        _SEGMENT * seg = video_body->getSegment(i);
        if (seg)
            segPts[i] = seg->_startTimeUs;
        else
            segPts[i] = ADM_NO_PTS;
    }
    UI_setSegments(numOfSegs, segPts);
    delete [] segPts;
    // progress bar
    GUI_setCurrentFrameAndTime(0);
}

/**
    \fn GUI_setCurrentFrameAndTime
    \brief Update some informations : current frame # and current time
*/
void GUI_setCurrentFrameAndTime(uint64_t offset)
{
    uint64_t pts=admPreview::getCurrentPts()+offset;
    double len;
   
    UI_setCurrentTime(pts);
    len=pts;
    len*=ADM_SCALE_SIZE;
    len/=video_body->getVideoDuration();  
    GUI_SetScale(len);
}

/**
    \fn A_jumpToTime
    \brief Jump to a given time
*/
bool A_jumpToTime(uint32_t hh,uint32_t mm,uint32_t ss,uint32_t ms)
{
    uint64_t pts, total = video_body->getVideoDuration();
    if(!total) return false;

    pts=hh*3600+mm*60+ss;
    pts*=1000;
    pts+=ms;
    pts*=1000;
    // Aim higher to avoid rejecting the target picture if PTS is not a multiple of 1000 us.
    // We can overshoot if two pics are less than 1ms apart, an unlikely scenario.
    pts+=999;

    if(pts >= total)
        pts = total-1;
    if(false==GUI_lastFrameBeforePts(pts)) // we are probably at the beginning of the video,
    {
        video_body->rewind(); // go to the first frame then
        admPreview::samePicture();
        GUI_setCurrentFrameAndTime();
    }
    return true;
}
/**
    \fn GUI_SeekByTime
*/
bool GUI_SeekByTime(int64_t time)
{
    uint64_t pts=admPreview::getCurrentPts();

    if (time < 0 && pts < -time)
    {   // we can't assume that pts=0 were legitimate, rewind to the first frame instead
        video_body->rewind();
        admPreview::samePicture();
        GUI_setCurrentFrameAndTime();
        return true;
    }else
    {
        pts += time;
    }
    ADM_info("Seek to:%s ms \n",ADM_us2plain(pts));
    return GUI_lastFrameBeforePts(pts);
}

/**
    \fn GUI_SeekByTime
*/
bool GUI_CoarseSeekByTime(int64_t time)
{
    uint64_t pts=admPreview::getCurrentPts();

    if (time < 0 && pts < -time)
    {   // we can't assume that pts=0 were legitimate, rewind to the first frame instead
        video_body->rewind();
        admPreview::samePicture();
        GUI_setCurrentFrameAndTime();
        return true;
    }else
    {
        pts += time;
    }
    uint64_t totalDuration = video_body->getVideoDuration();
    if(pts>totalDuration) pts=totalDuration;
    ADM_info("Coarse Seek to:%s ms \n",ADM_us2plain(pts));
    if (time >= 0)
    {
        if(false==video_body->getNKFramePTS(&pts))
        {
            ADM_warning("Cannot seek to %" PRIu64" ms\n",pts/1000);
            return false;
        }
    }
    else
    {
        if(false==video_body->getPKFramePTS(&pts))
        {
            ADM_warning("Cannot seek to %" PRIu64" ms\n",pts/1000);
            return false;
        }
    }
    ADM_info("Actual Seeking to  Time:%s ms \n",ADM_us2plain(pts));
    if(false==admPreview::seekToIntraPts(pts))
    {
        ADM_warning("Coarse Seeking: Seeking to intra at %" PRIu64" ms failed\n",pts/1000);
        return false;
    }
    GUI_setCurrentFrameAndTime();
    UI_purge();
    return true;
}


/**
    \fn GUI_PrevCutPoint
*/
void GUI_PrevCutPoint()
{
    if (playing)
        return;
    if (!avifileinfo)
        return;

    uint64_t pts=admPreview::getCurrentPts();
    uint64_t last_prev_pts=ADM_NO_PTS;
    int segNo, numOfSegs = video_body->getNbSegment();
    for(int i=1; i<numOfSegs; i++) // we are not interested in the first segment
    {
        _SEGMENT * seg = video_body->getSegment(i);
        if (seg)
        {
            if (seg->_startTimeUs < pts)
            {
                last_prev_pts = seg->_startTimeUs;
                segNo = i;
            } else
                break;
        }
    }

    if (last_prev_pts == ADM_NO_PTS)
        return;

    ADM_info("Seeking to cut point at %s ms\n",ADM_us2plain(last_prev_pts));

    // Can we use a fast lane?
    uint64_t tmp = video_body->getFirstFrameInSegmentPts(segNo);
    if(tmp != ADM_NO_PTS && admPreview::seekToTime(tmp))
    {
        GUI_setCurrentFrameAndTime();
        return;
    }
    // Does the start time of the segment match a keyframe?
    tmp = last_prev_pts + 1;
    bool gotPreviousKeyFrame = video_body->getPKFramePTS(&tmp);
    if (gotPreviousKeyFrame && tmp == last_prev_pts)
    {
        if(admPreview::seekToIntraPts(last_prev_pts))
            GUI_setCurrentFrameAndTime();
        return;
    }
    // Nope, seek to the previous keyframe and decode from there
    // until we have crossed the segment boundary.
    admPreview::deferDisplay(true);
    if (gotPreviousKeyFrame)
    {
        if (false == admPreview::seekToIntraPts(tmp))
        {
            admPreview::deferDisplay(false);
            return;
        }
    } else // no keyframe before the cut point
    {
        video_body->rewind();
    }
    while (admPreview::nextPicture())
    {
        tmp = admPreview::getCurrentPts();
        if (!tmp || tmp == ADM_NO_PTS)
        { // should never happen, try not to crash in admPreview::samePicture
            video_body->rewind();
            break;
        }
        if (tmp >= last_prev_pts) // we've crossed the segment boundary
            break;
    }
    admPreview::deferDisplay(false);
    admPreview::samePicture();
    GUI_setCurrentFrameAndTime();
}

/**
    \fn GUI_NextCutPoint
*/
void GUI_NextCutPoint()
{
    if (playing)
        return;
    if (!avifileinfo)
        return;

    uint64_t pts=admPreview::getCurrentPts();
    uint64_t first_next_pts=ADM_NO_PTS;
    int segNo, numOfSegs = video_body->getNbSegment();
    for(int i=1; i<numOfSegs; i++)
    {
        _SEGMENT * seg = video_body->getSegment(i);
        if (seg)
        {
            if (seg->_startTimeUs > pts)
            {
                first_next_pts = seg->_startTimeUs;
                segNo = i;
                break;
            }
        }
    }

    if (first_next_pts == ADM_NO_PTS)
        return;

    ADM_info("Seeking to cut point at %s ms\n",ADM_us2plain(first_next_pts));

    // Can we use the fast lane?
    uint64_t tmp = video_body->getFirstFrameInSegmentPts(segNo);
    if(tmp != ADM_NO_PTS && admPreview::seekToTime(tmp))
    {
        GUI_setCurrentFrameAndTime();
        return;
    }

    // Does the start time of the segment match a keyframe?
    tmp = first_next_pts + 1;
    bool gotPreviousKeyFrame = video_body->getPKFramePTS(&tmp);
    if (gotPreviousKeyFrame && tmp == first_next_pts)
    {
        if(admPreview::seekToIntraPts(first_next_pts))
            GUI_setCurrentFrameAndTime();
        return;
    }
    // Nope, seek to the last keyframe before the cut point if it is after
    // the current position and decode from there until we have crossed
    // the segment boundary.
    admPreview::deferDisplay(true);
    if (gotPreviousKeyFrame && tmp > pts)
    {
        if (false == admPreview::seekToIntraPts(tmp))
        {
            admPreview::deferDisplay(false);
            return;
        }
    }
    while (admPreview::nextPicture())
    {
        tmp = admPreview::getCurrentPts();
        if (!tmp || tmp == ADM_NO_PTS)
        { // should never happen, try not to crash in admPreview::samePicture
            video_body->rewind();
            break;
        }
        if (tmp >= first_next_pts) // we've crossed the segment boundary
            break;
    }
    admPreview::deferDisplay(false);
    admPreview::samePicture();
    GUI_setCurrentFrameAndTime();
}

/**
    \fn GUI_GoToTime
*/
bool GUI_GoToTime(uint64_t time)
{

    // We have to call the editor as the frames needed to decode the target frame may be hidden
    if(false==video_body->goToTimeVideo(time))
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("navigate","Seek"), QT_TRANSLATE_NOOP("navigate","Error seeking to %" PRIu64" ms"),time/1000);
    }
    admPreview::samePicture();
    GUI_setCurrentFrameAndTime();
    return true;
}   
/**
 * \brief go forward as much as possible from pts
 * @param pts
 * @return 
 */
bool GUI_infiniteForward(uint64_t pts)
{
    admPreview::deferDisplay(1);
    if(false==video_body->goToTimeVideo(pts))
    {
        ADM_warning("Seek to %s failed, retrying from the previous keyframe\n",ADM_us2plain(pts));
        // work around a possible inability to decode keyframe at pts
        uint64_t tmp=pts;
        if(!video_body->getPKFramePTS(&tmp))
            return false;
        ADM_info("Retrying from keyframe at %s\n",ADM_us2plain(tmp));
        if(false==video_body->goToTimeVideo(tmp))
        {
            ADM_error("Seek to the penultimate keyframe failed as well, giving up\n",ADM_us2plain(tmp));
            return false;
        }
    }
    bool refreshNeeded = true;
    while(admPreview::nextPicture())
    {
        refreshNeeded = false;
    }
    if(refreshNeeded)
        admPreview::samePicture();
    admPreview::deferDisplay(0);
    return true;
}

/**
    \fn GUI_lastFrameBeforePts
 */
bool GUI_lastFrameBeforePts(uint64_t pts)
{
    uint64_t tmp=pts;
    uint64_t current=admPreview::getCurrentPts();
    // Try to find a keyframe just before pts...
    if(!video_body->getPKFramePTS(&tmp))
        return false;

    if(tmp<=current && pts>current) // within the same GOP, seeking forward
        tmp=current; // no need to go back to the keyframe
    else if(!video_body->goToTimeVideo(tmp))
        return false;

    // Starting from tmp, approach the last frame before pts
    admPreview::deferDisplay(true);
    bool stepBack = false;
    while(true)
    {
        stepBack = admPreview::nextPicture();
        if(!stepBack)
            break;
        tmp = admPreview::getCurrentPts();
        if(tmp == ADM_NO_PTS)
            break;
        if(tmp >= pts)
            break;
    }
    if(stepBack)
        admPreview::previousPicture();
    admPreview::deferDisplay(false);
    admPreview::samePicture();
    GUI_setCurrentFrameAndTime();
    UI_purge();
    return true;
}

/**
 * \fn A_timedError
 * \brief display error unless the last error is too recent
 * @param s
 */
void A_timedError(bool *first, const char *s)
{
    if(*first || NaggingCountDown.done()) // else still running, do not nag
    {
        if(UI_navigationButtonsPressed()) // probably auto-repeat in action
            return;
        NaggingCountDown.reset();
        *first = false;
        GUI_Error_HIG(QT_TRANSLATE_NOOP("navigate","Error"),s);
        return;
    }
    NaggingCountDown.reset();
    *first = false;
}

// EOF
