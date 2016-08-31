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

#include "prefs.h"
#include "ADM_edScriptGenerator.h"
#include "DIA_fileSel.h"
#include "ADM_script2/include/ADM_script.h"
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
//#define NO_DELEGATE

extern ADM_Composer *video_body;

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
    QAbstractItemView *view = qobject_cast<QAbstractItemView*>(parent());
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
 * 
 * @param painter
 * @param option
 * @param index
 */
void FilterItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QAbstractItemView *view = qobject_cast<QAbstractItemView*>(parent());
    QLabel *label;

    if (view->indexWidget(index) == 0)
    {
            label = new QLabel();
            label->installEventFilter(filter);
            label->setAutoFillBackground(true);
            label->setFocusPolicy(Qt::TabFocus);
            label->setText(index.data().toString());
            view->setIndexWidget(index, label);
    }

    label = (QLabel*)view->indexWidget(index);

    if (option.state & QStyle::State_Selected)
            if (option.state & QStyle::State_HasFocus)
                    label->setBackgroundRole(QPalette::Highlight);
            else
                    label->setBackgroundRole(QPalette::Window);
    else
            label->setBackgroundRole(QPalette::Base);
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
            previewDialog->resetVideoStream(filter);
    else
    {
        previewDialog = new Ui_seekablePreviewWindow(this, filter, 0);
        connect(previewDialog, SIGNAL(accepted()), this, SLOT(closePreview()));
    }
    previewDialog->show();
}
/**
 * \fn closePreview
 */
void filtermainWindow::closePreview()
{
    if (previewDialog)
    {
        delete previewDialog;
        previewDialog = NULL;
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
    \fn    Add
    \brief Right click on an available filer
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
    \fn    Add
    \brief Right click on an available filer
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
  QSize sz;
  availableList->clear();
  for (uint32_t i = 0; i < nb; i++)
    {
        const char *name,*desc;
        uint32_t major,minor,patch;
          ADM_vf_getFilterInfo((VF_CATEGORY)family,i,&name, &desc,&major,&minor,&patch);
          QString str = QString("<b>") + name + QString("</b><br>\n<small>") + desc + QString("</small>");

          QListWidgetItem *item;
          item=new QListWidgetItem(str,availableList,ALL_FILTER_BASE+i+family*100);
          item->setToolTip(desc);
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
        QString str = QString("<b>") + name + QString("</b><br>\n<small>") + conf + QString("</small>");
        QListWidgetItem *item=new QListWidgetItem(str,activeList,ACTIVE_FILTER_BASE+i);
        printf("Active item :%p\n",item);
        activeList->addItem(item);
    }

}
  /**
  */
filtermainWindow::filtermainWindow(QWidget* parent) : QDialog(parent)
 {

    ui.setupUi(this);
    setupFilters();

    availableList=ui.listWidgetAvailable;
    activeList=ui.listWidgetActive;
    
    printf("active : %p\n",activeList);
    
#if 1 //def NO_DELEGATE    
    activeList->setSelectionMode(QAbstractItemView::SingleSelection);
    activeList->setDragEnabled(true);
    activeList->setDragDropMode(QAbstractItemView::InternalMove);
    activeList->setDropIndicatorShown(true);
    activeList->viewport()->setAcceptDrops(true);
    connect(activeList,SIGNAL(indexesMoved(const QModelIndexList &)),this,SLOT(indexesMoved(const QModelIndexList &)));
#endif    
    activeList->setItemDelegate(new FilterItemDelegate(activeList));
    
    connect(ui.listFilterCategory,SIGNAL(itemDoubleClicked(QListWidgetItem *)),
                this,SLOT(filterFamilyClick(QListWidgetItem *)));
    connect(ui.listFilterCategory,SIGNAL(itemClicked(QListWidgetItem *)),
                this,SLOT(filterFamilyClick(QListWidgetItem *)));

    connect(activeList,SIGNAL(itemDoubleClicked(QListWidgetItem *)),this,SLOT(activeDoubleClick(QListWidgetItem *)));
    connect(availableList,SIGNAL(itemDoubleClicked(QListWidgetItem *)),this,SLOT(allDoubleClick(QListWidgetItem *)));

    connect(ui.buttonClose, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(ui.pushButtonPreview, SIGNAL(clicked(bool)), this, SLOT(preview(bool)));


    availableList->setItemDelegate(new FilterItemDelegate(availableList));

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
    
    
    QAction *remove = new  QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Remove")),this);
    QAction *configure = new  QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Configure")),this);
    QAction *up = new  QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Move up")),this);
    QAction *down = new  QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Move down")),this);
    QAction *partial = new  QAction(QString(QT_TRANSLATE_NOOP("qmainfilter","Make partial")),this);
    
    activeList->setContextMenuPolicy(Qt::ActionsContextMenu);
    activeList->addAction(up);
    activeList->addAction(down);
    activeList->addAction(configure);
    activeList->addAction(remove);
    activeList->addAction(partial);
    
    connect(remove,SIGNAL(triggered()),this,SLOT(removeAction()));
    connect(configure,SIGNAL(triggered()),this,SLOT(configureAction()));
    connect(up,SIGNAL(triggered()),this,SLOT(moveUp()));
    connect(down,SIGNAL(triggered()),this,SLOT(moveDown()));
    connect(partial,SIGNAL(triggered()),this,SLOT(makePartial()));

 }
/**
    \fn dtor
*/
filtermainWindow::~filtermainWindow()
{
    if(previewDialog) 
        delete previewDialog;
    previewDialog=NULL;

}

/**
    \fn    Add
    \brief Right click on an available filer
*/
void filtermainWindow::addSlot(void)
{
    add(true);
}
/**
 * 
 * @return 
 */
void filtermainWindow::indexesMoved(const QModelIndexList & indexes)
{
    int n=indexes.size();
    printf(" -- Moved with %d inputs\n",n);
    for(int i=0;i<n;i++)
    {
        //indexes[i].
        
    }
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

