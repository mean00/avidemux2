/***************************************************************************
  FAC_toggle.cpp
  Handle dialog factory element : Toggle
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


#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QSpinBox>



#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_dialogFactoryQt4.h"
#include "ADM_vidMisc.h"

extern const char *shortkey(const char *);

class fixedNumDigitsSpinBox : public QSpinBox
{
public:
    int numDigits;
    fixedNumDigitsSpinBox(QWidget *parent) : QSpinBox(parent)
    {
        numDigits=2;
    }

    virtual QString textFromValue(int value) const
    {
        return QString("%1").arg(value, numDigits, 10, QChar('0'));
    }
};

/**
 *      \struct myTimeWidget
 */
typedef struct
{
    fixedNumDigitsSpinBox *hours;
    fixedNumDigitsSpinBox *minutes;
    fixedNumDigitsSpinBox *seconds;
    fixedNumDigitsSpinBox *mseconds;
}myTimeWidget;

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
  
  diaElemTimeStamp(uint32_t *v,const char *toggleTitle,const uint32_t vmin, const uint32_t vmax);
  virtual ~diaElemTimeStamp() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  int getRequiredLayout(void);
};

/**
 * \fn diaElemTimeStamp
 * @param percent
 * @param toggleTitle
 */
diaElemTimeStamp::diaElemTimeStamp(uint32_t *v,const char *toggleTitle,const uint32_t vmin, const uint32_t vmax)
  : diaElem(ELEM_TIMESTAMP),QtFactoryUtils(toggleTitle)
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
  myTimeWidget *w=(myTimeWidget *)myWidget;
  myWidget=NULL;
  if(w) delete w;
}
/**
 * \fn          setMe
 * \brief       construct UI to display editable timestamp
 * @param dialog
 * @param opaque
 * @param line
 */
void diaElemTimeStamp::setMe(void *dialog, void *opaque,uint32_t line)
{
  myTimeWidget *myTWidget = new myTimeWidget;
  myTWidget->hours=new fixedNumDigitsSpinBox((QWidget *)dialog);
  myTWidget->minutes=new fixedNumDigitsSpinBox((QWidget *)dialog);
  myTWidget->seconds=new fixedNumDigitsSpinBox((QWidget *)dialog);
  myTWidget->mseconds=new fixedNumDigitsSpinBox((QWidget *)dialog);
  myWidget=(void *)myTWidget; 

  myTWidget->minutes->setRange(0,59);
  myTWidget->seconds->setRange(0,59);
  myTWidget->mseconds->setRange(0,999);
  myTWidget->mseconds->numDigits=3;

  QLabel *textSemicolon1=new QLabel(":");
  QLabel *textSemicolon2=new QLabel(":");
  QLabel *textComma=new QLabel(",");
  
  QGridLayout *layout=(QGridLayout*) opaque;
  QHBoxLayout *hboxLayout = new QHBoxLayout();
  
  uint32_t ms=*(uint32_t *)param;
  // split time in ms into hh/mm/ss
  uint32_t hh,mm,ss,msec;
  ms2time(ms,&hh,&mm,&ss,&msec);
  myTWidget->hours->setValue(hh);
  myTWidget->minutes->setValue(mm);
  myTWidget->seconds->setValue(ss);
  myTWidget->mseconds->setValue(msec);

  myTWidget->hours->setSuffix(QT_TRANSLATE_NOOP("timestamp"," h"));
  myTWidget->minutes->setSuffix(QT_TRANSLATE_NOOP("timestamp"," m"));
  myTWidget->seconds->setSuffix(QT_TRANSLATE_NOOP("timestamp"," s"));

  myTWidget->hours->setAlignment(Qt::AlignRight);
  myTWidget->minutes->setAlignment(Qt::AlignRight);
  myTWidget->seconds->setAlignment(Qt::AlignRight);
  myTWidget->mseconds->setAlignment(Qt::AlignRight);

 QLabel *text=new QLabel(myQtTitle,(QWidget *)dialog);
 text->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
 text->setBuddy(myTWidget->hours);

    myTWidget->hours->selectAll();

 QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

 hboxLayout->addWidget(myTWidget->hours);
 hboxLayout->addWidget(textSemicolon1);

 hboxLayout->addWidget(myTWidget->minutes);
 hboxLayout->addWidget(textSemicolon2);

 hboxLayout->addWidget(myTWidget->seconds);

 hboxLayout->addWidget(textComma);
 hboxLayout->addWidget(myTWidget->mseconds);

 hboxLayout->addItem(spacer);

 layout->addWidget(text,line,0);
 layout->addLayout(hboxLayout,line,1);
 
}
/**
 * \fn getMe
 * \brief retrieve value from UI
 */
void diaElemTimeStamp::getMe(void)
{
        uint32_t val;
        myTimeWidget *widget=(myTimeWidget *)myWidget;
        uint32_t hh=widget->hours->value();
        uint32_t mm=widget->minutes->value();
        uint32_t ss=widget->seconds->value();
        uint32_t ms=widget->mseconds->value();
        
        uint32_t valueInMs;
        valueInMs=hh*3600*1000+mm*60*1000+ss*1000+ms;
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
diaElem  *qt4CreateTimeStamp(uint32_t *v,const char *toggleTitle,const uint32_t vmin, const uint32_t vmax)
{
	return new  ADM_Qt4Factory::diaElemTimeStamp(v,toggleTitle,vmin,vmax);
}
void qt4DestroyTimeStamp(diaElem *e)
{
	ADM_Qt4Factory::diaElemTimeStamp *a=(ADM_Qt4Factory::diaElemTimeStamp *)e;
	delete a;
}
//
//EOF
