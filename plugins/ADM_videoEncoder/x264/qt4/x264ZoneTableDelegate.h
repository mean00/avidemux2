#ifndef x264ZoneTableDelegate_h
#define x264ZoneTableDelegate_h

#include <QtCore/QModelIndex>
#include <QtGui/QItemDelegate>

class x264ZoneTableDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    x264ZoneTableDelegate(QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};
#endif
