#ifndef T_vumeter_h
#define T_vumeter_h

#include <QtGui/QPaintEvent>
#include <QtGui/QWidget>
#include <QtGui/QFrame>
#include "ADM_inttype.h"
/**
    \fn class ADM_Qvumeter
*/
class  ADM_Qvumeter : public QWidget
{
	Q_OBJECT

public:
	ADM_Qvumeter(QWidget *z);
	~ADM_Qvumeter();
	void paintEvent(QPaintEvent *ev);
};

bool UI_InitVUMeter(QFrame *host);
bool UI_vuUpdate(uint32_t volume[6]);
#endif	// T_preview_h
