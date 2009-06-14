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
//REGISTERX(VF_INTERLACING, "unstackfield",QT_TR_NOOP("Unstack fields"),QT_TR_NOOP(
    //"Interleave top and bottom part of the picture."),VF_UNSTACK_FIELD,1,unstackfield_create,unstackfield_script);
VF_DEFINE_FILTER(AVDMVideoUnStackField,swapParam,
    unstackfield,
                QT_TR_NOOP("Unstack fields"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Interleave top and bottom part of the picture."));


//_______________________UnStack Fields_______________________

char *AVDMVideoUnStackField::printConf( void )
{
        ADM_FILTER_DECLARE_CONF(" UnStack fields");
        
}

//______________________Unstack Fields_________________________________________
AVDMVideoUnStackField::AVDMVideoUnStackField(       AVDMGenericVideoStream *in,CONFcouple *setup)
{
UNUSED_ARG(setup);
        _in=in;
        memcpy(&_info,_in->getInfo(),sizeof(_info));    
        _uncompressed=new ADMImage(_info.width,_info.height);   

}

// ___ destructor_____________
AVDMVideoUnStackField::~AVDMVideoUnStackField()
{
                delete _uncompressed;
                _uncompressed=NULL;
}
//*************************************************************
/**
        Interleave frame*2 and frame*2+1
*/
uint8_t AVDMVideoUnStackField::getFrameNumberNoAlloc(uint32_t frame,
                                uint32_t *len,
                                ADMImage *data,
                                uint32_t *flags)
{
uint32_t ref,ref2;
ADMImage *ptr1,*ptr2;
                if(frame>=_info.nb_frames) return 0;

                 if(!_in->getFrameNumberNoAlloc(frame, len, _uncompressed, flags)) return 0;
                 
                  vidFielUnStack(_info.width ,_info.height,YPLANE(_uncompressed),YPLANE(data));
                data->copyInfo(_uncompressed);  
      return 1;
}
