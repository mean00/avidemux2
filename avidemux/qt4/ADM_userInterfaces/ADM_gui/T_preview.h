#ifndef T_preview_h
#define T_preview_h

#include <QPaintEvent>
#include <QWidget>
/**
    \class ADM_Qvideo
*/
class  ADM_Qvideo : public QWidget
{
	Q_OBJECT

public:
	ADM_Qvideo(QWidget *z);
	~ADM_Qvideo();
	void paintEvent(QPaintEvent *ev);
};
#endif	// T_preview_h
