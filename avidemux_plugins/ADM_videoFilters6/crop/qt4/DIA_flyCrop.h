#ifndef FLY_CROP_H
#define FLY_CROP_H
class flyCrop : public ADM_flyDialogQt4
{
  
  public:
   uint32_t left,right,top,bottom;
  public:
   uint8_t    processRgb(uint8_t *imageIn, uint8_t *imageOut);
   uint8_t    download(void);
   uint8_t    upload(void);
   uint8_t    autocrop(void);
              flyCrop (uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, QSlider *slider) 
                : ADM_flyDialogQt4(width, height,in,canvas, slider,0,RESIZE_LAST) {};
};
#endif
