//
// C++ Implementation:Spudecoder (subs for DVd like stream)
//
// Description:
//
//
// Author: Mean, fixounet@free.fr
//
// Copyright: See COPYING file that comes with this distribution
//
// see http://sam.zoy.org/writings/dvd/subtitles/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"
#include "ADM_default.h"
#include "ADM_videoFilter.h"
#include "DIA_fileSel.h"
#include "ADM_colorspace.h"
#include "ADM_vobsubinfo.h"
#define VOBSUB "/capture/sub/phone.sub"
#include "ADM_videoFilterDynamic.h"
#include "ADM_vidVobSub.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_FILTER
#include "ADM_osSupport/ADM_debug.h"
#include <math.h>

extern uint8_t DIA_vobsub(vobSubParam *param);

static FILTER_PARAM vobsubParam={3,{"subname","index","subShift"}};
//*************************************************************
//

//*************************************************************
SCRIPT_CREATE(vobsub_script,ADMVideoVobSub,vobsubParam);
BUILD_CREATE(vobsub_create,ADMVideoVobSub);
//*************************************************************
uint8_t ADMVideoVobSub::configure(AVDMGenericVideoStream *in)
{

       
  if(DIA_vobsub(_param))
       {
                cleanup();
                setup();
                return 1;
       }
        
        return 0;        
}
 
//*************************************************************
char *ADMVideoVobSub::printConf( void )
{
    ADM_FILTER_DECLARE_CONF(" VobSub");
        
}
//*************************************************************
ADMVideoVobSub::ADMVideoVobSub(  AVDMGenericVideoStream *in,CONFcouple *couples)
{

        _in=in;         
        memcpy(&_info,_in->getInfo(),sizeof(_info));    
        _info.encoding=1;       
        _parser=NULL;  
        _resampled=NULL;
        _chromaResampled=NULL;
        _original=NULL;        
        
        _param=NEW(vobSubParam);
        
        if(couples)
        {                 
                GET(subname);
                GET(index);                               
                GET(subShift);  
        }
        else
        {
#ifdef KK_ADM_DEBUG
                _param->subname=ADM_strdup(VOBSUB);
#else                
                _param->subname =NULL;
#endif                
                _param->index = 0;   
                _param->subShift=0;             
        }
        
        setup();
}
/*
    Alternate constructor for use by OCR
*/
ADMVideoVobSub::ADMVideoVobSub(  char *fileidx,uint32_t idx)
{

        _in=NULL;         
        memset(&_info,0,sizeof(_info));    
        _info.encoding=1;       
        _parser=NULL;  
        _resampled=NULL;
        _chromaResampled=NULL;
        _original=NULL;        
        
        _param=NEW(vobSubParam);
        
        
        _param->subname=ADM_strdup(fileidx);
        _param->index = idx;   
        _param->subShift=0;                    
        
        setup();
}
/*
    Returns bitmap & info for the Nth subs
*/

