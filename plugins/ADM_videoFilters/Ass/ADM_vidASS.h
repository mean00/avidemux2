/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ADM_ASS_H
#define _ADM_ASS_H



extern "C"
{
#include "ADM_libass/ass.h"
}
#include "ADM_vidAss_Params.h" 
class ADMVideoSubASS : public AVDMGenericVideoStream 
{
protected:
        virtual char* printConf(void);
        ASSParams* _params;
        ass_library_t *_ass_lib;
        ass_renderer_t *_ass_rend;
        ass_track_t *_ass_track;
        uint8_t init(void);
public:
        ADMVideoSubASS(AVDMGenericVideoStream *in, CONFcouple *conf);
        virtual ~ADMVideoSubASS();
        virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len, ADMImage *data,uint32_t *flags);
        uint8_t configure(AVDMGenericVideoStream *instream);
        uint8_t	getCoupledConf(CONFcouple **conf);
};

#endif
