
/***************************************************************************
                          ADM_vidTelecide.cpp  -  description
                             -------------------
	Strongly inspired by Donal Graft deinterlacer (decomb)

***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ADM_default.h"

#include "ADM_videoFilterDynamic.h"
#include"ADM_vidField.h"
#include"ADM_vidPalSmart.h"

#define MATCH_THRESH 100
#define ASM_DEINT
#define ASM_BLEND
static FILTER_PARAM nullParam={0,{""}};



extern  int32_t _l_w,_l_h;
extern uint8_t *_l_p,*_l_c,*_l_n;
extern uint8_t *_l_e,*_l_e2;

//REGISTERX(VF_INTERLACING, "palsmart",QT_TR_NOOP("PAL smart"),QT_TR_NOOP(
//    "Smartly revert non constant PAL field shift."),VF_TELECIDE,1,telecide_create,telecide_script);

//********** Register chunk ************

VF_DEFINE_FILTER(ADMVideoTelecide,nullParam,
    palsmart,
                QT_TR_NOOP("PAL smart"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Smartly revert non constant PAL field shift."));
//********** Register chunk ************
char *ADMVideoTelecide::printConf(void)
{
	ADM_FILTER_DECLARE_CONF(" Pal Smart");
}

//_______________________________________________________________


ADMVideoTelecide::ADMVideoTelecide(	AVDMGenericVideoStream *in,CONFcouple *setup)
										: ADMVideoFields(in,setup)
{
	vidCache=new VideoCache(4,in);
	_uncompressed=new ADMImage(_info.width,_info.height);
  	
}
ADMVideoTelecide::~ADMVideoTelecide()
{
 	
	delete vidCache;
	vidCache=NULL;
	delete _uncompressed;
	
}
/*
  	Interleave _uncompressed with in2
   		even line from in2 odd=0
      odd  line          odd=1
*/
uint8_t ADMVideoTelecide::interleave(	ADMImage *imgsrc, ADMImage *imgdst,uint8_t odd)
{
 	uint32_t w=_info.width;
	uint8_t  *src1,*dst;
	
	src1=YPLANE(imgsrc);
	dst=YPLANE(imgdst);
	if(odd)
	{
		src1+=w;
		dst+=w;
	}
	for(uint32_t y=(_info.height>>1);y>0;y--)
		{
        		memcpy(dst,src1,w);
			src1+=w<<1;
			dst+=w<<1;
		}
	return 1;
}

//
//		Try to march fields of a frame with previous  / next until it is not interlaced	
//
//

uint8_t ADMVideoTelecide::getFrameNumberNoAlloc(uint32_t frame,
			uint32_t *len,
			ADMImage *data,
			uint32_t *flags)
{
uint32_t uvlen;
uint32_t dummylen;
uint8_t motion;

uint32_t 	cmatch,nmatch,n2match;
ADMImage	*cur,*next;

			
		if(frame>=_info.nb_frames) return 0;			
		uvlen=    _info.width*_info.height;
		*len=uvlen+(uvlen>>1);
		
		cur=vidCache->getImage(frame);
		if(!cur) return 0;
		data->copyInfo(cur);
		if(!frame || frame==_info.nb_frames-1)
		{

			data->duplicate(cur);
			vidCache->unlockAll();
			return 1;
		
		}
		
		next=vidCache->getImage(frame-1);
		if(!next) 
		{
			vidCache->unlockAll();
			return 0;
		}		
		

		// for u & v , no action -> copy it as is
		memcpy(UPLANE(data),UPLANE(cur),uvlen>>2);
		memcpy(VPLANE(data),VPLANE(cur),uvlen>>2);
		data->copyInfo(cur);

        	// No interleaving detected
           	if(!(motion=hasMotion(data)) )
		{
			printf("\n Not interlaced !\n");
			memcpy(YPLANE(data),YPLANE(cur),uvlen);
			vidCache->unlockAll();
      			return 1; // over !
		}
		// else cmatch is the current match
		cmatch=getMatch(cur);

/*	------------------------------------------------------------------------------------
			Try to complete with next frame  fields
-----------------------------------------------------------------------------------
*/
		// Interleav next in even field
		
		interleave(cur,_uncompressed,1);
		interleave(next,_uncompressed,0);
		nmatch=getMatch(_uncompressed);
		
		interleave(cur,_uncompressed,0);
		interleave(next,_uncompressed,1);
		n2match=getMatch(_uncompressed);

		printf(" Cur  : %lu \n",cmatch);
		printf(" Next : %lu \n",nmatch);
		printf(" NextP: %lu \n",n2match);

		if((cmatch<nmatch)&&(cmatch<n2match))
		{
			printf("\n __ pure interlaced __\n");
			interleave(cur,_uncompressed,0);
			interleave(cur,_uncompressed,1);
			hasMotion(_uncompressed);
	  		doBlend(_uncompressed,data);			
			vidCache->unlockAll();
			return 1;
		}
		if( nmatch > n2match)
		{
			printf("\n -------Shifted-P is better \n");	
			if(hasMotion(_uncompressed))
			{
				 doBlend(_uncompressed,data);
				 printf(" but there is still motion \n");
			}
			else
				data->duplicate(_uncompressed);

		}
		else
		{
			printf("\n -------Shifted-O is better \n");
			interleave(cur,_uncompressed,1);
			interleave(next,_uncompressed,0);
			if(hasMotion(_uncompressed))
			{
				 doBlend(_uncompressed,data);
				 printf(" but there is still motion \n");
			}
			else
				data->duplicate(_uncompressed);
		}
		// which chroma is better ? from current or from next ?
		// search for a transition and see if there is also one ?
		vidCache->unlockAll();				
		return 1;						
}

/*
   	Returns the number of difference (interlacing) found


*/
uint32_t      ADMVideoTelecide::getMatch( ADMImage *image )
{

			uint32_t m=0,x,y;

			uint8_t *p,*n,*c;

			p=YPLANE(image) ;
			c=p+ _info.width;
			n=c+ _info.width;
			
			for(y=_info.height>>2;  y >2 ; y--)
			{
           			for(x=_info.width;x>0;x--)
				{
					if(  (*c-*p)*(*c-*n) >MATCH_THRESH) 
						m++;
					p++;c++;n++;
				}
				p+=3*_info.width;
				c+=3*_info.width;
				n+=3*_info.width;

			}

                 return m;
}

