/***************************************************************************
                         DVB-T subtitle filter
    
    This is a dummy DVB-T Filter and should be only used through OCR
    The filter exists just for debugging purpose and should not be exposed
    to end user at all.
    
    copyright            : (C) 2007 by mean
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



#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ADM_lavcodec.h"
#include "ADM_assert.h"
#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"
#include "fourcc.h"


#include "ADM_videoFilter.h"
#include "ADM_colorspace.h"
#include "ADM_codecs/ADM_codec.h"
#include "ADM_vidDvbSub.h"
static FILTER_PARAM assParam={7,
        { /* float */ "font_scale",
          /*float*/ "line_spacing",
          /* int */ "top_margin",
          /* int */ "bottom_margin",
          /* bool */ "extract_embedded_fonts",
          /* ADM_filename */ "fonts_dir",
          /* ADM_filename */ "subfile" }};

SCRIPT_CREATE(dvbsub_script,ADMVideoSubDVB,assParam);
BUILD_CREATE(dvbsub_create,ADMVideoSubDVB);

char *ADMVideoSubDVB::printConf() 
{
    ADM_FILTER_DECLARE_CONF(" DVB Subtitle ");
      
      
}


uint8_t ADMVideoSubDVB::configure(AVDMGenericVideoStream * instream)
{
  
   return 1;
}
//_______________________________________________________________

ADMVideoSubDVB::ADMVideoSubDVB(const char *ts, uint32_t pid,uint32_t w, uint32_t h) 
{
        _in=NULL;		
        memset(&_info,0,sizeof(_info));
        _info.width=w;
        _info.height=h;
        _inited=0;
        
      
        _uncompressed=new ADMImage(getInfo()->width,getInfo()->height);
        ADM_assert(_uncompressed);
        _info.encoding=1;
        
        decoder=new decoderFFSubs(1);
        binary=new ADMCompressedImage;
        binary->data=readBuffer+2;
        
        MPEG_TRACK track;
        track.pid=pid;

        
        demuxer=new dmx_demuxerTS(1,&track,0,DMX_MPG_TS);
      

      
}
//_______________________________________________________________

ADMVideoSubDVB::ADMVideoSubDVB(AVDMGenericVideoStream *in, CONFcouple *conf) 
{
        _in=in;		
        memcpy(&_info,_in->getInfo(),sizeof(_info));
        
        
      
        _uncompressed=new ADMImage(_in->getInfo()->width,_in->getInfo()->height);
        ADM_assert(_uncompressed);
        _info.encoding=1;
        
        decoder=new decoderFFSubs(1);
        binary=new ADMCompressedImage;
        binary->data=readBuffer+2;
        
        MPEG_TRACK track;
        track.pid=0x96;

        
        demuxer=new dmx_demuxerTS(1,&track,0,DMX_MPG_TS);
        _inited=init(NULL);

      
}
// **********************************
uint8_t ADMVideoSubDVB::init(const char  *tsFileName)
{
	// For test only!
#ifdef ADM_DEBUG
	if(!tsFileName) tsFileName="/capture/grey/Grey_anatomy_2007_05_22_Avec_Le_Temp.mpg";
#endif
    // First create our demuxer
     
     if(!demuxer->open(tsFileName))
     {
        printf("Cannot open TS\n");
        return 0; 
     }
      memset(&sub,0,sizeof(sub));
      _inited=1;
     return 1;
} 

//*******************************************
ADMVideoSubDVB::~ADMVideoSubDVB() 
{
      if(_uncompressed) DELETE(_uncompressed);
      if(decoder) delete decoder;
      if(binary) delete binary;
      if(demuxer) delete demuxer;
      decoder=NULL;
      binary=NULL;
      demuxer=NULL;
   
}
//*******************************************
#define _r(c)  ((c)&0xff)
#define _b(c)  (((c)>>16)&0xFF)
#define _g(c)  (((c)>>8)&0xFF)
#define _a(c)  ((c)>>24)
#define rgba2y(c)  ( (( 263*_r(c)  + 516*_g(c) + 100*_b(c)) >> 10) + 16  )
#define rgba2u(c)  ( (( 450*_r(c) - 376*_g(c) -  73*_b(c)) >> 10) + 128 )
#define rgba2v(c)  ( ((-152*_r(c) - 298*_g(c) + 450*_b(c)) >> 10) + 128 )

