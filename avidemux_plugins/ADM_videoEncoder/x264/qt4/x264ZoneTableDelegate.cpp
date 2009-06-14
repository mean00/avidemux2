/***************************************************************************
                         x264ZoneTableDelegate.cpp
                         -------------------------

    begin                : Fri May 16 2008
    copyright            : (C) 2008 by gruntster
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>

#include "x264ZoneTableDelegate.h"

x264ZoneTableDelegate::x264ZoneTableDelegate(QObject *parent) : QItemDelegate(parent)
{
}

QWidget *x264ZoneTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QSpinBox *spinBox;
	QComboBox *comboBox;

	switch (index.column())
	{
		case 0:
			spinBox = new QSpinBox(parent);

			spinBox->setMinimum(0);
			spinBox->setMaximum(9999999);

			return spinBox;
		case 1:
			spinBox = new QSpinBox(parent);

			spinBox->setMinimum(0);
			spinBox->setMaximum(9999999);

			return spinBox;
		case 2:
			comboBox = new QComboBox(parent);

			comboBox->addItem(QT_TR_NOOP("Quantiser"));
			comboBox->addItem(QT_TR_NOOP("Bitrate Factor"));

			return comboBox;
		case 3:
			spinBox = new QSpinBox(parent);

			spinBox->setMinimum(0);
			spinBox->setMaximum(9999);

			return spinBox;
	}
}

void x264ZoneTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	int value = index.model()->data(index, Qt::EditRole).toInt();

	if (index.column() == 2)
	{
		QComboBox *comboBox = static_cast<QComboBox*>(editor);
		comboBox->setCurrentIndex(value);
	}
	else
	{
		QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
		spinBox->setValue(value);
	}
}

void x264ZoneTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	int value;

	if (index.column() == 2)
	{
		QComboBox *comboBox = static_cast<QComboBox*>(editor);
		value = comboBox->currentIndex();
	}
	else
	{
		QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
		spinBox->interpretText();
		value = spinBox->value();
	}

	model->setData(index, value, Qt::EditRole);
}

void x264ZoneTableDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	editor->setGeometry(option.rect);
}
