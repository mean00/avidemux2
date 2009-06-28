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
#include <QtGui/QProgressBar>

#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_dialogFactoryQt4.h"

extern const char *shortkey(const char *);

namespace ADM_Qt4Factory
{
class diaElemBar : public diaElem
{
  protected :
        uint32_t per;
public:
  
  diaElemBar(uint32_t percent,const char *toggleTitle);
  virtual ~diaElemBar() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  int getRequiredLayout(void);
};

//********************************************************************
diaElemBar::diaElemBar(uint32_t percent,const char *toggleTitle)
  : diaElem(ELEM_BAR)
{
  per=percent;
  paramTitle=shortkey(toggleTitle);
 }

diaElemBar::~diaElemBar()
{
  if(paramTitle)
    delete paramTitle;
}
void diaElemBar::setMe(void *dialog, void *opaque,uint32_t line)
{
  QProgressBar *box=new QProgressBar((QWidget *)dialog);
  QGridLayout *layout=(QGridLayout*) opaque;
 
  box->setMinimum(0);
  box->setMaximum(100);
  box->setValue(per);
  box->show();
 
 QLabel *text=new QLabel(QString::fromUtf8(this->paramTitle),(QWidget *)dialog);
 text->setBuddy(box);
 layout->addWidget(text,line,0);
 layout->addWidget(box,line,1);
 
}
void diaElemBar::getMe(void)
{
}

int diaElemBar::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }
} // nameapsce

diaElem  *qt4CreateBar(uint32_t percent,const char *toggleTitle)
{
	return new  ADM_Qt4Factory::diaElemBar(percent,toggleTitle);
}
void qt4DestroyBar(diaElem *e)
{
	ADM_Qt4Factory::diaElemBar *a=(ADM_Qt4Factory::diaElemBar *)e;
	delete a;
}
//
//EOF
