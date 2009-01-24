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
#include "ADM_videoFilterDynamic.h"

#include "ADM_vidMPLD3Dlow.h"


#include "DIA_factory.h"
#define aprintf(...) {}
static FILTER_PARAM mp3Param={3,{"param1","param2","param3"}};

//********** Register chunk ************
VF_DEFINE_FILTER(ADMVideoMPD3Dlow,mp3Param,
                mpdenoise3d,
                QT_TR_NOOP("MPlayer denoise3d"),
                1,
                VF_NOISE,
                QT_TR_NOOP("Reduce noise, smooth image, increase compressibility."));
#define PARAM1_DEFAULT 4.0
#define PARAM2_DEFAULT 3.0
#define PARAM3_DEFAULT 6.0

 char 	*ADMVideoMPD3Dlow::printConf(void)
 {
	  ADM_FILTER_DECLARE_CONF(" MPlayer Denoise 3D (%2.1f - %2.1f - %2.1f)'",
						_param->param1,_param->param2,_param->param3);
        
}

uint8_t ADMVideoMPD3Dlow::configure(AVDMGenericVideoStream *instream)
{

        _in=instream;
        ELEM_TYPE_FLOAT fluma,fchroma,ftemporal;
#define PX(x) &x
#define OOP(x,y) f##x=(ELEM_TYPE_FLOAT )_param->y;
        
        OOP(luma,param1);
        OOP(chroma,param2);
        OOP(temporal,param3);
        
    diaElemFloat   luma(PX(fluma),QT_TR_NOOP("_Spatial luma strength:"),0.,100.);
    diaElemFloat   chroma(PX(fchroma),QT_TR_NOOP("S_patial chroma strength:"),0.,100.);
    diaElemFloat   temporal(PX(ftemporal),QT_TR_NOOP("_Temporal strength:"),0.,100.);
    
       diaElem *elems[3]={&luma,&chroma,&temporal};
  
   if(  diaFactoryRun(QT_TR_NOOP("MPlayer denoise3d"),3,elems))
        {
#undef OOP
#define OOP(x,y) _param->y=(double) f##x
                OOP(luma,param1);
                OOP(chroma,param2);
                OOP(temporal,param3);
          
                setup();
                return 1;
        }
        return 0;
}
ADMVideoMPD3Dlow::~ADMVideoMPD3Dlow()
{

 	DELETE(_param);
	delete  _uncompressed;
	delete  _stored;
	delete [] Line;
	Line=NULL;
	_param=NULL;
	_uncompressed=NULL;
	_stored=NULL;
}


#define LowPass(Prev, Curr, Coef) (Curr + Coef[Prev - Curr])


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



uint8_t  ADMVideoMPD3Dlow::setup(void)
{
 double LumSpac, LumTmp, ChromSpac, ChromTmp;

        LumSpac = _param->param1;
        LumTmp = _param->param3;

        ChromSpac = _param->param2;
        ChromTmp = LumTmp * ChromSpac / LumSpac;

        PrecalcCoefs((int *)Coefs[0], LumSpac);
        PrecalcCoefs((int *)Coefs[1], LumTmp);
        PrecalcCoefs((int *)Coefs[2], ChromSpac);
        PrecalcCoefs((int *)Coefs[3], ChromTmp);

	aprintf("\n Param : %lf %lf %lf \n",
		_param->param1,
		_param->param2,
		_param->param3);

	_last=0xFFFFFFF;

	return 1;
}
//--------------------------------------------------------
ADMVideoMPD3Dlow::ADMVideoMPD3Dlow(
									AVDMGenericVideoStream *in,CONFcouple *couples)

{
uint32_t page;

  Line=new uint8_t [in->getInfo()->width];
  memcpy(&_info,in->getInfo(),sizeof(_info));

	page=_info.width*_info.height;

 // _stored=new uint8_t[(page*3)>>1];
 // _uncompressed=new uint8_t[ (page*3)>>1];
 	_stored=new ADMImage(_info.width,_info.height);
	_uncompressed=new ADMImage(_info.width,_info.height);

  _info.encoding=1;
  _in=in;
  if(couples)
  {
			_param=NEW(MPD3D_PARAM);
			GET(param1);
			GET(param2);
			GET(param3);
	}
	else
	{
			_param=NEW( MPD3D_PARAM);
			_param->param1=PARAM1_DEFAULT;
			_param->param2=PARAM2_DEFAULT;
			_param->param3=PARAM3_DEFAULT;
	}
	setup();

}


uint8_t	ADMVideoMPD3Dlow::getCoupledConf( CONFcouple **couples)
{

			ADM_assert(_param);
			*couples=new CONFcouple(3);
 			//(*couples)->setCouple((char *)"param",*_param);
#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
			CSET(param1);
			CSET(param2);
			CSET(param3);
			return 1;

}
//                     1
//		Get in range in 121 + coeff matrix
//                     1
//
// If the value is too far away we ignore it
// else we blend

uint8_t ADMVideoMPD3Dlow::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
UNUSED_ARG(flags);

	int cw= _info.width>>1;
	int ch= _info.height>>1;
        int W = _info.width;
	int H  = _info.height;
	uint32_t dlen,dflags;

  		if(frame> _info.nb_frames-1) return 0;
		*len=(W*H*3)>>1;
		// First and last frame we don"t modify.
		if(!frame || (frame!=_last+1))
			{
				aprintf("D3D: First /last frame\n");
				 if(!_in->getFrameNumberNoAlloc(frame, &dlen,data,&dflags))
				 {
					 	return 0;
				 }
				// store for future use
				memcpy(_stored->data,data->data,*len);
				_last=frame;
				
				return 1;
			}
			ADM_assert(frame<_info.nb_frames);
			aprintf("D3D: next frame\n");
			// read uncompressed frame
	 		if(!_in->getFrameNumberNoAlloc(frame, &dlen,_uncompressed,&dflags))
				 {
					 	return 0;
				 }

		uint8_t *c,*d,*p;

		d=YPLANE(data);
		c=YPLANE(_uncompressed);
		p=YPLANE(_stored);
//

   	deNoise(c,p, d,
		Line, W, H,
                W,W,W,
               	Coefs[0] + 256,
                Coefs[0] + 256,
                Coefs[1] + 256);

	uint32_t page=W*H;
		d=UPLANE(data);
		c=UPLANE(_uncompressed);
		p=UPLANE(_stored);

	deNoise(c,p, d,
		Line, cw, ch,
                cw,cw,cw,
               	Coefs[2] + 256,
                Coefs[2] + 256,
                Coefs[3] + 256);

	page=page>>2;		
		d=VPLANE(data);
		c=VPLANE(_uncompressed);
		p=VPLANE(_stored);

	deNoise(c,p, d,
		Line, cw, ch,
                cw,cw,cw,
               	Coefs[2] + 256,
              	Coefs[2] + 256,
                Coefs[3] + 256);


	_last=frame;
	memcpy(YPLANE(_stored),YPLANE(data),W*H);
	memcpy(UPLANE(_stored),UPLANE(data),(W*H)>>2);
	memcpy(VPLANE(_stored),VPLANE(data),(W*H)>>2);
	data->copyInfo(_uncompressed);
	return 1;


}

#define ABS(A) ( (A) > 0 ? (A) : -(A) )

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


