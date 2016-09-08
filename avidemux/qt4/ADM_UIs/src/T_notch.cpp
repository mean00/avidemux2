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

#include "T_notch.h"
#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_dialogFactoryQt4.h"

extern const char *shortkey(const char *);

namespace ADM_qt4Factory
{

QCheckBoxReadOnly::QCheckBoxReadOnly(QCheckBox *box, bool state)
{
	this->box = box;
	this->state = state;
}

void QCheckBoxReadOnly::stateChanged(int state)
{
	box->setCheckState(this->state ? Qt::Checked : Qt::Unchecked);
}

class diaElemNotch : public diaElem,QtFactoryUtils
{
  uint32_t yesno;
public:
  
  diaElemNotch(uint32_t yes,const char *toggleTitle, const char *tip=NULL);
  virtual ~diaElemNotch() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void) {};
  int getRequiredLayout(void);
};

diaElemNotch::diaElemNotch(uint32_t yes,const char *toggleTitle, const char *tip)
  : diaElem(ELEM_NOTCH),QtFactoryUtils(toggleTitle)
{
  yesno=yes;
  this->tip=tip;
}

diaElemNotch::~diaElemNotch()
{
  
}
void diaElemNotch::setMe(void *dialog, void *opaque,uint32_t line)
{
  QCheckBox *box=new QCheckBox(myQtTitle,(QWidget *)dialog);
  QCheckBoxReadOnly *readOnlyReceiver = new QCheckBoxReadOnly(box, yesno);
 QGridLayout *layout=(QGridLayout*) opaque;
 myWidget=(void *)box; 
 if( yesno)
 {
    box->setCheckState(Qt::Checked); 
 }

 QObject::connect(box, SIGNAL(stateChanged(int)), readOnlyReceiver, SLOT(stateChanged(int)));
 layout->addWidget(box,line,0);
}

int diaElemNotch::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }
} // End of namespace
//****************************Hoook*****************

diaElem  *qt4CreateNotch(uint32_t yes,const char *toggleTitle, const char *tip)
{
	return new  ADM_qt4Factory::diaElemNotch(yes,toggleTitle, tip);
}
void qt4DestroyNotch(diaElem *e)
{
	ADM_qt4Factory::diaElemNotch *a=(ADM_qt4Factory::diaElemNotch *)e;
	delete a;
}
//EOF
