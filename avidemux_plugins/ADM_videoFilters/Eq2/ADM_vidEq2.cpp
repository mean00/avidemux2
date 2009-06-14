/***************************************************************************
                          Hue/Saturation filter ported from mplayer 
 (c)  * Software equalizer (brightness, contrast, gamma, saturation)
 *
 * Hampa Hug <hampa@hampa.ch> (original LUT gamma/contrast/brightness filter)
 * Daniel Moreno <comac@comac.darktech.org> (saturation, R/G/B gamma support)
 * Richard Felker (original MMX contrast/brightness code (vf_eq.c))
 * Michael Niedermayer <michalni@gmx.at> (LUT16)
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
#include "DIA_factory.h"

#include <math.h>

#include "ADM_vidEq2.h"
static FILTER_PARAM Eq2Param={8,{"contrast","brightness","saturation",
                                "gamma","gamma_weight","rgamma","ggamma","bgamma"}};

/*=====================================*/
/*=====================================*/
class  ADMVideoEq2:public AVDMGenericVideoStream
{

  protected:
    AVDMGenericVideoStream  *_in;           
    virtual char            *printConf(void);
            void            update(void);
            Eq2_Param       *_param;    
            VideoCache      *vidCache; 
            float           _hue;
            float           _saturation;
            Eq2Settings     settings;            
  public:
                
                        ADMVideoEq2(  AVDMGenericVideoStream *in,CONFcouple *setup);
    virtual             ~ADMVideoEq2();
    virtual uint8_t     configure(AVDMGenericVideoStream *in);
    virtual uint8_t     getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                          ADMImage *data,uint32_t *flags);

             uint8_t     getCoupledConf( CONFcouple **couples);
}     ;


VF_DEFINE_FILTER_UI(ADMVideoEq2,Eq2Param,
    eq2,
                                QT_TR_NOOP("MPlayer eq2"),
                                1,
                                VF_COLORS,
                                QT_TR_NOOP("Adjust contrast, brightness, saturation and gamma."));

uint8_t ADMVideoEq2::configure(AVDMGenericVideoStream *in)
{
  uint8_t r=1;
  float h,s;
  _in=in;   
  r=DIA_getEQ2Param(_param,in);
  update();
  return r;        
}
char *ADMVideoEq2::printConf( void )
{
  ADM_FILTER_DECLARE_CONF(" Eq2 :Cont:%1.2f Brigh:%1.2f Sat:%1.2f",
                _param->contrast,_param->brightness,_param->saturation);
  
}

ADMVideoEq2::ADMVideoEq2(  AVDMGenericVideoStream *in,CONFcouple *couples)
{
  
  _in=in;         
  memcpy(&_info,_in->getInfo(),sizeof(_info));    
  _info.encoding=1; 
  _param=new  Eq2_Param;
  if(couples)
  {                 
    GET(contrast); 
    GET(brightness);    
    GET(saturation); 
    GET(gamma); 
    GET(gamma_weight); 
    GET(rgamma); 
    GET(ggamma); 
    GET(bgamma); 
  }
  else
  {
    _param->contrast =1.0;                
    _param->brightness=0.0;
    _param->saturation =1.0;  
    _param->gamma =1.0; 
    _param->gamma_weight=1.0; 
    _param->rgamma =1.0; 
    _param->ggamma =1.0; 
    _param->bgamma =1.0;    
  }      
  vidCache=new VideoCache(1,_in);
  update();
    
   
}
void ADMVideoEq2::update(void)
{
   update_lut(&settings,_param);      
}
ADMVideoEq2::~ADMVideoEq2()
{
  delete _param;
  delete vidCache;
  
}
uint8_t ADMVideoEq2::getCoupledConf( CONFcouple **couples)
{
  ADM_assert(_param);
  *couples=new CONFcouple(8);


                CSET(contrast);
                CSET(brightness);
                CSET(saturation);
                CSET(gamma); 
                CSET(gamma_weight); 
                CSET(rgamma); 
                CSET(ggamma); 
                CSET(bgamma); 
                return 1;
}


