#ifndef T_preview_h
#define T_preview_h

#include <QPaintEvent>
#include <QWidget>
#include <QFrame>

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
        QFrame           *hostFrame;
protected:
        int _width,_height;
public:
	ADM_Qvideo(QFrame *z);
	~ADM_Qvideo();
	void paintEvent(QPaintEvent *ev);
        void setDrawer(ADM_QvideoDrawer *d)
        {
            drawer=d;    
        }
        // This disables internal double buffer of Qt
        // Set it to false if the native Qt redraw system is used
        void useExternalRedraw(bool external)
        {
                setAttribute( Qt::WA_PaintOnScreen, external );
        }
        void  setADMSize(int width,int height)
        {
             _width=width;
             _height=height;
             hostFrame->setFixedSize(_width,_height);
             hostFrame->adjustSize();
             hostFrame->updateGeometry();
             setFixedSize(_width,_height);             
             this->adjustSize();
             this->updateGeometry();
        }
        QSize sizeHint() const 
        {
            if(!_width || !_height) return QWidget::sizeHint();
            return QSize(_width,_height);
        }

};
#endif	// T_preview_h
