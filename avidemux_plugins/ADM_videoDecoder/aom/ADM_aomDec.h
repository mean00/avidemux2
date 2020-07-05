/***************************************************************************
            \file  ADM_aomDec.h
            \brief I/f to libaom (decoder)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_AOMDEC_H
#define ADM_AOMDEC_H
#include "ADM_codec.h"
/**
    \class decoderAom
*/
class decoderAom : public decoders
{
protected:
            bool drain;
            bool alive;
            void *cookie;
public:
            decoderAom (uint32_t w, uint32_t h, uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData, uint32_t bpp);
            ~decoderAom();
    virtual bool uncompress(ADMCompressedImage *in, ADMImage *out);
    virtual void setDrainingState(bool yesno) { drain=yesno; }
    virtual bool getDrainingState(void) { return drain; }
    virtual bool flush(void);
    virtual bool bFramePossible (void) { return false; }
    virtual const char *getDecoderName(void) { return "libaom"; }
    virtual bool initializedOk(void) { return alive; }
    virtual bool dontcopy(void) {return true; }
};

#endif
//EOF