vobSubBitmap *ADMVideoVobSub::getBitmap(uint32_t nb,uint32_t *start, uint32_t *end,uint32_t *first,uint32_t *last)
{
uint32_t top=0,bottom=0;
    ADM_assert(_vobSubInfo);
    ADM_assert(nb<_vobSubInfo->nbLines);
    
    // Seek & decode
    //_parser->_asyncJump2(0,_vobSubInfo->lines[nb].fileOffset);
    _parser->setPos(_vobSubInfo->lines[nb].fileOffset,0);
    if(!handleSub(nb))
        {
                printf("Error reading getBimap\n");
                *first=*last=0;
                return _original; // might be null (?)
        }
    *first=*last=0;
    if(_original)
    {
    uint32_t ox,oy;
        _original->buildYUV(_YUVPalette);
        ox=_original->_width;        
        oy=_original->_height;
        printf("Original :%lu x %lu (original  %d)\n",ox,oy,_vobSubInfo->height);
        ADM_assert(oy<=_vobSubInfo->height);
        
        // Search the 1st/last non null line
//#define DONTCLIP
#ifdef DONTCLIP     
         *first=0;
         *last=oy-1;   
#else
        if(oy>_original->_height) oy=_original->_height-1;

        while(top<oy && !_original->isDirty(top) ) top++;
        
        if(top==oy)
        {
                top=bottom=0;   // Empty bitmap ?
                *first=top;
                *last=bottom;
                printf("Empry bitmap\n");
                return NULL; 
        }
        bottom=oy-1;
        while(bottom>top && !_original->isDirty(bottom)) bottom--;
        
        // If true it means we have 2 subs, one on top, one on bottom
        //
#if 0
        if(bottom>(oy>>1) && top<(oy>>1) && (bottom-top>(oy>>1)))
        {
          // in that case, take only the lower one
          top=oy>>1;
          while(top<oy && !_original->isDirty(top)) top++;                    
        }
        printf("> clipped: %lu / %lu=%lu\n",top,bottom,bottom-top+1);
#endif
        *first=top;
        *last=bottom;
#endif
    }
    *start=_vobSubInfo->lines[nb].startTime;
    *end=_vobSubInfo->lines[nb].stopTime;
    return _original;
}
/*
    Returns the nb of lines found in the sub
*/
uint32_t     ADMVideoVobSub::getNbImage( void)
{
    if(!_parser) return 0;
    if(!_param) return 0;
    if(!_vobSubInfo) return 0;
    return _vobSubInfo->nbLines;

}
//************************************
uint8_t ADMVideoVobSub::setup(void)
{
  char *dup;
  int l;
  
   _vobSubInfo=NULL;
   if(_param->subname && strlen(_param->subname)>5)
        {
                printf("Opening %s\n",_param->subname);
                dup=ADM_strdup(_param->subname);
                l=strlen(dup);
                if(l>5)
                  if(dup[l-4]=='.')
                {
                  dup[l-3]='s';
                  dup[l-2]='u';
                  dup[l-1]='b';
                          
                }
                if(vobSubRead(_param->subname,_param->index,&_vobSubInfo))
                {
                        printf("Opening index \n");
                       
                        MPEG_TRACK track;
                        memset(&track,0,sizeof(track));
                        track.pes=_param->index+0x20;
                        track.pid=0;
                       // _parser=new ADM_mpegDemuxerProgramStream(_param->index+0x20,0xe0);
                        _parser=new dmx_demuxerPS(1,&track,0);
                        if(!_parser->open(dup))
                        {
                                printf("Mpeg Parser : opening %s failed\n",_param->subname);
                                delete _parser;
                                _parser=NULL;
                
                         }
                         
                }
                ADM_dealloc(dup);
        }
        
        
        if(!_parser)
        {
                printf("opening of vobsub file failed\n");
        }
         else
        {       // Recompute sub duration
                uint32_t end;
                vobSubLine *cur,*next;
                // Assuming max displat time = MAX_DISPLAY_TIME
                for(uint32_t i=0;i<_vobSubInfo->nbLines-1;i++)
                {
                        if(i && !_vobSubInfo->lines[i].startTime)
                        {
                                _vobSubInfo->lines[i].startTime=0xf0000000;
                                _vobSubInfo->lines[i].stopTime=0xf0000001;
                                
                        }
                        else
                        {
                                cur=&_vobSubInfo->lines[i];
                                next=&_vobSubInfo->lines[i+1];
                        
                                end=cur->startTime+MAX_DISPLAY_TIME;
                                if(end>=next->startTime) end=next->startTime-1;
                                cur->stopTime=end;                                
                        }
                }
                _vobSubInfo->lines[_vobSubInfo->nbLines-1].stopTime=
                        MAX_DISPLAY_TIME+_vobSubInfo->lines[_vobSubInfo->nbLines-1].startTime;
                // Convert all the palette from RGB to YUV
                paletteYUV();
        }
      
        _x1=_y1=_x2=_y2=0;
        _data=new uint8_t [VS_MAXPACKET];
        _subSize=0;
        _subW=_subH=0;
        
        memset(&_original,0,sizeof(_original));
        _currentSub=NOSUB;
        _initialPts=0;
                

}
//*************************************************************
uint8_t ADMVideoVobSub::cleanup(void)
{

        if(_parser) delete _parser;
        _parser=NULL;
        
        if(_original)
          delete _original;
        _original=NULL;        
        
        if(_resampled)
          delete _resampled;
        _resampled=NULL;
        
        if(_chromaResampled)
          delete _chromaResampled;
        _chromaResampled=NULL;
        
        
        if(_data) delete [] _data;
        _data=NULL;
        
        if(_vobSubInfo) destroySubInfo( _vobSubInfo);
        _vobSubInfo=NULL;
        

}
//*************************************************************
ADMVideoVobSub::~ADMVideoVobSub()
{
        cleanup();
         if(_param)
        {
                if(_param->subname)  ADM_dealloc(_param->subname);
                DELETE(_param);
        }
        _param=NULL;
}