uint8_t ADMVideoSubDVB::getFrameNumberNoAlloc(uint32_t frame, uint32_t *len, ADMImage *data, uint32_t *flags) 
{
uint8_t *org=NULL;

      // Read the original PIC
        
        if(!_in->getFrameNumberNoAlloc(frame, len, data, flags))
                return 0;

        if(!_inited) 
        {
          return 0; 
        }
        // Read the compressed DVB....
        
        uint32_t packetLen,dts,pts;
          if(!demuxer->readPes(readBuffer,&packetLen, &dts,&pts)) return 1;
          binary->dataLength=packetLen-3; // -2 for stream iD, -1 for ????
          if(packetLen<=5) return 1;
          // And decompress...
          decoder->uncompress(binary,&sub);
            
          // Process All rectangles
          
            printf("Found %d rects to process\n",sub.num_rects);
            for(int i=0;i<sub.num_rects;i++)
            {
              AVSubtitleRect *r=&(sub.rects[i]);
              // First convert RGB to Y+ALPHA
              for(int col=0;col<r->nb_colors;col++)
              {
                    // Color is RGB, convert to YUV
                    uint32_t y,u,v,a;
                    uint32_t rgb=r->rgba_palette[col];
                
                          y=rgba2y(rgb);
                          u=rgba2u(rgb)&0xff;
                          v=rgba2v(rgb)&0xff;
                          a=_a(rgb);
                          r->rgba_palette[col]=y+(u<<8)+(v<<16)+(a<<24);
                          printf("Color %d, alpha %u luma %u rgb:%x\n",col,a,y,rgb);
              }
              // Palette is ready, display !
              if(r->x>_info.width || r->y>_info.height)
              {
                  printf("[DVBSUB]Box is outside image\n");
                  goto _skip;
              }
#if 0
                  printf("x :%d\n",r->x);
                  printf("y :%d\n",r->y);
                  printf("w :%d\n",r->w);
                  printf("h :%d\n",r->h);
#endif
                  {
                      uint32_t clipW,clipH;
                      
                      clipW=FFMIN(_info.width,r->x+r->w)-r->x;
                      clipH=FFMIN(_info.height,r->y+r->h)-r->y;
                      
                      ADMImage image(r->w,r->h);
                      ADMImage imageU(r->w,r->h);
                      ADMImage imageV(r->w,r->h);
                      ADMImage alphaImage(r->w,r->h);
                      
                      uint8_t *ptr=image.data;
                      uint8_t *ptrU=imageU.data;
                      uint8_t *ptrV=imageV.data;
                      uint8_t *ptrAlpha=alphaImage.data;
                      uint8_t *in=r->bitmap;
                      for(int yy=0;yy<r->h;yy++)
                      {
                          for(int xx=0;xx<r->w;xx++)
                          {
                            uint32_t alpha,valout;
                            uint32_t val=*in++;
                            
                                  val=r->rgba_palette[val];
                                  
                                  *ptrAlpha++=(val>>24)&0xff;
                                  *ptr++=(val&0xff);;
                                  *ptrU++=(val>>8)&0xff;
                                  *ptrV++=(val>>16)&0xff;
                          }
                      }
              
                      // Merge Luma
                      for(int yy=0;yy<clipH;yy++)
                      {
                          org=data->data+(yy+r->y)*_info.width+r->x;
                          
                          ptrAlpha=alphaImage.data+yy*r->w;
                          ptr=image.data+yy*r->w;
                        
                          for(int xx=0;xx<clipW;xx++)
                          {
                            uint32_t val,before,alpha;
                            
                                  before=*org;
                                  val=*ptr++;
                                  alpha=*ptrAlpha++;
                                  
                                  val=val*alpha+(255-alpha)*before;
                                  val>>=8;
                                  *org++=val;
                          }
                      }
                      // Shrink alpha  & u & v
                     alphaImage.LumaReduceBy2();
                     imageU.LumaReduceBy2();
                     imageV.LumaReduceBy2();
                     r->x>>=1;
                     r->y>>=1;
                     r->w>>=1;
                     r->h>>=1;
                     clipH>>=1;
                     clipW>>=1;
                     
                     
                      uint8_t *orgU=(uint8_t *)(UPLANE(data)+(r->y)*(_info.width>>1)+(r->x));
                      uint8_t *orgV=(uint8_t *)(VPLANE(data)+(r->y)*(_info.width>>1)+(r->x));
#if 1
                      // Merge
                      for(int yy=0;yy<clipH;yy++)
                      {
                          ptrAlpha=alphaImage.data+yy*(r->w);
                          ptrU=imageU.data+yy*r->w;
                          ptrV=imageV.data+yy*r->w;
                        
                          for(int xx=0;xx<clipW;xx++)
                          {
                            uint32_t val,valU,valV,before,alpha;
                            uint32_t newU,newV;
                                  
                                  newU=*ptrU++;
                                  newV=*ptrV++;  // New color
                                 
                                   newU=(newU+newV)/2;
                                   newV=newU;
                                  
                                  alpha=*ptrAlpha++;
                                  
                                  before=*orgU; // old color
                                  valU=newU*alpha+(255-alpha)*before;
                                  
                                  before=*orgV;
                                  valV=newV*alpha+(255-alpha)*before;
                                  
                                  valU=valU>>8;
                                  valV=valV>>8;
                                  
                                  *orgU++=valU;
                                  *orgV++=valV;
                          }
                      }
#endif
              }
               // Delete palette & data
_skip:
               av_free(r->rgba_palette);
               av_free(r->bitmap);
            } // Next rec..
            memset(&sub,0,sizeof(sub));
        
        return 1;
}
/**
 * \fn 			getNextBitmap
 * \brief 		Decode a bitmap and store the result (luma only) in the caller supplied vobSubBitmap
 * @param data  (in) Vobsubbitmap to put image in
 * @param pts   Raw pts in 90 Khz Tick
 * @return      0 on failure, 1 on success 
 * */
