/***************************************************************************
                          ADM_vidMPLD3D.cpp  -  description
                             -------------------
Mplayer HQDenoise3d port to avidemux2
Original Authors
Copyright (C) 2003
Daniel Moreno <comac@comac.darktech.org>
	& A'rpi
Resynced with ffmpeg lavfilter
 * Copyright (c) 2003 Daniel Moreno <comac AT comac DOT darktech DOT org>
 * Copyright (c) 2010 Baptiste Coudurier

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
#include "ADM_coreVideoFilter.h"
#include "ADM_videoFilterCache.h"
#include "DIA_factory.h"

#include "ADM_vidMPLD3D.h"
#include "denoise3dHQ_desc.cpp"

#include "DIA_factory.h"
#define aprintf(...) {}
// -- ffmpeg start
#define av_malloc ADM_alloc
#define av_free   ADM_dezalloc
#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))

static inline unsigned int LowPassMul(unsigned int PrevMul, unsigned int CurrMul, int *Coef)
{
    //    int dMul= (PrevMul&0xFFFFFF)-(CurrMul&0xFFFFFF);
    int dMul= PrevMul-CurrMul;
    unsigned int d=((dMul+0x10007FF)>>12);
    return CurrMul + Coef[d];
}

static void deNoiseTemporal(unsigned char *FrameSrc,
                            unsigned char *FrameDest,
                            unsigned short *FrameAnt,
                            int W, int H, int sStride, int dStride,
                            int *Temporal)
{
    long X, Y;
    unsigned int PixelDst;

    for (Y = 0; Y < H; Y++) {
        for (X = 0; X < W; X++) {
            PixelDst = LowPassMul(FrameAnt[X]<<8, FrameSrc[X]<<16, Temporal);
            FrameAnt[X] = ((PixelDst+0x1000007F)>>8);
            FrameDest[X]= ((PixelDst+0x10007FFF)>>16);
        }
        FrameSrc  += sStride;
        FrameDest += dStride;
        FrameAnt += W;
    }
}

static void deNoiseSpacial(unsigned char *Frame,
                           unsigned char *FrameDest,
                           unsigned int *LineAnt,
                           int W, int H, int sStride, int dStride,
                           int *Horizontal, int *Vertical)
{
    long X, Y;
    long sLineOffs = 0, dLineOffs = 0;
    unsigned int PixelAnt;
    unsigned int PixelDst;

    /* First pixel has no left nor top neighbor. */
    PixelDst = LineAnt[0] = PixelAnt = Frame[0]<<16;
    FrameDest[0]= ((PixelDst+0x10007FFF)>>16);

    /* First line has no top neighbor, only left. */
    for (X = 1; X < W; X++) {
        PixelDst = LineAnt[X] = LowPassMul(PixelAnt, Frame[X]<<16, Horizontal);
        FrameDest[X]= ((PixelDst+0x10007FFF)>>16);
    }

    for (Y = 1; Y < H; Y++) {
        unsigned int PixelAnt;
        sLineOffs += sStride, dLineOffs += dStride;
        /* First pixel on each line doesn't have previous pixel */
        PixelAnt = Frame[sLineOffs]<<16;
        PixelDst = LineAnt[0] = LowPassMul(LineAnt[0], PixelAnt, Vertical);
        FrameDest[dLineOffs]= ((PixelDst+0x10007FFF)>>16);

        for (X = 1; X < W; X++) {
            unsigned int PixelDst;
            /* The rest are normal */
            PixelAnt = LowPassMul(PixelAnt, Frame[sLineOffs+X]<<16, Horizontal);
            PixelDst = LineAnt[X] = LowPassMul(LineAnt[X], PixelAnt, Vertical);
            FrameDest[dLineOffs+X]= ((PixelDst+0x10007FFF)>>16);
        }
    }
}

