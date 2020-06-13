#ifndef FLY_CROP_H
#define FLY_CROP_H
class flyCrop : public ADM_flyDialogRgb
{
  
  public:
   uint32_t left,right,top,bottom;
  public:
   uint8_t    processRgb(uint8_t *imageIn, uint8_t *imageOut);
   uint8_t    download(void) {return download(false);}
   uint8_t    download(bool even);
   uint8_t    upload(void);
   uint8_t    autocrop(void);
              flyCrop (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider)
                : ADM_flyDialogRgb(parent,width, height,in,canvas, slider,RESIZE_LAST) {};
};
#endif
