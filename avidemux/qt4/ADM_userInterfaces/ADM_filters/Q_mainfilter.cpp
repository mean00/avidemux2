/***************************************************************************

    \file Q_mainfilter.cpp
    \brief UI for filters
    \author mean, fixount@free.fr 2007/2009

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
#include "ADM_cpp.h"
#include <vector>
#include <sstream>
using std::string;
#include "Q_mainfilter.h"
#include "ADM_default.h"
#include "DIA_fileSel.h"
#include "DIA_factory.h"
#include "ADM_render/GUI_render.h"

#include "DIA_coreToolkit.h"
#include "ADM_edit.hxx"
#include "ADM_coreVideoFilter.h"
#include "ADM_coreVideoFilterFunc.h"
#include "ADM_filterCategory.h"
#include "ADM_videoFilterApi.h"
#include "ADM_videoFilters.h"
#include "ADM_toolkitQt.h"
#include "ADM_preview.h"

#include "prefs.h"
#include "ADM_edScriptGenerator.h"
#include "DIA_fileSel.h"
#include "ADM_script2/include/ADM_script.h"

#include <QMenu>
#include <QPainter>
#include <QKeyEvent>
/*******************************************************/
#define NB_TREE 8
#define myFg 0xFF
#define myBg 0xF0

/******************************************************/
#define ALL_FILTER_BASE       1000
#define ACTIVE_FILTER_BASE    3000
/******************************************************/
#define nb_active_filter ADM_vf_getSize()

/*******************************************************/
#define zprintf(...) {}

extern ADM_Composer *video_body;

int FilterItemDelegate::padding = 4;

FilterItemEventFilter::FilterItemEventFilter(QWidget *parent) : QObject(parent) {}
/**
 * 
 * @param object
 * @param event
 * @return 
 */
bool FilterItemEventFilter::eventFilter(QObject *object, QEvent *event)
{
    zprintf("Object : %p\n",object);
    zprintf("Parent : %p\n",parent());
#if !defined(NO_EVENT_FILTER)
    QListWidget *list=qobject_cast<QListWidget*>(parent());
    //printf("Event %d\n",event->type());
    switch(event->type())
    {
        case QEvent::KeyPress :
        case QEvent::MouseButtonPress:
        //case QEvent::ContextMenu:
            QCoreApplication::sendEvent(list, event);
            return true;
        case QEvent::FocusIn:       
            list->setFocus();
            return true;
        default:
            return QObject::eventFilter(object, event);
    }
    return false;
#else
      return QObject::eventFilter(object, event);
#endif
}
/**
 * 
 * @param parent
 */
FilterItemDelegate::FilterItemDelegate(QWidget *parent, bool alwaysHighlight) : QItemDelegate(parent)
{
    zprintf("Delegate : %p\n",this);
    this->alwaysHighlight = alwaysHighlight;
    filter = new FilterItemEventFilter(parent);
}

/**
    \fn sizeHint
*/
QSize FilterItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{ // The code below is stolen from http://3adly.blogspot.de/2013/09/qt-custom-qlistview-delegate-with-word.html
    if(!index.isValid())
        return QSize();

    QString filterNameText = index.data(FilterNameRole).toString();
    QString descText = index.data(DescriptionRole).toString();

    QFont filterNameFont = QApplication::font();
    QFont descFont = QApplication::font();
    int fontSize = descFont.pointSize();
    if(fontSize > 0)
    {
        if(fontSize > 8)
        {
            if(fontSize > 9)
            {
                fontSize -= 2;
            }else
            {
                fontSize--;
            }
            descFont.setPointSize(fontSize);
        }
    }else // i.e. the font size has been specified in pixels instead of points
    {
        fontSize = descFont.pixelSize();
        if(fontSize > 10)
        {
            fontSize -= 2;
            descFont.setPixelSize(fontSize);
        }
    }
    filterNameFont.setBold(true);
    QFontMetrics filterNameFm(filterNameFont);
    QFontMetrics descFm(descFont);

    QRect filterNameRect = filterNameFm.boundingRect(0, 0, option.rect.width() - padding * 2, 0, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, filterNameText);
    QRect descRect = descFm.boundingRect(0, 0, option.rect.width() - padding * 2, 0, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, descText);

    QSize size(option.rect.width(), filterNameRect.height() + descRect.height() +  padding * 3);
    return size;
}

/**
 * 
 * @param painter
 * @param option
 * @param index
 */
void FilterItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{ // The code below is stolen from http://3adly.blogspot.de/2013/09/qt-custom-qlistview-delegate-with-word.html
    if(!index.isValid())
        return;

    painter->save();

    QPen pen;
    QColor fg, bg;
    QPalette pal;
    if(option.state & QStyle::State_Selected)
    {
        if((option.state & QStyle::State_HasFocus) || alwaysHighlight)
        {
            fg = pal.color(QPalette::HighlightedText);
            bg = pal.color(QPalette::Highlight);
        }else
        {
            fg = pal.color(QPalette::WindowText);
            bg = pal.color(QPalette::Window);
        }
    }else
    {
        fg = pal.color(QPalette::Text);
        bg = pal.color(QPalette::Base);
    }

    bool disabled = index.data(DisabledRole).toBool();
    if (disabled)
        fg = pal.color(QPalette::Disabled, QPalette::WindowText);

    painter->fillRect(option.rect, bg);
    pen.setColor(fg);
    painter->setPen(pen);

    QString filterNameText = index.data(FilterNameRole).toString();
    QString descText = index.data(DescriptionRole).toString();


    QFont filterNameFont = QApplication::font();
    QFont descFont = QApplication::font();
    int fontSize = descFont.pointSize();
    if(fontSize > 0)
    {
        if(fontSize > 8)
        {
            if(fontSize > 9)
            {
                fontSize -= 2;
            }else
            {
                fontSize--;
            }
            descFont.setPointSize(fontSize);
        }
    }else // i.e. the font size has been specified in pixels instead of points
    {
        fontSize = descFont.pixelSize();
        if(fontSize > 10)
        {
            fontSize -= 2;
            descFont.setPixelSize(fontSize);
        }
    }
    filterNameFont.setBold(true);
    QFontMetrics filterNameFm(filterNameFont);
    QFontMetrics descFm(descFont);

    QRect filterNameRect = filterNameFm.boundingRect(option.rect.left() + padding,
                                     option.rect.top() + padding,
                                     option.rect.width() - padding * 2,
                                     0,
                                     Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                                     filterNameText);

    QRect descRect = descFm.boundingRect(filterNameRect.left(),
                                     filterNameRect.bottom() + padding,
                                     option.rect.width() - padding * 2,
                                     0,
                                     Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                                     descText);

    painter->setFont(filterNameFont);
    painter->drawText(filterNameRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, filterNameText);

    painter->setFont(descFont);
    painter->drawText(descRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, descText);

    painter->restore();
}
/**
    \fn preview
*/
void filtermainWindow::preview(bool b)
{
    QListWidgetItem *item=activeList->currentItem();
    if(!item)
    {
       printf("No selection\n");
       return;
    }

     int itag=item->type();
     ADM_assert(itag>=ACTIVE_FILTER_BASE);
     itag-=ACTIVE_FILTER_BASE;
     /* Filter 0 is the decoder ...*/
     ADM_info("Rank : %d\n",itag);
     ADM_coreVideoFilter     *filter=ADM_vf_getInstance(itag);
     ADM_assert(filter);
    bool                      enabled = ADM_vf_getEnabled(itag);
    uint32_t                  instanceTag=ADM_vf_getTag(itag);
    const char               *name= ADM_vf_getDisplayNameFromTag(instanceTag);
    if (previewDialog)
    {
            delete previewDialog;
            previewDialog=NULL;
    }

    QString title = QT_TRANSLATE_NOOP("qmainfilter","Preview");
    title += QString(" / ");
    if (!enabled)
        title += QT_TRANSLATE_NOOP("qmainfilter","DISABLED ");
    title += QString::fromUtf8(name);

    previewDialog = new Ui_seekablePreviewWindow(this, filter);
    previewDialog->setWindowTitle(title);
    previewDialog->setModal(true);
    connect(previewDialog, SIGNAL(accepted()), this, SLOT(closePreview()));
    connect(previewDialog, SIGNAL(rejected()), this, SLOT(closePreview()));
    qtRegisterDialog(previewDialog);
    previewDialog->show();
    previewDialog->seekablePreview->adjustCanvasPosition();
    previewDialog->canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}
/**
 * \fn closePreview
 */
void filtermainWindow::closePreview()
{
    if (previewDialog)
    {
        if(previewDialog->seekablePreview)
        {
            previewDialog->blockSignals(true); // we crash when this slot is invoked more than once
            previewDialog->seekablePreview->play(false);
            previewDialog->seekablePreview->goToTime(0);
        }
        qtUnregisterDialog(previewDialog);
    }
}

/**
        \fn     void setSelected(int sel)
        \brief  Set the sel line as selected in the active filter window
*/
void filtermainWindow::setSelected( int sel)
{
  activeList->setCurrentRow(sel);
}

/**
        \fn     add( bool b)
        \brief  Retrieve the selected filter and add it to the active filters
*/
void filtermainWindow::add( bool b)
{
  /* Now that we have the tab, get the selection */
   QListWidgetItem *item=availableList->currentItem();
   VF_FILTERS tag;
   if(item)
   {
     int itag=item->type();

     if(itag<ALL_FILTER_BASE || itag >= ALL_FILTER_BASE+(VF_MAX*100))
     {
            ADM_assert(0);
     }
     // Extract family & index
     itag-=ALL_FILTER_BASE;
     int index=itag%100;
     int family=(itag-index)/100;
     ADM_assert(family<VF_MAX);
     ADM_assert(index<ADM_vf_getNbFiltersInCategory((VF_CATEGORY)family));
     tag=index; //filterCategories[family][index]->tag;
     ADM_info("Tag : %d->family=%d, index=%d\n",itag,family,tag);

     if(ADM_vf_addFilterFromTag(video_body, itag,NULL,true) != NULL)
        {
            buildActiveFilterList();
            setSelected(nb_active_filter-1);
        }else
        {
            ADM_warning("Cannot add filter from tag\n");
        }
   }
}
/**
    \fn duplicate
*/
void filtermainWindow::duplicate()
{
    int filterIndex=getTagForActiveSelection();
    if(-1==filterIndex)
        return;
    ADM_vf_duplicateFilterAtIndex(video_body, filterIndex);
    buildActiveFilterList ();
    if(nb_active_filter)
        setSelected(nb_active_filter-1);
}
/**
    \fn removeAction
    \brief active filter context menu item
*/
void filtermainWindow::removeAction(void)
{
   remove(true);
}
/**
        \fn     remove( bool b)
        \brief  Remove selected filters from the active window list
*/
void filtermainWindow::remove( bool b)
{
    if(!nb_active_filter)
        return;
    int itag=getTagForActiveSelection();
    if(-1==itag)
        return;
    ADM_info("Deleting item %d\n",itag);
    ADM_vf_removeFilterAtIndex(itag);
    buildActiveFilterList ();
    if(nb_active_filter)
    {
        if (itag < nb_active_filter)
            setSelected(itag);
        else
            setSelected(nb_active_filter-1);
    }
}
/**
    \fn configureAction
    \brief active filter context menu item
*/
void filtermainWindow::configureAction(void)
{
    configure(true);
}

