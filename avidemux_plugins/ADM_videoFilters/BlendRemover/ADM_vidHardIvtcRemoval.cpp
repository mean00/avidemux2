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

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"

#include <math.h>

#include "ADM_videoFilter.h"

#include "DIA_enter.h"
#include "DIA_factory.h"

#define MUL 1
// Set it to 2 for post separate field

#include "ADM_vidBlendRemoval_param.h"

#define PROGRESSIVE  0x00000001
#define MAGIC_NUMBER (0xdeadbeef)
#define IN_PATTERN   0x00000002

class vidHardPDRemoval:public AVDMGenericVideoStream
{

    protected:
        virtual char *printConf ( void );
        VideoCache *vidCache;
        BLEND_REMOVER_PARAM *_param;
        uint32_t              _lastRemoved;
        ADMImage              *cand1,*cand2,*rebuild;
    public:

        vidHardPDRemoval ( AVDMGenericVideoStream * in, CONFcouple * setup );
        virtual         ~vidHardPDRemoval ();
        virtual uint8_t getFrameNumberNoAlloc ( uint32_t frame, uint32_t * len,
                                                ADMImage * data, uint32_t * flags );
        uint8_t configure ( AVDMGenericVideoStream * instream );
        virtual uint8_t getCoupledConf ( CONFcouple ** couples );

};
//***************************
static FILTER_PARAM field_unblend_template =
    { 4,"threshold","show","noise","identical"};

VF_DEFINE_FILTER ( vidHardPDRemoval,field_unblend_template,
                   hardivtcremove,
                   QT_TR_NOOP ( ""Hard pulldown removal"" ),
                   1,
                   VF_ITERLACING,
                   QT_TR_NOOP ( "Remove IVTC that has been analog captured or resized.") );

