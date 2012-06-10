/***************************************************************************
                          ADM_vidFastConvolution.cpp  -  description
                             -------------------
    begin                : Sat Nov 23 2002
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
#define convolution_param convolution_param_fast
#include "ADM_vidConvolution.hxx"
#include "convolution_desc.cpp"
#include "DIA_factory.h"
/**
    \fn getCoupledConf
*/
bool         AVDMFastVideoConvolution::getCoupledConf(CONFcouple **couples)
{
 return ADM_paramSave(couples, convolution_param,&param);
}

void AVDMFastVideoConvolution::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, convolution_param, &param);
}

/**
    \fn ctor
*/
AVDMFastVideoConvolution::AVDMFastVideoConvolution(
			ADM_coreVideoFilter *in,CONFcouple *couples) : ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,convolution_param,&param))
    {
        param.chroma=1;
        param.luma=1;
	}
    image=new ADMImageDefault(info.width,info.height);
    myName="Convolution";
}
/**
    \fn dtor
*/
AVDMFastVideoConvolution::~AVDMFastVideoConvolution()
{
	if(image) delete image;
    image=NULL;
}
/**
    \fn processPlane
*/
bool AVDMFastVideoConvolution::processPlane(ADMImage *s,ADMImage *d,ADM_PLANE plane)
{
        uint8_t *src=s->GetReadPtr(plane);
        uint8_t *dst=d->GetWritePtr(plane);
        uint32_t sPitch=s->GetPitch(plane);
        uint32_t dPitch=d->GetPitch(plane);
        uint32_t w=info.width;  
        uint32_t h=info.height;
        if(plane!=PLANAR_Y) 
        {
            w>>=1;
            h>>=1;
        }
	// first and last line
		memcpy(dst,src,w);
        memcpy(dst+(h-1)*dPitch,src+(h-1)*sPitch,w);
    // Other lines
        uint8_t *o1,*x1,*x2,*x3;
        o1=dst+dPitch;
		x1=src;
		x2=x1+sPitch;
		x3=x2+sPitch;

		// Luma
		for(int32_t y=1;y<h-1;y++)
		{
			doLine(x1,x2,x3,o1,w);
			x1=x2;
			x2=x3;
			x3+=sPitch; 
			o1+=dPitch;                 
		}
        return true;
}
/**
    \fn getNextFrame
*/
bool         AVDMFastVideoConvolution::getNextFrame(uint32_t *fn,ADMImage *data)
{
uint8_t *x1,*x2,*x3,*o1;
uint32_t stride,page;

	
	ADM_assert(image);					
	// read uncompressed frame
	if(!previousFilter->getNextFrame(fn,image)) return 0;               
         
    // Luma...
	if(!param.luma)
	{
        ADMImage::copyPlane(image,data,PLANAR_Y);
	}
	else
	{
        processPlane(image,data,PLANAR_Y);
	}
    // chroma u & v
	if(!param.chroma)
	{
        ADMImage::copyPlane(image,data,PLANAR_U);
        ADMImage::copyPlane(image,data,PLANAR_V);
	}
	else
	{
        processPlane(image,data,PLANAR_U);
        processPlane(image,data,PLANAR_V);
	}
	data->copyInfo(image);
    return 1;
}
/**
    \fn configure
*/
bool AVDMFastVideoConvolution::configure(void)
{
  
  //return DIA_getLumaChroma(&(_param->luma),&(_param->chroma)) ; 
  diaElemToggle luma(&(param.luma),QT_TR_NOOP("_Process luma"),QT_TR_NOOP("Process luma plane"));
  diaElemToggle chroma(&(param.chroma),QT_TR_NOOP("P_rocess chroma"));
  
  diaElem *elems[2]={&luma,&chroma};
  
  return diaFactoryRun(QT_TR_NOOP("Fast Convolution"),2,elems);
}
/**
    \fn getConfiguration
*/
const char   *AVDMFastVideoConvolution::getConfiguration(void)
{
    return "oops, you should never see that";
}

// EOF
