#ifndef x264ZoneTableModel_h
#define x264ZoneTableModel_h

#include <QtCore/QAbstractTableModel>
#include <QtCore/QPair>
#include <QtCore/QList>

#include "../zoneOptions.h"

class x264ZoneTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    x264ZoneTableModel(QObject *parent = 0);
    x264ZoneTableModel(QList<x264ZoneOptions*> zoneData, QObject *parent = 0);
	~x264ZoneTableModel();

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
	bool insertRows(int position, int rows, const QModelIndex &index, x264ZoneOptions** zoneOptions);
	bool removeRows(void);
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());
    QList<x264ZoneOptions*> getList();

private:
    QList<x264ZoneOptions*> _zoneData;
};

#endif
