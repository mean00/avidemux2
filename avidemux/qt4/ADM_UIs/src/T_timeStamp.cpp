/***************************************************************************
  FAC_timeStamp.cpp
  Handle dialog factory element : timestamp
  (C) 2006 Mean Fixounet@free.fr 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include <QClipboard>
#include "ADM_default.h"
#include "ADM_vidMisc.h"
#include "T_timeStamp.h"

extern const char *shortkey(const char *);

namespace ADM_Qt4Factory
{
/**
 *      \class diaElemTimeStamp
 *      \brief Qt4 version of diaElemTimeStamp
 */
class diaElemTimeStamp : public diaElem,QtFactoryUtils
{
protected :
    uint32_t valueMin;
    uint32_t valueMax;
public:
    diaElemTimeStamp(uint32_t *v, const char *toggleTitle, const uint32_t vmin, const uint32_t vmax);
    virtual ~diaElemTimeStamp();
    void setMe(void *dialog, void *opaque, uint32_t line);
    void getMe(void);
    int getRequiredLayout(void);
};

/**
 * \fn diaElemTimeStamp
 * @param percent
 * @param toggleTitle
 */
diaElemTimeStamp::diaElemTimeStamp(uint32_t *v, const char *toggleTitle, const uint32_t vmin, const uint32_t vmax)
    : diaElem(ELEM_TIMESTAMP), QtFactoryUtils(toggleTitle)
{
    param=v;
    valueMin=vmin;
    valueMax=vmax;
}
/**
 * \fn diaElemTimeStamp
 * \brief dtor
 */
diaElemTimeStamp::~diaElemTimeStamp()
{
    ADM_QTimeStamp *w=(ADM_QTimeStamp *)myWidget;
    myWidget=NULL;
    if(w) delete w;
}

/**
    \fn updateRange
*/
void ADM_QTimeStamp::updateRange(int i)
{
    UNUSED_ARG(i);

    uint32_t hh1,mm1,ss1,msec1;
    uint32_t hh2,mm2,ss2,msec2;
    ms2time(_min,&hh1,&mm1,&ss1,&msec1);
    ms2time(_max,&hh2,&mm2,&ss2,&msec2);

    myTWidget->hours->setRange(hh1,hh2);
    myTWidget->minutes->setRange(0,59);
    myTWidget->seconds->setRange(0,59);
    myTWidget->mseconds->setRange(0,999);

    uint32_t hh=myTWidget->hours->value();
    uint32_t mm=myTWidget->minutes->value();
    uint32_t ss=myTWidget->seconds->value();
    uint32_t ms=myTWidget->mseconds->value();

    if(hh==hh2)
    {
        myTWidget->minutes->setMaximum(mm2);
        if(mm>=mm2)
        {
            myTWidget->seconds->setMaximum(ss2);
            if(ss>=ss2)
            {
                myTWidget->mseconds->setMaximum(msec2);
            }
        }
    }

    if(hh==hh1)
    {
        myTWidget->minutes->setMinimum(mm1);
        if(mm<=mm1)
        {
            myTWidget->seconds->setMinimum(ss1);
            if(ss<=ss1)
            {
                myTWidget->mseconds->setMinimum(msec1);
            }
        }
    }

    bool hoursEnabled, minutesEnabled, secondsEnabled, msecondsEnabled;
    hoursEnabled=minutesEnabled=secondsEnabled=msecondsEnabled=true;

    if(!hh2)
    {
        hoursEnabled=false;
        if(!mm2)
        {
            minutesEnabled=false;
            if(!ss2)
            {
                secondsEnabled=false;
                if(!msec2)
                {
                    msecondsEnabled=false;
                }
            }
        }
    }

    myTWidget->hours->setEnabled(hoursEnabled);
    myTWidget->minutes->setEnabled(minutesEnabled);
    myTWidget->seconds->setEnabled(secondsEnabled);
    myTWidget->mseconds->setEnabled(msecondsEnabled);
}

