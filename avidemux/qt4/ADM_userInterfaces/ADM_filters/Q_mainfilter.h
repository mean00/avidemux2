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
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
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
    void up(bool b);
    void down(bool b);
    void remove(bool b);
    void configure(bool b);
    void saveFilters(bool b);
    void loadFilters(bool b);
    void partial(bool b);
    void activeDoubleClick( QListWidgetItem  *item);
    void allDoubleClick( QListWidgetItem  *item);
    void filterFamilyClick(QListWidgetItem *item);
    void filterFamilyClick(int  item);
    void preview(bool b);
    void closePreview(void);
    // context menu
    void add(void);
    void removeAction(void);
    void configureAction(void);
protected:
    void removeAtIndex(QListWidgetItem *item) ;
private:
    void setSelected(int sel);
    void displayFamily(uint32_t family);
    void setupFilters(void);
};