static void deNoise(unsigned char *Frame,
                    unsigned char *FrameDest,
                    unsigned int *LineAnt,
                    unsigned short **FrameAntPtr,
                    int W, int H, int sStride, int dStride,
                    int *Horizontal, int *Vertical, int *Temporal)
{
    long X, Y;
    long sLineOffs = 0, dLineOffs = 0;
    unsigned int PixelAnt;
    unsigned int PixelDst;
    unsigned short* FrameAnt=(*FrameAntPtr);

    if (!FrameAnt) {
        (*FrameAntPtr) = FrameAnt = (unsigned short *)av_malloc(W*H*sizeof(unsigned short));
        for (Y = 0; Y < H; Y++) {
            unsigned short* dst=&FrameAnt[Y*W];
            unsigned char* src=Frame+Y*sStride;
            for (X = 0; X < W; X++) dst[X]=src[X]<<8;
        }
    }

    if (!Horizontal[0] && !Vertical[0]) {
        deNoiseTemporal(Frame, FrameDest, FrameAnt,
                        W, H, sStride, dStride, Temporal);
        return;
    }
    if (!Temporal[0]) {
        deNoiseSpacial(Frame, FrameDest, LineAnt,
                       W, H, sStride, dStride, Horizontal, Vertical);
        return;
    }

    /* First pixel has no left nor top neighbor. Only previous frame */
    LineAnt[0] = PixelAnt = Frame[0]<<16;
    PixelDst = LowPassMul(FrameAnt[0]<<8, PixelAnt, Temporal);
    FrameAnt[0] = ((PixelDst+0x1000007F)>>8);
    FrameDest[0]= ((PixelDst+0x10007FFF)>>16);

    /* First line has no top neighbor. Only left one for each pixel and
     * last frame */
    for (X = 1; X < W; X++) {
        LineAnt[X] = PixelAnt = LowPassMul(PixelAnt, Frame[X]<<16, Horizontal);
        PixelDst = LowPassMul(FrameAnt[X]<<8, PixelAnt, Temporal);
        FrameAnt[X] = ((PixelDst+0x1000007F)>>8);
        FrameDest[X]= ((PixelDst+0x10007FFF)>>16);
    }

    for (Y = 1; Y < H; Y++) {
        unsigned int PixelAnt;
        unsigned short* LinePrev=&FrameAnt[Y*W];
        sLineOffs += sStride, dLineOffs += dStride;
        /* First pixel on each line doesn't have previous pixel */
        PixelAnt = Frame[sLineOffs]<<16;
        LineAnt[0] = LowPassMul(LineAnt[0], PixelAnt, Vertical);
        PixelDst = LowPassMul(LinePrev[0]<<8, LineAnt[0], Temporal);
        LinePrev[0] = ((PixelDst+0x1000007F)>>8);
        FrameDest[dLineOffs]= ((PixelDst+0x10007FFF)>>16);

        for (X = 1; X < W; X++) {
            unsigned int PixelDst;
            /* The rest are normal */
            PixelAnt = LowPassMul(PixelAnt, Frame[sLineOffs+X]<<16, Horizontal);
            LineAnt[X] = LowPassMul(LineAnt[X], PixelAnt, Vertical);
            PixelDst = LowPassMul(LinePrev[X]<<8, LineAnt[X], Temporal);
            LinePrev[X] = ((PixelDst+0x1000007F)>>8);
            FrameDest[dLineOffs+X]= ((PixelDst+0x10007FFF)>>16);
        }
    }
}

static void PrecalcCoefs(int *Ct, double Dist25)
{
    int i;
    double Gamma, Simil, C;

    Gamma = log(0.25) / log(1.0 - Dist25/255.0 - 0.00001);

    for (i = -255*16; i <= 255*16; i++) {
        Simil = 1.0 - FFABS(i) / (16*255.0);
        C = pow(Simil, Gamma) * 65536.0 * i / 16.0;
        Ct[16*256+i] = lrint(C);
    }

    Ct[0] = !!Dist25;
}


//--------
DECLARE_VIDEO_FILTER(   ADMVideoMPD3D,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_NOISE,            // Category
                        "MplayerDenoise3DHQ",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("mp3d","Mplayer Denoise 3D HQ"),            // Display name
                        QT_TRANSLATE_NOOP("mp3d","Reduce noise, smooth image, increase compressibility. HQ Version.") // Description
                    );

#define PARAM1_DEFAULT 4.0
#define PARAM2_DEFAULT 3.0
#define PARAM3_DEFAULT 6.0
/**
    \fn getConfiguration
*/
const char 	*ADMVideoMPD3D::getConfiguration(void)
 {
      static char str[1024];
	  snprintf(str,1023," MPlayer Denoise 3D (Sp : %2.1f - %2.1f, Tmp:%2.1f - %2.1f)'",
						param.luma_spatial,param.chroma_spatial,param.luma_temporal,param.chroma_temporal);
      return str;
        
}
/**
    \fn configure
*/
bool ADMVideoMPD3D::configure(void)
{

        
        ELEM_TYPE_FLOAT fluma_spatial,fchroma_spatial,fluma_temporal,fchroma_temporal;
#define PX(x) &x
#define OOP(x,y) f##x=(ELEM_TYPE_FLOAT )param.x;
        
        OOP(luma_spatial,Luma);
        OOP(chroma_spatial,Chroma);
        OOP(luma_temporal,LumaTemporal);
        OOP(chroma_temporal,ChromaTemporal);
        
        diaElemFloat   luma(PX(fluma_spatial),QT_TRANSLATE_NOOP("mp3d","_Spatial luma strength:"),0.1,100.);
        diaElemFloat   chroma(PX(fchroma_spatial),QT_TRANSLATE_NOOP("mp3d","S_patial chroma strength:"),0.,100.);
        diaElemFloat   lumaTemporal(PX(fluma_temporal),QT_TRANSLATE_NOOP("mp3d","Luma _Temporal strength:"),0.,100.);
        diaElemFloat   chromaTemporal(PX(fchroma_temporal),QT_TRANSLATE_NOOP("mp3d","Luma _Temporal strength:"),0.,100.);
    
        diaElem *elems[4]={&luma,&chroma,&lumaTemporal,&chromaTemporal};
  
        if(  diaFactoryRun(QT_TRANSLATE_NOOP("mp3d","MPlayer denoise3d"),4,elems))
        {
#undef OOP
#define OOP(x,y) param.x=(float) f##x
                OOP(luma_spatial,Luma);
                OOP(chroma_spatial,Chroma);
                OOP(luma_temporal,LumaTemporal);
                OOP(chroma_temporal,ChromaTemporal);
          
                setup();
                return 1;
        }
        return 0;
}
/**
    \fn dtor
*/
ADMVideoMPD3D::~ADMVideoMPD3D()
{
    if (context.Line)
	{
		delete [] context.Line;
		context.Line=NULL;
	}

    for(int i=0;i<3;i++)
    {
        unsigned short *t=context.Frame[i];
        context.Frame[i]=NULL;
        if(t) av_free(t);
    }
}


