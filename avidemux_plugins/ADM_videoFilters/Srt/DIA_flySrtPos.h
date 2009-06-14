

#ifndef FLY_SRTPOS_H
#define FLY_SRTPOS_H
typedef struct
{
    uint32_t fontSize;
    uint32_t position;
}SRT_POS_PARAM;

class flySrtPos : public FLY_DIALOG_TYPE
{
  
  public:
   SRT_POS_PARAM  param;
  public:
   uint8_t    process(void);
   uint8_t    download(void);
   uint8_t    upload(void);
   uint8_t    update(void);
   flySrtPos (uint32_t width,uint32_t height,AVDMGenericVideoStream *in,
                                    void *canvas, void *slider) : FLY_DIALOG_TYPE(width, height,in,canvas, slider,1,RESIZE_AUTO) {};
};

#endif
