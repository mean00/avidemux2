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
FilterItemDelegate::FilterItemDelegate(QWidget *parent) : QItemDelegate(parent)
{
    zprintf("Delegate : %p\n",this);
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
        if(option.state & QStyle::State_HasFocus)
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
    if (previewDialog)
    {
            delete previewDialog;
            previewDialog=NULL;
    }
    previewDialog = new Ui_seekablePreviewWindow(this, filter, 0);
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
        qtUnregisterDialog(previewDialog);
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
    ADM_vf_moveFilterUp(itag);
    buildActiveFilterList ();
    setSelected(itag-1);
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
        const char *name= ADM_vf_getDisplayNameFromTag(instanceTag);
        const char *conf=instance->getConfiguration();
        printf("%d %s\n",i,name);

        QString s1 = QString::fromUtf8(name);
        QString s2 = QString::fromUtf8(conf);

        QListWidgetItem *item=new QListWidgetItem(NULL,activeList,ACTIVE_FILTER_BASE+i);
        item->setData(FilterItemDelegate::FilterNameRole, s1);
        item->setData(FilterItemDelegate::DescriptionRole, s2);
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
    QMenu *cm=new QMenu();

    QAction *up = new QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Move up")),cm);
    QAction *down = new QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Move down")),cm);
    QAction *configure = new QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Configure")),cm);
    QAction *remove = new QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Remove")),cm);
    QAction *partial = new QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Make partial")),cm);

    cm->addAction(up);
    cm->addAction(down);
    cm->addAction(configure);
    cm->addAction(remove);
    cm->addAction(partial);

    connect(up,SIGNAL(triggered()),this,SLOT(moveUp()));
    connect(down,SIGNAL(triggered()),this,SLOT(moveDown()));
    connect(configure,SIGNAL(triggered()),this,SLOT(configureAction()));
    connect(remove,SIGNAL(triggered()),this,SLOT(removeAction()));
    connect(partial,SIGNAL(triggered()),this,SLOT(makePartial()));

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
    QListWidgetItem *item=activeList->currentItem();
    if(!item)
        return;

    int itag=item->type();
    ADM_assert(itag>=ACTIVE_FILTER_BASE);
    itag -= ACTIVE_FILTER_BASE;
    uint32_t tag=ADM_vf_getTag(itag);
    canPartialize=ADM_vf_canBePartialized(tag);

    int row=item->listWidget()->row(item);
    if(!row)
        canMoveUp=false;
    if(row==nb_active_filter-1)
        canMoveDown=false;

    contextMenu->actions().at(0)->setEnabled(canMoveUp);
    contextMenu->actions().at(1)->setEnabled(canMoveDown);
    contextMenu->actions().at(4)->setEnabled(canPartialize);
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

    connect(activeList,SIGNAL(itemDoubleClicked(QListWidgetItem *)),this,SLOT(activeDoubleClick(QListWidgetItem *)));
    connect(availableList,SIGNAL(itemDoubleClicked(QListWidgetItem *)),this,SLOT(allDoubleClick(QListWidgetItem *)));

    connect(ui.buttonClose, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(ui.pushButtonPreview, SIGNAL(clicked(bool)), this, SLOT(preview(bool)));

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
    QKeySequence seq(keycode);
    bool alt = false;
    prefs->get(KEYBOARD_SHORTCUTS_USE_ALTERNATE_KBD_SHORTCUTS,&alt);
    if(alt)
    {
        std::string sc;
        prefs->get(KEYBOARD_SHORTCUTS_ALT_DELETE,sc);
        if(sc.size())
        {
            QString qs = QString::fromUtf8(sc.c_str());
            seq = QKeySequence::fromString(qs);
        }
    }
    rem->setShortcut(seq);
    addAction(rem);
    connect(rem,SIGNAL(triggered(bool)),this,SLOT(remove(bool)));

    activeList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(activeList,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(activeListContextMenu(const QPoint &)));

    this->installEventFilter(this);
    originalTime = admPreview::getCurrentPts();

#if 1
#define HINT_LENGTH 256
    char hint[HINT_LENGTH];
    hint[0] = '\0';
    QKeySequence acc(Qt::ControlModifier + Qt::Key_Return);
    snprintf(hint,HINT_LENGTH,QT_TRANSLATE_NOOP("qmainfilter","Press %s to accept the dialog"),acc.toString().toUtf8().constData());
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
            if(ui.listFilterCategory->hasFocus())
                filterFamilyClick(ui.listFilterCategory->currentRow());
            else if(availableList->hasFocus())
                add(true);
            else if(activeList->hasFocus())
                configure(true);
            else
                accept();
            return true;
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

