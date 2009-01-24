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
#include <QtGui/QCheckBox>

#include "ADM_default.h"
#include "DIA_factory.h"

extern const char *shortkey(const char *);

namespace ADM_qt4Factory
{


class diaElemNotch : public diaElem
{
  uint32_t yesno;
public:
  
  diaElemNotch(uint32_t yes,const char *toggleTitle, const char *tip=NULL);
  virtual ~diaElemNotch() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void) {};
};
diaElemNotch::diaElemNotch(uint32_t yes,const char *toggleTitle, const char *tip)
  : diaElem(ELEM_NOTCH)
{
  yesno=yes;
  paramTitle=toggleTitle;
  this->tip=tip;
}

diaElemNotch::~diaElemNotch()
{
  
}
void diaElemNotch::setMe(void *dialog, void *opaque,uint32_t line)
{
  QCheckBox *box=new QCheckBox(QString::fromUtf8(paramTitle),(QWidget *)dialog);
 QGridLayout *layout=(QGridLayout*) opaque;
 myWidget=(void *)box; 
 if( yesno)
 {
    box->setCheckState(Qt::Checked); 
 }
 box->show();
 layout->addWidget(box,line,0);
}
//******************************************************
//****************************************************
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

//EOF
