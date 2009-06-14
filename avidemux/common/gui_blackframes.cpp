/***************************************************************************
                          gui_blackframes.cpp  -  description
                             -------------------

            Detect black frames

    
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
#include "ADM_coreAudio.h"
#include "audioprocess.hxx"
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


//**********************************************************************

uint8_t  countLightPixels(int darkness)
{

    uint32_t width = avifileinfo->width;
    uint32_t height = avifileinfo->height;
    uint32_t sz = width * height ;
    const int maxnonb=(width* height)>>8;

    return 0;
#warning FIXME
#warning FIXME
#warning FIXME
#if 0
    uint8_t *buff;

    int cnt4=0;

    buff=rdr_decomp_buffer->data+ sz;

    while(--buff>rdr_decomp_buffer->data)
    {
      if(*buff > darkness )
	cnt4++;
	if(cnt4>=maxnonb)
	  return(1);
    }
#endif
    return(0);
}
//**********************************************************************
// Fast version to decide if a frame is black or not
// Return 0 if the frame is black
// return 1 else
//**********************************************************************

static const int  sliceOrder[8]={3,4,2,5,1,6,0,7};
/**
        \fn sliceScanNotBlack
        \brief The image is split into 8 slices, returns if the given slice is black or not
*/
static int sliceScanNotBlack(int darkness, int maxnonb, int sliceNum,ADMImage *img)
{


    uint32_t width = img->_width;
    uint32_t height = img->_height>>3;
    uint32_t sz = width * height ;    

    uint8_t *buff,*start;

    int cnt4=0;

    start=img->data+ sz*sliceNum;
    buff=start+sz;

    while(--buff>start)
    {
      if(*buff > darkness )
      {
        cnt4++;
        if(cnt4>=maxnonb)
          return(1);
      }
    }
    return(0);

}
/**
    \fn fastIsNotBlack
    \brief Quickly check if the frame is black or not
*/
uint8_t  fastIsNotBlack(int darkness,ADMImage *img)
{

    uint32_t width = img->_width;
    uint32_t height = img->_height;    
    uint32_t maxnonb=(width* height)>>8;
    
        maxnonb>>=3;
        // Divide the screen in 8 part  : 0 1 2 3 4 5 6 7 
        // Scan 2 & 3 first, if still good, go on
        for(uint32_t i=0;i<6;i++)
        {
                if(sliceScanNotBlack(darkness,maxnonb,sliceOrder[i],img)) return 1;
        }
        // The slice 0 & 7 are particular and we admit twice as much
        if(sliceScanNotBlack(darkness,maxnonb*2,0,img)) return 1;
        if(sliceScanNotBlack(darkness,maxnonb*2,7,img)) return 1;

    return(0);
}


//**********************************************************************

void GUI_NextPrevBlackFrame(int dir)
{
   uint32_t f;
   uint32_t flags;
   uint16_t reresh_count=0;
   uint32_t orgFrame;
   uint32_t r=1;
  if (playing)
		return;
    if (! avifileinfo)
       return;

    ADMImage *buffer=admPreview::getBuffer();
    if(!buffer) return;

   const int darkness=40;

   DIA_workingBase *work=createWorking(QT_TR_NOOP("Seeking"));
   orgFrame=curframe;
   int total;
    // Avoid it being 0
    if(dir=1) total=avifileinfo->nb_frames-curframe+1;
        else total=curframe+1;
   while(1)
   {
        int current=abs(curframe-orgFrame);
      f=curframe+dir;
      if(work->update(current,total)) break;

      if((f==0 && dir==-1)|| (f==avifileinfo->nb_frames-1&&dir==1)) break;

     if( !video_body->getUncompressedFrame(f ,buffer,&flags))
       {
          r=0;
          break;
       }
    if(!work->isAlive())
    {
          r=0;
          break;
    }
        curframe=f;

        if(!fastIsNotBlack(darkness,buffer)) break;
        reresh_count++;
        if(reresh_count>100)
        {
                GUI_setCurrentFrameAndTime();
                reresh_count=0;
        }       
   }
   delete work;
   if(!r)
   {
      curframe=orgFrame;
   }
//    admPreview::update( curframe) ;
    GUI_setCurrentFrameAndTime();

   return ;
}
/**
    \fn A_ListAllBlackFrames
    \brief Scan for all black frames and output that in a separate (text) file
*/
uint8_t A_ListAllBlackFrames(char *name)
{
    uint32_t f;
    uint32_t flags;
    uint32_t startframe;
    uint16_t mm,hh,ss,ms;

    uint16_t reresh_count=0;

    char *outfile;
    FILE *fd;

    outfile=name;    

    if ( playing )
        return 0;
    if ( !avifileinfo )
        return 0;
   ADMImage *buffer=admPreview::getBuffer();
    if(!buffer) return 0;
   
    if ( !outfile )
        return 0;
    fd=fopen(outfile, "wb");
    if ( fd == NULL )
    {
        fprintf(stderr, "cannot create output file for list of black frames\n");
        return 0;
    }

    const int darkness=40;

    startframe=curframe;
    DIA_workingBase *work=createWorking(QT_TR_NOOP("Finding black frames"));
    printf("\n** Listing all black frames **\n");

    for (f=0; f<avifileinfo->nb_frames; f++) {
       if( work->update( 100 * f / avifileinfo->nb_frames ) ) 
            break;
        if ( !video_body->getUncompressedFrame(f,buffer,&flags) ) 
        {
            break;
        }

        curframe=f;
        if ( !fastIsNotBlack(darkness,buffer) ) 
        {
            frame2time(curframe,avifileinfo->fps1000,&hh,&mm,&ss,&ms);
            printf("\tBlack frame: frame %d  time %02d:%02d:%02d.%03d\n", curframe, hh, mm, ss, ms);
            fprintf(fd, "\tBlack frame: frame %d  time %02d:%02d:%02d.%03d\n", curframe, hh, mm, ss, ms);
        }
        reresh_count++;
        if(reresh_count>100)
        {
                GUI_setCurrentFrameAndTime();
                reresh_count=0;
        }
    }

    printf("** done **\n\n");
    fclose(fd);
    delete work;
    GUI_GoToFrame(startframe);
    return 1;
}
//EOF
