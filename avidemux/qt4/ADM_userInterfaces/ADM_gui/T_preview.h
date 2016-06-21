#ifndef T_preview_h
#define T_preview_h

#include <QPaintEvent>
#include <QWidget>

/**
 * 
 */
class ADM_QvideoDrawer
{
public:  
    virtual ~ADM_QvideoDrawer() {}
    virtual bool    draw(QWidget *widget, QPaintEvent *ev)=0;
};

/**
    \class ADM_Qvideo
*/
class  ADM_Qvideo : public QWidget
{
	Q_OBJECT
        ADM_QvideoDrawer *drawer;
        bool             doOnce;
public:
	ADM_Qvideo(QWidget *z);
	~ADM_Qvideo();
	void paintEvent(QPaintEvent *ev);
        void setDrawer(ADM_QvideoDrawer *d)
        {
            drawer=d;    
        }
};
#endif	// T_preview_h
