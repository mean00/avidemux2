/***************************************************************************
                          ADM_vidChromaShift.cpp  -  description
                             -------------------
	Try to remove the blue-to-theleft & red to the right effect by shifting chroma

    begin                : Sun Aug 14 2003
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
#include "DIA_flyDialog.h"
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"

#include "ADM_vidChromaShift.h"
#include "chromashift_desc.cpp"


// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   ADMVideoChromaShift,   // Class
                        1,0,0,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_COLORS,            // Category
                        "chromashift",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("chromashift","ChromaShift"),            // Display name
                        QT_TRANSLATE_NOOP("chromashift","Shift chroma U/V to fix badly synced luma/chroma.") // Description
                    );
/**
    \fn
    \brief
*/
const char   *ADMVideoChromaShift::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255,"Chroma shift U:%d  V:%d",  _param.u,_param.v);
    return s;
}
/**
    \fn
    \brief
*/
ADMVideoChromaShift::ADMVideoChromaShift(ADM_coreVideoFilter *in,CONFcouple *couples) : ADM_coreVideoFilter(in,couples)
{
        if(!couples || !ADM_paramLoad(couples,chromashift_param,&_param))
		{
            // Default value
            _param.u=0;
            _param.v=0;
        }
        _uncompressed=new ADMImageDefault(info.width,info.height);
        ADM_assert(_uncompressed);

}
/**
    \fn
    \brief
*/
bool         ADMVideoChromaShift::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, chromashift_param,&_param);
}

void ADMVideoChromaShift::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, chromashift_param, &_param);
}

/**
    \fn
    \brief
*/

ADMVideoChromaShift::~ADMVideoChromaShift()
{
	if(_uncompressed)
 		delete _uncompressed;
	_uncompressed=NULL;

}

/**
    \fn     fixup
    \brief   blacken column with no chroma info
*/
bool ADMVideoChromaShift::fixup(ADMImage *target,int32_t val)
{
uint32_t line,page;
uint8_t *zero;
uint8_t *zerochromaU,*zerochromaV;
int pitch,pitchU,pitchV;
int width=target->GetWidth(PLANAR_Y);
int height=target->GetHeight(PLANAR_Y);
/*
	If val is >0
		Source  ++++++++
		Target   __++++++
*/
	
	if(val>0)
		{
			line=val;
            // luma
			zero=target->GetWritePtr(PLANAR_Y);
			pitch=target->GetPitch(PLANAR_Y);
			for(uint32_t h=height;h>0;h--)
			{
				memset(zero,0,val);
				zero+=pitch;
			}
            // chroma u
            zerochromaU=target->GetWritePtr(PLANAR_U);
            zerochromaV=target->GetWritePtr(PLANAR_V);
            pitchU=target->GetPitch(PLANAR_U);
            pitchV=target->GetPitch(PLANAR_V);
			val>>=1;
			for(uint32_t h=height>>1;h>0;h--)
			{
				memset(zerochromaU,128,val);
				memset(zerochromaV,128,val);
				zerochromaU+=pitchU;
                zerochromaV+=pitchV;
			}
		}
/*
	if val is <0
		Source ++++++
		Target  ++++__

*/

		else
		{
			val=-val;

            zero=target->GetWritePtr(PLANAR_Y)+width-val;
			pitch=target->GetPitch(PLANAR_Y);

            zerochromaU=target->GetWritePtr(PLANAR_U)+(width-val)/2;
            zerochromaV=target->GetWritePtr(PLANAR_V)+(width-val)/2;
            pitchU=target->GetPitch(PLANAR_U);
            pitchV=target->GetPitch(PLANAR_V);


			for(uint32_t h=height;h>0;h--)
			{
				memset(zero,0,val);
				zero+=pitch;
			}

			val>>=1;
			for(uint32_t h=height>>1;h>0;h--)
			{
				memset(zerochromaU,128,val);
				memset(zerochromaV,128,val);
				zerochromaU+=pitchU;
                zerochromaV+=pitchV;
			}
		}
    return true;
}
/**
    \fn     shift
    \brief
*/
bool ADMVideoChromaShift::shiftPlane(ADM_PLANE plane,ADMImage *s,ADMImage *d,int32_t val)
{
    
        return shift(d->GetWritePtr(plane),s->GetReadPtr(plane),s->GetPitch(plane),
                        d->GetPitch(plane),s->GetWidth(plane),s->GetHeight(plane),val);
}
bool ADMVideoChromaShift::shift(uint8_t *target,uint8_t *source, 
                                    uint32_t source_pitch,uint32_t dest_pitch,
                                    uint32_t width, uint32_t height,int32_t val)
{
uint32_t line;

/*
	If val is >0
		Source  ++++++++
		Target   __++++++
*/
	if(val>0)
		{
			line=width-val;
			target+=val;
			for(uint32_t h=height;h>0;h--)
			{
			memcpy(target,source,line);
			target+=dest_pitch;
			source+=source_pitch;
			}
		}
/*
	if val is <0
		Source ++++++
		Target  ++++__

*/

		else
		{
			val=-val;
			line=width-val;
			source+=val;

			for(uint32_t h=height;h>0;h--)
			{
			memcpy(target,source,line);
			target+=dest_pitch;
			source+=source_pitch;
			}
		}
	return true;
}
// UI
bool DIA_getChromaShift( ADM_coreVideoFilter *instream,chromashift    *param );
bool ADMVideoChromaShift::configure( )

{
    return DIA_getChromaShift(previousFilter,&_param);

}

/**
    \fn getNextFrame
    \brief
*/
bool         ADMVideoChromaShift::getNextFrame(uint32_t *fn,ADMImage *image)
{
        if(!previousFilter->getNextFrame(fn,_uncompressed)) return false;

        image->copyInfo(_uncompressed);
        image->copyPlane(_uncompressed,image,PLANAR_Y);

		if(!_param.u)
                image->copyPlane(_uncompressed,image,PLANAR_U);	
		else
				shiftPlane(PLANAR_U,_uncompressed,image,_param.u);

		if(!_param.v)
                image->copyPlane(_uncompressed,image,PLANAR_V);		
        else
				shiftPlane(PLANAR_V,_uncompressed,image,_param.v);

		if(_param.u)
			fixup(image,_param.u*2);
		if(_param.v)
			fixup(image,_param.v*2);

      
      return 1;
}
// EOF

