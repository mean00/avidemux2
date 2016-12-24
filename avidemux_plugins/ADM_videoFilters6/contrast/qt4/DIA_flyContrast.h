

#ifndef FLY_CONTRAST_H
#define FLY_CONTRAST_H
#include "contrast.h"
/**
    \class flyContrast
*/
class flyContrast : public ADM_flyDialogQt4
{
  public:
   contrast     param;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);

   uint8_t    download(void);
   uint8_t    upload(void);
   uint8_t    update(void);
   flyContrast (uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, QSlider *slider) 
                : ADM_flyDialogQt4(width, height,in,canvas, slider,1,RESIZE_AUTO) {};
};
#endif
