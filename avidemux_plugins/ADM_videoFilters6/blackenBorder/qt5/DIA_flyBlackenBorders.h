#pragma once
class flyBlacken : public ADM_flyDialogRgb
{
  friend class Ui_blackenWindow;
  public:
   uint32_t   left,right,top,bottom;
   bool       rubber_is_hidden;
  public:
   uint8_t    processRgb(uint8_t *imageIn, uint8_t *imageOut);
   uint8_t    download(void);
   uint8_t    upload(void) {return upload(true,true);}
   uint8_t    upload(bool redraw, bool rubber);
   uint8_t    autocrop(void);
              flyBlacken (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider);
              ~flyBlacken();
protected:
                ADM_rubberControl *rubber;
    bool        blockChanges(bool block);
    bool        bandResized(int x,int y,int w, int h);
    int         _ox,_oy,_ow,_oh;

};

