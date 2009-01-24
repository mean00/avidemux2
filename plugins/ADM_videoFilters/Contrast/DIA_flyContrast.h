

#ifndef FLY_CONTRAST_H
#define FLY_CONTRAST_H
class flyContrast : public FLY_DIALOG_TYPE
{
  
  public:
   CONTRAST_PARAM  param;
  public:
   uint8_t    process(void);
   uint8_t    download(void);
   uint8_t    upload(void);
   uint8_t    update(void);
   flyContrast (uint32_t width,uint32_t height,AVDMGenericVideoStream *in,
                                    void *canvas, void *slider) : FLY_DIALOG_TYPE(width, height,in,canvas, slider,1,RESIZE_AUTO)
                    {
                      
                    };
};

#endif
