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

#include "ADM_notification.h"
#include "ui_notification.h"

uint  ADM_notification::COUNT_NOTIFICATION = 0;
QVector< ADM_notification*>  ADM_notification::vectorNotification;

void  ADM_notification::setPopupOpacity(float opacity)
{
    popupOpacity = opacity;
    setWindowOpacity(opacity);
}

float  ADM_notification::getPopupOpacity() const { return popupOpacity; }

void  ADM_notification::setTimelapse(float timelapse)
{
    this->timeLapse = timelapse;
    if (timelapse == 0.0f && m_transparent)
        setPopupOpacity(0.85f); // set opacity when toastr slides (hiding)
    else
        setPopupOpacity(1.0f);
}

float  ADM_notification::getTimelapse() const { return this->timeLapse; }

QRect  ADM_notification::getParentGeometry() const
{
    if (parentIsDesktop())// is parent null (it means parent is Desktop)
    {
        QScreen* screen = QGuiApplication::primaryScreen();
        QRect geometry = screen->geometry();
        return geometry;
        //return QApplication::desktop()->geometry();
    }
    else {
        //_parent->adjustSize();
        auto geo = _parent->geometry();
        return geo;
    }
}

void  ADM_notification::Quit()
{
    QWidget::hide();
    COUNT_NOTIFICATION--;
    deleteFromList(this);
    updateAllLocations(true);
    delete this;
}
bool  ADM_notification::parentIsDesktop() const { return (this->_parent == nullptr); }

void  ADM_notification::calculateStartXY(QRect rect, NOTIFICATION_DIRECTION direction, QRect& out_rect)
{
    out_rect = rect;
    const QRect targetGeometry = getParentGeometry();    
    const int screen_w = targetGeometry.width();
    const int screen_h = targetGeometry.height();


    const int save_h = out_rect.height(); // when we use setY function, height too changes so we need to save it first then change Y coordinate and restore back height at the end.

    if (direction == NOTIFICATION_DIRECTION::RIGHT_TO_LEFT) {
        out_rect.setX(screen_w + MARGIN_WIDTH);
        out_rect.setY(MARGIN_WIDTH);
    }
    else if (direction == NOTIFICATION_DIRECTION::LEFT_TO_RIGHT) {
        out_rect.setX(-MARGIN_WIDTH);
        out_rect.setY(MARGIN_WIDTH);
    }
    else if (direction == NOTIFICATION_DIRECTION::TOP_TO_BOTTOM) {
        out_rect.setX((screen_w - width())/2);
        out_rect.setY(-MARGIN_WIDTH);
    }
    else if (direction == NOTIFICATION_DIRECTION::BOTTOM_TO_TOP) {
        out_rect.setX((screen_w - width())/2);
        out_rect.setY(screen_h+MARGIN_WIDTH);
    }
    out_rect.setHeight(save_h); // if we don't do that, height will return minmumSize always (30)
}
void  ADM_notification::updateLocations(bool animated)
{
    if (m_closing) // prevent animation on closing widget
        return;
    const QRect targetGeometry = getParentGeometry();
    const int screen_x = targetGeometry.x(); // for multiple monitors

    const int screen_w = targetGeometry.width();
    const int screen_h = targetGeometry.height();

    const int START_Y = MARGIN_WIDTH + (this->getCurrentIndex() * (this->height() + SPACE_BETWEEN));

    switch (directionShow) {
        case NOTIFICATION_DIRECTION::RIGHT_TO_LEFT:
        {
            moveanimated(screen_w - MARGIN_WIDTH - width(),
                START_Y, animated);
        } break;
        case NOTIFICATION_DIRECTION::LEFT_TO_RIGHT:
        {
            moveanimated(MARGIN_WIDTH,
                START_Y, animated);
        }break;
        case NOTIFICATION_DIRECTION::BOTTOM_TO_TOP:
        {
            moveanimated((screen_w - width())/2,
                screen_h - (START_Y + height()), animated);
        }break;
        case NOTIFICATION_DIRECTION::TOP_TO_BOTTOM:
        {
            moveanimated((screen_w - width())/ 2,
                START_Y, animated);
        }break;
    }
}

void  ADM_notification::updateAllLocations(bool animated)
{
    for (int i = 0; i < vectorNotification.length(); i++) {
        auto data = vectorNotification.at(i);
        data->updateLocations(animated);
    }
}

void  ADM_notification::deleteFromList( ADM_notification* target)
{
    const int myIndex = vectorNotification.indexOf(target);
    if (myIndex > -1 && myIndex < vectorNotification.count())
        vectorNotification.remove(myIndex);
}