/**
    \fn setup
*/
uint8_t  ADMVideoMPD3D::setup(void)
{
    double LumSpac, LumTmp, ChromSpac, ChromTmp;
    double Param1, Param2, Param3, Param4;

    Param1=param.luma_spatial;
    Param2=param.chroma_spatial;
    Param3=param.luma_temporal;
    Param4=param.chroma_temporal;

    if(Param1<0.1) Param1=0.1;
    LumSpac   = Param1;
    ChromSpac = Param2 * Param1 / Param1;
    LumTmp    = Param3 * Param1 / Param1;
    ChromTmp  = LumTmp * ChromSpac / LumSpac;
    
    PrecalcCoefs(context.Coefs[0], LumSpac);
    PrecalcCoefs(context.Coefs[1], LumTmp);
    PrecalcCoefs(context.Coefs[2], ChromSpac);
    PrecalcCoefs(context.Coefs[3], ChromTmp);

   return 1;
}
/**
    \fn ctor
*/
ADMVideoMPD3D::ADMVideoMPD3D(	ADM_coreVideoFilter *in,CONFcouple *couples) 
        : ADM_coreVideoFilterCached(3,in,couples)
{
uint32_t page;
  memset(&context,0,sizeof(context));
  
  context.Line=new unsigned int [in->getInfo()->width];
  page=info.width*info.height;
  
  if(!couples || !ADM_paramLoad(couples,denoise3dhq_param,&param))
  {  		
            param.mode=4;	
			param.luma_spatial=PARAM1_DEFAULT;
			param.chroma_spatial=PARAM2_DEFAULT;
			param.luma_temporal=PARAM3_DEFAULT;
            param.chroma_temporal=PARAM3_DEFAULT*PARAM2_DEFAULT/PARAM1_DEFAULT; //   ChromTmp  = LumTmp * ChromSpac / LumSpac;
  }
  setup();

}

/**
    \fn getCoupledConf
*/
bool	ADMVideoMPD3D::getCoupledConf( CONFcouple **couples)
{
    return ADM_paramSave(couples, denoise3dhq_param,&param);
	
}

void ADMVideoMPD3D::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, denoise3dhq_param, &param);
}

/**
    \fn getNextFrame
*/
bool ADMVideoMPD3D::getNextFrame(uint32_t *fn,ADMImage *image)
{
	int cw= info.width>>1;
	int ch= info.height>>1;
    int W = info.width;
	int H  = info.height;
	uint32_t dlen,dflags;

    ADMImage *src, *dst;
    
        *fn=nextFrame;
        uint32_t n = nextFrame;
        printf("MP3d: next frame= %d\n",(int)n);
        src = vidCache->getImage(n);
        if(!src) return false;
        
		uint8_t *c,*d;

		d=YPLANE(image);
		c=YPLANE(src);

        deNoise(c,d,
                context.Line,
                &context.Frame[0],
                W,H,
                image->GetPitch(PLANAR_Y),
                src->GetPitch(PLANAR_Y),
                context.Coefs[0],
                context.Coefs[0],
                context.Coefs[1]);

	
		d=UPLANE(image);
		c=UPLANE(src);

       deNoise(c,d,
                context.Line,
                &context.Frame[1],
                cw,ch,
                image->GetPitch(PLANAR_U),
                src->GetPitch(PLANAR_U),
                context.Coefs[2],
                context.Coefs[2],
                context.Coefs[3]);

        d=VPLANE(image);
        c=VPLANE(src);

         deNoise(c,d,
                context.Line,
                &context.Frame[1],
                cw,ch,
                image->GetPitch(PLANAR_V),
                src->GetPitch(PLANAR_V),
                context.Coefs[2],
                context.Coefs[2],
                context.Coefs[3]);


	nextFrame++;
	image->copyInfo(src);
    vidCache->unlockAll();
	return 1;
}
/**
    \fn goToTime
    \brief flush acc if seeking
*/
bool         ADMVideoMPD3D::goToTime(uint64_t usSeek)
{
    for(int i=0;i<3;i++)
    {
        unsigned short *t=context.Frame[i];
        context.Frame[i]=NULL;
        if(t) av_free(t);
    }
    // Flush 
    return ADM_coreVideoFilterCached::goToTime(usSeek);
}
// EOF


