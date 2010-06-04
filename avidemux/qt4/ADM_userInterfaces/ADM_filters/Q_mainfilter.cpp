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
#include <vector>
#include "Q_mainfilter.h"
#include "ADM_default.h"
#include "DIA_fileSel.h"
#include "DIA_factory.h"
#include "ADM_render/GUI_render.h"

#include "DIA_coreToolkit.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_coreVideoFilter.h"
#include "ADM_filterCategory.h"
#include "ADM_videoFilterApi.h"
#include "ADM_videoFilters.h"

#include "prefs.h"


/*******************************************************/
#define NB_TREE 8
#define myFg 0xFF
#define myBg 0xF0
static int max=0;
/******************************************************/
#define ALL_FILTER_BASE       1000
#define ACTIVE_FILTER_BASE    3000
/******************************************************/
#define nb_active_filter ADM_vf_getSize()

/*******************************************************/

FilterItemEventFilter::FilterItemEventFilter(QWidget *parent) : QObject(parent) {}

bool FilterItemEventFilter::eventFilter(QObject *object, QEvent *event) 
{
	QAbstractItemView *view = qobject_cast<QAbstractItemView*>(parent());

	if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseButtonPress)
	{
		QCoreApplication::sendEvent(view, event);

		return true;
	}
	else if (event->type() == QEvent::FocusIn)
	{
		view->setFocus();

		return true;
	}
	else
		return QObject::eventFilter(object, event);
}

FilterItemDelegate::FilterItemDelegate(QWidget *parent) : QItemDelegate(parent)
{
	filter = new FilterItemEventFilter(parent);
}

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
#if 0
		if (previewDialogX != INT_MIN)
			previewDialog->move(previewDialogX, previewDialogY);
