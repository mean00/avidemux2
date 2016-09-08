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
#include <QSpinBox>

#include "T_toggle.h"
#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_dialogFactoryQt4.h"

extern const char *shortkey(const char *);

namespace ADM_qt4Factory
{
class diaElemToggle : public diaElemToggleBase,QtFactoryUtils
{
  protected:
public:
            diaElemToggle(bool *toggleValue,const char *toggleTitle, const char *tip=NULL);
  virtual   ~diaElemToggle() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  void      finalize(void);
  void      updateMe();
  uint8_t   link(uint32_t onoff,diaElem *w);
  int getRequiredLayout(void);
};

class diaElemToggleUint : public diaElem,QtFactoryUtils
{
  protected:
        uint32_t *emb;
        const char *embName;
        void *widgetUint;
        uint32_t _min,_max;
public:
            diaElemToggleUint(uint32_t *toggleValue,const char *toggleTitle, uint32_t *uintval,
            					const char *name,uint32_t min,uint32_t max,const char *tip=NULL);
  virtual   ~diaElemToggleUint() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  void      finalize(void);
  void      updateMe();
  int getRequiredLayout(void);
};
class diaElemToggleInt : public diaElem,QtFactoryUtils
{
  protected:
	  		 int32_t *emb;
	         const char *embName;
	         void *widgetUint;
	         int32_t _min,_max;
public:
            diaElemToggleInt(uint32_t *toggleValue,const char *toggleTitle, int32_t *uintval,
            				const char *name,int32_t min,int32_t max,const char *tip=NULL);
  virtual   ~diaElemToggleInt() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      finalize(void);
  void      updateMe();
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
};

void ADM_QCheckBox::changed(int i)
{
	switch(_type)
	{
		case TT_TOGGLE:
			((diaElemToggle *)_toggle)->updateMe();break;
		case TT_TOGGLE_UINT:
			((diaElemToggleUint *)_toggle)->updateMe();break;
		case TT_TOGGLE_INT:
			((diaElemToggleInt *)_toggle)->updateMe();break;
		default:
			ADM_assert(0);break;
	}
}

ADM_QCheckBox::ADM_QCheckBox(const QString & str,QWidget *root,void *toggle,TOG_TYPE type) : QCheckBox(str,root)
{
	_toggle=toggle;
	_type=type;
}

void ADM_QCheckBox::connectMe(void)
{
	QObject::connect(this, SIGNAL(stateChanged(int)), this, SLOT(changed(int )));
}

diaElemToggle::diaElemToggle(bool *toggleValue,const char *toggleTitle, const char *tip)
  : diaElemToggleBase(),QtFactoryUtils(toggleTitle)
{
  param=(void *)toggleValue;
  this->tip=tip;
  myWidget=NULL;
  nbLink=0;
}

diaElemToggle::~diaElemToggle()
{
//  ADM_QCheckBox *box=(ADM_QCheckBox *)myWidget;
 // if(box) delete box;
  myWidget=NULL;
}
void diaElemToggle::setMe(void *dialog, void *opaque,uint32_t l)
{
 ADM_QCheckBox *box=new ADM_QCheckBox(myQtTitle,(QWidget *)dialog,this,TT_TOGGLE);
 QVBoxLayout *layout=(QVBoxLayout*) opaque;
 myWidget=(void *)box; 
 if( *(bool *)param)
 {
    box->setCheckState(Qt::Checked); 
 }

 layout->addWidget(box);
 box->connectMe();
}
void diaElemToggle::getMe(void)
{
  ADM_QCheckBox *box=(ADM_QCheckBox *)myWidget;
  bool *val=(bool *)param;
  if(Qt::Checked==box->checkState())
  {
    *val=true; 
  }else
    *val=false;
}
void diaElemToggle::enable(uint32_t onoff) 
{
  ADM_QCheckBox *box=(ADM_QCheckBox *)myWidget;
  ADM_assert(box);
  if(onoff)
    box->setEnabled(true);
  else
    box->setDisabled(true);
}

void   diaElemToggle::finalize(void)
{
  updateMe(); 
}
void   diaElemToggle::updateMe(void)
{
 
  uint32_t val;
  uint32_t rank=0;
  if(!nbLink) return;
  ADM_assert(myWidget);
  
  ADM_QCheckBox *box=(ADM_QCheckBox *)myWidget;
  
  if(Qt::Checked==box->checkState())
  {
    rank=1;
  }
  /* Now search through the linked list to see if something happens ...*/
  
   /* 1 disable everything */
  for(int i=0;i<nbLink;i++)
  {
    dialElemLink *l=&(links[i]);
    l->widget->enable(0);
  }
  /* Then enable */
  for(int i=0;i<nbLink;i++)
  {
      dialElemLink *l=&(links[i]);
      if(l->onoff==rank)  l->widget->enable(1);
  }
}

uint8_t   diaElemToggle::link(uint32_t onoff,diaElem *w)
{
    ADM_assert(nbLink<MENU_MAX_lINK);
    links[nbLink].onoff=onoff;
    links[nbLink].widget=w;
    nbLink++;
    return 1;
}

int diaElemToggle::getRequiredLayout(void) { return FAC_QT_VBOXLAYOUT; }

//******************************************************
// An UInt and a toggle linked...
//******************************************************
diaElemToggleUint::diaElemToggleUint(uint32_t *toggleValue,const char *toggleTitle, uint32_t *uintval, const char *name,uint32_t min,uint32_t max,const char *tip)
  : diaElem(ELEM_TOGGLE_UINT),QtFactoryUtils(toggleTitle)
{
  param=(void *)toggleValue;
  this->tip=tip;
  embName=name;
  emb=uintval;
  widgetUint=NULL;
  _min=min;
  _max=max;
}

diaElemToggleUint::~diaElemToggleUint()
{
}
void diaElemToggleUint::setMe(void *dialog, void *opaque,uint32_t line)
{
 ADM_QCheckBox *box=new ADM_QCheckBox(myQtTitle,(QWidget *)dialog,this,TT_TOGGLE_UINT);
 QGridLayout *layout=(QGridLayout*) opaque;
 QHBoxLayout *hboxLayout = new QHBoxLayout();
 myWidget=(void *)box; 
 if( *(uint32_t *)param)
 {
    box->setCheckState(Qt::Checked); 
 }

 // Now add spin
 QSpinBox *spin=new QSpinBox((QWidget *)dialog);
 widgetUint=(void *)spin; 
   
 spin->setMinimum(_min);
 spin->setMaximum(_max);
 spin->setValue(*(uint32_t *)emb);

 QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

 hboxLayout->addWidget(spin);
 hboxLayout->addItem(spacer);

 layout->addWidget(box,line,0);
 layout->addLayout(hboxLayout,line,1);
 box->connectMe();
}

void diaElemToggleUint::getMe(void)
{
  ADM_QCheckBox *box=(ADM_QCheckBox *)myWidget;
  uint32_t *val=(uint32_t *)param;
  if(Qt::Checked==box->checkState())
  {
    *val=1; 
  }else
    *val=0;
  //
    uint32_t u;
  QSpinBox *spin=(QSpinBox *)widgetUint;
  u=spin->value();
 if(u<_min) u=_min;
 if(u>_max) u=_max;
 *emb=u;
  
}
void   diaElemToggleUint::finalize(void)
{
  updateMe();
}
void   diaElemToggleUint::updateMe(void)
{
  uint32_t val;
  uint32_t rank=false;
  ADM_assert(myWidget);
  
  ADM_QCheckBox *box=(ADM_QCheckBox *)myWidget;
  QSpinBox *spin=(QSpinBox *)widgetUint;
  
  if(Qt::Checked==box->checkState())
  {
    rank=true;
  }
  spin->setEnabled(rank);
}
void   diaElemToggleUint::enable(uint32_t onoff)
{
    ADM_QCheckBox *box=(ADM_QCheckBox *)myWidget;
      QSpinBox *spin=(QSpinBox *)widgetUint;
  ADM_assert(box);
  if(onoff)
  {
    box->setEnabled(true);
    spin->setEnabled(true);
  }
  else
  {
    box->setEnabled(false);
    spin->setEnabled(false);
  }
}

int diaElemToggleUint::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }

