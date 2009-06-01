/***************************************************************************
    copyright            : (C) 2007 by mean
    email                : fixounet@free.fr
    
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

#include "config.h"
#include <vector>

#include "Q_mainfilter.h"
#include "ADM_default.h"
#include "DIA_fileSel.h"

# include "prefs.h"
//#include "ADM_audiodevice/audio_out.h"

#include "ADM_userInterfaces/ADM_render/GUI_render.h"

#include "DIA_coreToolkit.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"
#include "ADM_video/ADM_vidPartial.h"
#include "ADM_filter/vidVCD.h"
#include "DIA_factory.h"
/*******************************************************/
#define NB_TREE 8
#define myFg 0xFF
#define myBg 0xF0
static int max=0;
/******************************************************/
#define ALL_FILTER_BASE       1000
#define EXTERNAL_FILTER_BASE  2000
#define ACTIVE_FILTER_BASE    3000
/******************************************************/
extern FILTER videofilters[VF_MAX_FILTER];
extern uint32_t nb_active_filter;
extern std::vector <FilterDescriptor *> allfilters;
extern std::vector <FilterDescriptor *> filterCategories[VF_MAX];


extern const char  *filterGetNameFromTag(VF_FILTERS tag);
extern ADM_Composer *video_body;
extern AVDMGenericVideoStream *filterCreateFromTag(VF_FILTERS tag,CONFcouple *conf, AVDMGenericVideoStream *in) ;
extern uint32_t curframe;
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

void filtermainWindow::preview(bool b)
{
	QListWidgetItem *item = activeList->currentItem();

	if (!item)
	{
		printf("No selection\n");
		return;
	}

	int itag = item->type();
	ADM_assert(itag > ACTIVE_FILTER_BASE);
	itag -= ACTIVE_FILTER_BASE;

	DIA_filterPreview(QT_TR_NOOP("Preview"), videofilters[itag].filter, curframe);
}