int  ADM_notification::getCurrentIndex() const
{    
    auto it = std::find(this->vectorNotification.begin(), this->vectorNotification.end(), this);
    if (it != this->vectorNotification.end())
        return (int)(it - this->vectorNotification.begin());

    return -1;
}

 ADM_notification:: ADM_notification(QMainWindow* parent, bool transparent)
    : QFrame(parent)
    , ui(new Ui::Notification)
{
    ui->setupUi(this);
    _parent = parent;
    setParent(_parent);
    this->setAttribute(Qt::WA_Hover, true);
    setWindowFlags(Qt::FramelessWindowHint |
        (parentIsDesktop() ? Qt::Tool : Qt::Widget) |
        (parentIsDesktop() ? Qt::WindowStaysOnTopHint : Qt::Widget));
    m_transparent = transparent;
    if (m_transparent)
        setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(20);
    effect->setXOffset(4);
    effect->setYOffset(4);
    setGraphicsEffect(effect);

    QFont font = ui->textLabel->font();
    font.setWeight(QFont::Medium);
    ui->textLabel->setFont(font);
    m_iconVisible = true; ui->iconLabel->setVisible(true);

    animationOpacity.setTargetObject(this);
    //animationOpacity.setPropertyName("popupOpacity");
    animationOpacity.setPropertyName("timelapse");
    connect(&animationOpacity, &QAbstractAnimation::finished, this, & ADM_notification::hide);



    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, & ADM_notification::hideAnimation);
}

 ADM_notification::~ ADM_notification() { delete ui; }

void  ADM_notification::moveCustom(QPoint tarPoint, bool animated)
{
    if (!animated)
        move(tarPoint);
    else {
        QPoint curPoint = pos();
        createAnimation(curPoint, tarPoint);
    }
}

QPropertyAnimation*  ADM_notification::createAnimation(QPoint pointStart, QPoint pointEnd) {    
    QPropertyAnimation* anim = new QPropertyAnimation(this, "pos");
    anim->setStartValue(pointStart);
    anim->setEndValue(pointEnd);
    anim->setDuration(animateTimeout);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QPropertyAnimation::DeleteWhenStopped);
    return anim;
}

void  ADM_notification::setText(QString text) { ui->textLabel->setText(text); }

void  ADM_notification::setDuration(int msec) { this->displayTimeout = msec; }

void  ADM_notification::setShowDuration(uint msec) { this->showTimeout = msec; }

void  ADM_notification::setHideDuration(uint msec) { this->hideTimeout = msec; }

void  ADM_notification::setStyle(NOTIFICATION_STYLE style)
{
    QString imgpath = ":/new/prefix1/pics/";

    auto palette = QApplication::palette();
    backgroundColor = palette.color(QPalette::Window);
    frameColor = palette.color(QPalette::WindowText);
    if (m_transparent)
    {
        backgroundColor.setAlpha(this->opacity);
        frameColor.setAlpha(this->opacity);
    }

    switch (style)
    {
        case NOTIFICATION_STYLE::INFO: {
            QPixmap icon = QPixmap(imgpath+"notify_info.png");
            ui->iconLabel->setPixmap(icon);
        } break;
        case NOTIFICATION_STYLE::WARNING: {
            QPixmap icon = QPixmap(imgpath+"notify_warning.png");
            ui->iconLabel->setPixmap(icon);
        } break;
        case NOTIFICATION_STYLE::ERROR: {
            QPixmap icon = QPixmap(imgpath+"notify_error.png");
            ui->iconLabel->setPixmap(icon);
        } break;
    }
}

void  ADM_notification::setIcon(QPixmap pixmap)
{
    ui->iconLabel->show();
    ui->iconLabel->setPixmap(pixmap);
}

void  ADM_notification::setOpacity(uint opacity)
{
    if (m_transparent)
    {
        this->backgroundColor.setAlpha(opacity);
        this->frameColor.setAlpha(opacity);
        this->opacity = opacity;
    }
}

