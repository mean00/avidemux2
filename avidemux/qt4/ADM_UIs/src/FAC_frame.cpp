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

#include "ADM_default.h"
#include "DIA_factory.h"

extern const char *shortkey(const char *);


namespace ADM_qt4Factory
{
class diaElemFrame : public diaElemFrameBase
{
  
public:
  
  diaElemFrame(const char *toggleTitle, const char *tip=NULL);
  virtual ~diaElemFrame() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void) ;
  void swallow(diaElem *widget);
  void enable(uint32_t onoff);
  void finalize(void);
};

diaElemFrame::diaElemFrame(const char *toggleTitle, const char *tip)
  : diaElemFrameBase()
{
  param=NULL;
  paramTitle=shortkey(toggleTitle);
  this->tip=tip;
   nbElems=0;
  frameSize=0;
  setSize(2);
}
void diaElemFrame::swallow(diaElem *widget)
{
   elems[nbElems]=widget;
  frameSize+=widget->getSize();
 // setSize(frameSize);
  nbElems++;
  ADM_assert(nbElems<DIA_MAX_FRAME); 
}
diaElemFrame::~diaElemFrame()
{
  if(paramTitle)
    delete paramTitle;
}
void diaElemFrame::setMe(void *dialog, void *opaque,uint32_t line)
{
  
   QGridLayout *layout=(QGridLayout*) opaque;  
   QGridLayout *layout2;
   
   layout2=new QGridLayout((QWidget *)dialog);
   myWidget=(void *)layout2; 

    QLabel *text=new QLabel( (QWidget *)dialog);
    QString string = QString::fromUtf8(paramTitle);
    
    string="<b>"+string+"</b>";
    text->setText(string);
 layout->addWidget(text,line,0);
 layout->addLayout(layout2,line+1,0);
 int  v=0;
  for(int i=0;i<nbElems;i++)
  {
    elems[i]->setMe(dialog,layout2,v); 
    v+=elems[i]->getSize();
  }
  myWidget=(void *)layout2;
}
//*****************************
void diaElemFrame::getMe(void)
{
   for(int i=0;i<nbElems;i++)
  {
    elems[i]->getMe(); 
  }
}
void diaElemFrame::finalize(void)
{
   for(int i=0;i<nbElems;i++)
  {
    elems[i]->finalize(); 
  }
}
void diaElemFrame::enable(uint32_t onoff)
{
  
}
} // End of namespace
//****************************Hoook*****************

diaElem  *qt4CreateFrame(const char *toggleTitle, const char *tip)
{
	return new  ADM_qt4Factory::diaElemFrame(toggleTitle,tip);
}
void qt4DestroyFrame(diaElem *e)
{
	ADM_qt4Factory::diaElemFrame *a=(ADM_qt4Factory::diaElemFrame *)e;
	delete a;
}
//EOF
