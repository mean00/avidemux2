/***************************************************************************
  FAC_frame.cpp
  Handle dialog factory element : Frame
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
#include <QGroupBox>

#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_dialogFactoryQt4.h"

extern const char *shortkey(const char *);


namespace ADM_qt4Factory
{
class diaElemFrame : public diaElemFrameBase,QtFactoryUtils
{
  
public:
  
  diaElemFrame(const char *toggleTitle, const char *tip=NULL);
  virtual ~diaElemFrame() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void) ;
  void swallow(diaElem *widget);
  void enable(uint32_t onoff);
  void finalize(void);
  int getRequiredLayout(void);
};

diaElemFrame::diaElemFrame(const char *toggleTitle, const char *tip)
  : diaElemFrameBase(),QtFactoryUtils(toggleTitle)
{
  param=NULL;
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
}

void diaElemFrame::setMe(void *dialog, void *opaque,uint32_t line)
{
	QVBoxLayout *layout = (QVBoxLayout*)opaque;
	QGroupBox *groupBox = new QGroupBox(myQtTitle);
	QVBoxLayout *vboxlayout = new QVBoxLayout(groupBox);
	QLayout *layout2 = NULL;
	int currentLayout = 0;
	int v;

	for (int i = 0; i < nbElems; i++)
	{
		if (elems[i]->getRequiredLayout() != currentLayout)
		{
			if (layout2)
				vboxlayout->addLayout(layout2);

			switch (elems[i]->getRequiredLayout())
			{
				case FAC_QT_GRIDLAYOUT:
					layout2 = new QGridLayout();
					break;
				case FAC_QT_VBOXLAYOUT:
					layout2 = new QVBoxLayout();
					break;
			}

			currentLayout = elems[i]->getRequiredLayout();
			v = 0;
		}

		elems[i]->setMe(groupBox, layout2, v); 
		v += elems[i]->getSize();
	}

	if (layout2)
		vboxlayout->addLayout(layout2);

	layout->addWidget(groupBox);
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

int diaElemFrame::getRequiredLayout(void) { return FAC_QT_VBOXLAYOUT; }
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
