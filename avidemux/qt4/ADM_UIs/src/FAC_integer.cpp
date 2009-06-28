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


#include <QtGui/QSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_dialogFactoryQt4.h"

extern const char *shortkey(const char *);


namespace ADM_qt4Factory
{
class diaElemInteger : public diaElem
{

public:
  int32_t min,max;
  diaElemInteger(int32_t *intValue,const char *toggleTitle, int32_t min, int32_t max,const char *tip=NULL);
  virtual ~diaElemInteger() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
};
/* Same but unsigned */
class diaElemUInteger : public diaElem
{

public:
  uint32_t min,max;
  diaElemUInteger(uint32_t *intValue,const char *toggleTitle, uint32_t min, uint32_t max,const char *tip=NULL);
  virtual ~diaElemUInteger() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
};

//********************************************************************
diaElemInteger::diaElemInteger(int32_t *intValue,const char *toggleTitle, int32_t min, int32_t max,const char *tip)
  : diaElem(ELEM_TOGGLE)
{
  param=(void *)intValue;
  paramTitle=shortkey(toggleTitle);
  this->min=min;
  this->max=max;
  this->tip=tip;
 }

diaElemInteger::~diaElemInteger()
{
  if(paramTitle)
    delete paramTitle;
}
void diaElemInteger::setMe(void *dialog, void *opaque,uint32_t line)
{
  QSpinBox *box=new QSpinBox((QWidget *)dialog);
  QGridLayout *layout=(QGridLayout*) opaque;
  QHBoxLayout *hboxLayout = new QHBoxLayout();
 myWidget=(void *)box; 
   
 box->setMinimum(min);
 box->setMaximum(max);
 box->setValue(*(int32_t *)param);
 
 QLabel *text=new QLabel( QString::fromUtf8(this->paramTitle),(QWidget *)dialog);
 text->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
 text->setBuddy(box);

 QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

 hboxLayout->addWidget(box);
 hboxLayout->addItem(spacer);

 layout->addWidget(text,line,0);
 layout->addLayout(hboxLayout,line,1);
}
void diaElemInteger::getMe(void)
{
  int32_t val;
 QSpinBox *box=(QSpinBox *)myWidget;
 val=box->value();
 if(val<min) val=min;
 if(val>max) val=max;
 *(int32_t *)param=val;
 
}
void diaElemInteger::enable(uint32_t onoff) 
{
 QSpinBox *box=(QSpinBox *)myWidget;
  ADM_assert(box);
  if(onoff)
    box->setEnabled(1);
  else
    box->setDisabled(1);
}

int diaElemInteger::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }

//******************************************************
diaElemUInteger::diaElemUInteger(uint32_t *intValue,const char *toggleTitle, uint32_t min, uint32_t max,const char *tip)
  : diaElem(ELEM_TOGGLE)
{
  param=(void *)intValue;
  paramTitle=shortkey(toggleTitle);
  this->min=min;
  this->max=max;
  this->tip=tip;
 }
 

diaElemUInteger::~diaElemUInteger()
{
  if(paramTitle)
    delete paramTitle;
}
void diaElemUInteger::setMe(void *dialog, void *opaque,uint32_t line)
{
  QSpinBox *box=new QSpinBox((QWidget *)dialog);
  QGridLayout *layout=(QGridLayout*) opaque;
  QHBoxLayout *hboxLayout = new QHBoxLayout();
 myWidget=(void *)box; 
   
 box->setMinimum(min);
 box->setMaximum(max);
 box->setValue(*(uint32_t *)param);
 
 QLabel *text=new QLabel( QString::fromUtf8(this->paramTitle),(QWidget *)dialog);
 text->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
 text->setBuddy(box);

 QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

 hboxLayout->addWidget(box);
 hboxLayout->addItem(spacer);

 layout->addWidget(text,line,0);
 layout->addLayout(hboxLayout,line,1);
}
void diaElemUInteger::getMe(void)
{
   uint32_t val;
 QSpinBox *box=(QSpinBox *)myWidget;
 val=box->value();
 if(val<min) val=min;
 if(val>max) val=max;
 *(uint32_t *)param=val;

}

void diaElemUInteger::enable(uint32_t onoff) 
{
 QSpinBox *box=(QSpinBox *)myWidget;
  ADM_assert(box);
  if(onoff)
    box->setEnabled(1);
  else
    box->setDisabled(1);
}

int diaElemUInteger::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }

//********************************************
//****************************************************
} // End of namespace
//****************************Hoook*****************

diaElem  *qt4CreateInteger(int32_t *intValue,const char *toggleTitle,int32_t min, int32_t max,const char *tip)
{
	return new  ADM_qt4Factory::diaElemInteger(intValue,toggleTitle,min,max,tip);
}
void qt4DestroyInteger(diaElem *e)
{
	ADM_qt4Factory::diaElemInteger *a=(ADM_qt4Factory::diaElemInteger *)e;
	delete a;
}
diaElem  *qt4CreateUInteger(uint32_t *intValue,const char *toggleTitle,uint32_t min, uint32_t max,const char *tip)
{
	return new  ADM_qt4Factory::diaElemUInteger(intValue,toggleTitle,min,max,tip);
}
void qt4DestroyUInteger(diaElem *e)
{
	ADM_qt4Factory::diaElemUInteger *a=(ADM_qt4Factory::diaElemUInteger *)e;
	delete a;
}
//EOF