/**
    \fn duplicateAction
    \brief active filter context menu item
*/
void filtermainWindow::duplicateAction(void)
{
    duplicate();
}
/**
 * \fn getTagForActiveSelection
 */
int filtermainWindow::getTagForActiveSelection()
{
   QListWidgetItem *item=activeList->currentItem();
   if(!item)
   {
      ADM_warning("No selection\n");
      return -1;
   }

    int itag=item->type();
    ADM_assert(itag>=ACTIVE_FILTER_BASE);
    itag-=ACTIVE_FILTER_BASE;
    /* Filter 0 is the decoder ...*/ 
    return itag;
}
/**
        \fn     configure( bool b)
        \brief  Configure the selected active filter
*/
void filtermainWindow::configure( bool b)
{
    int itag=getTagForActiveSelection();
    if(-1==itag)
        return;
    ADM_vf_configureFilterAtIndex(itag);
    buildActiveFilterList ();
    setSelected(itag);
}
/**
        \fn     up( bool b)
        \brief  Move selected filter one place up
*/
void filtermainWindow::moveUp( )
{
    int itag=getTagForActiveSelection();
    if(-1==itag)
        return;
    if(0==itag) // already at the top
        return;
    ADM_vf_moveFilterUp(itag);
    buildActiveFilterList ();
    setSelected(itag-1);
}

/**
 * 
 */
void filtermainWindow::togglePartial()
{
    int filterIndex=getTagForActiveSelection();
    if(-1==filterIndex)
        return;
    uint32_t tag=ADM_vf_getTag(filterIndex);
    if (tag == VF_PARTIAL_FILTER)
        absolvePartial();
    else
        makePartial();
        
}
/**
 * 
 */
void filtermainWindow::makePartial()
{
    ADM_info("Partial\n");
    int filterIndex=getTagForActiveSelection();
    if(-1==filterIndex)
        return;
    ADM_info("Partial %d\n",filterIndex);
    // Check we can partialize it...
    uint32_t tag=ADM_vf_getTag(filterIndex);
    if(!ADM_vf_canBePartialized(tag))
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("qmainfilter","Partial"),QT_TRANSLATE_NOOP("qmainfilter","This filter cannot be made partial"));
        return;
    }
    // Get info about that filter..
    if(ADM_vf_partialize(filterIndex))
    {
        ADM_info("Partializing done, rebuilding \n");
        buildActiveFilterList ();
        setSelected(filterIndex);
    }else
    {
        ADM_info("CANCEL \n");
    }
}
/**
 * 
 */
void filtermainWindow::absolvePartial()
{
    ADM_info("Absolve Partial\n");
    int filterIndex=getTagForActiveSelection();
    if(-1==filterIndex)
        return;
    ADM_info("Absolve Partial %d\n",filterIndex);
    // Check we can partialize it...
    uint32_t tag=ADM_vf_getTag(filterIndex);
    if(tag!=VF_PARTIAL_FILTER)
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("qmainfilter","Partial"),QT_TRANSLATE_NOOP("qmainfilter","This filter is not partial"));
        return;
    }
    // Get info about that filter..
    if(ADM_vf_absolvePartialized(filterIndex))
    {
        ADM_info("Absolving partialized done, rebuilding \n");
        buildActiveFilterList ();
        setSelected(filterIndex);
    }else
    {
        ADM_info("CANCEL \n");
    }
}
/**
 * 
 */
void filtermainWindow::toggleEnabled()
{
    int filterIndex=getTagForActiveSelection();
    if(-1==filterIndex)
        return;
    ADM_vf_toggleFilterEnabledAtIndex(filterIndex);
    buildActiveFilterList ();
    setSelected(filterIndex);
}
/**
        \fn     down( bool b)
        \brief  Move selected filter one place down
*/
void filtermainWindow::moveDown( )
{
    int itag=getTagForActiveSelection();
    if(-1==itag)
        return;
    if (((int) itag < (int) (nb_active_filter - 1)))
    {
        ADM_vf_moveFilterDown(itag);
        buildActiveFilterList ();
        setSelected(itag+1);
    }
}
/**
        \fn     filtermainWindow::filterFamilyClick( QListWidgetItem  *item)
        \brief  Select family among color etc...
*/

void filtermainWindow::filterFamilyClick(QListWidgetItem *item)
{

    int family= ui.listFilterCategory->currentRow();
    if(family>=0)
        displayFamily(family);

}
void filtermainWindow::filterFamilyClick(int  m)
{

    if(m>=0)
            displayFamily(m);

}

/**
 * \fn displayFamily
 * @param family
 */
