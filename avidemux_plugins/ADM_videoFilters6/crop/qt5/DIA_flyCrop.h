#pragma once

class flyCrop;

class flyCrop : public ADM_flyDialogRgb
{
    ADM_rubberControl *rubber;
    double      ar;
    int         _ox,_oy,_ow,_oh;
    uint32_t    left,right,top,bottom;
    bool        rubber_is_hidden;
    bool        drag_enabled;
    bool        keep_aspect;
    int         ar_select;
    int         _lw,_lh;

    void        dimensions();
    int         autoRun(uint8_t *in,int w,int h, int increment);
    int         autoRunV(uint8_t *in, int stride, int w, int increment);

public:
    uint8_t     processRgb(uint8_t *imageIn, uint8_t *imageOut);
    uint8_t     download(void) {return download(false);}
    uint8_t     download(bool even);
    uint8_t     upload(void) {return upload(true,true);}
    uint8_t     upload(bool redraw, bool toRubber);
    uint8_t     autocrop(void);
    bool        bandResized(int x,int y,int w, int h);
    bool        blockChanges(bool block);

    float       getZoomValue(void) {return _zoom;}
    double      getAspectRatio(void) {return ar;}
    int         getAspectRatioIndex(void) {return ar_select;}
    void        setAspectRatioIndex(int index);
    bool        getDragEnabled(void) {return drag_enabled;}
    void        setDragEnabled(bool drag) {drag_enabled=drag;}
    bool        getKeepAspect(void) {return keep_aspect;}
    void        setKeepAspect(bool keep) {keep_aspect=keep;}
    void        lockDimensions(void) {_lw=_w-left-right; _lh=_h-top-bottom;};
    void        getDimensions(int * w, int * h) {if (w) *w=_lw; if(h) *h=_lh;};

    bool        getCropMargins(int *marginLeft, int *marginRight, int *marginTop, int *marginBottom);
    void        setCropMargins(int marginLeft, int marginRight, int marginTop, int marginBottom);

    void        initRubber(void);
    void        hideRubber(bool hidden);
    void        hideRubberGrips(bool hideTopLeft, bool hideBottomRight);
    void        adjustRubber(int x, int y, int w, int h);
    bool        stateOfRubber(void) {return rubber_is_hidden;}
    int         lockRubber(bool lock);

                flyCrop (QDialog *parent, uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                         ADM_QCanvas *canvas, ADM_QSlider *slider);
    virtual     ~flyCrop();
};

