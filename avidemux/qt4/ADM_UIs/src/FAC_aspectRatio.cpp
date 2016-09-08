/***************************************************************************
  FAC_aspectRatio.cpp
  Handle dialog factory element : Aspect Ratio
  (C) 2008 Gruntster
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDialog>
#include <QSpinBox>
#include <QGridLayout>
#include <QLabel>

#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_dialogFactoryQt4.h"

extern const char *shortkey(const char *);

namespace ADM_qt4Factory
{
class diaElemAspectRatio : public diaElem,QtFactoryUtils
{
public:
	uint32_t *den;
	void *denControl, *label;

	diaElemAspectRatio(uint32_t *num, uint32_t *den, const char *title, const char *tip = NULL);
	virtual ~diaElemAspectRatio();
	void setMe(void *dialog, void *opaque, uint32_t line);
	void getMe(void);
	void enable(uint32_t onoff);
	int getRequiredLayout(void);
};

diaElemAspectRatio::diaElemAspectRatio(uint32_t *num, uint32_t *den, const char *title, const char *tip) : diaElem(ELEM_ASPECT_RATIO)
{
	param = (void *)num;
	this->den = den;
	titleFromShortKey(title);
	this->tip = tip;
}

diaElemAspectRatio::~diaElemAspectRatio()
{
	
}

void diaElemAspectRatio::setMe(void *dialog, void *opaque, uint32_t line)
{
	QLabel *text = new QLabel(myQtTitle);
	QSpinBox *numBox = new QSpinBox();
	QLabel *label = new QLabel(":");
	QSpinBox *denBox = new QSpinBox();
	QGridLayout *layout = (QGridLayout*) opaque;
	QHBoxLayout *hboxLayout = new QHBoxLayout();

	myWidget = (void*)numBox;
	this->label = (void*)label;
	this->denControl = (void*)denBox;

	text->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	text->setBuddy(numBox);

	numBox->setMinimum(1);
	numBox->setMaximum(255);

	denBox->setMinimum(1);
	denBox->setMaximum(255);

	numBox->setValue(*(uint32_t*)param);
	denBox->setValue(*(uint32_t*)den);

	QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	hboxLayout->addWidget(numBox);
	hboxLayout->addWidget(label);
	hboxLayout->addWidget(denBox);
	hboxLayout->addItem(spacer);

	layout->addWidget(text,line,0);
	layout->addLayout(hboxLayout,line,1);
}

void diaElemAspectRatio::getMe(void)
{
	*(uint32_t*)param = ((QSpinBox*)myWidget)->value();
	*(uint32_t*)den = ((QSpinBox*)denControl)->value();
}

void diaElemAspectRatio::enable(uint32_t onoff) 
{
	QSpinBox *numBox = (QSpinBox*)myWidget;
	QSpinBox *denBox = (QSpinBox*)denControl;
	QLabel *label = (QLabel*)this->label;

	numBox->setEnabled(onoff);
	denBox->setEnabled(onoff);
	label->setEnabled(onoff);
}

int diaElemAspectRatio::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }
} // End of namespace
//****************************Hoook*****************

diaElem  *qt4CreateAspectRatio(uint32_t *num, uint32_t *den, const char *title, const char *tip)
{
	return new ADM_qt4Factory::diaElemAspectRatio(num, den, title, tip);
}

void qt4DestroyAspectRatio(diaElem *e)
{
	ADM_qt4Factory::diaElemAspectRatio *a = (ADM_qt4Factory::diaElemAspectRatio *)e;
	delete a;
}
