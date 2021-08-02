#pragma once
class flyBlacken : public ADM_flyDialogRgb
{
public:
    uint32_t    left,right,top,bottom;
    bool        rubber_is_hidden;

    float       getZoomValue(void) { return _zoom; }
    void        initRubber(void);
    void        adjustRubber(int x, int y, int w, int h);
    void        hideRubber(bool hide);
    int         lockRubber(bool lock);
    bool        blockChanges(bool block);

    uint8_t     processRgb(uint8_t *imageIn, uint8_t *imageOut);
    uint8_t     download(void);
    uint8_t     upload(void) {return upload(true,true);}
    void        setTabOrder(void);
                flyBlacken(QDialog *parent, uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider);
                ~flyBlacken();
protected:
    ADM_rubberControl *rubber;
    uint8_t     upload(bool redraw, bool rubber);
    bool        bandResized(int x,int y,int w, int h);
    bool        bandMoved(int x,int y,int w, int h);
    int         _ox,_oy,_ow,_oh;

};

