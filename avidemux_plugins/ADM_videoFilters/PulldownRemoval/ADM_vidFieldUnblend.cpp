/***************************************************************************
                         
        Hard ivtc removal for image

        A B C D E -> A BC CD D E

    copyright            : (C) 2005 by mean
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
#include <string.h>
#include "ADM_default.h"
#include <math.h>

#include "DIA_coreToolkit.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"


#include "DIA_enter.h"

#define MUL 1
// Set it to 2 for post separate field

typedef struct HARD_IVTC_PARAM
{
        uint32_t threshold;
        uint32_t noise;
        uint32_t show;
}HARD_IVTC_PARAM;

class vidHardPDRemoval:public AVDMGenericVideoStream
{

protected:
  virtual char *printConf (void);
  VideoCache *vidCache;
  HARD_IVTC_PARAM *_param;
  uint32_t              _lastRemoved;
  ADMImage              *cand1,*cand2,*rebuild;
public:

                        vidHardPDRemoval (AVDMGenericVideoStream * in, CONFcouple * setup);
        virtual         ~vidHardPDRemoval ();
  virtual uint8_t getFrameNumberNoAlloc (uint32_t frame, uint32_t * len,
                                         ADMImage * data, uint32_t * flags);
  uint8_t configure (AVDMGenericVideoStream * instream);
  virtual uint8_t getCoupledConf (CONFcouple ** couples);

};

static FILTER_PARAM field_unblend_template =
  { 3,"threshold","show","noise"};

BUILD_CREATE (hardivtc_create, vidHardPDRemoval);
SCRIPT_CREATE (hardivtc_script, vidHardPDRemoval, field_unblend_template);
//*************************************
uint8_t vidHardPDRemoval::configure (AVDMGenericVideoStream * in)
{
int v,w;
        _param->show=GUI_YesNo(QT_TR_NOOP("Metrics"),QT_TR_NOOP("Do you want to print metrics on screen ?" ));
        v=_param->threshold;
        w=_param->noise;        
        if(DIA_GetIntegerValue(&v, 2, 99,"Treshold","Treshold value (smaller = harder to match)"))
        {
                if(DIA_GetIntegerValue(&w, 2, 99,"Noise","Noise threshold"))
                {
                	_param->threshold=v;
                    _param->noise=w;
                    _lastRemoved=0xFFFF;
                    return 1;
                }
        }
        return 0;
}
/*************************************/
char *vidHardPDRemoval::printConf (void)
{
  ADM_FILTER_DECLARE_CONF(" Field Unblend Thresh:%d Noise:%d",_param->threshold,_param->noise);
}

#define MAX_BLOCKS 50
/*************************************/
vidHardPDRemoval::vidHardPDRemoval (AVDMGenericVideoStream * in, CONFcouple * couples)
{

  _in = in;
  memcpy (&_info, _in->getInfo (), sizeof (_info));
  _info.encoding = 1;
  vidCache = new VideoCache (10, in);
  _uncompressed=new ADMImage(_info.width,_info.height);
  cand1=new ADMImage(_info.width,_info.height);
  cand2=new ADMImage(_info.width,_info.height);
  rebuild=new ADMImage(_info.width,_info.height);

 _param=new HARD_IVTC_PARAM;
 _lastRemoved=0xFFFF;
 if(couples)
 {
#undef GET
#define GET(x) couples->getCouple(#x,&(_param->x))
      GET (threshold);
      GET (show);
      GET (noise);
  }
  else
  {
        _param->threshold=10;
        _param->show=0;
        _param->noise=5;
  }
}
//____________________________________________________________________
vidHardPDRemoval::~vidHardPDRemoval ()
{

  delete vidCache;
  vidCache = NULL;
  delete _uncompressed;
  _uncompressed=NULL;
  delete _param;
  _param=NULL;
  delete cand1;
  delete cand2;
  delete rebuild;
  cand1=NULL;
  cand2=NULL;
  rebuild=NULL;
}
static void merge(ADMImage *src1,ADMImage *src2,ADMImage *tgt)
{
uint32_t ww,hh;
uint8_t *s1,*s2,*out;
int o;

        s1=YPLANE(src1);
        s2=YPLANE(src2);
        out=YPLANE(tgt);
        ww=src1->_width;
        hh=src1->_height;
        for(int y=0;y<hh;y++)
                for(int x=0;x<ww;x++)
                {
                        o=*s1+*s2;
                        o>>=1;
                        *out=o;
                        s1++;
                        s2++;
                        out++;

                }




}
static float computeDiff(ADMImage *src1,ADMImage *src2,uint32_t noise)
{
float df=0;
int delta;
uint32_t ww,hh;
uint8_t *s1,*s2;

        s1=YPLANE(src1);
        s2=YPLANE(src2);
        ww=src1->_width;
        hh=src1->_height;

          for(int y=0;y<hh;y++)
                for(int x=0;x<ww;x++)
                {
                        delta=abs(*s1-*s2);
                        if(delta>noise)
                                df+=delta;
                        s1++;
                        s2++;

                }
        return df;
}
static float computeDiff2(ADMImage *src1,ADMImage *src2,ADMImage *cand)
{
float df=0;
int delta;
uint32_t ww,hh;
uint8_t *s1,*s2,*d1;
int a1,a2,t1;
        s1=YPLANE(src1);
        s2=YPLANE(src2);
        
        d1=YPLANE(cand);
        ww=src1->_width;
        hh=src1->_height;

          for(int y=0;y<hh;y++)
                for(int x=0;x<ww;x++)
                {
                        a1=*s1;
                        a2=*s2;
                        t1=*d1;
                        if(a1==a2) ;
                        else
                        if(a1>a2)
                        {
                                if(t1 <=a1 && t1>=a2) df+=1;
                                        else df-=1;
                        }else
                                if(t1 <=a2 && t1>=a1) df+=1;
                                        else df-=1;
                        s1++;
                        s2++;
                        d1++;
                }
        return df;
}

