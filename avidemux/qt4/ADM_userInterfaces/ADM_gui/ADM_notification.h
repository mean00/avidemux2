// MIT License
// 
// Copyright (c) 2023 Emre
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#ifndef ADM_NOTIFICATION_H
#define ADM_NOTIFICATION_H

#include <QWidget>
#include <QFrame>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QTime>
#include <QTimer>
#include <QLabel>
#include <QGridLayout>
#include <QPainter>
#include <QDesktopWidget>
#include <qscreen.h>
#include <QApplication>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Notification; }
QT_END_NAMESPACE


class  ADM_notification : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(float popupOpacity READ getPopupOpacity WRITE setPopupOpacity)
    Q_PROPERTY(float timelapse READ getTimelapse WRITE setTimelapse)

    void setPopupOpacity(float opacity);
    float getPopupOpacity() const;    
    

    void setTimelapse(float timelapse);
    float getTimelapse() const;

    QMainWindow* _parent = nullptr;
public:

   

     ADM_notification(QMainWindow *parent = nullptr, bool transparent = true); // if parent is null, Desktop will be the parent.
    ~ ADM_notification();

    enum NOTIFICATION_STYLE {
        INFO,
        WARNING,
        ERROR
    };

    enum NOTIFICATION_DIRECTION {        
        RIGHT_TO_LEFT,
        TOP_TO_BOTTOM,
        BOTTOM_TO_TOP,        
        LEFT_TO_RIGHT
    };

    void setText(QString text);
    void setDuration(int msec);
    void setShowDuration(uint msec);
    void setHideDuration(uint msec);
    void setStyle(NOTIFICATION_STYLE style);
    void setIcon(QPixmap pixmap);
    void setOpacity(uint opacity);

    
    QRect getParentGeometry() const;
    int   getCurrentIndex()   const;
    bool  parentIsDesktop()   const;

    void  moveCustom(QPoint rect, bool animated = true);
    void  moveanimated(int x, int y, bool animated = true) { moveCustom(QPoint(x, y), animated); }
    void  Quit();

    QPropertyAnimation* createAnimation(QPoint pointStart, QPoint pointEnd);
    static void updateAllLocations(bool animated = false); // For update locations of all Toastr widgets
private:
    void calculateStartXY(QRect rect, NOTIFICATION_DIRECTION direction, QRect &out_rect);
    void updateLocations(bool animated = true);


    void deleteFromList( ADM_notification* target);
public slots:
    void show(NOTIFICATION_DIRECTION direction);

private slots:
    void hideAnimation();
    void hide();

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    bool m_closing = false; // When toastr hiding, if user resize or move parent widget it will cause a bug, for fix that.
    Ui::Notification* ui;

    static uint COUNT_NOTIFICATION;
    static QVector< ADM_notification*> vectorNotification;
    

    QTimer *timer;
    int displayTimeout = 2000;
    uint showTimeout = 150;
    uint hideTimeout = 500;
    uint animateTimeout = 350;

    NOTIFICATION_DIRECTION directionShow = NOTIFICATION_DIRECTION::RIGHT_TO_LEFT;
    NOTIFICATION_STYLE currentStyle;
    QPropertyAnimation animationOpacity;
    QPropertyAnimation* animationSlide = nullptr;
    float popupOpacity=1.0f;
    float timeLapse=1.0f;

    QColor backgroundColor = Qt::gray;
    QColor frameColor = Qt::black;
    uint opacity = 200;

    const static uchar ROUND_RECT{10};
    const static uchar MARGIN_WIDTH{36};
    const static uchar SPACE_BETWEEN{10};

    bool m_iconVisible = true;
    bool m_transparent = true;

protected:
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual QSize sizeHint() const;
};


#endif // ADM_NOTIFICATION_H
