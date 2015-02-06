/***************************************************************************
  FAC_integer.cpp
  Handle dialog factory element : Integer
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

#include "T_slider.h"
#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_dialogFactoryQt4.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

extern const char *shortkey(const char *);

namespace ADM_qt4Factory
{
class diaElemUSlider : public diaElemSliderBase
{
  protected:
	  uint32_t min,max,incr;
public:
	diaElemUSlider(uint32_t *value,const char *toggleTitle, uint32_t min,uint32_t max,uint32_t incr = 1, const char *tip=NULL);
  virtual   ~diaElemUSlider() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
  
};
class diaElemSlider : public diaElemSliderBase
{
  protected:
    
	  int32_t min,max,incr;
    
public:
	diaElemSlider(int32_t *value,const char *toggleTitle, int32_t min,int32_t max,int32_t incr = 1, const char *tip=NULL);
  virtual   ~diaElemSlider() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
};

int SpinSlider::value()
{
	return spinner->value();
}

SpinSlider::SpinSlider (QWidget * parent)
    : QWidget (parent)
{
    slider = new QSlider (Qt::Horizontal);
    spinner = new QSpinBox;

    QWidget::connect (slider, SIGNAL(valueChanged(int)), spinner, SLOT(setValue(int)));
    QWidget::connect (spinner, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
    QWidget::connect (spinner, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget (slider);
    hbox->addWidget (spinner);

    setLayout (hbox);
}

void SpinSlider::setValue (int value)
{
    spinner->setValue (value);
}

void SpinSlider::setMinimum (int value)
{
    spinner->setMinimum (value);
    slider->setMinimum (value);
}

void SpinSlider::setMaximum (int value)
{
    spinner->setMaximum (value);
    slider->setMaximum (value);
}

//----------------------------------------------------------------------

diaElemUSlider::diaElemUSlider(uint32_t *value,const char *toggleTitle, uint32_t min,uint32_t max,uint32_t incr,const char *tip)
  : diaElemSliderBase()
{
  param=(void *)value;
  paramTitle=shortkey(toggleTitle);
  this->min=min;
  this->max=max;
  this->incr=incr;
  this->tip=tip;
  size = 2;
}

diaElemUSlider::~diaElemUSlider()
{ 
  if(paramTitle)
  {
      ADM_dealloc(paramTitle);
  }
}

void diaElemUSlider::setMe(void *dialog, void *opaque,uint32_t line)
{
  SpinSlider *slider = new SpinSlider ((QWidget *)dialog);
  slider->setMinimum(min);
  slider->setMaximum(max);
  slider->setValue(*(uint32_t *)param);
  slider->show();
 
  myWidget = (void *)slider;

  QLabel *text = new QLabel (QString::fromUtf8(paramTitle), (QWidget *)dialog);
  text->setBuddy (slider);

  QGridLayout *layout = (QGridLayout*) opaque;
  layout->addWidget(text,line,0);
  layout->addWidget(slider,line,1);
}

void diaElemUSlider::getMe(void)
{
  SpinSlider *box=(SpinSlider *)myWidget;
  uint32_t val=box->value();
  if(val<min) val=min;
  if(val>max) val=max;
  *(uint32_t *)param=val;
}

void diaElemUSlider::enable(uint32_t onoff) 
{
  SpinSlider *box=(SpinSlider *)myWidget;
  ADM_assert(box);
  if(onoff)
    box->setEnabled(1);
  else
    box->setDisabled(1);
}

int diaElemUSlider::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }

diaElemSlider::diaElemSlider(int32_t *value,const char *toggleTitle, int32_t min,int32_t max,int32_t incr,const char *tip)
  : diaElemSliderBase()
{
  param=(void *)value;
  paramTitle=shortkey(toggleTitle);
  this->min=min;
  this->max=max;
  this->incr=incr;
  this->tip=tip;
  size = 2;
}

diaElemSlider::~diaElemSlider()
{ 
  if(paramTitle)
    ADM_dealloc( paramTitle);
}

void diaElemSlider::setMe(void *dialog, void *opaque,uint32_t line)
{
  SpinSlider *slider = new SpinSlider ((QWidget *)dialog);
  slider->setMinimum(min);
  slider->setMaximum(max);
  slider->setValue(*(int32_t *)param);
  slider->show();
 
  myWidget = (void *)slider;

  QLabel *text = new QLabel (QString::fromUtf8(paramTitle), (QWidget *)dialog);
  text->setBuddy (slider);

  QGridLayout *layout = (QGridLayout*) opaque;
  layout->addWidget(text,line,0);
  layout->addWidget(slider,line,1);
}

void diaElemSlider::getMe(void)
{
  SpinSlider *box=(SpinSlider *)myWidget;
  int32_t val=box->value();
  if(val<min) val=min;
  if(val>max) val=max;
  *(int32_t *)param=val;
}

void diaElemSlider::enable(uint32_t onoff) 
{
  SpinSlider *box=(SpinSlider *)myWidget;
  ADM_assert(box);
  if(onoff)
    box->setEnabled(1);
  else
    box->setDisabled(1);
}

int diaElemSlider::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }
//****************************************************
} // End of namespace
//****************************Hoook*****************

diaElem  *qt4CreateSlider(int32_t *value,const char *toggleTitle, int32_t min,int32_t max,int32_t incr , const char *tip)
{
	return new  ADM_qt4Factory::diaElemSlider(value,toggleTitle,min,max,incr,tip);
}
void qt4DestroySlider(diaElem *e)
{
	ADM_qt4Factory::diaElemSlider *a=(ADM_qt4Factory::diaElemSlider *)e;
	delete a;
}
diaElem  *qt4CreateUSlider(uint32_t *value,const char *toggleTitle, uint32_t min,uint32_t max,uint32_t incr , const char *tip)
{
	return new  ADM_qt4Factory::diaElemUSlider(value,toggleTitle,min,max,incr,tip);
}
void qt4DestroyUSlider(diaElem *e)
{
	ADM_qt4Factory::diaElemUSlider *a=(ADM_qt4Factory::diaElemUSlider *)e;
	delete a;
}
//****************************************************
//EOF