//******************************************************
// An Int and a toggle linked...
//******************************************************

diaElemToggleInt::diaElemToggleInt(uint32_t *toggleValue,const char *toggleTitle, int32_t *uintval, const char *name,int32_t min,int32_t max,const char *tip)
  : diaElem(ELEM_TOGGLE_INT),QtFactoryUtils(toggleTitle)
{
  param=(void *)toggleValue;
  this->tip=tip;
  embName=name;
  emb=uintval;
  widgetUint=NULL;
  _min=min;
  _max=max;
}

diaElemToggleInt::~diaElemToggleInt()
{
  
}
void diaElemToggleInt::setMe(void *dialog, void *opaque,uint32_t line)
{
 ADM_QCheckBox *box=new ADM_QCheckBox(myQtTitle,(QWidget *)dialog,this,TT_TOGGLE_INT);
 QGridLayout *layout=(QGridLayout*) opaque;
 QHBoxLayout *hboxLayout = new QHBoxLayout();
 myWidget=(void *)box; 
 if( *(uint32_t *)param)
 {
    box->setCheckState(Qt::Checked); 
 }

 // Now add spin
 QSpinBox *spin=new QSpinBox((QWidget *)dialog);
 widgetUint=(void *)spin; 
   
 spin->setMinimum(_min);
 spin->setMaximum(_max);
 spin->setValue(*emb);

 QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

 hboxLayout->addWidget(spin);
 hboxLayout->addItem(spacer);

 layout->addWidget(box,line,0);
 layout->addLayout(hboxLayout,line,1);
 box->connectMe();
}