uint8_t ADMVideoEq2::getFrameNumberNoAlloc(uint32_t frame,
                                             uint32_t *len,
                                             ADMImage *data,
                                             uint32_t *flags)
{
  ADMImage *mysrc=NULL;
  

  if(frame>=_info.nb_frames) return 0;
  
  mysrc=vidCache->getImage(frame);
  if(!mysrc) return 0;
  
  uint32_t w,h;
  w=_info.width;
  h=_info.height;

#ifdef ADM_CPU_X86
  if(CpuCaps::hasMMX())
  {
        affine_1d_MMX(&(settings.param[0]),YPLANE(data),YPLANE(mysrc),w,h);
        w>>=1;
        h>>=1;
        affine_1d_MMX(&(settings.param[2]),UPLANE(data),UPLANE(mysrc),w,h);
        affine_1d_MMX(&(settings.param[1]),VPLANE(data),VPLANE(mysrc),w,h);       
   }
   else
#endif
   {
        apply_lut(&(settings.param[0]),YPLANE(data),YPLANE(mysrc),w,h);
        w>>=1;
        h>>=1;
        apply_lut(&(settings.param[2]),UPLANE(data),UPLANE(mysrc),w,h);
        apply_lut(&(settings.param[1]),VPLANE(data),VPLANE(mysrc),w,h);       
    }
  vidCache->unlockAll();
  
  
  return 1;
}

void update_lut(Eq2Settings *settings,Eq2_Param *_param)
{
     memset(settings,0,sizeof(settings));

    settings->param[0].lut_clean=0;
    settings->param[1].lut_clean=0;
    settings->param[2].lut_clean=0;
    settings->contrast=_param->contrast;
    settings->brightness=_param->brightness;
    settings->saturation=_param->saturation;
    
    settings->ggamma=_param->ggamma;
    settings->bgamma=_param->bgamma;
    settings->rgamma=_param->rgamma;
    settings->gamma=_param->gamma;

    settings->gamma_weight=_param->gamma_weight;
    
    if(settings->ggamma<0.1) settings->ggamma=0.1;
    
    settings->param[0].c=_param->contrast;  
    settings->param[0].b=_param->brightness;;
    settings->param[0].g=settings->gamma*settings->ggamma;
    settings->param[0].w=settings->gamma_weight;
    
    
    	
    settings->param[1].c=_param->saturation;
    settings->param[1].b=0;
    settings->param[1].g=sqrt(settings->bgamma/settings->ggamma);
    settings->param[1].w=settings->gamma_weight;
    
    settings->param[2].c=_param->saturation;
    settings->param[2].b=0;
    settings->param[2].g=sqrt(settings->rgamma/settings->ggamma);
    settings->param[2].w=settings->gamma_weight;
    
    //printf("GGamma:%f\n",settings->ggamma);
    create_lut(&(settings->param[0]));
    create_lut(&(settings->param[1]));
    create_lut(&(settings->param[2])); 
}          

void create_lut (oneSetting *par)
{
  unsigned i;
  double   g, v;
  double   lw, gw;

  g = par->g;
  gw = par->w;
  lw = 1.0 - gw;

  if ((g < 0.001) || (g > 1000.0)) {
    g = 1.0;
  }

  g = 1.0 / g;

  for (i = 0; i < 256; i++) {
    v = (double) i / 255.0;
    v = par->c * (v - 0.5) + 0.5 + par->b;

    if (v <= 0.0) {
      par->lut[i] = 0;
    }
    else {
      v = v*lw + pow(v, g)*gw;

      if (v >= 1.0) {
        par->lut[i] = 255;
      }
      else {
        par->lut[i] = (unsigned char) (256.0 * v);
      }
    }
  }

#ifdef LUT16
  for(i=0; i<256*256; i++){
    par->lut16[i]= par->lut[i&0xFF] + (par->lut[i>>8]<<8);
  }
#endif

  par->lut_clean = 1;
}

#ifdef ADM_CPU_X86

