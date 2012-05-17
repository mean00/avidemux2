#ifndef FLY_CHROMASHIFT_H
#define FLY_CHROMASHIFT_H
#include "chromashift.h"
/**
    \class flyChromaShift
*/  
class flyChromaShift : public FLY_DIALOG_TYPE
{
  
  public:
   chromashift     param;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);

   uint8_t    download(void);
   uint8_t    upload(void);
   uint8_t    update(void);
   flyChromaShift (uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    void *canvas, void *slider) 
                : FLY_DIALOG_TYPE(width, height,in,canvas, slider,1,RESIZE_AUTO) {};
};
#endif
//EOF
