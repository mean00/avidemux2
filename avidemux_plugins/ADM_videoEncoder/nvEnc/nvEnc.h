/***************************************************************************
                          \fn ADM_VideoEncoders
                          \brief Internal handling of video encoders
                             -------------------

    copyright            : (C) 2002/2009 by mean
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
#pragma once
#include "nvenc.h"
#include "ADM_coreVideoEncoder.h"
enum NVencPreset
{
  NV_PRESET_HP=1,
  NV_PRESET_HQ=2,
  NV_PRESET_BD=3,
  NV_PRESET_LL=4,
  NV_PRESET_LLHP=5,
  NV_PRESET_LLHQ=6
};


#define NVENC_CONF_DEFAULT \
{ \
		NV_PRESET_HQ, \
                10000, \
                20000,\
	}



/**
        \class ADM_ffNvEncEncoder
        \brief Dummy encoder that does nothing

*/
class ADM_nvEncEncoder : public ADM_coreVideoEncoder
{
protected:

               uint8_t      *nv12;
               int          nv12Stride;               
public:
                           ADM_nvEncEncoder(ADM_coreVideoFilter *src,bool globalHeader);
virtual                    ~ADM_nvEncEncoder();
               bool         setup(void) ;    /// Call once before using            
virtual        bool         isDualPass(void) {return false;}
virtual        bool         startPass2(void) {return true;}

virtual        bool        getExtraData(uint32_t *l,uint8_t **d);
virtual        bool        configureContext(void);
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "H264";}

};