void affine_1d_MMX (oneSetting *par, unsigned char *dst, unsigned char *src,
  unsigned int w, unsigned int h)
{
  unsigned i;
  int      contrast, brightness;
  unsigned dstep, sstep,w3;
  int      pel;
  short    int brvec[4];
  short    int contvec[4];
  w3=w>>3;
//  printf("\nmmx: src=%p dst=%p w=%d h=%d ds=%d ss=%d\n",src,dst,w,h,dstride,sstride);
  if(par->g!=1.0) return apply_lut(par,dst,src,w,h);
  //printf("MMX\n");
  contrast = (int) (par->c * 256 * 16);
  brightness = ((int) (100.0 * par->b + 100.0) * 511) / 200 - 128 - contrast / 32;

  brvec[0] = brvec[1] = brvec[2] = brvec[3] = brightness;
  contvec[0] = contvec[1] = contvec[2] = contvec[3] = contrast;

  
  asm volatile (
        "movq (%0), %%mm3 \n\t"
        "movq (%1), %%mm4 \n\t"
        ::  "r" (brvec),"r" (contvec)
        
        );
  while (h-- > 0) {
    asm volatile (
      "pxor %%mm0, %%mm0 \n\t"
      "movl %4, %%eax\n\t"
      ".p2align 4 \n\t"
      "lop%=: \n\t"
      "movq (%0), %%mm1 \n\t"
      "movq (%0), %%mm2 \n\t"
      "punpcklbw %%mm0, %%mm1 \n\t"
      "punpckhbw %%mm0, %%mm2 \n\t"
      "psllw $4, %%mm1 \n\t"
      "psllw $4, %%mm2 \n\t"
      "pmulhw %%mm4, %%mm1 \n\t"
      "pmulhw %%mm4, %%mm2 \n\t"
      "paddw %%mm3, %%mm1 \n\t"
      "paddw %%mm3, %%mm2 \n\t"
      "packuswb %%mm2, %%mm1 \n\t"
      "add $8, %0 \n\t"
      "movq %%mm1, (%1) \n\t"
      "add $8, %1 \n\t"
      "decl %%eax \n\t"
      "jnz lop%= \n\t"
      : "=r" (src), "=r" (dst)
      : "0" (src), "1" (dst), "r" (w3)
      : "%eax"
    );

    for (i = w & 7; i > 0; i--) {
      pel = ((*src++ * contrast) >> 12) + brightness;
      if (pel & 768) {
        pel = (-pel) >> 31;
      }
      *dst++ = pel;
    }

  }

  asm volatile ( "emms \n\t" ::: "memory" );
}
#endif

void apply_lut (oneSetting *par, unsigned char *dst, unsigned char *src,
  unsigned int w, unsigned int h)
{           
            
    
  unsigned int dstride,  sstride;
  unsigned int i, j, w2;
  unsigned char *lut;
  uint16_t *lut16;

   dstride=sstride=w;

  lut = par->lut;
#ifdef LUT16
  lut16 = par->lut16;
  w2= (w>>3)<<2;
  for (j = 0; j < h; j++) {
    uint16_t *src16= (uint16_t*)src;
    uint16_t *dst16= (uint16_t*)dst;
    for (i = 0; i < w2; i+=4) {
      dst16[i+0] = lut16[src16[i+0]];
      dst16[i+1] = lut16[src16[i+1]];
      dst16[i+2] = lut16[src16[i+2]];
      dst16[i+3] = lut16[src16[i+3]];
    }
    i <<= 1;
#else
  w2= (w>>3)<<3;
  for (j = 0; j < h; j++) {
    for (i = 0; i < w2; i+=8) {
      dst[i+0] = lut[src[i+0]];
      dst[i+1] = lut[src[i+1]];
      dst[i+2] = lut[src[i+2]];
      dst[i+3] = lut[src[i+3]];
      dst[i+4] = lut[src[i+4]];
      dst[i+5] = lut[src[i+5]];
      dst[i+6] = lut[src[i+6]];
      dst[i+7] = lut[src[i+7]];
    }
#endif
    for (; i < w; i++) {
      dst[i] = lut[src[i]];
    }

    src += sstride;
    dst += dstride;
  }
}  