//*************************************************************
uint8_t ADMVideoVobSub::getCoupledConf( CONFcouple **couples)
{
                        ADM_assert(_param);
                        *couples=new CONFcouple(3);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
                        if(_param->subname)
                                CSET(subname);
                        else
                          (*couples)->setCouple("subname","none") ;
                        CSET(index);
                        CSET(subShift);
                        

                        return 1;
}
//*************************************************************
uint8_t ADMVideoVobSub::getFrameNumberNoAlloc(uint32_t frame,
                                uint32_t *len,
                                ADMImage *data,
                                uint32_t *flags)
{

uint64_t time;
uint32_t sub;

        if(frame>= _info.nb_frames) return 0;
        // read uncompressed frame
        if(!_in->getFrameNumberNoAlloc(frame, len,data,flags)) return 0;

        if(!_parser)        
        {
                //
                printf("No valid vobsub to process\n");
                
                return 1;
        
        }
        
        time=(frame+_info.orgFrame);
        time=(time*1000*1000)/_info.fps1000;
        
        sub=lookupSub(time);  
        // Should we re-use the current one ? 
        if(sub==NOSUB)
        {
                aprintf("No matching sub for time %llu frame%lu\n",time,frame);
                return 1;
        }
        // If it is a new sub, decode it...
        if(sub!=_currentSub )
        {                
                // _parser->_asyncJump2(0,_vobSubInfo->lines[sub].fileOffset);
                _parser->setPos(_vobSubInfo->lines[sub].fileOffset,0);
                 //_initialPts=_parser->getPTS();
                 handleSub(sub);
                _currentSub=sub;
                Palettte2Display(); // Create the bitmap
                // Time to resize the bitmap
                // First try : Do it bluntly
                
        }                
        
        // and if there is something to display, display it
        //
        vobSubBitmap *src;
        //src=_original;
        src=_resampled;
        
        if(src)          
        {
               
          aprintf("We have %lu %lu to merge\n",src->_width,src->_height);
                // Merge
                
                uint32_t stridein,strideout,len;
                uint8_t *in,*out,*mask,*in2;
                uint16_t old,nw,alp;
                uint32_t xx,yy;
                
                stridein=src->_width;
                strideout=_info.width;
                
                if(strideout>stridein)
                {
                        len=stridein;
                        xx=src->_width;
                }
                else
                {
                        xx=_info.width;
                        len=strideout;
                }
                if(src->_height>_info.height) yy=_info.height;
                        else                 yy=src->_height;           
                in=src->_bitmap;
                mask=src->_alphaMask;
               
                out=data->data+_info.width*src->placeTop;
                // auto center
                uint32_t center=_info.width-src->_width;
                out+=(center>>2)*2;
                
                for(uint32_t y=0;y<yy;y++)
                {
                  for(uint32_t x=0;x<xx;x++)
                        {
                               old=out[x];
                               nw=in[x];
                               alp=mask[x];

                                if(alp) 
                                {
                                  if(alp>7)  nw=old*(16-alp-1)+(alp+1)*nw;
                                        else nw=old*(16-alp)+(alp)*nw;
                                  out[x]=nw>>4;                                         
                                }

                               //out[x]=nw;
                        }
                        //memcpy(out,in,len);
                        out+=strideout;
                        in+=stridein;
                        mask+=stridein;
                }
                
                // Now do chroma u & chroma V
#define DOCHROMA                
#if defined(DOCHROMA)                
                uint32_t crosspage=(_info.width*_info.height)>>2;
                
                strideout=_info.width>>1;
                stridein=_chromaResampled->_width;
                
                out=data->data+_info.width*_info.height;
                out+=(src->placeTop>>1)*(_info.width>>1);
                mask=_chromaResampled->_alphaMask;
                // Center
                out+=(center>>2)*1;
                if(strideout>stridein) xx=stridein;
                else            xx=strideout;
                
                int16_t left=(_info.height>>1)-(_chromaResampled->_height+(_original->placeTop>>1));
                
                if(left<_chromaResampled->_height) yy=left;
                else yy=_chromaResampled->_height;
                
                for(uint32_t y=0;y<yy;y++)
                {
                   for(uint32_t x=0;x<xx;x++)
                   {
                        if(mask[x]>10)         
                        {
                          int16_t val;
                          val=out[x];
                          val-=128;
                         
                          
                          
                          nw=val*(16-alp);
                          
                          val/=4;
                          val=val+128;
                                
                          out[x]=val; 
                          out[crosspage+x]=val;
                        }                                                 
                   }
                   out+=strideout;
                   mask+=stridein;
                }
#endif                
        }
        return 1;
}
//*************************************************************************
//
//      Convert the original bitmap to a rescaled & repositionned one
//      that will be blended into the current picutr
//
//*************************************************************************
uint8_t ADMVideoVobSub::Palettte2Display( void )
{
        ADM_assert(_parser);
        ADM_assert(_vobSubInfo);
     
        // Then Process the RLE Datas
        // To get the _bitmap yuv data
        ADM_assert(_original);
        
        // Set correct color
        _original->buildYUV(_YUVPalette);
        
        // rebuild the scaled one
        // Compute the target size
        uint32_t fx,fy;
        uint32_t ox,oy;
        uint32_t sx,sy;
        /*
                Fx, fy : Final size of the image (i.e size of the current picture)
                ox,oy  : Original size of the image where the sub is coming from
                sx,sy  : Size of the  sub
        
                And we want the final size of the sub
                        + coordinates but that we will do later
        
        */
        
        fx=_info.width;
        fy=_info.height;
        
        ox=_vobSubInfo->width;
        oy=_vobSubInfo->height;
        
        sx=_subW;
        sy=_subH;
        
        // Search the 1st/last non null line
        uint32_t top=0,bottom=0;

        if(oy>_original->_height) oy=_original->_height-1;

        while(top<oy && !_original->isDirty(top)) top++;
        
        bottom=_original->_height-1;
        if(top==bottom)
        {
                printf("Empty sub ?");
                return 0;
        }
        
        while(bottom && !_original->isDirty(bottom)) bottom--;
        
        // If true it means we have 2 subs, one on top, one on bottom
        //
        if(bottom>(oy>>1) && top<(oy>>1) && (bottom-top>(oy>>1)))
        {
          // in that case, take only the lower one
          top=oy>>1;
          while(top<oy && !_original->isDirty(top)) top++;                    
        }
        //
        //  The useful part is between top & bottom lines
        //
        
        // Shrink factor
        // The shrink factor is the one used to shrink from the original video
        // to the resize video
        
        double scale,l;
        scale=fx;
        scale/=ox;
        printf("top %lu : bottom :%lu Scale :%f ox:%lu oy:%lu fx:%lu \n",top,bottom,scale,ox,oy,fx);
        
        // We rescale the sub by the same factor
        // Only the visible / useful part
        l=scale;
        l=l*sx;
        sx=(uint32_t )floor(l);
        
       
        l=scale;
        l=l*(bottom-top);
        sy=(uint32_t )floor(l);
        

        // And we resize that useful part of sub
        // to our final bitmap (resampled)
                
        _original->subResize(&_resampled,sx,sy,top, bottom-top);
                
        uint32_t tail;
        
        // Set the position of the sub so that it is ok
        
        tail=16+sy;
        
        if(tail>fy) tail=0;
        else
        {
            tail=fy-tail;           
        }
        
        _resampled->placeTop=tail;
        
        _resampled->subResize(&_chromaResampled,sx>>1,sy>>1,0,sy);
        return 1;
}

//
//      Return the index in the sub table of the sub matching the time
//
uint32_t ADMVideoVobSub::lookupSub(uint64_t time)
{
int64_t head,tail, cur;
int32_t i;
        cur=(int64_t)time;
        i=0;
        while(i<_vobSubInfo->nbLines-1)
        {
                head=(int64_t)_vobSubInfo->lines[i].startTime;
                tail=(int64_t) _vobSubInfo->lines[i].stopTime;
                head+=_param->subShift;
                tail+=_param->subShift;
                if(head<=cur &&tail>cur)
                {
                  aprintf("Matching for time %llu : sub %lu starting at :%lu (shift %lu)\n",
                                        time,i,_vobSubInfo->lines[i].startTime,_param->subShift);
                        return i; 
                }   
                if(head>cur) return NOSUB;                   
                i++;       
        }
        return NOSUB;


}
//EOF