uint8_t ADMVideoSubDVB::getNextBitmap(vobSubBitmap *data,uint32_t *pts) 
{
uint8_t *org=NULL;

        uint32_t packetLen,dts; //,pts;
        // Clear incoming picture
        	data->clear();
        
          if(!demuxer->readPes(readBuffer,&packetLen, &dts,pts)) return 0;
          binary->dataLength=packetLen-3; // -2 for stream iD, -1 for ????
          if(packetLen<=5) return 1;
          // And decompress...
          decoder->uncompress(binary,&sub);
            
          // Process All rectangles
          
            printf("Found %d rects to process\n",sub.num_rects);
            for(int i=0;i<sub.num_rects;i++)
            {
              AVSubtitleRect *r=&(sub.rects[i]);
              // First convert RGB to Y+ALPHA
              for(int col=0;col<r->nb_colors;col++)
              {
                    // Color is RGB, convert to YUV
                    uint32_t y,u,v,a;
                    uint32_t rgb=r->rgba_palette[col];
                
                          y=rgba2y(rgb);
                          u=rgba2u(rgb)&0xff;
                          v=rgba2v(rgb)&0xff;
                          a=_a(rgb);
                          r->rgba_palette[col]=y+(u<<8)+(v<<16)+(a<<24);
#if 0
                          printf("Color %d, alpha %u luma %u rgb:%x\n",col,a,y,rgb);
#endif
              }
              // Palette is ready, display !
              if(r->x>_info.width || r->y>_info.height)
              {
                  printf("[DVBSUB]Box is outside image\n");
                  goto _skipX;
              }
#if 0
                  printf("x :%d\n",r->x);
                  printf("y :%d\n",r->y);
                  printf("w :%d\n",r->w);
                  printf("h :%d\n",r->h);
#endif
                  {
                      uint32_t clipW,clipH;
                      
                      clipW=FFMIN(_info.width,r->x+r->w)-r->x;
                      clipH=FFMIN(_info.height,r->y+r->h)-r->y;
                      
                      ADMImage image(r->w,r->h);
                      ADMImage alphaImage(r->w,r->h);
                      
                      uint8_t *ptr=image.data;
                      uint8_t *ptrAlpha=alphaImage.data;
                      uint8_t *in=r->bitmap;
                      for(int yy=0;yy<r->h;yy++)
                      {
                          for(int xx=0;xx<r->w;xx++)
                          {
                            uint32_t alpha,valout;
                            uint32_t val=*in++;
                            
                                  val=r->rgba_palette[val];
                                  
                                  *ptrAlpha++=(val>>24)&0xff;
                                  *ptr++=(val&0xff);;
                          }
                      }
              
                      // Merge Luma
                      
                      for(int yy=0;yy<clipH;yy++)
                      {
                          org=data->_bitmap+(yy+r->y)*_info.width+r->x;
                          
                          ptrAlpha=alphaImage.data+yy*r->w;
                          ptr=image.data+yy*r->w;
                          int clean=0;
                          for(int xx=0;xx<clipW;xx++)
                          {
                            uint32_t val,before,alpha;
                            
                                  //before=*org;
                                  val=*ptr++;
                                  alpha=*ptrAlpha++;
                                  
                                  val=val*alpha;//+(255-alpha)*before;
                                  val=val>>8;
                                  if(val>10) clean=1; // Remove noise
                                  *org++=val;
                                  
                          }
                          if(clean) data->setDirty(r->y+yy);
                      }
                  }
                   // We dont need chroma here...
               // Delete palette & data
_skipX:
               av_free(r->rgba_palette);
               av_free(r->bitmap);
            } // Next rec..
            memset(&sub,0,sizeof(sub));
        
        return 1;
}
uint8_t	ADMVideoSubDVB::getCoupledConf(CONFcouple **conf) 
{

        *conf=new CONFcouple(0);
        return 1;
}
/************************************************/
