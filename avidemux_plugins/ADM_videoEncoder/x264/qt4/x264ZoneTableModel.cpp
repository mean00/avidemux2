/***************************************************************************
                           x264ZoneTableModel.cpp
                           ----------------------

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

#include "x264ZoneTableModel.h"

x264ZoneTableModel::x264ZoneTableModel(QObject *parent) : QAbstractTableModel(parent)
{
}

x264ZoneTableModel::x264ZoneTableModel(QList<x264ZoneOptions*> zoneData, QObject *parent) : QAbstractTableModel(parent)
{
	_zoneData = zoneData;
}

x264ZoneTableModel::~x264ZoneTableModel()
{
	for (int zone = 0; zone < _zoneData.size(); zone++)
		delete _zoneData[zone];

	_zoneData.clear();
}

int x264ZoneTableModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	return _zoneData.size();
}

int x264ZoneTableModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	return 4;
}

QVariant x264ZoneTableModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= _zoneData.size() || index.row() < 0)
		return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

	x264ZoneOptions *zoneOptions = _zoneData.at(index.row());

	switch (index.column())
	{
		case 0:
			return zoneOptions->getFrameStart();
		case 1:
			return zoneOptions->getFrameEnd();
		case 2:
			if (role == Qt::DisplayRole)
			{
				switch (zoneOptions->getZoneMode())
				{
					case ZONE_MODE_QUANTISER:
						return QT_TR_NOOP("Quantiser");
					case ZONE_MODE_BITRATE_FACTOR:
						return QT_TR_NOOP("Bitrate Factor");
				}
			}
			else if (role == Qt::EditRole)
			{
				switch (zoneOptions->getZoneMode())
				{
					case ZONE_MODE_QUANTISER:
						return 0;
					case ZONE_MODE_BITRATE_FACTOR:
						return 1;
				}
			}

			break;
		case 3:
			return zoneOptions->getZoneParameter();
	}

	return QVariant();
}

QVariant x264ZoneTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
	{
		switch (section)
		{
			case 0:
				return QT_TR_NOOP("Frame Start");    
			case 1:
				return QT_TR_NOOP("Frame End");
			case 2:
				return QT_TR_NOOP("Mode");
			case 3:
				return QT_TR_NOOP("Value");
			default:
				return QVariant();
		}
	}

	return QVariant();
}

bool x264ZoneTableModel::insertRows(int position, int rows, const QModelIndex &index)
{
	beginInsertRows(index, position, position + rows - 1);

	for (int row = 0; row < rows; row++)
	{
		x264ZoneOptions *zoneOptions = new x264ZoneOptions;
		_zoneData.insert(position + row, zoneOptions);
	}

	endInsertRows();

	return true;
}

bool x264ZoneTableModel::insertRows(int position, int rows,  const QModelIndex &index, x264ZoneOptions** zoneOptions)
{
	beginInsertRows(index, position, position + rows - 1);

	for (int row = 0; row < rows; row++)
		_zoneData.insert(position + row, zoneOptions[row]);

	endInsertRows();

	return true;
}

bool x264ZoneTableModel::removeRows(int row, int count, const QModelIndex &index)
{
	beginRemoveRows(index, row, row + count - 1);

	for (int rowCount = 0; rowCount < count; rowCount++)
	{
		delete _zoneData[row];
		_zoneData.removeAt(row);
	}

	endRemoveRows();

	return true;
}

bool x264ZoneTableModel::removeRows(void)
{
	if (_zoneData.size())
		return removeRows(0, _zoneData.size(), QModelIndex());
}

bool x264ZoneTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::EditRole)
	{
		int row = index.row();

		x264ZoneOptions *zoneOptions = _zoneData.value(row);

		switch (index.column())
		{
			case 0:
				zoneOptions->setFrameRange(value.toUInt(), zoneOptions->getFrameEnd());
				break;
			case 1:
				zoneOptions->setFrameRange(zoneOptions->getFrameStart(), value.toUInt());
				break;
			case 2:
				if (value.toUInt() == 0 && zoneOptions->getZoneMode() != ZONE_MODE_QUANTISER)
					zoneOptions->setQuantiser(26);
				else if (value.toUInt() == 1 && zoneOptions->getZoneMode() != ZONE_MODE_BITRATE_FACTOR)
					zoneOptions->setBitrateFactor(100);

				break;
			case 3:
				if (zoneOptions->getZoneMode() == ZONE_MODE_QUANTISER)
					zoneOptions->setQuantiser(value.toUInt());
				else
					zoneOptions->setBitrateFactor(value.toUInt());

				break;
			default:
				return false;
		}

		_zoneData.replace(row, zoneOptions);
		emit(dataChanged(index, index));

		return true;
	}

	return false;
}

Qt::ItemFlags x264ZoneTableModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

QList<x264ZoneOptions*> x264ZoneTableModel::getList()
{
	return _zoneData;
}
