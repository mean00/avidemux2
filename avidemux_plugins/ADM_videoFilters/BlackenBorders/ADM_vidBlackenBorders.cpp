/***************************************************************************
                          ADM_vidBSMear.cpp  -  description
                             -------------------
         change part of video into black borders

          Each one ,must be even

          Copy / Paste from crop,almost the same thing


    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by mean
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
#include "DIA_coreToolkit.h"
#include "ADM_videoFilterDynamic.h"

#include "DIA_factory.h"
typedef struct
{
        uint32_t left,right;
        uint32_t top,bottom;
}BLACKEN_PARAMS;
class  AVDMVideoStreamBSMear:public AVDMGenericVideoStream
 {

 protected:
                BLACKEN_PARAMS          *_param;

 public:

                                AVDMVideoStreamBSMear(  AVDMGenericVideoStream *in,CONFcouple *setup);
                virtual         ~AVDMVideoStreamBSMear();
                virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                                                        ADMImage *data,uint32_t *flags);
                virtual         char            *printConf(void) ;
                                uint8_t         configure( AVDMGenericVideoStream *instream);
                                uint8_t getCoupledConf( CONFcouple **couples);
 }     ;

static FILTER_PARAM cropParam={4,{"left","right","top","bottom"}};

//********************************************
VF_DEFINE_FILTER(AVDMVideoStreamBSMear,cropParam,
    blacken,
                QT_TR_NOOP("Blacken borders"),
                1,
                VF_TRANSFORM,
                QT_TR_NOOP("Fill borders with pure black. Doesn't alter size."));
//********************************************
char *AVDMVideoStreamBSMear::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" Black l:%lu  r:%lu  u:%lu x d:%lu",
 				_param->left,
 					_param->right,
 					_param->top,
 					_param->bottom);
        
}

//_______________________________________________________________

AVDMVideoStreamBSMear::AVDMVideoStreamBSMear(  	AVDMGenericVideoStream *in,CONFcouple *couples)
{


  	_in=in;		
   	memcpy(&_info,_in->getInfo(),sizeof(_info));  		

		if(couples)
		{
			_param=NEW(BLACKEN_PARAMS);
			GET(left);
			GET(right);
			GET(top);
			GET(bottom);
		}	
			else 	
		{	// default parameter	
				_param=NEW(BLACKEN_PARAMS);
				_param->left=_param->top=
						_param->right=_param->bottom=0;
		}										
 	
  _info.encoding=1;

  	  	
}

uint8_t	AVDMVideoStreamBSMear::getCoupledConf( CONFcouple **couples)
{

			ADM_assert(_param);
			*couples=new CONFcouple(4);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
	CSET(left);
	CSET(right);
	CSET(top);
	CSET(bottom);
			return 1;

}
AVDMVideoStreamBSMear::~AVDMVideoStreamBSMear()
{
 	DELETE(_param);
 	
}

//
//	Blacken borders, just setting luma to null should be enough
//

uint8_t AVDMVideoStreamBSMear::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{

			if(frame>=_info.nb_frames) 
			{
				printf("Filter : out of bound!\n");
				return 0;
			}
	
			ADM_assert(_param);									
								
			// read uncompressed frame directly into follower
			// and blacken there
			
       		if(!_in->getFrameNumberNoAlloc(frame, len,data,flags)) return 0;
       		  *len= _info.width*_info.height+(_info.width*_info.height>>1);       			
       		  // blacken top
       		  uint8_t *srcY=YPLANE(data);
		  uint8_t *srcU=UPLANE(data);
		  uint8_t *srcV=VPLANE(data);
       		  uint32_t bytes=_info.width*_param->top;
		  uint32_t page=_info.width*_info.height;
       		
       		  memset(srcY,0x10,bytes);
		  memset(srcU,0x80,bytes>>2);
		  memset(srcV,0x80,bytes>>2);
       		  // left & right
       		  uint32_t stride=_info.width;
		  
       		  for(uint32_t y=_info.height;y>0;y--)
       		  {
       		        memset(srcY,0x10,_param->left);
       		        memset(srcY+stride-_param->right,0,_param->right);       		
       		        srcY+=stride;       		
		 }
		 for(uint32_t y=_info.height>>1;y>0;y--)
       		  {
       		        
			memset(srcU,0x80,_param->left>>1);
			memset(srcV,0x80,_param->left>>1);
			memset(srcU+((stride-_param->right)>>1),0x80,_param->right>>1);
			memset(srcV+((stride-_param->right)>>1),0x80,_param->right>>1);
			srcU+=stride>>1;
			srcV+=stride>>1;
       		  }
       		
       		  // backen bottom
       		  srcY=YPLANE(data)+_info.width*_info.height-1;
       		
       		 bytes=_info.width*_param->bottom;
       	 	 srcY-=bytes;
       		 memset(srcY,0x10,bytes);
		// chroma
		 srcU=UPLANE(data)+(page>>2)-1;
		 srcU-=bytes>>2;
       		 memset(srcU,0x80,bytes>>2);
		 
		 srcV=VPLANE(data)+(page>>2)-1;
		 srcV-=bytes>>2;
       		 memset(srcV,0x80,bytes>>2);
		 
		  
       		  	
       		         		       		
      return 1;
}
uint8_t AVDMVideoStreamBSMear::configure(AVDMGenericVideoStream *in)
{
	_in=in;
	ADM_assert(_param);
        uint32_t width,height;
#define MAKEME(x) uint32_t x=_param->x;
        while(1)
        {
          MAKEME(left);
          MAKEME(right);
          MAKEME(top);
          MAKEME(bottom);
          
          width=_in->getInfo()->width;
          height=_in->getInfo()->height;
          
          diaElemUInteger dleft(&left, QT_TR_NOOP("_Left border:"), 0,width);
          diaElemUInteger dright(&right, QT_TR_NOOP("_Right border:"), 0,width);
          diaElemUInteger dtop(&(top), QT_TR_NOOP("_Top border:"), 0,height);
          diaElemUInteger dbottom(&(bottom), QT_TR_NOOP("_Bottom border:"), 0,height);
            
          diaElem *elems[4]={&dleft,&dright,&dtop,&dbottom};
          if(diaFactoryRun(QT_TR_NOOP("Blacken Borders"),4,elems))
          {
            if((left&1) || (right&1)|| (top&1) || (bottom&1) ||
                     (top+bottom>=height)|| (left+right>width))
            {
              GUI_Error_HIG(QT_TR_NOOP("Incorrect parameters"), QT_TR_NOOP("All parameters must be even and within range."));
              continue;
            }
            else
            {
  #undef MAKEME
  #define MAKEME(x) _param->x=x;
                MAKEME(left);
                MAKEME(right);
                MAKEME(top);
                MAKEME(bottom);
                _info.width=width+left+right;
                _info.height=height+top+bottom;
                return 1;
            }
          }
          return 0;
      }
}