// so srcR=2*src-srcP
static uint8_t tinyUnblend(uint8_t *dst, uint8_t *src1, uint8_t *src2,uint32_t w, uint32_t h)
{
int delta;
uint32_t ww,hh;
uint8_t *s1,*s2,*d1;
int a1,a2;
        s1=src1;
        s2=src2;
        
        d1=dst;
        ww=w;
        hh=h;

          for(int y=0;y<hh;y++)
                for(int x=0;x<ww;x++)
                {
                        a1=*s1;
                        a2=*s2;
                        a1=2*a1-a2;
                        if(a1<0) a1=0;
                        if(a1>255) a1=255;
                        *d1=a1;                         

                        s1++;
                        s2++;
                        d1++;
                }
        return 1;
}
static uint8_t tinyAverage(uint8_t *dst, uint8_t *src1, uint8_t *src2,uint32_t w, uint32_t h)
{
int delta;
uint32_t ww,hh;
uint8_t *s1,*s2,*d1;
int a1,a2;
        s1=src1;
        s2=src2;
        
        d1=dst;
        ww=w;
        hh=h;

          for(int y=0;y<hh;y++)
                for(int x=0;x<ww;x++)
                {
                        a1=*s1;
                        a2=*s2;
                        a1=a1+a2;
                        a1>>=1;
                        if(a1<0) a1=0;
                        if(a1>255) a1=255;
                        *d1=a1;                         

                        s1++;
                        s2++;
                        d1++;
                }
        return 1;
}

/*

                src=blend of srcP and R         => src= 1/2(srcP+R)
                                                   2*src-srcP=R

                srcN=blend of srcNN and R       => srcN=1/2 (srcNN+R)
                                                        2*srcN-srcNN=R

*/

static uint8_t tinyRestore(uint8_t *dst, uint8_t *srcP, uint8_t *src,uint8_t *srcN,uint8_t *srcNN,uint32_t w, uint32_t h)
{


uint8_t *s,*sp,*sn,*snn,*d1;
int a1,a2,a3,a4,sum,delta;

        sp=srcP;
        s=src;
        sn=srcN;
        snn=srcNN;
        
        d1=dst;

          for(int y=0;y<h;y++)
                for(int x=0;x<w;x++)
                {
                        a1=*sp;
                        a2=*s;
                        a3=*sn;
                        a4=*snn;

                        sum=2*a2+2*a3-a1-a4;
                        sum=sum/2;
                        
                        a1=sum;                        

                        if(a1<0) a1=0;
                        if(a1>255) a1=255;
                        *d1=a1;

                        s++;
                        sp++;
                        sn++;
                        snn++;
                        d1++;
                }
        return 1;
}

