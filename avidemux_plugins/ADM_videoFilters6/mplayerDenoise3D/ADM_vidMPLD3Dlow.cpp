/***************************************************************************
                          ADM_vidMPLD3D.cpp  -  description
                             -------------------
Mplayer HQDenoise3d port to avidemux2
Original Authors
Copyright (C) 2003
Daniel Moreno <comac@comac.darktech.org>
	& A'rpi
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

#include "ADM_vidMPLD3Dlow.h"
#include "denoise3d_desc.cpp"

#include "DIA_factory.h"
#define aprintf(...) {}

DECLARE_VIDEO_FILTER(   ADMVideoMPD3Dlow,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_NOISE,            // Category
                        "MplayerDenoise3D",            // internal name (must be uniq!)
                         QT_TRANSLATE_NOOP("mp3dlow","Mplayer Denoise 3D"),            // Display name
                         QT_TRANSLATE_NOOP("mp3dlow","Reduce noise, smooth image, increase compressibility.") // Description
                    );

#define PARAM1_DEFAULT 4.0
#define PARAM2_DEFAULT 3.0
#define PARAM3_DEFAULT 6.0
/**
    \fn getConfiguration
*/
const char 	*ADMVideoMPD3Dlow::getConfiguration(void)
 {
      static char str[1024];
	  snprintf(str,1023," MPlayer Denoise 3D (%2.1f - %2.1f - %2.1f)'",
						param.luma,param.chroma,param.temporal);
      return str;
        
}
/**
    \fn configure
*/
bool ADMVideoMPD3Dlow::configure(void)
{

        
        ELEM_TYPE_FLOAT fluma,fchroma,ftemporal;
#define PX(x) &x
#define OOP(x,y) f##x=(ELEM_TYPE_FLOAT )param.x;
        
        OOP(luma,Luma);
        OOP(chroma,Chroma);
        OOP(temporal,Temporal);
        
        diaElemFloat   luma(PX(fluma),QT_TRANSLATE_NOOP("mp3dlow","_Spatial luma strength:"),0.,100.);
        diaElemFloat   chroma(PX(fchroma),QT_TRANSLATE_NOOP("mp3dlow","S_patial chroma strength:"),0.,100.);
        diaElemFloat   temporal(PX(ftemporal),QT_TRANSLATE_NOOP("mp3dlow","_Temporal strength:"),0.,100.);
    
        diaElem *elems[3]={&luma,&chroma,&temporal};
  
        if(  diaFactoryRun(QT_TRANSLATE_NOOP("mp3dlow","MPlayer denoise3d"),3,elems))
        {
#undef OOP
#define OOP(x,y) param.x=(float) f##x
                OOP(luma,Luma);
                OOP(chroma,Chroma);
                OOP(temporal,Temporal);
          
                setup();
                return 1;
        }
        return 0;
}
/**
    \fn dtor
*/
ADMVideoMPD3Dlow::~ADMVideoMPD3Dlow()
{
	delete [] Line;
	Line=NULL;
}


#define LowPass(Prev, Curr, Coef) (Curr + Coef[Prev - Curr])

/**
    \fn deNoise
*/
void ADMVideoMPD3Dlow::deNoise(unsigned char *Frame,        // mpi->planes[x]
                    unsigned char *FramePrev,    // pmpi->planes[x]
                    unsigned char *FrameDest,    // dmpi->planes[x]
                    unsigned char *LineAnt,      // vf->priv->Line (width bytes)
                    int W, int H, int sStride, int pStride, int dStride,
                    int *Horizontal, int *Vertical, int *Temporal)
{
    int X, Y;
    int sLineOffs = 0, pLineOffs = 0, dLineOffs = 0;
    unsigned char PixelAnt;

    /* First pixel has no left nor top neightbour. Only previous frame */
    LineAnt[0] = PixelAnt = Frame[0];
    FrameDest[0] = LowPass(FramePrev[0], LineAnt[0], Temporal);

    /* Fist line has no top neightbour. Only left one for each pixel and
     * last frame */
    for (X = 1; X < W; X++)
    {
        PixelAnt = LowPass(PixelAnt, Frame[X], Horizontal);
        LineAnt[X] = PixelAnt;
        FrameDest[X] = LowPass(FramePrev[X], LineAnt[X], Temporal);
    }

    for (Y = 1; Y < H; Y++)
    {
	sLineOffs += sStride, pLineOffs += pStride, dLineOffs += dStride;
        /* First pixel on each line doesn't have previous pixel */
        PixelAnt = Frame[sLineOffs];
        LineAnt[0] = LowPass(LineAnt[0], PixelAnt, Vertical);
        FrameDest[dLineOffs] = LowPass(FramePrev[pLineOffs], LineAnt[0], Temporal);

        for (X = 1; X < W; X++)
        {
            /* The rest are normal */
            PixelAnt = LowPass(PixelAnt, Frame[sLineOffs+X], Horizontal);
            LineAnt[X] = LowPass(LineAnt[X], PixelAnt, Vertical);
            FrameDest[dLineOffs+X] = LowPass(FramePrev[pLineOffs+X], LineAnt[X], Temporal);
        }
    }
}