void filtermainWindow::displayFamily(uint32_t family)
{
    ADM_assert(family<VF_MAX);

    uint32_t nb=ADM_vf_getNbFiltersInCategory((VF_CATEGORY)family);
    ADM_info("Video filter Family :%u, nb %d\n",family,nb);
    availableList->clear();
    for (uint32_t i = 0; i < nb; i++)
    {
        const char *name,*desc;
        uint32_t major,minor,patch;
        ADM_vf_getFilterInfo((VF_CATEGORY)family,i,&name, &desc,&major,&minor,&patch);

        QString s1 = QString::fromUtf8(name);
        QString s2 = QString::fromUtf8(desc);

        QListWidgetItem *item;
        item=new QListWidgetItem(NULL,availableList,ALL_FILTER_BASE+i+family*100);
        item->setData(FilterItemDelegate::FilterNameRole, s1);
        item->setData(FilterItemDelegate::DescriptionRole, s2);
        availableList->addItem(item);
    }

    if (nb)
       availableList->setCurrentRow(0);
}

/**
        \fn     filtermainWindow::activeDoubleClick( QListWidgetItem  *item)
        \brief  One of the active window has been double clicked, call configure
*/
void filtermainWindow::activeDoubleClick( QListWidgetItem  *item)
{
    configure(0);
}
/**
        \fn     filtermainWindow::activeDoubleClick( QListWidgetItem  *item)
        \brief  One of the active window has been double clicked, call configure
*/
void filtermainWindow::allDoubleClick( QListWidgetItem  *item)
{
    add(0);
}

/**
        \fn setup
        \brief Prepare
*/
void filtermainWindow::setupFilters(void)
{

}

/**
        \fn     buildActiveFilterList(void)
        \brief  Build and display all active filters (may be empty)
*/
void filtermainWindow::buildActiveFilterList(void)
{
    activeList->clear();
    int nb=ADM_vf_getSize();
    printf("%d active filters\n",nb);
    for (uint32_t i = 0; i < nb; i++)
    {
        uint32_t                instanceTag=ADM_vf_getTag(i);
        ADM_coreVideoFilter     *instance=ADM_vf_getInstance(i);
        bool                    enabled = ADM_vf_getEnabled(i);
        const char *name= ADM_vf_getDisplayNameFromTag(instanceTag);
        const char *conf=instance->getConfiguration();
        printf("%d %s\n",i,name);

        QString s1 = QString::fromUtf8(name);
        QString s2 = QString::fromUtf8(conf);

        QListWidgetItem *item=new QListWidgetItem(NULL,activeList,ACTIVE_FILTER_BASE+i);
        item->setData(FilterItemDelegate::FilterNameRole, s1);
        item->setData(FilterItemDelegate::DescriptionRole, s2);
        item->setData(FilterItemDelegate::DisabledRole, !enabled);
        printf("Active item :%p\n",item);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        activeList->addItem(item);
    }
}

/**
    \fn activeListContextMenu
*/
void filtermainWindow::activeListContextMenu(const QPoint &pos)
{
    if(!nb_active_filter)
        return;
    if(!activeList->currentItem())
        return;

    QMenu *cm=new QMenu(this);

    QAction *up = new QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Move up")),cm);
    QAction *down = new QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Move down")),cm);
    QAction *configure = new QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Configure")),cm);
    QAction *duplicate = new QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Duplicate")),cm);
    QAction *remove = new QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Remove")),cm);
    QAction *partial = new QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Make partial")),cm);
    QAction *enabled = new QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Enable/Disable")),cm);

    up->setShortcut(shortcutMoveUp);
    down->setShortcut(shortcutMoveDown);
    configure->setShortcut(shortcutConfigure);
    duplicate->setShortcut(shortcutDuplicate);
    remove->setShortcut(shortcutRemove);
    partial->setShortcut(shortcutTogglePartial);
    enabled->setShortcut(shortcutToggleEnabled);

#if defined(__APPLE__) && QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    up->setShortcutVisibleInContextMenu(true);
    down->setShortcutVisibleInContextMenu(true);
    configure->setShortcutVisibleInContextMenu(true);
    duplicate->setShortcutVisibleInContextMenu(true);
    remove->setShortcutVisibleInContextMenu(true);
    partial->setShortcutVisibleInContextMenu(true);
    enabled->setShortcutVisibleInContextMenu(true);
#endif

    cm->addAction(up);
    cm->addAction(down);
    cm->addAction(configure);
    cm->addAction(duplicate);
    cm->addAction(remove);
    cm->addAction(partial);
    cm->addAction(enabled);

    connect(up,SIGNAL(triggered()),this,SLOT(moveUp()));
    connect(down,SIGNAL(triggered()),this,SLOT(moveDown()));
    connect(configure,SIGNAL(triggered()),this,SLOT(configureAction()));
    connect(duplicate,SIGNAL(triggered()),this,SLOT(duplicateAction()));
    connect(remove,SIGNAL(triggered()),this,SLOT(removeAction()));
    connect(partial,SIGNAL(triggered()),this,SLOT(togglePartial()));
    connect(enabled,SIGNAL(triggered()),this,SLOT(toggleEnabled()));

    updateContextMenu(cm);
    cm->exec(activeList->viewport()->mapToGlobal(pos));
    delete cm;
    cm = NULL;
}