/**
        \fn     void setSelected(int sel)
        \brief  Set the sel line as selected in the active filter window
*/
void filtermainWindow::setSelected( int sel)
{
  if(!sel) return;
  activeList->setCurrentRow(sel-1);
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
     if(itag>=ALL_FILTER_BASE && itag < EXTERNAL_FILTER_BASE)
     {
    	 // Extract family & index
    	 itag-=ALL_FILTER_BASE;
    	 int index=itag%100;
    	 int family=(itag-index)/100;
    	 ADM_assert(family<VF_MAX);
    	 ADM_assert(index<filterCategories[family].size());
         tag=filterCategories[family][index]->tag;
     }else
     {
        ADM_assert(0); 
     }
      printf("Tag : %d\n",itag);
      
      // Create our filter...
      
       CONFcouple *coup;
        videofilters[nb_active_filter].filter = filterCreateFromTag (tag, NULL, videofilters[nb_active_filter - 1].filter);
        if(!videofilters[nb_active_filter].filter->    configure (videofilters[nb_active_filter - 1].filter))
        {
            delete videofilters[nb_active_filter].filter;
            return;
        }
        videofilters[nb_active_filter].filter->getCoupledConf (&coup);
        videofilters[nb_active_filter].tag = tag;
        videofilters[nb_active_filter].conf = coup;
        nb_active_filter++;
		buildActiveFilterList();
        setSelected(nb_active_filter - 1);
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
      printf("No selection\n");
      return;
   }
    
     int itag=item->type();
     ADM_assert(itag>ACTIVE_FILTER_BASE);
     itag-=ACTIVE_FILTER_BASE;
     /* Filter 0 is the decoder ...*/
      printf("Rank : %d\n",itag); 
     ADM_assert(itag);
     /**/
      if (videofilters[itag].conf)
            {
                    delete videofilters[itag].conf;
                    videofilters[itag].conf = NULL;
            }
        // recreate derivated filters

        for (uint32_t i = itag ; i < nb_active_filter-1; i++)
        {
                    delete videofilters[i ].filter;
                    videofilters[i ].filter = filterCreateFromTag(videofilters[i+1].tag,
                                                    videofilters[i+1].conf,
                                                    videofilters[i - 1].filter);
                    videofilters[i ].conf = videofilters[i+1].conf;
                    videofilters[i ].tag = videofilters[i+1].tag;
        }
        
        /* Delete last filter which is now at last filter -1 */
            delete videofilters[nb_active_filter - 1].filter;
            videofilters[nb_active_filter - 1].filter = NULL;
            nb_active_filter--;
            buildActiveFilterList ();
            if(nb_active_filter>1)
            {
              if(itag<nb_active_filter-1)
                  setSelected(itag);
              else
                  setSelected(nb_active_filter-1);
            }
  
}
#define MAKE_BUTTON(button,call) \
void filtermainWindow::button( bool b) \
{ \
    call(); \
    getFirstVideoFilter (); \
    buildActiveFilterList ();  \
	setSelected(nb_active_filter - 1); \
}
//MAKE_BUTTON(DVD,setDVD)
//MAKE_BUTTON(VCD,setVCD)
//MAKE_BUTTON(SVCD,setSVCD)
//MAKE_BUTTON(halfD1,setHalfD1)
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
      printf("No selection\n");
      return;
   }
    
     int itag=item->type();
     ADM_assert(itag>ACTIVE_FILTER_BASE);
     itag-=ACTIVE_FILTER_BASE;
     /* Filter 0 is the decoder ...*/
      printf("Rank : %d\n",itag); 
     ADM_assert(itag);
     /**/
     
        if(!videofilters[itag].filter->configure (videofilters[itag - 1].filter)) return;
        /* Recreate chain if needed , config has changed */
        CONFcouple *couple;
        videofilters[itag].filter->getCoupledConf (&couple);
        videofilters[itag].conf = couple;
        getFirstVideoFilter ();
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
      printf("No selection\n");
      return;
   }
    
     int itag=item->type();
     ADM_assert(itag>ACTIVE_FILTER_BASE);
     itag-=ACTIVE_FILTER_BASE;
     /* Filter 0 is the decoder ...*/
      printf("Rank : %d\n",itag); 
     ADM_assert(itag);
     
     if (itag < 2) return;
        // swap action parameter & action parameter -1
        FILTER tmp;
        memcpy (&tmp, &videofilters[itag - 1], sizeof (FILTER));
        memcpy (&videofilters[itag - 1],
            &videofilters[itag], sizeof (FILTER));
        memcpy (&videofilters[itag], &tmp, sizeof (FILTER));
        getFirstVideoFilter ();
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
      printf("No selection\n");
      return;
   }
    
     int itag=item->type();
     ADM_assert(itag>ACTIVE_FILTER_BASE);
     itag-=ACTIVE_FILTER_BASE;
     /* Filter 0 is the decoder ...*/
      printf("Rank : %d\n",itag); 
     ADM_assert(itag);
     
    if (((int) itag < (int) (nb_active_filter - 1)) && (itag))
        {
            // swap action parameter & action parameter -1
            FILTER tmp;
            memcpy (&tmp, &videofilters[itag + 1], sizeof (FILTER));
            memcpy (&videofilters[itag + 1],
                        &videofilters[itag], sizeof (FILTER));
            memcpy (&videofilters[itag], &tmp, sizeof (FILTER));
            getFirstVideoFilter ();
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
  printf("Family :%u\n",family);
  ADM_assert(family<VF_MAX);
  std::vector <FilterDescriptor *> vec=filterCategories[family];
  availableList->clear();
  QSize sz;
  
  for (uint32_t i = 0; i < vec.size(); i++)
    {
          QString str = QString("<b>") + vec[i]->name + QString("</b><br>\n<small>") + vec[i]->description + QString("</small>");
         
          QListWidgetItem *item;
          item=new QListWidgetItem(str,availableList,ALL_FILTER_BASE+i+family*100);
          item->setToolTip(vec[i]->description);
          availableList->addItem(item);
     }

  if (vec.size())
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
  printf("partial\n"); 
   QListWidgetItem *item=activeList->currentItem();
   if(!item)
   {
      printf("No selection\n");
      return;
   }
    
     int itag=item->type();
     ADM_assert(itag>ACTIVE_FILTER_BASE);
     itag-=ACTIVE_FILTER_BASE;
     /* Filter 0 is the decoder ...*/
      printf("Rank : %d\n",itag); 
      ADM_assert(itag);
     
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

	for (uint32_t i = 1; i < nb_active_filter; i++)
	{
		const char *name = filterGetNameFromTag(videofilters[i].tag);
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
    connect(ui.pushButtonDVD, SIGNAL(clicked(bool)), this, SLOT(DVD(bool)));
    connect(ui.pushButtonVCD, SIGNAL(clicked(bool)), this, SLOT(VCD(bool)));
    connect(ui.pushButtonSVCD, SIGNAL(clicked(bool)), this, SLOT(SVCD(bool)));
    connect(ui.pushButtonHalfDVD, SIGNAL(clicked(bool)), this, SLOT(halfD1(bool)));
	connect(ui.pushButtonPreview, SIGNAL(clicked(bool)), this, SLOT(preview(bool)));

	activeList->setItemDelegate(new FilterItemDelegate(activeList));
	availableList->setItemDelegate(new FilterItemDelegate(availableList));

    displayFamily(0);
    buildActiveFilterList();
	setSelected(nb_active_filter - 1);
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
  void **params=(void **)cookie;
  AVDMGenericVideoStream *son=(AVDMGenericVideoStream *)params[0];
  AVDMGenericVideoStream *previous=(AVDMGenericVideoStream *)params[1];
  son->configure(previous);
}
/** 
    \fn DIA_getPartial
    \brief Partial dialog
    
*/

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
//EOF