/**
    \fn setSelectionAndBuddy
*/
void ADM_QTimeStamp::setSelectionAndBuddy(QLabel *label)
{
#define BUDDY(x) if(label) label->setBuddy(myTWidget->x);
    if(myTWidget->hours->isEnabled())
    {
        BUDDY(hours)
        myTWidget->hours->selectAll();
    }else
    {
        if(myTWidget->minutes->isEnabled())
        {
            BUDDY(minutes)
            myTWidget->minutes->selectAll();
        }else
        {
            if(myTWidget->seconds->isEnabled())
            {
                BUDDY(seconds)
                myTWidget->seconds->selectAll();
            }else
            {
                if(myTWidget->mseconds->isEnabled())
                {
                    BUDDY(mseconds)
                    myTWidget->mseconds->selectAll();
                }
            }
        }
    }
}

/**
    \fn ctor
*/
ADM_QTimeStamp::ADM_QTimeStamp(QString title, QWidget *dialog, QGridLayout *grid, uint32_t min, uint32_t max, uint32_t time, uint32_t line)
{
    myTWidget=new myTimeWidget;
    myTWidget->hours=new fixedNumDigitsSpinBox(dialog);
    myTWidget->minutes=new fixedNumDigitsSpinBox(dialog);
    myTWidget->seconds=new fixedNumDigitsSpinBox(dialog);
    myTWidget->mseconds=new fixedNumDigitsSpinBox(dialog);

    myTWidget->mseconds->numDigits=3;

    QLabel *textSemicolon1=new QLabel(":");
    QLabel *textSemicolon2=new QLabel(":");
    QLabel *textComma=new QLabel(",");

    myTWidget->hours->setSuffix(QT_TRANSLATE_NOOP("timestamp"," h"));
    myTWidget->minutes->setSuffix(QT_TRANSLATE_NOOP("timestamp"," m"));
    myTWidget->seconds->setSuffix(QT_TRANSLATE_NOOP("timestamp"," s"));

    myTWidget->hours->setAlignment(Qt::AlignRight);
    myTWidget->minutes->setAlignment(Qt::AlignRight);
    myTWidget->seconds->setAlignment(Qt::AlignRight);
    myTWidget->mseconds->setAlignment(Qt::AlignRight);

    QLabel *text=new QLabel(title, dialog);
    text->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QRegExp timeRegExp("^[0-9]{2}:[0-5][0-9]:[0-5][0-9]\\.[0-9]{3}$");
    timeValidator = new QRegExpValidator(timeRegExp, this);

    _min=min;
    _max=max;
    updateRange(0);

    setTime(time);
    setSelectionAndBuddy(text);

    QObject::connect(myTWidget->hours, SIGNAL(valueChanged(int)), this, SLOT(updateRange(int)));
    QObject::connect(myTWidget->minutes, SIGNAL(valueChanged(int)), this, SLOT(updateRange(int)));
    QObject::connect(myTWidget->seconds, SIGNAL(valueChanged(int)), this, SLOT(updateRange(int)));
    QObject::connect(myTWidget->mseconds, SIGNAL(valueChanged(int)), this, SLOT(updateRange(int)));

    myTWidget->hours->installEventFilter(this);
    myTWidget->minutes->installEventFilter(this);
    myTWidget->seconds->installEventFilter(this);
    myTWidget->mseconds->installEventFilter(this);

    QHBoxLayout *hboxLayout = new QHBoxLayout();
    QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addWidget(myTWidget->hours);
    hboxLayout->addWidget(textSemicolon1);

    hboxLayout->addWidget(myTWidget->minutes);
    hboxLayout->addWidget(textSemicolon2);

    hboxLayout->addWidget(myTWidget->seconds);

    hboxLayout->addWidget(textComma);
    hboxLayout->addWidget(myTWidget->mseconds);

    hboxLayout->addItem(spacer);

    grid->addWidget(text,line,0);
    grid->addLayout(hboxLayout,line,1);
}

