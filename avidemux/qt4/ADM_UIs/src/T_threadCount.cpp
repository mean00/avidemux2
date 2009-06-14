/***************************************************************************
FAC_toggle.cpp
Handle dialog factory element : Thread Count
(C) 2007 Gruntster 
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "T_threadCount.h"
#include "ADM_default.h"
#include "DIA_factory.h"

extern const char* shortkey(const char*);

namespace ADM_qt4Factory
{

class diaElemThreadCount : public diaElem
{

public:
  
  diaElemThreadCount(uint32_t *value, const char *title, const char *tip = NULL);
  virtual ~diaElemThreadCount() ;
  void setMe(void *dialog, void *opaque, uint32_t line);
  void getMe(void);
};

void ADM_QthreadCount::radioGroupChanged(QAbstractButton *s)
{
	spinBox->setEnabled(radiobutton3->isChecked());
}

ADM_QthreadCount::ADM_QthreadCount(QWidget *widget, const char *title, uint32_t value, QGridLayout *layout, int line) : QWidget(widget) 
{
	radiobutton1 = new QRadioButton(QString::fromUtf8(QT_TR_NOOP("Disabled")), widget);
	radiobutton2 = new QRadioButton(QString::fromUtf8(QT_TR_NOOP("Auto-detect")), widget);
	radiobutton3 = new QRadioButton(QString::fromUtf8(QT_TR_NOOP("Custom")), widget);

	buttonGroup = new QButtonGroup;
	buttonGroup->addButton(radiobutton1);
	buttonGroup->addButton(radiobutton2);
	buttonGroup->addButton(radiobutton3);

	spinBox = new QSpinBox();
	spinBox->setRange(2, 32);

	text = new QLabel(QString::fromUtf8(title), widget);
	text->setBuddy(radiobutton1);

	layout->addWidget(text, line, 0);
	layout->addWidget(radiobutton1, line, 1);
	layout->addWidget(radiobutton2, line, 2);
	layout->addWidget(radiobutton3, line, 3);
	layout->addWidget(spinBox, line, 4);

	QObject::connect(buttonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(radioGroupChanged(QAbstractButton*)));

	spinBox->setEnabled(value > 1);

	if (value == 0)
		radiobutton2->setChecked(true);
	else if (value == 1)
		radiobutton1->setChecked(true);
	else
	{
		radiobutton3->setChecked(true);
		spinBox->setValue(value);
	}
}

ADM_QthreadCount::~ADM_QthreadCount() 
{
	if (buttonGroup)
		delete buttonGroup;

	if (radiobutton1)
		delete radiobutton1;

	if (radiobutton2)
		delete radiobutton2;

	if (radiobutton3)
		delete radiobutton3;

	if (spinBox)
		delete spinBox;

	if (text)
		delete text;
}

diaElemThreadCount::diaElemThreadCount(uint32_t *value, const char *title, const char *tip) : diaElem(ELEM_THREAD_COUNT)
{
	param = (void*)value;
	paramTitle = shortkey(title);
	this->tip = tip;
}

diaElemThreadCount::~diaElemThreadCount()
{
	delete paramTitle;
}

void diaElemThreadCount::setMe(void *dialog, void *opaque, uint32_t line)
{
	QGridLayout *layout = (QGridLayout*)opaque;

	ADM_QthreadCount *threadCount = new ADM_QthreadCount((QWidget*)dialog, paramTitle, *(uint32_t *)param, layout, line);

	myWidget = (void*)threadCount;
}

void diaElemThreadCount::getMe(void)
{
	ADM_QthreadCount *threadCount = (ADM_QthreadCount*)myWidget;
	uint32_t *val = (uint32_t*)param;

	if ((threadCount->radiobutton1)->isChecked())
		*val = 1;
	else if ((threadCount->radiobutton2)->isChecked())
		*val = 0;
	else
		*val = (threadCount->spinBox)->value();
}
//**********************
} // End of namesapce
//**********************
diaElem  *qt4CreateThreadCount(uint32_t *value, const char *title, const char *tip)
{
	return new  ADM_qt4Factory::diaElemThreadCount(value,title,tip);
}
void qt4DestroyThreadCount(diaElem *e)
{
	ADM_qt4Factory::diaElemThreadCount *a=(ADM_qt4Factory::diaElemThreadCount *)e;
	delete a;
}
//EOF