/**
    \fn updateContextMenu
    \brief Disable not applicable entries in the active filters context menu
*/
void filtermainWindow::updateContextMenu(QMenu *contextMenu)
{
    if(!nb_active_filter)
        return;
    if(!contextMenu->actions().size())
        return;
    bool canMoveUp=true;
    bool canMoveDown=true;
    bool canPartialize=true;
    bool partialized=false;
    QListWidgetItem *item=activeList->currentItem();
    if(!item)
        return;

    int itag=item->type();
    ADM_assert(itag>=ACTIVE_FILTER_BASE);
    itag -= ACTIVE_FILTER_BASE;
    uint32_t tag=ADM_vf_getTag(itag);
    canPartialize=ADM_vf_canBePartialized(tag);
    partialized=(tag==VF_PARTIAL_FILTER);

    int row=item->listWidget()->row(item);
    if(!row)
        canMoveUp=false;
    if(row==nb_active_filter-1)
        canMoveDown=false;

    const char *textEnable = ADM_vf_getEnabled(itag) ?
        QT_TRANSLATE_NOOP("qmainfilter","Disable") :
        QT_TRANSLATE_NOOP("qmainfilter","Enable");
    
    const char *textPartial = partialized ? 
        QT_TRANSLATE_NOOP("qmainfilter","Make global") :
        QT_TRANSLATE_NOOP("qmainfilter","Make partial");

    for(int i = 0; i < contextMenu->actions().size(); i++)
    {
        QAction *a = contextMenu->actions().at(i);

        if(a->shortcut() == shortcutTogglePartial)
            a->setText(QString::fromUtf8(textPartial));

#define MATCHME(x,y) if(a->shortcut() == shortcut##x) { a->setEnabled(y); continue; }
        MATCHME(MoveUp,canMoveUp)
        MATCHME(MoveDown,canMoveDown)
        MATCHME(TogglePartial,canPartialize||partialized)

        if(a->shortcut() == shortcutToggleEnabled)
            a->setText(QString::fromUtf8(textEnable));
    }
}

/**
    \brief ctor
*/
filtermainWindow::filtermainWindow(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    setupFilters();

    availableList=ui.listWidgetAvailable;
    activeList=ui.listWidgetActive;
    
    printf("active : %p\n",activeList);
    
    activeList->setSelectionMode(QAbstractItemView::SingleSelection);
    activeList->setDragEnabled(true);
    activeList->setDragDropMode(QAbstractItemView::InternalMove);
    activeList->setDropIndicatorShown(true);
    connect(activeList->model(),SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),this,SLOT(rowsMovedSlot()));

    availableList->setItemDelegate(new FilterItemDelegate(availableList));
    activeList->setItemDelegate(new FilterItemDelegate(activeList));

    availableList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    activeList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(ui.listFilterCategory,SIGNAL(itemDoubleClicked(QListWidgetItem *)),
                this,SLOT(filterFamilyClick(QListWidgetItem *)));
    connect(ui.listFilterCategory,SIGNAL(itemClicked(QListWidgetItem *)),
                this,SLOT(filterFamilyClick(QListWidgetItem *)));
    connect(ui.listFilterCategory,SIGNAL(currentRowChanged(int)),
                this,SLOT(filterFamilyClick(int)));

    connect(activeList,SIGNAL(itemDoubleClicked(QListWidgetItem *)),this,SLOT(activeDoubleClick(QListWidgetItem *)));
    connect(availableList,SIGNAL(itemDoubleClicked(QListWidgetItem *)),this,SLOT(allDoubleClick(QListWidgetItem *)));

    connect(ui.buttonClose, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(ui.pushButtonPreview, SIGNAL(clicked(bool)), this, SLOT(preview(bool)));

    ADM_vf_updateBridge(video_body);
    displayFamily(0);
    buildActiveFilterList();
    setSelected(nb_active_filter - 1);

    previewDialog = NULL;
    previewDialogX = INT_MIN;
    previewDialogY = INT_MIN;
    //____________________
    //  Context Menu
    //____________________
    QAction *add = new  QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Add")),this);
    availableList->setContextMenuPolicy(Qt::ActionsContextMenu);
    availableList->addAction(add );
    connect(add,SIGNAL(triggered(bool )),this,SLOT(addSlot()));

    QAction *rem = new QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Remove")),this);
    int keycode;
#ifdef __APPLE__
    keycode = Qt::Key_Backspace;
#else
    keycode = Qt::Key_Delete;
#endif
    shortcutRemove = QKeySequence(keycode);
    bool alt = false;
    prefs->get(KEYBOARD_SHORTCUTS_USE_ALTERNATE_KBD_SHORTCUTS,&alt);
    if(alt)
    {
        std::string sc;
        prefs->get(KEYBOARD_SHORTCUTS_ALT_DELETE,sc);
        if(sc.size())
        {
            QString qs = QString::fromUtf8(sc.c_str());
            shortcutRemove = QKeySequence::fromString(qs);
        }
    }
    rem->setShortcut(shortcutRemove);
    addAction(rem);
    connect(rem,SIGNAL(triggered(bool)),this,SLOT(remove(bool)));

    // TODO make configurable
    shortcutMoveUp = QKeySequence(Qt::ShiftModifier | Qt::Key_Up);
    shortcutMoveDown = QKeySequence(Qt::ShiftModifier | Qt::Key_Down);
    shortcutConfigure = QKeySequence(Qt::Key_Return);
    shortcutDuplicate = QKeySequence(Qt::ControlModifier | Qt::Key_D);
    shortcutTogglePartial = QKeySequence(Qt::ShiftModifier | Qt::Key_P);
    shortcutToggleEnabled = QKeySequence(Qt::ShiftModifier | Qt::Key_D);

    QAction *movup = new QAction(this);
    movup->setShortcut(shortcutMoveUp);
    addAction(movup);
    connect(movup,SIGNAL(triggered()),this,SLOT(moveUp()));

    QAction *movdw = new QAction(this);
    movdw->setShortcut(shortcutMoveDown);
    addAction(movdw);
    connect(movdw,SIGNAL(triggered()),this,SLOT(moveDown()));

    QAction *dupl = new QAction(this);
    dupl->setShortcut(shortcutDuplicate);
    addAction(dupl);
    connect(dupl,SIGNAL(triggered()),this,SLOT(duplicate()));

    QAction *mkpartl = new QAction(this);
    mkpartl->setShortcut(shortcutTogglePartial);
    addAction(mkpartl);
    connect(mkpartl,SIGNAL(triggered()),this,SLOT(togglePartial()));

    QAction *tglenbl = new QAction(this);
    tglenbl->setShortcut(shortcutToggleEnabled);
    addAction(tglenbl);
    connect(tglenbl,SIGNAL(triggered()),this,SLOT(toggleEnabled()));


    activeList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(activeList,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(activeListContextMenu(const QPoint &)));

    this->installEventFilter(this);
    ui.pushButtonPreview->installEventFilter(this);
    originalTime = admPreview::getCurrentPts();

#if 1
#define HINT_LENGTH 256
    char hint[HINT_LENGTH];
    hint[0] = '\0';
    QKeySequence acc(Qt::ControlModifier | Qt::Key_Return);
    snprintf(hint,
        HINT_LENGTH,
        QT_TRANSLATE_NOOP("qmainfilter","Press %s to accept the dialog"),
        acc.toString(QKeySequence::NativeText).toUtf8().constData());
    hint[HINT_LENGTH-1] = '\0';
    ui.labelAcceptHint->setText(QString::fromUtf8(hint));
#undef HINT_LENGTH
#else
    ui.labelAcceptHint->setVisible(false);
#endif
}

