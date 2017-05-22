#pragma once

#include "ADM_codecFFsimple.h"
/**
    \class decoderFF
    \brief Base class for lavcodec based decoder
*/
class ADM_COREVIDEOCODEC6_EXPORT decoderFFVP9:public decoderFFSimple
{
protected:

protected:

public:
                        decoderFFVP9 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
        virtual         ~ decoderFFVP9 ();
        virtual bool    uncompress (ADMCompressedImage * in, ADMImage * out);
        virtual const char *getDecoderName(void) 
                          {
                              if(hwDecoder)
                                  return hwDecoder->getName();
                              return "Lavcodec VP9";
                          }
};