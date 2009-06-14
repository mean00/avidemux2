

#ifndef FLY_ASHARP_H
#define FLY_ASHARP_H
class flyASharp : public FLY_DIALOG_TYPE
{
  
  public:
   ASHARP_PARAM  param;
  public:
   uint8_t    process(void);
   uint8_t    download(void);
   uint8_t    upload(void);
   uint8_t    update(void);
   flyASharp (uint32_t width,uint32_t height,AVDMGenericVideoStream *in,
                                    void *canvas, void *slider) : FLY_DIALOG_TYPE(width, height,in,canvas, slider,1,RESIZE_AUTO) {};
};
#endif
