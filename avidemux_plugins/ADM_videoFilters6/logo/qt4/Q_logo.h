#ifndef Q_mpdelogo_h
#define Q_mpdelogo_h

#include <QMouseEvent>
#include <QPainter>

#include "ui_logo.h"
#include "ADM_image.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyLogo.h"


class ADM_LogoCanvas : public ADM_QCanvas
{
    Q_OBJECT

public:
                ADM_LogoCanvas(QWidget *parent, uint32_t w, uint32_t h) : ADM_QCanvas(parent,w,h) {};
        virtual ~ADM_LogoCanvas() {};

protected:
        void    mouseReleaseEvent(QMouseEvent * event);

signals:
        void    movedSignal(int newx, int newy);
};

class Ui_logoWindow : public QDialog
{
	Q_OBJECT

protected: 
        int                 lock;
        std::string         lastFolder;
        Ui_logoDialog       ui;
        ADM_coreVideoFilter *_in;
        flyLogo             *myLogo;
        ADM_LogoCanvas      *canvas;

        float               imageScale;
        bool                tryToLoadimage(const char *image);
        bool                enableLowPart(void);
public:
        ADMImage            *image;
        ADMImage            *scaledImage;
        std::string         imageName;

public:
                            Ui_logoWindow(QWidget *parent, logo *param, ADM_coreVideoFilter *in);
                            ~Ui_logoWindow();
public slots:
	void                gather(logo *param);

private slots:
	void                sliderUpdate(int foo);
	void                valueChanged(int foo);
        void                valueChanged(double foo);
        void                moved(int x, int y);
        void                scaleChanged(double foo);
        void                imageSelect();

private:
        void                resizeEvent(QResizeEvent *event);
        void                showEvent(QShowEvent *event);
};
#endif