/**
    \fn setup
*/
uint8_t  ADMVideoMPD3Dlow::setup(void)
{
 double LumSpac, LumTmp, ChromSpac, ChromTmp;

        LumSpac = param.luma;
        LumTmp = param.temporal;

        ChromSpac = param.chroma;
        ChromTmp = LumTmp * ChromSpac / LumSpac;

        PrecalcCoefs((int *)Coefs[0], LumSpac);
        PrecalcCoefs((int *)Coefs[1], LumTmp);
        PrecalcCoefs((int *)Coefs[2], ChromSpac);
        PrecalcCoefs((int *)Coefs[3], ChromTmp);

        aprintf("\n Param : %lf %lf %lf \n",
            param.luma,
            param.chroma,
            param.temporal);

       return 1;
}
/**
    \fn ctor
*/
ADMVideoMPD3Dlow::ADMVideoMPD3Dlow(	ADM_coreVideoFilter *in,CONFcouple *couples) 
        : ADM_coreVideoFilterCached(3,in,couples)
{
uint32_t page;
  
  Line=new uint8_t [in->getInfo()->width];
  page=info.width*info.height;
  
  if(!couples || !ADM_paramLoad(couples,denoise3d_param,&param))
  {  			
			param.luma=PARAM1_DEFAULT;
			param.chroma=PARAM2_DEFAULT;
			param.temporal=PARAM3_DEFAULT;
  }
  setup();

}

/**
    \fn getCoupledConf
*/
bool	ADMVideoMPD3Dlow::getCoupledConf( CONFcouple **couples)
{
    return ADM_paramSave(couples, denoise3d_param,&param);
	
}

void ADMVideoMPD3Dlow::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, denoise3d_param, &param);
}
/**
    \fn getNextFrame
*/
bool ADMVideoMPD3Dlow::getNextFrame(uint32_t *fn,ADMImage *image)
{
	int cw= info.width>>1;
	int ch= info.height>>1;
    int W = info.width;
	int H  = info.height;
	uint32_t dlen,dflags;

    ADMImage *src, *dst, * prev, *next;
    
        *fn=nextFrame;
        uint32_t n = nextFrame;
printf("MP3d: next frame= %d\n",(int)n);
        src = vidCache->getImage(n);
        if(!src) return false;
        // If possible get previous image...
        if (n>0)
                prev =  vidCache->getImage( n-1); // get previous frame
        else
                prev=src;
        
		uint8_t *c,*d,*p;

		d=YPLANE(image);
		c=YPLANE(src);
		p=YPLANE(prev);
//

        deNoise(c,p, d,
                Line, W, H,
                W,W,W,
               	Coefs[0] + 256,
                Coefs[0] + 256,
                Coefs[1] + 256);
	
		d=UPLANE(image);
		c=UPLANE(src);
		p=UPLANE(prev);

        deNoise(c,p, d,
                Line, cw, ch,
                cw,cw,cw,
               	Coefs[2] + 256,
                Coefs[2] + 256,
                Coefs[3] + 256);

        d=VPLANE(image);
        c=VPLANE(src);
        p=VPLANE(prev);

        deNoise(c,p, d,
                    Line, cw, ch,
                    cw,cw,cw,
                    Coefs[2] + 256,
                    Coefs[2] + 256,
                    Coefs[3] + 256);


	nextFrame++;
	image->copyInfo(src);
    vidCache->unlockAll();
	return 1;
}

#define ABS(A) ( (A) > 0 ? (A) : -(A) )
/**
    \fn PrecalcCoefs
*/
void ADMVideoMPD3Dlow::PrecalcCoefs(int *Ct, double Dist25)
{
 int i;
    double Gamma, Simil, C;
    double d;
    Gamma = log(0.25) / log(1.0 - Dist25/255.0);

    for (i = -256; i <= 255; i++)
    {
    	if(i>0) d=(double)i/255.;
		else d=(double)-i/255.;
        Simil = 1.0 - d;
//        Ct[256+i] = lround(pow(Simil, Gamma) * (double)i);
 	C= (double)i;
        C *= pow(Simil, Gamma);

  	Ct[256+i] = (int)((C<0) ? (C-0.5) : (C+0.5));

    }
}

// EOF


