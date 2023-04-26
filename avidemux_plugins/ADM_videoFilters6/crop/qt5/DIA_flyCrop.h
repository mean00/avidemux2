#pragma once

class flyCrop;

class flyCrop : public ADM_flyDialogRgb
{
    ADM_rubberControl *rubber;
    double      ar;
    int         _ox,_oy,_ow,_oh;
    uint32_t    left,right,top,bottom;
    bool        rubber_is_hidden;
    int         ar_select;
    int         _lw,_lh;

    void        dimensions();
    int         autoRun(uint8_t *in,int w,int h, int increment, int blackLevel);
    int         autoRunV(uint8_t *in, int stride, int w, int increment, int blackLevel);

public:
    uint8_t     processRgb(uint8_t *imageIn, uint8_t *imageOut);
    uint8_t     download(void) {return download(false);}
    uint8_t     download(bool even);
    uint8_t     upload(void) {return upload(true,true);}
    uint8_t     upload(bool redraw, bool toRubber);
    uint8_t     autocrop(void);
    bool        bandResized(int x,int y,int w, int h);
    bool        bandMoved(int x,int y,int w, int h);
    bool        blockChanges(bool block);

    void        setTabOrder(void);

    float       getZoomValue(void) {return _zoom;}
    double      getAspectRatio(void) {return ar;}
    int         getAspectRatioIndex(void) {return ar_select;}
    void        setAspectRatioIndex(int index);
    bool        getKeepAspect(void) {return ar_select > 0;}
    void        lockDimensions(void) {_lw=_w-left-right; _lh=_h-top-bottom;};

    bool        getCropMargins(int *marginLeft, int *marginRight, int *marginTop, int *marginBottom);
    void        setCropMargins(int marginLeft, int marginRight, int marginTop, int marginBottom);

    void        hideRubber(bool hidden);
    void        hideRubberGrips(bool hideTopLeft, bool hideBottomRight);
    void        adjustRubber(int x, int y, int w, int h);
    bool        stateOfRubber(void) {return rubber_is_hidden;}
    int         lockRubber(bool lock);

                flyCrop (QDialog *parent, uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                         ADM_QCanvas *canvas, ADM_flyNavSlider *slider);
    virtual     ~flyCrop();
};