void  ADM_notification::hide()
{
    //if (getPopupOpacity() == 0.0) {
    if (getTimelapse() == 0.0){
        QPoint pointCurrent = pos();
        QPoint pointEnd = pos();
        int deltaX = 0, deltaY = 0;

        if (directionShow == NOTIFICATION_DIRECTION::RIGHT_TO_LEFT)
            deltaX =  (width() + MARGIN_WIDTH);
        else if (directionShow == NOTIFICATION_DIRECTION::LEFT_TO_RIGHT)
            deltaX = -(width() + MARGIN_WIDTH);
        else if (directionShow == NOTIFICATION_DIRECTION::BOTTOM_TO_TOP)
            deltaY =  (height() + MARGIN_WIDTH);
        else if (directionShow == NOTIFICATION_DIRECTION::TOP_TO_BOTTOM)
            deltaY = -(height() + MARGIN_WIDTH);

        

        pointEnd.setX(pointEnd.x() + deltaX);
        pointEnd.setY(pointEnd.y() + deltaY);

        //rectEnd.setWidth(geoCurrent.width()); // when rectEnd.x changes, rectEnd.width too changes. we don't want that.
        //rectEnd.setHeight(geoCurrent.height());// when rectEnd.y changes, rectEnd.height too changes. we don't want that.
        animationSlide = createAnimation(pointCurrent, pointEnd);

        m_closing = true; // Fix for Animation combine bug (2 animations at the same time)
    
        if(true)connect(animationSlide, &QPropertyAnimation::finished, 
            [this]() {
                this->Quit();
        });
    }
}

void  ADM_notification::show(NOTIFICATION_DIRECTION direction)
{
    directionShow = direction;
    vectorNotification.append(this);
    COUNT_NOTIFICATION++;
    adjustSize();

    QRect curRect = geometry(); QRect outRect;
    calculateStartXY(curRect, direction, outRect);
    setGeometry(outRect);


    //setWindowOpacity(0.0); 

    animationOpacity.setDuration(this->showTimeout);
    animationOpacity.setStartValue(0.0);
    animationOpacity.setEndValue(1.0);

    updateLocations();

    QWidget::show();
    QObject::connect(&animationOpacity, &QPropertyAnimation::valueChanged, [this]() {
        this->update();
    });

    animationOpacity.start();
    timer->start(this->displayTimeout);
}

void  ADM_notification::hideAnimation()
{
    if (this->displayTimeout < 0)
        return;
    timer->stop();
    animationOpacity.setDuration(this->hideTimeout);
    animationOpacity.setStartValue(1.0f);
    animationOpacity.setEndValue(0.0f);
    animationOpacity.start();
}

void  ADM_notification::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter paint;
    paint.begin(this);
    paint.setRenderHints(QPainter::Antialiasing);
    paint.setBrush(QBrush(this->backgroundColor));
    paint.setPen(Qt::NoPen);
    auto calcRect = this->rect();
    //calcRect.setX((calcRect.width() * animation.currentValue().toFloat()));
    if (m_transparent)
        paint.drawRoundedRect(calcRect, ROUND_RECT, ROUND_RECT);
    else
        paint.drawRect(calcRect);
    QPainterPath path;
    if (m_transparent)
        path.addRoundedRect(calcRect, ROUND_RECT, ROUND_RECT);
    else
        path.addRect(calcRect);
    QPen pen(this->frameColor, 1);
    //paint.setPen(pen);
    paint.strokePath(path, pen);
    paint.end();   
}

void  ADM_notification::enterEvent(QEvent* event)
{
    timer->stop();
    this->backgroundColor.setAlpha(255);
    this->frameColor.setAlpha(255);
    QWidget::enterEvent(event);
}

void  ADM_notification::leaveEvent(QEvent* event)
{
    timer->start();
    if (m_transparent)
    {
        this->backgroundColor.setAlpha(opacity);
        this->frameColor.setAlpha(opacity);
    }
    QWidget::leaveEvent(event);
}

void  ADM_notification::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    Quit();    
}
QSize  ADM_notification::sizeHint() const{

    QSize result{maximumWidth(), MARGIN_WIDTH};

    if (parentIsDesktop())
        return result;

    auto cm = layout()->contentsMargins();
    //result += QSize(0, cm.top() + cm.bottom());


    ui->textLabel->adjustSize();
    ui->iconLabel->adjustSize();
    QSize size_label_text = ui->textLabel->size();
    QSize size_label_icon = ui->iconLabel->size();
    QFontMetrics fm(ui->textLabel->font());
    int availableWidth = maximumWidth();
    if (m_iconVisible)
        availableWidth -= ui->iconLabel->width();

    // For calculate text height / Yazi uzun oldugunda asagi dogru widgetin uzamasi icin
    QRect text_rect = fm.boundingRect(QRect(0, 0,
                                           availableWidth - cm.top() - cm.bottom(), 0),
                                     Qt::TextDontClip | Qt::TextWordWrap, ui->textLabel->text());


    result+= QSize(0, std::max(text_rect.height(), ui->iconLabel->height()));
    //return QSize(200,200); // test
    return result;
}