/**
    \fn eventFilter
*/
bool ADM_QTimeStamp::eventFilter(QObject* watched, QEvent* event)
{
    QKeyEvent *keyEvent;
    if(event->type() == QEvent::KeyPress)
    {
        keyEvent = (QKeyEvent*)event;
        if(keyEvent->key() == Qt::Key_V)
        {
            if(keyEvent->modifiers() & Qt::ControlModifier)
            {
                QClipboard *clipboard = QApplication::clipboard();
                QString txt = clipboard->text();
                if(txt.size() == 12) // dd:dd:dd.ddd
                {
                    int pos;
                    uint32_t ms = 0;
                    if(QValidator::Acceptable == timeValidator->validate(txt,pos))
                    {
                        bool success = false;
                        int mult = 3600 * 1000;
                        QStringRef *ref = NULL;
                        for(int i=0; i<4; i++)
                        {
                            ref = new QStringRef(&txt, i*3, (i < 3) ? 2 : 3);
                            int val = ref->toInt(&success);
                            delete ref;
                            ref = NULL;
                            if(!success) break;
                            if(val < 0)
                            {
                                success = false;
                                break;
                            }
                            if(i < 3)
                            {
                                val *= mult;
                                ms += val;
                                mult /= 60;
                                continue;
                            }
                            ms += val;
                        }
                        if(success && ms >= _min && ms <= _max)
                        {
                            setTime(ms);
                            updateRange(0);
                            return true;
                        }
                    }
                }
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

/**
    \fn block
*/
void ADM_QTimeStamp::blockChanges(bool block)
{
#define BLOCK(x) myTWidget->x->blockSignals(block);
    BLOCK(hours)
    BLOCK(minutes)
    BLOCK(seconds)
    BLOCK(mseconds)
}

/**
    \fn setTime
*/
void ADM_QTimeStamp::setTime(uint32_t time)
{
    uint32_t hh,mm,ss,msec;
    ms2time(time,&hh,&mm,&ss,&msec);

    blockChanges(true);
    myTWidget->hours->setValue(hh);
    myTWidget->minutes->setValue(mm);
    myTWidget->seconds->setValue(ss);
    myTWidget->mseconds->setValue(msec);
    blockChanges(false);
}

/**
    \fn getTime
*/
uint32_t ADM_QTimeStamp::getTime(void)
{
    uint32_t hh=myTWidget->hours->value();
    uint32_t mm=myTWidget->minutes->value();
    uint32_t ss=myTWidget->seconds->value();
    uint32_t ms=myTWidget->mseconds->value();

    uint32_t time=hh*3600*1000+mm*60*1000+ss*1000+ms;

    return time;
}

/**
 * \fn          setMe
 * \brief       construct UI to display editable timestamp
 * @param dialog
 * @param opaque
 * @param line
 */
void diaElemTimeStamp::setMe(void *dialog, void *opaque, uint32_t line)
{
    uint32_t ms=*(uint32_t *)param;
    if(ms < valueMin)
        ms=valueMin;
    if(ms > valueMax)
        ms=valueMax;
    QGridLayout *layout=(QGridLayout*) opaque;
    ADM_QTimeStamp *stamp=new ADM_QTimeStamp(myQtTitle,(QWidget *)dialog,layout,valueMin,valueMax,ms,line);
    myWidget=(void *)stamp;
}

/**
 * \fn getMe
 * \brief retrieve value from UI
 */
void diaElemTimeStamp::getMe(void)
{
    ADM_QTimeStamp *widget=(ADM_QTimeStamp *)myWidget;
    uint32_t valueInMs=widget->getTime();
    *(uint32_t *)param=valueInMs;
}

int diaElemTimeStamp::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }
} // nameapsce
/**
 * \fn qt4CreateTimeStamp
 * @param v
 * @param toggleTitle
 * @param vmin
 * @param vmax
 * @return 
 */
diaElem  *qt4CreateTimeStamp(uint32_t *v, const char *toggleTitle, const uint32_t vmin, const uint32_t vmax)
{
    return new ADM_Qt4Factory::diaElemTimeStamp(v,toggleTitle,vmin,vmax);
}
void qt4DestroyTimeStamp(diaElem *e)
{
    ADM_Qt4Factory::diaElemTimeStamp *a=(ADM_Qt4Factory::diaElemTimeStamp *)e;
    delete a;
}
//
//EOF