void diaElemToggleInt::getMe(void)
{
  ADM_QCheckBox *box=(ADM_QCheckBox *)myWidget;
  uint32_t *val=(uint32_t *)param;
  if(Qt::Checked==box->checkState())
  {
    *val=1; 
  }else
    *val=0;
  //
    int32_t u;
  QSpinBox *spin=(QSpinBox *)widgetUint;
  u=spin->value();
 if(u<_min) u=_min;
 if(u>_max) u=_max;
 *emb=u;
}

int diaElemToggleInt::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }

void   diaElemToggleInt::finalize(void)
{
  updateMe();
}
void   diaElemToggleInt::updateMe(void)
{
  uint32_t val;
  uint32_t rank=false;
  ADM_assert(myWidget);
  
  ADM_QCheckBox *box=(ADM_QCheckBox *)myWidget;
  QSpinBox *spin=(QSpinBox *)widgetUint;
  
  if(Qt::Checked==box->checkState())
  {
    rank=true;
  }
  spin->setEnabled(rank);
}
void   diaElemToggleInt::enable(uint32_t onoff)
{
    ADM_QCheckBox *box=(ADM_QCheckBox *)myWidget;
      QSpinBox *spin=(QSpinBox *)widgetUint;
  ADM_assert(box);
  if(onoff)
  {
    box->setEnabled(true);
    spin->setEnabled(true);
  }
  else
  {
    box->setEnabled(false);
    spin->setEnabled(false);
  }
}

} // End of namespace
//****************************Hoook*****************

diaElem  *qt4CreateToggleUint(uint32_t *toggleValue,const char *toggleTitle, uint32_t *uintval,
		const char *name,uint32_t min,uint32_t max,const char *tip)
{
	return new  ADM_qt4Factory::diaElemToggleUint(toggleValue,toggleTitle, uintval,
			name,min,max,tip);
}
void qt4DestroyToggleUint(diaElem *e)
{
	ADM_qt4Factory::diaElemToggleUint *a=(ADM_qt4Factory::diaElemToggleUint *)e;
	delete a;
}

diaElem  *qt4CreateToggleInt(uint32_t *toggleValue,const char *toggleTitle, int32_t *uintval,
		const char *name,int32_t min,int32_t max,const char *tip)
{
	return new  ADM_qt4Factory::diaElemToggleInt(toggleValue,toggleTitle, uintval,
			name,min,max,tip);
}
void qt4DestroyToggleInt(diaElem *e)
{
	ADM_qt4Factory::diaElemToggleInt *a=(ADM_qt4Factory::diaElemToggleInt *)e;
	delete a;
}
diaElem  *qt4CreateToggle(bool *toggleValue,const char *toggleTitle, const char *tip)
{
	return new  ADM_qt4Factory::diaElemToggle(toggleValue,toggleTitle, tip);
}
void qt4DestroyToggle(diaElem *e)
{
	ADM_qt4Factory::diaElemToggle *a=(ADM_qt4Factory::diaElemToggle *)e;
	delete a;
}



//EOF