//*************************************
uint8_t vidHardPDRemoval::configure (AVDMGenericVideoStream * in)
{
       _in=in;
    
#define PX(x) &(_param->x)
        
    diaElemUInteger   thresh(PX(threshold),QT_TR_NOOP("_Threshold:"),0,99,
        QT_TR_NOOP("If value is smaller than threshold it is considered valid."
            " Smaller value might mean more false positive"));
    diaElemUInteger   noise(PX(noise),QT_TR_NOOP("_Noise:"),0,99,QT_TR_NOOP("If pixels are closer than noise, they are considered to be the same"));
    diaElemUInteger   identical(PX(identical),QT_TR_NOOP("_Identical:"),0,99,QT_TR_NOOP("If metric is less than identical, images are considered identical"));
    diaElemToggle     show(PX(show),QT_TR_NOOP("_Show metrics"),QT_TR_NOOP("Show metric in image (debug)"));
    
       diaElem *elems[]={&thresh,&noise,&identical,&show};
  
   if(  diaFactoryRun(QT_TR_NOOP("Hard IVTC Removal"),sizeof(elems)/sizeof(diaElem *),elems))
   {
        _lastRemoved=0xFFFFFFF;
        return 1;
    }
        return 0;
}
/*************************************/
char *vidHardPDRemoval::printConf (void)
{
	ADM_FILTER_DECLARE_CONF( " Field Unblend Thresh:%d Noise:%d",_param->threshold,_param->noise);
}
static void hint(ADMImage *img)
{
       unsigned int hint;

                 hint= PROGRESSIVE;
                
                 hint |= IN_PATTERN;
                
                PutHintingData(YPLANE(img), hint);  

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

 _param=new BLEND_REMOVER_PARAM;
 _lastRemoved=0xFFFFFFF;
 if(couples)
 {
#undef GET
#define GET(x) couples->getCouple(#x,&(_param->x))
      GET (threshold);
      GET (show);
      GET (noise);
      GET (identical);
  }
  else
  {
        _param->threshold=10;
        _param->show=0;
        _param->noise=5;
        _param->identical=2;
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
#ifdef ADM_CPU_X86
static uint8_t tinyRestoreMMX(uint8_t *dst, uint8_t *srcP, uint8_t *src,uint8_t *srcN,uint8_t *srcNN,uint32_t w, uint32_t h)
{


uint8_t *s,*sp,*sn,*snn,*d1;
int a1,a2,a3,a4,sum,delta,l,ll;

        sp=srcP;
        s=src;
        sn=srcN;
        snn=srcNN;
        
        d1=dst;

        l=w*h;
        ll=l>>2;
#ifdef GCC_2_95_X
        __asm__(
                         "pxor %mm7,%mm7"
                ::
                 );
#else
        __asm__(
                         "pxor %%mm7,%%mm7"
                ::
                 );
#endif
        for(int x=0;x<ll;x++)
                {
                        __asm__(
                        "movd           (%0),%%mm0 \n"
                        "movd           (%1),%%mm1 \n"
                        "movd           (%2),%%mm2 \n"
                        "movd           (%3),%%mm3 \n"
                        "punpcklbw      %%mm7,%%mm0 \n"
                        "punpcklbw      %%mm7,%%mm1 \n"
                        "punpcklbw      %%mm7,%%mm2 \n"
                        "punpcklbw      %%mm7,%%mm3 \n"  //sum=2*m1+2*m2-m0-m3;
                        
                        "paddw          %%mm2,%%mm1 \n"
                        "paddw          %%mm1,%%mm1 \n"
                        "paddw          %%mm3,%%mm0 \n"
                        
                        "psubusw        %%mm0,%%mm1 \n" // mm1=sum
                        "psraw          $1,%%mm1 \n"    //2 
                        "packuswb       %%mm1,  %%mm1\n"
                        "movd           %%mm1,(%4) \n"

                : : "r" (sp),"r" (s),"r"(sn),"r"(snn),"r"(d1)
                );

                        s+=4;
                        sp+=4;
                        sn+=4;
                        snn+=4;
                        d1+=4;
                }
        if(l&3) tinyRestore(d1, sp, s,sn,snn,l&3, 1);
        return 1;
}
#endif

static uint8_t    restore(ADMImage *tgt,ADMImage *srcP,ADMImage *src,ADMImage *srcN,ADMImage *srcNN)
{
int delta;
uint32_t ww,hh;
uint8_t *s1,*s2,*d1;
int a1,a2,t1;

#ifdef ADM_CPU_X86
        if(CpuCaps::hasMMX())
        {
              tinyRestoreMMX(YPLANE(tgt),YPLANE(srcP),YPLANE(src),YPLANE(srcN),YPLANE(srcNN),tgt->_width,tgt->_height);
              tinyRestoreMMX(UPLANE(tgt),UPLANE(srcP),UPLANE(src),UPLANE(srcN),UPLANE(srcNN),tgt->_width>>1,tgt->_height>>1);
              tinyRestoreMMX(VPLANE(tgt),VPLANE(srcP),VPLANE(src),VPLANE(srcN),VPLANE(srcNN),tgt->_width>>1,tgt->_height>>1);
              return 1;
        }
#endif

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
        float distMerged, distN,distP,distM,distR,skip=0;
        char txt[255];

        if(inframe>= _info.nb_frames) return 0;
        if(inframe<1 || inframe>inframe>_info.nb_frames-3 )
        {
                skip=1;
        }
        if(inframe>_lastRemoved+1 && inframe<_lastRemoved+5 )
        {
                skip=1;
        }
        
        if(skip)
        {
                data->duplicate(vidCache->getImage(inframe));
                hint(data);
                vidCache->unlockAll();
                return 1;
        }
        
        if(_lastRemoved==inframe-1)
        {
                data->duplicate(rebuild);
                hint(data);
                if(_param->show&&inframe)
                {
                        sprintf(txt," Telecined 2");
                        drawString(data,2,4,txt);
                }
                
                return 1;
        }

        //data->duplicate(rebuild);

        srcP=vidCache->getImage(inframe-1);
        src=vidCache->getImage(inframe);
        srcN=vidCache->getImage(inframe+1);
        srcNN=vidCache->getImage(inframe+2);

        if(!src || !srcP || !srcN || !srcNN)
        {
                data->duplicate(vidCache->getImage(inframe));
                vidCache->unlockAll();
                return 1;
        }
        
        // Let's rebuild the pseudo R, where we have A AR RB B
        // If then we got R1 very close to R2, and that AR is very close to src
        // Decide it is hard telecined (frame blending)
#if 1
        restore(rebuild,srcP,src,srcN,srcNN);
#else
        cand1->substract(src,srcP);
        cand2->substract(srcN,srcNN);
        rebuild->merge(cand1,cand2);
#endif  
#if 0
        data->duplicate(rebuild);
        vidCache->unlockAll();
        return 1;
#endif

        // And remerge...
        cand1->merge(srcP,rebuild);
        cand2->merge(srcNN,rebuild);
        
        distP=ADMImage::lumaDiff(cand1,src,_param->noise);
        distN=ADMImage::lumaDiff(cand2,srcN,_param->noise);
        distM=ADMImage::lumaDiff(src,srcP,_param->noise);
        distR=ADMImage::lumaDiff(src,srcN,_param->noise);
        
        
        
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
        medium*=1000;

        if(medium<_param->identical)
        {
                 data->duplicate(src);
                 vidCache->unlockAll();
                if(_param->show)
                {
                        sprintf(txt," %% %02.1f : Identical",medium);
                        drawString(data,2,3,txt);

                }
                return 1;
        }
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
                hint(data);
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

                sprintf(txt," %% %02.1f",medium);
                drawString(display,2,3,txt);

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
  *couples = new CONFcouple (4);
#undef CSET
#define CSET(x)  (*couples)->setCouple(#x,(_param->x))
  CSET (threshold);
  CSET (show);
  CSET (noise);
  CSET (identical);
  
  return 1;
}


//EOF
