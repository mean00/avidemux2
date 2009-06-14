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


#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>

#include "ADM_default.h"
#include "DIA_factory.h"

extern const char *shortkey(const char *);

namespace ADM_qt4Factory
{


class diaElemFloat : public diaElem
{

public:
  ELEM_TYPE_FLOAT min,max;
  diaElemFloat(ELEM_TYPE_FLOAT *intValue,const char *toggleTitle, ELEM_TYPE_FLOAT min, 
               ELEM_TYPE_FLOAT max,const char *tip=NULL);
  virtual ~diaElemFloat() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  void      enable(uint32_t onoff) ;
};


//********************************************************************
diaElemFloat::diaElemFloat(ELEM_TYPE_FLOAT *intValue,const char *toggleTitle, ELEM_TYPE_FLOAT min, ELEM_TYPE_FLOAT max,const char *tip)
  : diaElem(ELEM_TOGGLE)
{
  param=(void *)intValue;
  paramTitle=shortkey(toggleTitle);
  this->min=min;
  this->max=max;
  this->tip=tip;
 }

diaElemFloat::~diaElemFloat()
{
  if(paramTitle)
    delete paramTitle;
}
void diaElemFloat::setMe(void *dialog, void *opaque,uint32_t line)
{
  QDoubleSpinBox *box=new QDoubleSpinBox((QWidget *)dialog);
  QGridLayout *layout=(QGridLayout*) opaque;
 myWidget=(void *)box; 
   
 box->setMinimum(min);
 box->setMaximum(max);
 box->setValue(*(ELEM_TYPE_FLOAT *)param);
 
 box->show();
 
 QLabel *text=new QLabel( QString::fromUtf8(this->paramTitle),(QWidget *)dialog);
 text->setBuddy(box);
 layout->addWidget(text,line,0);
 layout->addWidget(box,line,1);
 
}
void diaElemFloat::getMe(void)
{
  double val;
 QDoubleSpinBox *box=(QDoubleSpinBox *)myWidget;
 val=box->value();
 if(val<min) val=min;
 if(val>max) val=max;
 *(ELEM_TYPE_FLOAT *)param=val;
 
}
void diaElemFloat::enable(uint32_t onoff) 
{
   QDoubleSpinBox *box=(QDoubleSpinBox *)myWidget;
  ADM_assert(box);
  if(onoff)
    box->setEnabled(1);
  else
    box->setDisabled(1);
}
} // End of namespace
//****************************Hoook*****************

diaElem  *qt4CreateFloat(ELEM_TYPE_FLOAT *intValue,const char *toggleTitle, ELEM_TYPE_FLOAT min,
        ELEM_TYPE_FLOAT max,const char *tip)
{
	return new  ADM_qt4Factory::diaElemFloat(intValue,toggleTitle,min,max,tip);
}
void qt4DestroyFloat(diaElem *e)
{
	ADM_qt4Factory::diaElemFloat *a=(ADM_qt4Factory::diaElemFloat *)e;
	delete a;
}
