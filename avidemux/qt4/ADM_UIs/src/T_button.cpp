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

#include "T_button.h"
#include "ADM_default.h"
#include "ADM_dialogFactoryQt4.h"

extern const char *shortkey(const char *);
namespace ADM_Qt4Factory
{
class diaElemButton : public diaElem
{
  protected:
  public:
    void            *_cookie;
    ADM_FAC_CALLBACK *_callBack;
            diaElemButton(const char *toggleTitle, ADM_FAC_CALLBACK *cb,void *cookie,const char *tip=NULL);
  virtual   ~diaElemButton() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
};

ADM_Qbutton::ADM_Qbutton(QWidget *z,QGridLayout *layout,const char *blah,int line,ADM_FAC_CALLBACK *cb, void *cookie) : QWidget(z) 
{
	_cb=cb;
	_cookie=cookie;
	button=new QPushButton(QString::fromUtf8(blah),z);
	button->show();
	layout->addWidget(button,line,0);
	QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(clicked(bool )));
}

void ADM_Qbutton::clicked(bool i)
{
    _cb(_cookie);
}
ADM_Qbutton::~ADM_Qbutton()
{
}
//*****************************
diaElemButton:: diaElemButton(const char *toggleTitle, ADM_FAC_CALLBACK *cb,void *cookie,const char *tip)
  : diaElem(ELEM_BUTTON)
{
  param=(void *)NULL;
  paramTitle=shortkey(toggleTitle);
  this->tip=tip;
  _cookie=cookie;
  _callBack=cb;
  
}

diaElemButton::~diaElemButton()
{
	delete paramTitle;
}

void diaElemButton::setMe(void *dialog, void *opaque,uint32_t line)
{
  QGridLayout *layout=(QGridLayout*) opaque;
  
  ADM_Qbutton *b=new ADM_Qbutton( (QWidget *)dialog,layout,paramTitle,line,_callBack,_cookie);
  myWidget=(void *)b;
}
void diaElemButton::getMe(void)
{
  if(paramTitle)
  {
    ADM_dealloc(paramTitle);
    paramTitle=NULL; 
  }
}
void   diaElemButton::enable(uint32_t onoff)
{

}

int diaElemButton::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }

}; // End of namespace
//****************************Hoook*****************

diaElem  *qt4CreateButton(const char *toggleTitle, ADM_FAC_CALLBACK *cb,void *cookie,const char *tip)
{
	return new  ADM_Qt4Factory::diaElemButton(toggleTitle,cb,cookie,tip);
}
void qt4DestroyButton(diaElem *e)
{
	ADM_Qt4Factory::diaElemButton *a=(ADM_Qt4Factory::diaElemButton *)e;
	delete a;
}
//
//EOF