static uint8_t    unblend(ADMImage *tgt,ADMImage *src,ADMImage *srcP)
{
int delta;
uint32_t ww,hh;
uint8_t *s1,*s2,*d1;
int a1,a2,t1;

        tinyUnblend(YPLANE(tgt),YPLANE(src),YPLANE(srcP),tgt->_width,tgt->_height);
        tinyUnblend(UPLANE(tgt),UPLANE(src),UPLANE(srcP),tgt->_width>>1,tgt->_height>>1);
        tinyUnblend(VPLANE(tgt),VPLANE(src),VPLANE(srcP),tgt->_width>>1,tgt->_height>>1);
        return 1;


}
static uint8_t    averageF(ADMImage *tgt,ADMImage *src,ADMImage *srcP)
{
int delta;
uint32_t ww,hh;
uint8_t *s1,*s2,*d1;
int a1,a2,t1;

        tinyAverage(YPLANE(tgt),YPLANE(src),YPLANE(srcP),tgt->_width,tgt->_height);
        tinyAverage(UPLANE(tgt),UPLANE(src),UPLANE(srcP),tgt->_width>>1,tgt->_height>>1);
        tinyAverage(VPLANE(tgt),VPLANE(src),VPLANE(srcP),tgt->_width>>1,tgt->_height>>1);
        return 1;


}
static uint8_t    restore(ADMImage *tgt,ADMImage *srcP,ADMImage *src,ADMImage *srcN,ADMImage *srcNN)
{
int delta;
uint32_t ww,hh;
uint8_t *s1,*s2,*d1;
int a1,a2,t1;

        tinyRestore(YPLANE(tgt),YPLANE(srcP),YPLANE(src),YPLANE(srcN),YPLANE(srcNN),tgt->_width,tgt->_height);
        tinyRestore(UPLANE(tgt),UPLANE(srcP),UPLANE(src),UPLANE(srcN),UPLANE(srcNN),tgt->_width>>1,tgt->_height>>1);
        tinyRestore(VPLANE(tgt),VPLANE(srcP),VPLANE(src),VPLANE(srcN),VPLANE(srcNN),tgt->_width>>1,tgt->_height>>1);
        return 1;


}

uint8_t vidHardPDRemoval::getFrameNumberNoAlloc (uint32_t inframe,
                                uint32_t * len,
                                ADMImage * data, uint32_t * flags)
{

	
	ADMImage *srcP,*srcN,*srcNN,*src,*final,*display;
        float distMerged, distN,distP,distM,distR;
        char txt[255];
        if(inframe>= _info.nb_frames) return 0;
        if(inframe<1 || inframe>inframe>_info.nb_frames-2 )
        {
                data->duplicate(vidCache->getImage(inframe));
                vidCache->unlockAll();
                return 1;
        }
        if(_lastRemoved==inframe-1)
        {
                data->duplicate(rebuild);
                if(_param->show&&inframe)
                {
                        sprintf(txt," Telecined 2");
                        drawString(data,2,4,txt);
                }
                
                return 1;
        }

        data->duplicate(rebuild);

        srcP=vidCache->getImage(inframe-1);
        src=vidCache->getImage(inframe);
        srcN=vidCache->getImage(inframe+1);
        srcNN=vidCache->getImage(inframe+2);
        
        // Let's rebuild the pseudo R, where we have A AR RB B
        // If then we got R1 very close to R2, and that AR is very close to src
        // Decide it is hard telecined (frame blending)
#if 1
       restore(rebuild,srcP,src,srcN,srcNN);
#else
        unblend(cand1,src,srcP);
        unblend(cand2,srcN,srcNN);
        averageF(rebuild,cand1,cand2);
#endif  
#if 0
        data->duplicate(rebuild);
        vidCache->unlockAll();
        return 1;
#endif

        // And remerge...
        averageF(cand1,srcP,rebuild);
        averageF(cand2,srcNN,rebuild);
        
        distP=computeDiff(cand1,src,_param->noise);
        distN=computeDiff(cand2,srcN,_param->noise);
        distM=computeDiff(src,srcP,_param->noise);
        distR=computeDiff(src,srcN,_param->noise);
        
        
        
        double medium;

         if(distM>1&&distR>1)
        {
                if(distM>distR) medium=distR;
                          else  medium=distM;
                  //medium=min(distM,distR);
                 
                 medium/=100;
                 distN/=medium;
                 distP/=medium;
                 distR/=medium;
         
         }

        medium=medium/(_info.width*_info.height);
        double mn;

        if(inframe == _lastRemoved+5)
        {
                distN=(distN*7)/10;
                distP=(distP*7)/10;
                
        }
                //data->duplicate(src); 
        if(distN<_param->threshold && distP<_param->threshold)
        {
                data->duplicate(rebuild);
                _lastRemoved=inframe;
                if(_param->show && inframe == _lastRemoved+5)
                {
                        sprintf(txt," Fav");
                        drawString(data,2,5,txt);      
                }
        }
        else
                data->duplicate(src);
        if(_param->show)
        {
                display=data;

                sprintf(txt," N %02.1f",distN);
                drawString(display,2,0,txt);

                sprintf(txt," P %02.1f",distP);
                drawString(display,2,1,txt);

                sprintf(txt," R %02.1f",distR);
                drawString(display,2,2,txt);


                if(_lastRemoved==inframe)
                {
                        sprintf(txt," Telecined 1",distP);
                        drawString(display,2,4,txt);
                }
        }


          
        vidCache->unlockAll();
	return 1;
}
uint8_t vidHardPDRemoval::getCoupledConf (CONFcouple ** couples)
{

  ADM_assert (_param);
  *couples = new CONFcouple (3);
#undef CSET
#define CSET(x)  (*couples)->setCouple(#x,(_param->x))
  CSET (threshold);
  CSET (show);
  CSET (noise);
  
  return 1;
}


//EOF
