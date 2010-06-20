/***************************************************************************
                            \file   ADM_guiBitrate 
                            \brief	Simple bitrate analyzer

    copyright            : (C) 2003/2009 by mean
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

#include <math.h>
#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "DIA_factory.h"
#include "ADM_editor/ADM_edit.hxx"
/**
      \fn GUI_displayBitrate
      \brief display a bargraph with bitrates
*/
extern ADM_Composer *video_body;
void GUI_displayBitrate( void )
{

 uint32_t total,max;
 uint32_t len,idx,nb_frame,k,changed;
 uint32_t round,sum,average[60];;

 float f;

float    display[20];
uint32_t medium=0;
uint32_t nbIFrame=0;
uint32_t nbPFrame=0;
uint32_t nbBFrame=0;
uint32_t curBFrame=0;
uint32_t maxBFrame=0;
#if 0
	// 1st compute the total
	uint32_t start,end;
    uint64_t deltaTime=video_body->getMarkerBPts()-video_body->getMarkerAPts();
    start=video_body->searchFrameBefore(video_body->getMarkerAPts());
    end=video_body->searchFrameBefore(video_body->getMarkerBPts());
    
    // Se
	max=0;
	if((deltaTime<100000LL) || abs(start-end)<5)
	{
          GUI_Error_HIG(QT_TR_NOOP("No data"), NULL);
		return ;
	}
    uint64_t increment=video_body->getFrameIncrement();
    f=increment;
    if(f<1000) f=1000;

    f=1000000./f; // ~ FPS
    f=f*1000;     // 1000*fps
    uint32_t fps1000=(uint32_t)f;
	round= (fps1000+990)/1000; // round up of fps
	if(!round) return;

    printf("[Histogram] fps roundup %"LU" fps, time increment=%"LLU" us\n",round,increment);
	video_body->getFrameSize (start, &len);
	for(k=0;k<round;k++) average[k]=0;

	sum=0;
	changed=0;
    total=0;
    uint32_t flags;

        /*  1 st pass, compute max
            to do so : We compute the sum of frame size on [changed] frames i.e ~ 1 second        
        */
	for( k=start;k<end;k++)
	{
 		video_body->getFrameSize (k, &len);
        total+=len;
		sum-=average[changed];
		average[changed]=len;
		sum+=average[changed];
		//printf("Frame %lu Br: %lu\n",k,(sum*8)/1000);
		if(sum>max) max=sum;
		changed++;
		changed%=round;
        video_body->getFlags (k, &flags);
        if(!(flags&(AVI_KEY_FRAME+AVI_B_FRAME)) 
        {
          nbPFrame++;
          curBFrame=0;
        }
        if(flags&AVI_KEY_FRAME) 
        {
          nbIFrame++;
          curBFrame=0;
        }
        if(flags&AVI_B_FRAME)
        {
          nbBFrame++;
          curBFrame++;
          if(curBFrame>maxBFrame) 
            maxBFrame=curBFrame;
          
        }
	}
        
        float g=total;
        g/=(end-start);
        g*=fps1000; //avifileinfo->fps1000;
        g/=(1000.*1000.);
        medium=(uint32_t)(g*8);
        
        max=(max*8)/1000; // Max is now in kByte
        
        printf("Average :%u max%u\n",medium,max);

        /* Round up to the closer 200 kb */
#define ROUNDUP 200
        max=(max+ROUNDUP-1)/ROUNDUP;
        max=max*ROUNDUP;
        

	if(!max)
	{
          GUI_Error_HIG(QT_TR_NOOP("No data"), NULL);
		return ;
	}
	nb_frame=end-start;
	// and now build an histogram
	for(k=0;k<20;k++) display[k]=0;

	video_body->getFrameSize (start, &len);
	for(k=0;k<round;k++) average[k]=0;

	sum=0;
	changed=0;
        uint32_t step=max/20;
        uint32_t sumkb;
        
	for(uint32_t k=start;k<end;k++)
	{
		video_body->getFrameSize (k, &len);
		// which case ?
                if(sum>average[changed])
		  sum-=average[changed];
                else
                   sum=0;
		average[changed]=len;
		sum+=average[changed];
		changed++;
		changed%=round;

		f=(sum*8)/1000;
		f=f/max;
		f=f*20.;
		idx=(uint32_t )floor(f+0.5);
	//	printf("idx : %d\n",idx);
		if(idx>19) idx=19;
		display[idx]++;
	}
        
        
        

        // So now we have a distribution

	for(k=0;k<20;k++) 
        {
          printf("%02u %04u %04u\n",k,(uint32_t)display[k],(uint32_t)(100.*display[k])/nb_frame);
          display[k]=(1000*display[k])/nb_frame;
        }
	
        // Normalize
        uint32_t mxx=0;
        for(k=0;k<20;k++)
        {
           if(display[k]>mxx) mxx=(uint32_t)display[k];
        }   
        for(k=0;k<20;k++)
        {
           display[k]=(display[k]*80)/mxx;
        }   
        
        
        diaElemBar *bar[20];
        char str[256];
        for(int i=0;i<20;i++)
        {
          printf("%03u--%03u :%02u\n",step*i,step*(i+1),(uint32_t)display[i]);
          sprintf(str,"%04u--%04u",step*i,step*(i+1));
          bar[19-i]=new  diaElemBar((uint32_t)display[i],str);
            
        }
#define DOME(a,b,c)       sprintf(bf,"%u",b); diaElemReadOnlyText a(bf,c);
        char bf[100];
        DOME(mx,max,QT_TR_NOOP("Max. bitrate:"));
        DOME(med,medium,QT_TR_NOOP("Average bitrate:"));
        DOME(nI,nbIFrame,QT_TR_NOOP("Number of I frames:"));
        DOME(nP,nbPFrame,QT_TR_NOOP("Number of P frames:"));
        DOME(nB,nbBFrame,QT_TR_NOOP("Number of B frames:"));
        DOME(nMB,maxBFrame,QT_TR_NOOP("Max. B frames:"));
        
        diaElemBar foo(0,"foo");
#define P(X) bar[X] 
  
      diaElem *elems[20+6]={
            &mx,
            &med,
            &nI,&nP,&nB,&nMB,
            P(0),P(1),P(2),P(3),P(4),P(5),P(6),P(7),P(8),P(9),
            P(10),P(11),P(12),P(13),P(14),P(15),P(16),P(17),P(18),P(19)
          };
    
        diaFactoryRun(QT_TR_NOOP("Bitrate Histogram"),20+6,elems);
        
        
        for(int i=0;i<20;i++)
        {
          delete bar[i];
        }
#endif
}
//EOF
