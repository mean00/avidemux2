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
#if 0
#include <math.h>
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "ADM_videoFilterCache.h"
#include "DIA_factory.h"

#include "ADM_vidMPLD3D.h"
#include "denoise3d_desc.cpp"

#include "DIA_factory.h"
#define aprintf(...) {}

static inline unsigned int LowPassMul(unsigned int PrevMul, unsigned int CurrMul, int* Coef){
//    int dMul= (PrevMul&0xFFFFFF)-(CurrMul&0xFFFFFF);
    int dMul= PrevMul-CurrMul;
    int d=((dMul+0x10007FF)/(65536/16));
    return CurrMul + Coef[d];
}

DECLARE_VIDEO_FILTER(   ADMVideoMPD3D,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_NOISE,            // Category
                        "MplayerDenoise3DHQ",            // internal name (must be uniq!)
                        "Mplayer Denoise 3D HQ",            // Display name
                        "Reduce noise, smooth image, increase compressibility. HQ Version." // Description
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
	  snprintf(str,1023," MPlayer Denoise 3D (%2.1f - %2.1f - %2.1f)'",
						param.luma,param.chroma,param.temporal);
      return str;
        
}
/**
    \fn configure
*/
bool ADMVideoMPD3D::configure(void)
{

        
        ELEM_TYPE_FLOAT fluma,fchroma,ftemporal;
#define PX(x) &x
#define OOP(x,y) f##x=(ELEM_TYPE_FLOAT )param.x;
        
        OOP(luma,Luma);
        OOP(chroma,Chroma);
        OOP(temporal,Temporal);
        
        diaElemFloat   luma(PX(fluma),QT_TR_NOOP("_Spatial luma strength:"),0.,100.);
        diaElemFloat   chroma(PX(fchroma),QT_TR_NOOP("S_patial chroma strength:"),0.,100.);
        diaElemFloat   temporal(PX(ftemporal),QT_TR_NOOP("_Temporal strength:"),0.,100.);
    
        diaElem *elems[3]={&luma,&chroma,&temporal};
  
        if(  diaFactoryRun(QT_TR_NOOP("MPlayer denoise3d"),3,elems))
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
ADMVideoMPD3D::~ADMVideoMPD3D()
{
	delete [] Line;
	Line=NULL;
}


/**
        \fn deNoise
*/
void ADMVideoMPD3D::deNoise(unsigned char *Frame,        // mpi->planes[x]
                    unsigned char *FrameDest,    // dmpi->planes[x]
                   uint32_t 		 *LineAnt,      // vf->priv->Line (width bytes)
		    unsigned short 	*FrameAntPtr,
                    int W, int H, int sStride, int dStride,
                    int *Horizontal, int *Vertical, int *Temporal)
{
    int X, Y;
    int sLineOffs = 0, dLineOffs = 0;
    unsigned int PixelAnt;
    int PixelDst;
    unsigned short* FrameAnt=(FrameAntPtr);
    

    /* First pixel has no left nor top neightbour. Only previous frame */
    LineAnt[0] = PixelAnt = Frame[0]<<16;
    PixelDst = LowPassMul(FrameAnt[0]<<8, PixelAnt, Temporal);
    FrameAnt[0] = ((PixelDst+0x1000007F)/256);
    FrameDest[0]= ((PixelDst+0x10007FFF)/65536);

    /* Fist line has no top neightbour. Only left one for each pixel and
     * last frame */
    for (X = 1; X < W; X++){
        LineAnt[X] = PixelAnt = LowPassMul(PixelAnt, Frame[X]<<16, Horizontal);
        PixelDst = LowPassMul(FrameAnt[X]<<8, PixelAnt, Temporal);
	FrameAnt[X] = ((PixelDst+0x1000007F)/256);
	FrameDest[X]= ((PixelDst+0x10007FFF)/65536);
    }

    for (Y = 1; Y < H; Y++){
	unsigned int PixelAnt;
	unsigned short* LinePrev=&FrameAnt[Y*W];
	sLineOffs += sStride, dLineOffs += dStride;
        /* First pixel on each line doesn't have previous pixel */
        PixelAnt = Frame[sLineOffs]<<16;
        LineAnt[0] = LowPassMul(LineAnt[0], PixelAnt, Vertical);
	PixelDst = LowPassMul(LinePrev[0]<<8, LineAnt[0], Temporal);
	LinePrev[0] = ((PixelDst+0x1000007F)/256);
	FrameDest[dLineOffs]= ((PixelDst+0x10007FFF)/65536);

        for (X = 1; X < W; X++){
	    int PixelDst;
            /* The rest are normal */
            PixelAnt = LowPassMul(PixelAnt, Frame[sLineOffs+X]<<16, Horizontal);
            LineAnt[X] = LowPassMul(LineAnt[X], PixelAnt, Vertical);
	    PixelDst = LowPassMul(LinePrev[X]<<8, LineAnt[X], Temporal);
	    LinePrev[X] = ((PixelDst+0x1000007F)/256);
	    FrameDest[dLineOffs+X]= ((PixelDst+0x10007FFF)/65536);
        }
    }
}



/**
    \fn setup
*/
uint8_t  ADMVideoMPD3D::setup(void)
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
ADMVideoMPD3D::ADMVideoMPD3D(	ADM_coreVideoFilter *in,CONFcouple *couples) 
        : ADM_coreVideoFilter(in,couples)
{
uint32_t page;
  vidCache=new VideoCache(3,in);
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
bool	ADMVideoMPD3D::getCoupledConf( CONFcouple **couples)
{
    return ADM_paramSave(couples, denoise3d_param,&param);
	
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
                Line,ant, W, H,
                W,W,
               	Coefs[0] ,
                Coefs[0] ,
                Coefs[1] );
	
		d=UPLANE(image);
		c=UPLANE(src);
		p=UPLANE(prev);

        deNoise(c,p, d,
                Line,ant, cw, ch,
                cw,cw,
               	Coefs[2] ,
                Coefs[2] ,
                Coefs[3] );

        d=VPLANE(image);
        c=VPLANE(src);
        p=VPLANE(prev);

        deNoise(c,p, d,
                    Line,ant, cw, ch,
                    cw,cw,
                    Coefs[2] ,
                    Coefs[2] ,
                    Coefs[3] );


	nextFrame++;
	image->copyInfo(src);
        vidCache->unlockAll();
	return 1;
}

#define ABS(A) ( (A) > 0 ? (A) : -(A) )
/**
    \fn PrecalcCoefs
*/

void ADMVideoMPD3D::PrecalcCoefs(int *Ct, double Dist25)
{
    int i;
    double Gamma, Simil, C;

    Gamma = log(0.25) / log(1.0 - Dist25/255.0 - 0.00001);

    for (i = -256*16; i < 256*16; i++)
    {
        Simil = 1.0 - ABS(i) / (16*255.0);
	C=4096.*(double)i;
        C *= pow(Simil, Gamma) ;
       Ct[16*256+i] = (int)((C<0) ? (C-0.5) : (C+0.5));
    }

}
#endif
// EOF