/**
    \brief dtor
*/
filtermainWindow::~filtermainWindow()
{
    if(previewDialog) 
        delete previewDialog;
    previewDialog=NULL;
    admPreview::seekToTime(originalTime);
}

/**
    \fn eventFilter
    \brief keyboard accessibility
*/
bool filtermainWindow::eventFilter(QObject* watched, QEvent* event)
{
    QKeyEvent *keyEvent;
    if(event->type() == QEvent::KeyPress)
    {
        keyEvent = (QKeyEvent*)event;
        if(keyEvent->key() == Qt::Key_Return)
        {
            if(keyEvent->modifiers() & Qt::ControlModifier)
            {
                accept();
                return true;
            }
            if(watched == ui.pushButtonPreview)
                preview(true);
            else if(ui.listFilterCategory->hasFocus())
                filterFamilyClick(ui.listFilterCategory->currentRow());
            else if(availableList->hasFocus())
                add(true);
            else if(activeList->hasFocus())
                configure(true);
            else
                accept();
            return true;
        }
        if(keyEvent->key() == Qt::Key_Left)
        {
            if(ui.listWidgetAvailable->hasFocus())
            {
                ui.listFilterCategory->setFocus();
                return true;
            }
            if(ui.listWidgetActive->hasFocus())
            {
                ui.listWidgetAvailable->setFocus();
                return true;
            }
        }
        if(keyEvent->key() == Qt::Key_Right)
        {
            if(ui.listFilterCategory->hasFocus())
            {
                ui.listWidgetAvailable->setFocus();
                return true;
            }
            if(ui.listWidgetAvailable->hasFocus())
            {
                ui.listWidgetActive->setFocus();
                return true;
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

/**
    \fn addSlot
    \brief context menu item of an available filter
*/
void filtermainWindow::addSlot(void)
{
    add(true);
}

/**
    \fn rowsMovedSlot
    \brief actually move filters upon reordering active list items using drag-n-drop
*/
void filtermainWindow::rowsMovedSlot(void)
{
    int moved = activeList->currentRow();
    int itag = activeList->item(moved)->type();
    ADM_assert(itag>=ACTIVE_FILTER_BASE);
    itag -= ACTIVE_FILTER_BASE;
    if(moved==itag) // this can't happen, can it?
        return;
    if(moved<itag)
    {
        for(int i=0; i<itag-moved; i++)
        {
            ADM_vf_moveFilterUp(itag-i);
        }
    }else
    {
        for(int i=0; i<moved-itag; i++)
        {
            ADM_vf_moveFilterDown(itag+i);
        }
    }
    buildActiveFilterList();
    setSelected(moved);
}

/*******************************************************/

int GUI_handleVFilter(void);
/**
      \fn     GUI_handleVFilter(void)
      \brief  Show the main filter window allowing user to add/remove/configure video filters
*/
int GUI_handleVFilter(void)
{
    filtermainWindow dialog(qtLastRegisteredDialog());
    qtRegisterDialog(&dialog);
    dialog.exec();
    qtUnregisterDialog(&dialog);
    return 0;
}
//EOF
#if 0

/**
 * \fn loadFilters
 * \brief load a filter from .py
 * @param b
 */
extern void call_scriptEngine(const char *scriptFile);
void filtermainWindow::loadFilters( bool b)
{
    printf("Load filters\n");

    char name[1024];

    if(!FileSel_SelectRead(QT_TRANSLATE_NOOP("qmainfilter","Load video filters.."),name,1023,NULL))
        return;
    call_scriptEngine(name);
    buildActiveFilterList();
}
/**
 * \fn saveFilters
 * \brief save a filter from .py
 * @param b
 */
void filtermainWindow::saveFilters( bool b)
{
    printf("save filters\n");
    char name[1024];

    if(!FileSel_SelectWrite(QT_TRANSLATE_NOOP("qmainfilter","Save video filters.."),name,1023,NULL))
        return;
    printf("save filters, part 2\n");
    IScriptEngine *engine=getPythonScriptEngine();
    IScriptWriter *writer = engine->createScriptWriter();

    ADM_ScriptGenerator generator(video_body, writer);
    std::stringstream stream(std::stringstream::in | std::stringstream::out);
    std::string fileName = std::string(name);

    generator.init(stream);
    generator.saveVideoFilters();
    generator.end();
    delete writer;

    if (fileName.rfind(".") == std::string::npos)
    {
            fileName += "." + engine->defaultFileExtension();
    }

    FILE *file = ADM_fopen(fileName.c_str(), "wt");
    string script = stream.str();

    ADM_fwrite(script.c_str(), script.length(), 1, file);
    ADM_fclose(file);
}

/**
 * // Stubb
 * @param b
 */
void filtermainWindow::partial(bool b)
{
    
}
#endif





/**
        \fn     add( bool b)
        \brief  Retrieve the selected filter and add it to the active filters
*/
void filterquickWindow::add( bool b)
{
  /* Now that we have the tab, get the selection */
   QListWidgetItem *item=availableList->currentItem();
   VF_FILTERS tag;
   if(item)
   {
     int itag=item->type();

     if(itag<ALL_FILTER_BASE || itag >= ALL_FILTER_BASE+(VF_MAX*100))
     {
            ADM_assert(0);
     }
     // Extract family & index
     itag-=ALL_FILTER_BASE;
     int index=itag%100;
     int family=(itag-index)/100;
     ADM_assert(family<VF_MAX);
     ADM_assert(index<ADM_vf_getNbFiltersInCategory((VF_CATEGORY)family));
     tag=index; //filterCategories[family][index]->tag;
     ADM_info("Tag : %d->family=%d, index=%d\n",itag,family,tag);

     if(ADM_vf_addFilterFromTag(video_body, itag,NULL,true) != NULL)
        {
         ADM_assert(ADM_vf_canBePartialized(itag));
         ADM_vf_partialize(nb_active_filter-1, true);   // skip dialog
            accept();
        }else
        {
            ADM_warning("Cannot add filter from tag\n");
            ui.lineEditSearch->clear();     // clear search
        }
   }
}

/**
 * \fn displayFamily
 * @param family
 */
void filterquickWindow::displayPartialFilters(const QString &search)
{
    int itemCount=0;
    availableList->clear();
    for (uint32_t family=0; family<VF_MAX; family++)
    {
        if (family==VF_HIDDEN)
            continue;

        uint32_t nb=ADM_vf_getNbFiltersInCategory((VF_CATEGORY)family);
        ADM_info("Video filter Family :%u, nb %d\n",family,nb);
        for (uint32_t i = 0; i < nb; i++)
        {
            const char *name,*desc;
            uint32_t major,minor,patch;
            ADM_vf_getFilterInfo((VF_CATEGORY)family,i,&name, &desc,&major,&minor,&patch);
            uint32_t tag = i+family*100;
            if(!ADM_vf_canBePartialized(tag))
                continue;

            QString s1 = QString::fromUtf8(name);
            QString s2 = QString::fromUtf8(desc);
            QString s3 = ADM_vf_getInternalNameFromTag(tag);

            bool show = search.isEmpty() ||
                s1.contains(search, Qt::CaseInsensitive) ||
                s2.contains(search, Qt::CaseInsensitive) ||
                s3.contains(search, Qt::CaseInsensitive);

            if (!show) continue;

            QListWidgetItem *item = new QListWidgetItem(NULL,availableList,ALL_FILTER_BASE+i+family*100);
            item->setData(Qt::DisplayRole, s1); // for sorting
            item->setData(FilterItemDelegate::FilterNameRole, s1);
            item->setData(FilterItemDelegate::DescriptionRole, s2);
            item->setData(FilterItemDelegate::InternalNameRole, s3);
            availableList->addItem(item);
            itemCount++;
        }
    }

    if (!itemCount)
        return;

    availableList->sortItems();
    availableList->setCurrentRow(0);
    if (search.isEmpty())
        return;

    for (int i=0; i<availableList->count(); i++)
    {
        QListWidgetItem * item = availableList->item(i);
        if (!item) continue;
        if (item->data(FilterItemDelegate::FilterNameRole).toString().startsWith(search, Qt::CaseInsensitive))
        {
            availableList->setCurrentRow(i);
            return;
        }
    }
    for (int i=0; i<availableList->count(); i++)
    {
        QListWidgetItem * item = availableList->item(i);
        if (!item) continue;
        if (item->data(FilterItemDelegate::FilterNameRole).toString().contains(search, Qt::CaseInsensitive))
        {
            availableList->setCurrentRow(i);
            return;
        }
    }
    for (int i=0; i<availableList->count(); i++)
    {
        QListWidgetItem * item = availableList->item(i);
        if (!item) continue;
        if (item->data(FilterItemDelegate::InternalNameRole).toString().contains(search, Qt::CaseInsensitive))
        {
            availableList->setCurrentRow(i);
            return;
        }
    }
}

/**
        \fn     filterquickWindow::activeDoubleClick( QListWidgetItem  *item)
        \brief  One of the active window has been double clicked, call configure
*/
void filterquickWindow::allDoubleClick( QListWidgetItem  *item)
{
    add(0);
}

/**
        \fn setup
        \brief Prepare
*/
void filterquickWindow::setupFilters(void)
{

}


/**
    \brief ctor
*/
filterquickWindow::filterquickWindow(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    setupFilters();

    availableList=ui.listWidgetAvailable;
    
    

    availableList->setItemDelegate(new FilterItemDelegate(availableList, true));

    availableList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(availableList,SIGNAL(itemDoubleClicked(QListWidgetItem *)),this,SLOT(allDoubleClick(QListWidgetItem *)));

    connect(ui.buttonClose, SIGNAL(clicked(bool)), this, SLOT(accept()));

    ADM_vf_updateBridge(video_body);
    displayPartialFilters(QString(""));

    //____________________
    //  Context Menu
    //____________________
    QAction *add = new  QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Add")),this);
    availableList->setContextMenuPolicy(Qt::ActionsContextMenu);
    availableList->addAction(add );
    connect(add,SIGNAL(triggered(bool )),this,SLOT(addSlot()));


    this->installEventFilter(this);
    originalTime = admPreview::getCurrentPts();
    
    ui.lineEditSearch->installEventFilter(this);
    connect(ui.lineEditSearch, SIGNAL(textChanged(const QString &)), this, SLOT(searchChange(const QString &)));
    ui.lineEditSearch->setFocus();

}

/**
    \brief dtor
*/
filterquickWindow::~filterquickWindow()
{
    admPreview::seekToTime(originalTime);
}

/**
    \fn eventFilter
    \brief keyboard accessibility
*/
bool filterquickWindow::eventFilter(QObject* watched, QEvent* event)
{
    QKeyEvent *keyEvent;
    if(event->type() == QEvent::KeyPress)
    {
        keyEvent = (QKeyEvent*)event;
        if(keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            if(availableList->hasFocus() || ui.lineEditSearch->hasFocus())
                add(true);
            return true;
        }
        if(keyEvent->key() == Qt::Key_Escape)
        {
            reject();
            return true;
        }
        if(keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down)
        {
            bool up = keyEvent->key() == Qt::Key_Up;
            if(ui.lineEditSearch->hasFocus())
            {
                if (availableList->count() > 0)
                {
                    int row = availableList->currentRow();
                    row += (up ? -1:1);
                    if ((row >=0) && (row < availableList->count()))
                        availableList->setCurrentRow(row);
                }
                return true;
            }
        }
        if(keyEvent->key() == Qt::Key_PageUp || keyEvent->key() == Qt::Key_PageDown)
        {
            bool up = keyEvent->key() == Qt::Key_PageUp;
            if(ui.lineEditSearch->hasFocus())
            {
                if (availableList->count() > 0)
                {
                    int row = availableList->currentRow();
                    row += (up ? -6:6);
                    if (row < 0)
                        row = 0;
                    if (row >= availableList->count())
                        row = availableList->count()-1;
                    availableList->setCurrentRow(row);
                }
                return true;
            }
        }
        if((keyEvent->key() == Qt::Key_Home) && (ui.lineEditSearch->hasFocus()))
        {
            if (availableList->count() > 0)
            {
                availableList->setCurrentRow(0);
            }
            return true;
        }
        if((keyEvent->key() == Qt::Key_End) && (ui.lineEditSearch->hasFocus()))
        {
            if (availableList->count() > 0)
            {
                availableList->setCurrentRow(availableList->count()-1);
            }
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}

/**
    \fn addSlot
    \brief context menu item of an available filter
*/
void filterquickWindow::addSlot(void)
{
    add(true);
}

/**
    \fn searchChange
*/
void filterquickWindow::searchChange(const QString &newValue)
{
    displayPartialFilters(newValue);
}


/*******************************************************/

int GUI_handleVPartialFilter(void);
/**
      \fn     GUI_handleVPartialFilter(void)
*/
int GUI_handleVPartialFilter(void)
{
    // check markers
    uint64_t totalDuration = video_body->getVideoDuration();
    uint64_t markerA = video_body->getMarkerAPts();
    uint64_t markerB = video_body->getMarkerBPts();
    if (markerB < markerA)
    {
        uint64_t tmp = markerA;
        markerA = markerB;
        markerB = tmp;
    }
    
    if (totalDuration == 0) // should be not possible
        return 0;
    if (markerA==0 && markerB >= totalDuration)
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("qmainfilter","Can not add partial filter"), QT_TRANSLATE_NOOP("qmainfilter","A selection by markers has to be made."));
        return 0;
    }
    
    filterquickWindow dialog(qtLastRegisteredDialog());
    qtRegisterDialog(&dialog);
    dialog.exec();
    qtUnregisterDialog(&dialog);
    return 0;
}

