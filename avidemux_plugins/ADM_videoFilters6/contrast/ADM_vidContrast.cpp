/***************************************************************************
                          ADM_vidContrast.cpp  -  description
                             -------------------
    begin                : Sun Sep 22 2002
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
#include <math.h>

#include "DIA_flyDialog.h"
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"

#include "ADM_vidContrast.h"
#include "contrast_desc.cpp"


// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   ADMVideoContrast,   // Class
                        1,0,0,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_COLORS,            // Category
                        "contrast",            // internal name (must be uniq!)
                        "Contrast",            // Display name
                        QT_TR_NOOP("Adjust contrast, brightness and colors.") // Description
                    );


/**
    \fn
    \brief
*/
const char   *ADMVideoContrast::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255,"Contrast coef=%f offset=%d",  _param.coef,_param.offset);
    return s;
}
/**
    \fn      ctor
    \brief
*/
ADMVideoContrast::ADMVideoContrast(ADM_coreVideoFilter *in,CONFcouple *couples) : ADM_coreVideoFilter(in,couples)
{
        if(!couples || !ADM_paramLoad(couples,contrast_param,&_param))
		{
            // Default value
              _param.offset = 0;
              _param.coef = 1.0f;
              _param.doLuma = 1;
              _param.doChromaU = 1;
              _param.doChromaV = 1;
        }

        buildContrastTable (_param.coef, _param.offset, _tableFlat, _tableNZ);

}
/**
    \fn    getCoupledConf
    \brief
*/
bool         ADMVideoContrast::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, contrast_param,&_param);
}

void ADMVideoContrast::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, contrast_param, &_param);
}

/**
    \fn      dtor
    \brief
*/

ADMVideoContrast::~ADMVideoContrast()
{

}

/**
    \fn getNextFrame
*/
bool         ADMVideoContrast::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if(!previousFilter->getNextFrame(fn,image)) return false;    

    if(_param.doLuma)
        doContrast(image,image,_tableFlat,PLANAR_Y);
    

    if(_param.doChromaU)
        doContrast(image,image,_tableNZ,PLANAR_U);
    
    if(_param.doChromaV)
        doContrast(image,image,_tableNZ,PLANAR_V);
    
  return 1;
}
/**
    \fn buildContrastTable
*/
uint8_t buildContrastTable (float coef, int8_t off,  uint8_t * tableFlat, uint8_t * tableNZ)
{
  float f;

  for (uint32_t i = 0; i < 256; i++)
    {
      f = i;
      f *= coef;
//                                      f= (f-128)*coef+128;
      f += off;
      if (f < 0.)
	f = 0.;
      if (f > 255.)
	f = 255.;
      *(tableFlat + i) = (uint8_t) floor (f+0.49);

      f = i;
      f -= 128;
      f *= coef;
//                                      f= (f-128)*coef+128;

//                                      f+=off;
      if (f < -127.)
	f = -127.;
      if (f > 127.)
	f = 127.;
      f += 128.;
      *(tableNZ + i) = (uint8_t) floor (f+0.49);
    }
  return 0;
}
/**
    \fn doContrast
*/
bool doContrast (ADMImage * in, ADMImage * out, uint8_t * table,  ADM_PLANE plane)
{

  int sourcePitch=in->GetPitch(plane);
  int destPitch=out->GetPitch(plane);
  uint8_t *s=in->GetReadPtr(plane);
  uint8_t *d=out->GetWritePtr(plane);
  int width=in->GetWidth(plane);
  int height=in->GetHeight(plane);
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
        d[x]=table[s[x]];
    d+=destPitch;
    s+=sourcePitch;
  }
  return true;

}
/**
    \fn configure
*/
bool DIA_getContrast( ADM_coreVideoFilter *instream,contrast    *param );
bool ADMVideoContrast::configure( )

{
    if( DIA_getContrast(previousFilter,&_param))
    {
        buildContrastTable (_param.coef, _param.offset, _tableFlat, _tableNZ);
        return true;
    }
    return false;
}

// EOF