#endif
	}
	previewDialog->show();
}

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

     if(true==ADM_vf_addFilterFromTag(itag,NULL,true))
        {
            buildActiveFilterList();
            setSelected(nb_active_filter-1);
        }
     
   }

}
/**
        \fn     remove( bool b)
        \brief  Remove selected filters from the active window list
*/
void filtermainWindow::remove( bool b)
{
   /* Get selection if any */
  /* Now that we have the tab, get the selection */
   QListWidgetItem *item=activeList->currentItem();
   if(!item)
   {
      ADM_warning("No selection\n");
      return;
   }
     
     int itag=item->type();
     ADM_assert(itag>=ACTIVE_FILTER_BASE);
     itag-=ACTIVE_FILTER_BASE;
     ADM_info("Deleting item %d\n",itag);
     ADM_vf_removeFilterAtIndex(itag);
     buildActiveFilterList ();
     if(nb_active_filter)
     {
          setSelected(nb_active_filter-1);
     }
}
#if 1
#define MAKE_BUTTON(button,call) \
void filtermainWindow::button( bool b)  {}
#else
#define MAKE_BUTTON(button,call) \
void filtermainWindow::button( bool b) \
{ \
    call(); \
    getFirstVideoFilter (); \
    buildActiveFilterList ();  \
	setSelected(nb_active_filter - 1); \
}
#endif
MAKE_BUTTON(DVD,setDVD)
MAKE_BUTTON(VCD,setVCD)
MAKE_BUTTON(SVCD,setSVCD)
MAKE_BUTTON(halfD1,setHalfD1)
/**
        \fn     configure( bool b)
        \brief  Configure the selected active filter
*/
void filtermainWindow::configure( bool b)
{
   /* Get selection if any */
  /* Now that we have the tab, get the selection */
   QListWidgetItem *item=activeList->currentItem();
   if(!item)
   {
      ADM_warning("No selection\n");
      return;
   }
    
     int itag=item->type();
     ADM_assert(itag>=ACTIVE_FILTER_BASE);
     itag-=ACTIVE_FILTER_BASE;
     /* Filter 0 is the decoder ...*/
     ADM_info("Rank : %d\n",itag); 
   
   //   ADM_assert(itag);
     /**/
        ADM_vf_configureFilterAtIndex(itag);
        buildActiveFilterList ();
		setSelected(itag);
}
/**
        \fn     up( bool b)
        \brief  Move selected filter one place up
*/
void filtermainWindow::up( bool b)
{
   QListWidgetItem *item=activeList->currentItem();
   if(!item)
   {
      ADM_warning("No selection\n");
      return;
   }
    
     int itag=item->type();
     ADM_assert(itag>=ACTIVE_FILTER_BASE);
     itag-=ACTIVE_FILTER_BASE;
      ADM_info("Rank : %d\n",itag); 
     
        if (!itag ) return;
        ADM_vf_moveFilterUp(itag);
        buildActiveFilterList ();
        setSelected(itag-1);
}
/**
        \fn     down( bool b)
        \brief  Move selected filter one place down
*/
void filtermainWindow::down( bool b)
{
   QListWidgetItem *item=activeList->currentItem();
   if(!item)
   {
      ADM_warning("No selection\n");
      return;
   }
    
     int itag=item->type();
     ADM_assert(itag>=ACTIVE_FILTER_BASE);
     itag-=ACTIVE_FILTER_BASE;
     /* Filter 0 is the decoder ...*/
      ADM_info("Rank : %d\n",itag); 
     //ADM_assert(itag);
     
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
        \fn     filtermainWindow::partial( bool b)
        \brief  Partialize one filter
*/
void filtermainWindow::partial( bool b)
{
#if 0
  printf("partial\n"); 
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
      //ADM_assert(itag);
     
        AVDMGenericVideoStream *replace;
        CONFcouple *conf;
        conf = videofilters[itag].conf;
        if (videofilters[itag].tag == VF_PARTIAL_FILTER)	// cannot recurse
        {
            GUI_Error_HIG (QT_TR_NOOP("The filter is already partial"), NULL);
            return;
        }

        replace =new ADMVideoPartial (videofilters[itag - 1].
                                      filter,
                                      videofilters[itag].tag,
                                      conf);
        
        if(replace->configure (videofilters[itag - 1].filter))
        {
            delete videofilters[itag].filter;
            if (conf) delete conf;
            videofilters[itag].filter = replace;
            replace->getCoupledConf (&conf);
            videofilters[itag].conf = conf;
            videofilters[itag].tag = VF_PARTIAL_FILTER;
            getFirstVideoFilter ();
            buildActiveFilterList ();
			setSelected(itag);
        }
        else delete replace;
#endif
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
	for (uint32_t i = 0; i < nb; i++)
	{
            uint32_t                instanceTag=ADM_vf_getTag(i);
            ADM_coreVideoFilter     *instance=ADM_vf_getInstance(i);
            const char *name= ADM_vf_getDisplayNameFromTag(instanceTag);
            const char *conf=instance->getConfiguration();
            printf("%d %s\n",i,name);
#if 0            
		const char *name = instance->;
		const char *conf = videofilters[i].filter->printConf ();
		int namelen = strlen (name);

		while (*conf == ' ')
			++conf;

		if (strncasecmp (name, conf, namelen) == 0)
		{
			conf += namelen;
			while (*conf == ' ' || *conf == ':')
				++conf;
		}
#endif
		QString str = QString("<b>") + name + QString("</b><br>\n<small>") + conf + QString("</small>");
		QListWidgetItem *item=new QListWidgetItem(str,activeList,ACTIVE_FILTER_BASE+i);
		activeList->addItem(item);
	}

}
  /**
  */
filtermainWindow::filtermainWindow()     : QDialog()
 {
        
    ui.setupUi(this);
    setupFilters();  
      
    availableList=ui.listWidgetAvailable;
    activeList=ui.listWidgetActive;
    connect(ui.listFilterCategory,SIGNAL(itemDoubleClicked(QListWidgetItem *)),
                this,SLOT(filterFamilyClick(QListWidgetItem *)));
    connect(ui.listFilterCategory,SIGNAL(itemClicked(QListWidgetItem *)),
                this,SLOT(filterFamilyClick(QListWidgetItem *)));

    connect(activeList,SIGNAL(itemDoubleClicked(QListWidgetItem *)),this,SLOT(activeDoubleClick(QListWidgetItem *)));
    connect(availableList,SIGNAL(itemDoubleClicked(QListWidgetItem *)),this,SLOT(allDoubleClick(QListWidgetItem *)));
    
    connect((ui.toolButtonConfigure),SIGNAL(clicked(bool)),this,SLOT(configure(bool)));
    connect((ui.toolButtonAdd),SIGNAL(clicked(bool)),this,SLOT(add(bool)));
    connect((ui.pushButtonRemove),SIGNAL(clicked(bool)),this,SLOT(remove(bool)));
    connect((ui.toolButtonUp),SIGNAL(clicked(bool)),this,SLOT(up(bool)));
    connect((ui.toolButtonDown),SIGNAL(clicked(bool)),this,SLOT(down(bool)));
    connect((ui.toolButtonPartial),SIGNAL(clicked(bool)),this,SLOT(partial(bool)));
    connect(ui.buttonClose, SIGNAL(clicked(bool)), this, SLOT(accept()));
#if 0
    connect(ui.pushButtonDVD, SIGNAL(clicked(bool)), this, SLOT(DVD(bool)));
    connect(ui.pushButtonVCD, SIGNAL(clicked(bool)), this, SLOT(VCD(bool)));
    connect(ui.pushButtonSVCD, SIGNAL(clicked(bool)), this, SLOT(SVCD(bool)));
    connect(ui.pushButtonHalfDVD, SIGNAL(clicked(bool)), this, SLOT(halfD1(bool)));
#endif
	connect(ui.pushButtonPreview, SIGNAL(clicked(bool)), this, SLOT(preview(bool)));

	activeList->setItemDelegate(new FilterItemDelegate(activeList));
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
    QAction *add = new  QAction(QString("Add"),this);
    availableList->setContextMenuPolicy(Qt::ActionsContextMenu);
    availableList->addAction(add );
    connect(add,SIGNAL(activated()),this,SLOT(add()));

	//previewFrameIndex = curframe;
    QAction *remove = new  QAction(QString("Remove"),this);
    QAction *configure = new  QAction(QString("Configure"),this);
    activeList->setContextMenuPolicy(Qt::ActionsContextMenu);
    activeList->addAction(remove);
    activeList->addAction(configure);
    connect(remove,SIGNAL(activated()),this,SLOT(remove()));
    connect(configure,SIGNAL(activated()),this,SLOT(configure()));

 }
/**
    \fn dtor
*/
filtermainWindow::~filtermainWindow()
{
    if(previewDialog) delete previewDialog;
    previewDialog=NULL;

}
/*******************************************************/

int GUI_handleVFilter(void);
static void updateFilterList (filtermainWindow *dialog);

/**
      \fn     GUI_handleVFilter(void)
      \brief  Show the main filter window allowing user to add/remove/configure video filters


*/
int GUI_handleVFilter(void)
{
        filtermainWindow dialog;
        if(QDialog::Accepted==dialog.exec())
        {
        }
	return 0;
}
/** 
    \fn partialCb
    \brief Partial callback to configure the swallowed filter
    
*/
static void partialCb(void *cookie);
void partialCb(void *cookie)
{
#if 0
  void **params=(void **)cookie;
  AVDMGenericVideoStream *son=(AVDMGenericVideoStream *)params[0];
  AVDMGenericVideoStream *previous=(AVDMGenericVideoStream *)params[1];
  son->configure(previous);
#endif
}
/** 
    \fn DIA_getPartial
    \brief Partial dialog
    
*/
#if 0
uint8_t DIA_getPartial(PARTIAL_CONFIG *param,AVDMGenericVideoStream *son,AVDMGenericVideoStream *previous)
{

#define PX(x) &(param->x)
  void *params[2]={son,previous};
         uint32_t fmax=previous->getInfo()->nb_frames;
         if(fmax) fmax--;
         
         diaElemUInteger  start(PX(_start),QT_TR_NOOP("Partial Start Frame:"),0,fmax);
         diaElemUInteger  end(PX(_end),QT_TR_NOOP("Partial End Frame:"),0,fmax);
         diaElemButton    button(QT_TR_NOOP("Configure child"), partialCb,params);
         
         diaElem *tabs[]={&start,&end,&button};
        return diaFactoryRun(QT_TR_NOOP("Partial Video Filter"),3,tabs);

    
}
#endif

/**
    \fn    Add
    \brief Right click on an available filer
*/
void filtermainWindow::add(void)
{
    add(true);
}
/**
    \fn    Add
    \brief Right click on an available filer
*/
void filtermainWindow::remove(void)
{
    remove(true);
}
/**
    \fn    Add
    \brief Right click on an available filer
*/
void filtermainWindow::configure(void)
{
    configure(true);
}

//EOF

