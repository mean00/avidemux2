

#ifndef FLY_ASHARP_H
#define FLY_ASHARP_H
/**
    \class flyASharp
*/
#include "asharp.h"
#define uc uint8_t
class flyASharp : public FLY_DIALOG_TYPE
{
  
  public:
   asharp     param;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);

   uint8_t    download(void);
   uint8_t    upload(void);
   uint8_t    update(void);
   flyASharp (uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    void *canvas, void *slider) : 
                FLY_DIALOG_TYPE(width, height,in,canvas, slider,1,RESIZE_AUTO) {};
};

void asharp_run_c(      uc* planeptr, int pitch,
                                        int height, int width, 
                                        int     T,int D, int B, int B2, bool bf );


#endif
