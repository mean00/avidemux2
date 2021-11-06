/***************************************************************************
  T_floatResettable.cpp
  Handle dialog factory element : float with reset button
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

#include "T_floatResettable.h"
#include "ADM_default.h"
#include "ADM_dialogFactoryQt4.h"

extern const char *shortkey(const char *);

namespace ADM_Qt4Factory
{

class diaElemFloatResettable : public diaElem,QtFactoryUtils
{
protected:
    ELEM_TYPE_FLOAT min,max,reset;
    int decimals;

public:
    diaElemFloatResettable(ELEM_TYPE_FLOAT *value, const char *toggleTitle,
        ELEM_TYPE_FLOAT min, ELEM_TYPE_FLOAT max, ELEM_TYPE_FLOAT reset,
        const char *tip = NULL, int decimals = 2);
    virtual ~diaElemFloatResettable();
    void setMe(void *dialog, void *opaque,uint32_t line);
    void getMe(void);
    void enable(uint32_t onoff);
    int getRequiredLayout(void);
};

diaElemFloatResettable::diaElemFloatResettable(ELEM_TYPE_FLOAT *value, const char *toggleTitle,
    ELEM_TYPE_FLOAT min, ELEM_TYPE_FLOAT max, ELEM_TYPE_FLOAT reset,
    const char *tip, int decimals) : diaElem(ELEM_TOGGLE), QtFactoryUtils(toggleTitle)
{
    param = (void *)value;
    paramTitle = shortkey(toggleTitle);
    this->min = min;
    this->max = max;
    this->reset = reset;
    this->tip = tip;
    this->decimals = decimals;
}

diaElemFloatResettable::~diaElemFloatResettable()
{
    ADM_QDoubleSpinboxResettable *bx = (ADM_QDoubleSpinboxResettable *)myWidget;
    delete bx;
    myWidget = NULL;
    ADM_dealloc(paramTitle);
    paramTitle = NULL;
}

void diaElemFloatResettable::setMe(void *dialog, void *opaque, uint32_t line)
{
    QGridLayout *grid = (QGridLayout *)opaque;
    QWidget *parent = (QWidget *)dialog;
    ELEM_TYPE_FLOAT *val = (ELEM_TYPE_FLOAT *)param;
    void *ptr = (void *) this;
    ADM_QDoubleSpinboxResettable *bx = new ADM_QDoubleSpinboxResettable(parent, grid, ptr, paramTitle, tip, line, decimals, min, max, reset, *val);
    myWidget = (void *)bx;
}

void diaElemFloatResettable::getMe(void)
{
    ADM_QDoubleSpinboxResettable *bx = (ADM_QDoubleSpinboxResettable *)myWidget;
    if(!bx) return;
    double val = bx->readout();
    if(val > max) val = max;
    if(val < min) val = min;
    *(ELEM_TYPE_FLOAT *)param = val;
}

void diaElemFloatResettable::enable(uint32_t onoff) 
{
    ADM_QDoubleSpinboxResettable *bx = (ADM_QDoubleSpinboxResettable *)myWidget;
    if(!bx) return;
    bx->enable(!!onoff);
}

int diaElemFloatResettable::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }

//********************************************************************

/**
    \fn ctor
*/
ADM_QDoubleSpinboxResettable::ADM_QDoubleSpinboxResettable(
        QWidget *parent, QGridLayout *grid, void *elem,
        const char *title, const char *tip, int line, int dec,
        double min, double max, double rst, double current) : QWidget(parent)
{
    cookie = elem;

    ADM_assert(max >= min);
    ADM_assert(rst >= min);
    ADM_assert(rst <= max);

    if(current > max) current = max;
    if(current < min) current = min;

    _rst = rst;

    box = new QDoubleSpinBox(parent);
    box->setMinimum(min);
    box->setMaximum(max);
    box->setDecimals(dec);
    box->setSingleStep(0.1);
    box->setValue(current);
    if(tip)
        box->setToolTip(QString::fromUtf8(tip));

    text = new QLabel(title,parent);
    text->setBuddy(box);

    QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    button = new QPushButton(QString::fromUtf8(QT_TRANSLATE_NOOP("adm","Reset")), parent);

    QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(reset(bool)));

    QHBoxLayout *hboxLayout = new QHBoxLayout();
    hboxLayout->addWidget(box);
    hboxLayout->addWidget(button);
    hboxLayout->addItem(spacer);

    grid->addWidget(text,line,0);
    grid->addLayout(hboxLayout,line,1);
}

/**
    \fn dtor
*/
ADM_QDoubleSpinboxResettable::~ADM_QDoubleSpinboxResettable()
{
    diaElem *elem = (diaElem *)cookie;
    elem->myWidget = NULL; // avoid crash from double delete FIXME
}

/**
    \fn reset
*/
void ADM_QDoubleSpinboxResettable::reset(bool checked)
{
    UNUSED_ARG(checked);
    box->setValue(_rst);
}

/**
    \fn enable
*/
void ADM_QDoubleSpinboxResettable::enable(bool onoff)
{
    text->setEnabled(onoff);
    box->setEnabled(onoff);
    button->setEnabled(onoff);
}

/**
    \fn readout
*/
double ADM_QDoubleSpinboxResettable::readout(void)
{
    return box->value();
}

} // End of namespace
//****************************Hoook*****************

diaElem *qt4CreateFloatResettable(ELEM_TYPE_FLOAT *initialValue, const char *title,
    ELEM_TYPE_FLOAT min, ELEM_TYPE_FLOAT max, ELEM_TYPE_FLOAT reset, const char *tip, int decimals)
{
    return new ADM_Qt4Factory::diaElemFloatResettable(initialValue,title,min,max,reset,tip,decimals);
}
void qt4DestroyFloatResettable(diaElem *e)
{
    ADM_Qt4Factory::diaElemFloatResettable *a=(ADM_Qt4Factory::diaElemFloatResettable *)e;
    delete a;
}
