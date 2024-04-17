/***************************************************************************
  FAC_tiling.cpp
  Handle dialog factory element : Tiling
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

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>

#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_dialogFactoryQt4.h"

namespace ADM_qt4Factory
{
class diaElemTiling : public diaElem, QtFactoryUtils
{
private:
    uint32_t maxLog2Cols, maxLog2Rows;
    void *rowControl, *label;
public:
    diaElemTiling(uint32_t *tiling, uint32_t *maxlog2cols, uint32_t *maxlog2rows, const char *title, const char *tip = NULL);
    virtual ~diaElemTiling();
    void setMe(void *dialog, void *opaque, uint32_t line);
    void getMe(void);
    void enable(uint32_t onoff);
    int getRequiredLayout(void);
};

diaElemTiling::diaElemTiling(uint32_t *tiling, uint32_t *maxlog2cols, uint32_t *maxlog2rows, const char *title, const char *tip) : diaElem(ELEM_TILING), QtFactoryUtils(title)
{
    param = (void *)tiling; // (log2(nbRows) << 16) + log2(nbCols)
    this->maxLog2Cols = *maxlog2cols;
    this->maxLog2Rows = *maxlog2rows;
    this->tip = tip;
}

diaElemTiling::~diaElemTiling()
{

}

void diaElemTiling::setMe(void *dialog, void *opaque, uint32_t line)
{
    QLabel *text = new QLabel(myQtTitle);
    QComboBox *colBox = new QComboBox();
    QLabel *label = new QLabel("x");
    QComboBox *rowBox = new QComboBox();
    QGridLayout *layout = (QGridLayout *) opaque;
    QHBoxLayout *hboxLayout = new QHBoxLayout();

    myWidget = (void*)colBox;
    this->label = (void*)label;
    this->rowControl = (void*)rowBox;

    text->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    text->setBuddy(colBox);

    colBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("tiling", "Columns: 1")));
    rowBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("tiling", "Rows: 1")));

    for(int i = 1; i <= ((maxLog2Cols > maxLog2Rows) ? maxLog2Cols : maxLog2Rows); i++)
    {
        QString s = QString("%1").arg(1 << i);
        if (i <= maxLog2Cols)
            colBox->addItem(s);
        if (i <= maxLog2Rows)
            rowBox->addItem(s);
    }

    uint32_t tiling = *(uint32_t *)param;
    colBox->setCurrentIndex(tiling & 0xFFFF);
    rowBox->setCurrentIndex((tiling >> 16) & 0xFFFF);

    QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addWidget(colBox);
    hboxLayout->addWidget(label);
    hboxLayout->addWidget(rowBox);
    hboxLayout->addItem(spacer);

    colBox->setToolTip(QString::fromUtf8(tip));
    rowBox->setToolTip(QString::fromUtf8(tip));

    layout->addWidget(text,line,0);
    layout->addLayout(hboxLayout,line,1);
}

void diaElemTiling::getMe(void)
{
    int idx;
    uint32_t tiling;

    QComboBox *colBox = (QComboBox *)myWidget;
    idx = colBox->currentIndex();
    if(idx < 0) idx = 0;
    tiling = (uint32_t)idx & 0xFFFF;

    QComboBox *rowBox = (QComboBox *)rowControl;
    idx = rowBox->currentIndex();
    if(idx < 0) idx = 0;
    tiling += ((uint32_t)idx & 0xFFFF) << 16;
    *(uint32_t *)param = tiling;
}

void diaElemTiling::enable(uint32_t onoff)
{
    QComboBox *colBox = (QComboBox *)myWidget;
    QComboBox *rowBox = (QComboBox *)rowControl;
    QLabel *label = (QLabel*)this->label;

    colBox->setEnabled(onoff);
    rowBox->setEnabled(onoff);
    label->setEnabled(onoff);
}

int diaElemTiling::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }
} // End of namespace
//****************************Hoook*****************

diaElem *qt4CreateTiling(uint32_t *tiling, uint32_t *maxlog2cols, uint32_t *maxlog2rows, const char *title, const char *tip)
{
    return new ADM_qt4Factory::diaElemTiling(tiling, maxlog2cols, maxlog2rows, title, tip);
}

void qt4DestroyTiling(diaElem *e)
{
    ADM_qt4Factory::diaElemTiling *a = (ADM_qt4Factory::diaElemTiling *)e;
    delete a;
}
// EOF
