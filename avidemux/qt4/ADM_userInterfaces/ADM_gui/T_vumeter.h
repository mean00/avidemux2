#ifndef T_vumeter_h
#define T_vumeter_h

#include <QPaintEvent>
#include <QWidget>
#include <QFrame>
#include "ADM_inttype.h"
/**
    \fn class ADM_Qvumeter
*/
class  ADM_Qvumeter : public QWidget
{
    Q_OBJECT
public:
    typedef struct {
        double maxPos;
        uint32_t maxColor;
    } peak_t;

    peak_t peaks[8];

    uint8_t *rgbDataBuffer;

    ADM_Qvumeter(QWidget *z, int width, int height);
    ~ADM_Qvumeter();
    void paintEvent(QPaintEvent *ev);
};

bool UI_InitVUMeter(QFrame *host);
bool UI_vuUpdate(uint32_t volume[8]);
#endif	// T_preview_h
