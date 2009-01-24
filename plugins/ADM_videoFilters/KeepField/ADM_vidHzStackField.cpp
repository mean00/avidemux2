/***************************************************************************
                          Separate Fields.cpp  -  description
                             -------------------
Convert a x*y * f fps video into -> x*(y/2)*fps/2 video

Same idea as for avisynth separatefield


    begin                : Thu Mar 21 2002
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

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"
#include "ADM_vidFieldUtil.h"
#include "ADM_vidSeparateField.h"

static FILTER_PARAM swapParam={0,{""}};
//REGISTERX(VF_INTERLACING, "hzstackfield",QT_TR_NOOP("Hz Stack fields"),
//QT_TR_NOOP("Put botj fields side by side."),VF_HZSTACKFIELD,1,hzstackfield_create,hzstackfield_script);
VF_DEFINE_FILTER(AVDMVideoHzStackField,swapParam,
    hzstackfield,
                QT_TR_NOOP("separatefields"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Put botj fields side by side."));

//*************************************************************
//_______________________Hz Stack Fields_______________________

char *AVDMVideoHzStackField::printConf( void )
{
        ADM_FILTER_DECLARE_CONF(" Hz Stack fields");
        
}

//_______________________________________________________________
AVDMVideoHzStackField::AVDMVideoHzStackField(	AVDMGenericVideoStream *in,CONFcouple *setup)
{
UNUSED_ARG(setup);
        _in=in;
        memcpy(&_info,_in->getInfo(),sizeof(_info));
        _info.width<<=1;
        _info.height>>=1;
        _uncompressed=new ADMImage(in->getInfo()->width,in->getInfo()->height);	
}

// ___ destructor_____________
AVDMVideoHzStackField::~AVDMVideoHzStackField()
{
        delete _uncompressed;
        _uncompressed=NULL;
}

uint8_t AVDMVideoHzStackField::getFrameNumberNoAlloc(uint32_t frame,
                            uint32_t *len,
                            ADMImage *data,
                            uint32_t *flags)
{
uint32_t ref,ref2;
ADMImage *ptr1,*ptr2;
        if(frame>=_info.nb_frames) return 0;
        if(!_in->getFrameNumberNoAlloc(frame, len, _uncompressed, flags)) return 0;

        uint32_t pg=_info.width*_info.height;
        // Duplicate _uncompressed
        memcpy(YPLANE(data),YPLANE(_uncompressed),pg);
        memcpy(UPLANE(data),UPLANE(_uncompressed),pg>>2);
        memcpy(VPLANE(data),VPLANE(_uncompressed),pg>>2);


        return 1;
}


