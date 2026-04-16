#ifndef T_preview_h
#define T_preview_h

#include <QFrame>
#include <QPaintEvent>
#include <QWidget>

/**
 *
 */
class ADM_QvideoDrawer
{
  public:
    virtual ~ADM_QvideoDrawer()
    {
    }
    virtual bool draw(QWidget *widget, QPaintEvent *ev) = 0;
};

/**
    \class ADM_Qvideo
*/
class ADM_Qvideo : public QWidget
{
    Q_OBJECT
    ADM_QvideoDrawer *drawer;
    bool doOnce;
    QFrame *hostFrame;

  protected:
    int _width, _height;

  public:
    ADM_Qvideo(QFrame *z);
    ~ADM_Qvideo();
    void paintEvent(QPaintEvent *ev);
    void setADMSize(int width, int height);
    bool eventFilter(QObject *obj, QEvent *event) override;
    void setDrawer(ADM_QvideoDrawer *d)
    {
        drawer = d;
    }
    // This disables internal double buffer of Qt
    // Set it to false if the native Qt redraw system is used
    QPaintEngine *paintEngine() const
    {
        return NULL; // Disable
    }
    void useExternalRedraw(bool external)
    {
#ifndef __APPLE__
        setAttribute(Qt::WA_PaintOnScreen, external);
#endif
    }
    QSize sizeHint() const
    {
        if (!_width || !_height)
            return QWidget::sizeHint();
        return QSize(_width, _height);
    }
};
#endif // T_preview_h
