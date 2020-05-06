#pragma once

class flyCrop;

class flyCrop : public ADM_flyDialogRgb
{
  friend class Ui_cropWindow;
  public:
   uint32_t left,right,top,bottom;
   bool rubber_is_hidden;
  public:
   uint8_t    processRgb(uint8_t *imageIn, uint8_t *imageOut);
   uint8_t    download(void) {return download(false);}
   uint8_t    download(bool even);
   uint8_t    upload(void) {return upload(true,true);}
   uint8_t    upload(bool redraw, bool toRubber);
   uint8_t    autocrop(void);
   bool       bandResized(int x,int y,int w, int h);
              flyCrop (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider);
    virtual   ~flyCrop();
protected:
    ADM_rubberControl *rubber;
    bool        blockChanges(bool block);
    int         autoRun(uint8_t *in,int w,int h, int increment);
    int         autoRunV(uint8_t *in, int stride, int w, int increment);
private:
    int _ox,_oy,_ow,_oh;
    void dimensions();
};

