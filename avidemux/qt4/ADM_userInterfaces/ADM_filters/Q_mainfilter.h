/***************************************************************************

    \file Q_mainfilter.h
    \brief UI for filters
    \author mean, fixount@free.fr 2007/2015

    * We hide some info the the "type"
    * I.e.
    0--1000 : QT4 internal
    2000-3000: Filters
                 Each family starts a category*100 then filter in the order they are in categories
                 ** 10 Categories MAX !!
    3000-4000  filterFamilyClick Filter
    8000-9000  Active Filter


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once
#include <QItemDelegate>
#include "ui_mainfilter.h"
#include "ui_quickfilter.h"
#include "ADM_inttype.h"
#include "Q_seekablePreview.h"
/**
 * \class FilterItemEventFilter
 * @return 
 */
class FilterItemEventFilter : public QObject
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *object, QEvent *event);

public:
    FilterItemEventFilter(QWidget *parent = 0);
};
/**
 * \class FilterItemDelegate
 * @return 
 */
class FilterItemDelegate : public QItemDelegate
{
    Q_OBJECT

private:
    FilterItemEventFilter *filter;

public:
    FilterItemDelegate(QWidget *parent = 0);
    enum datarole { FilterNameRole = Qt::UserRole, DescriptionRole, DisabledRole };
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    static int padding;
};
/**
 * \class filtermainWindow
 * @return 
 */
class filtermainWindow : public QDialog
{
    Q_OBJECT

public:
            filtermainWindow(QWidget* parent);
            ~filtermainWindow();
    void    buildActiveFilterList(void);

    Ui_mainFilterDialog ui;
    QListWidget *availableList;
    QListWidget *activeList;
protected:
    
    int previewDialogX, previewDialogY;
    Ui_seekablePreviewWindow *previewDialog;
public slots:   
    void add(bool b);
    void moveUp();
    void moveDown();
    void togglePartial();
    void makePartial();
    void absolvePartial();
    void toggleEnabled();
    void duplicate();
    void remove(bool b);
    void configure(bool b);
    void activeDoubleClick( QListWidgetItem  *item);
    void allDoubleClick( QListWidgetItem  *item);
    void filterFamilyClick(QListWidgetItem *item);
    void filterFamilyClick(int  item);
    void preview(bool b);
    void closePreview(void);
    // context menu
    void addSlot(void);
    void removeAction(void);
    void configureAction(void);
    void duplicateAction(void);
    void activeListContextMenu(const QPoint &pos);
    // reorder active filters using drag-n-drop
    void rowsMovedSlot(void);
protected:
    int  getTagForActiveSelection();
private:
    void setSelected(int sel);
    void displayFamily(uint32_t family);
    void setupFilters(void);
    void updateContextMenu(QMenu *contextMenu);
    bool eventFilter(QObject* watched, QEvent* event);
    uint64_t originalTime;
private:
    QKeySequence shortcutMoveUp;
    QKeySequence shortcutMoveDown;
    QKeySequence shortcutConfigure;
    QKeySequence shortcutDuplicate;
    QKeySequence shortcutRemove;
    QKeySequence shortcutTogglePartial;
    QKeySequence shortcutToggleEnabled;

};




class filterquickWindow : public QDialog
{
    Q_OBJECT

public:
            filterquickWindow(QWidget* parent);
            ~filterquickWindow();

    Ui_quickFilterDialog ui;
    QListWidget *availableList;
public slots:   
    void add(bool b);
    void allDoubleClick( QListWidgetItem  *item);
    // context menu
    void addSlot(void);
private:
    void displayPartialFilters(void);
    void setupFilters(void);
    bool eventFilter(QObject* watched, QEvent* event);
    uint64_t originalTime;

};
