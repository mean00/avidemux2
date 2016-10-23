/***************************************************************************
  DIA_dialogFactory.cpp
  (C) 2007 Mean Fixounet@free.fr 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QSpacerItem>
#include <QTabWidget>

#include "T_dialogFactory.h"
#include "ADM_default.h"
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
#include "DIA_coreUI_internal.h"
#include "ADM_toolkitQt.h"
#include "ADM_dialogFactoryQt4.h"

static void insertTab(uint32_t index, diaElemTabs *tab, QTabWidget *wtab);
/**
    \fn qt4DiaFactoryRun(const char *title,uint32_t nb,diaElem **elems)
    \brief  Run a dialog made of nb elems, each elem being described in the **elems
    @return 0 on failure, 1 on success
*/



/**
    \fn qt4DiaFactoryRun(const char *title,uint32_t nb,diaElem **elems)
    \brief  Run a dialog made of nb elems, each elem being described in the **elems
    @return 0 on failure, 1 on success
*/
class factoryCookie
{
public:
        factoryCookie(const char *title)
        {
            dialog=new QDialog(qtLastRegisteredDialog());
            qtRegisterDialog(dialog);
            dialog->setWindowTitle(QString::fromUtf8(title));
            vboxlayout = new QVBoxLayout();
            
            tabWidget=NULL;
            layout=NULL;
            
        }
        virtual ~factoryCookie()
        {
            if(dialog)
            {
                qtUnregisterDialog(dialog);
                delete dialog;
            }
            dialog=NULL;
        }
public:
        QDialog     *dialog;
        QVBoxLayout *vboxlayout;
        QLayout     *layout;
        QTabWidget  *tabWidget;        
        std::vector <diaElem *>items;
};
/**
 * 
 * @param title
 * @param nb
 * @param elems
 * @return 
 */
void * qt4DiaFactoryPrepare(const char *title,uint32_t nb,diaElem **elems)
{
  factoryCookie *cookie=new factoryCookie(title);

  ADM_assert(title);
  ADM_assert(nb);
  ADM_assert(elems);
  
    int currentLayout = 0;
    int  v=0;
    for(int i=0;i<nb;i++)
    {
       ADM_assert(elems[i]);
       if (elems[i]->getRequiredLayout() != currentLayout)
       {
           if (cookie->layout)
               cookie->vboxlayout->addLayout(cookie->layout);

           switch (elems[i]->getRequiredLayout())
           {
               case FAC_QT_GRIDLAYOUT:
                   cookie->layout = new QGridLayout();
                   break;
               case FAC_QT_VBOXLAYOUT:
                   cookie->layout = new QVBoxLayout();
                   break;
           }

           currentLayout = elems[i]->getRequiredLayout();
           v = 0;
       }

       elems[i]->setMe( (void *)(cookie->dialog),cookie->layout,v);
       v+=elems[i]->getSize();
    }

    for(int i=0;i<nb;i++)
    {
        ADM_assert(elems[i]);
        elems[i]->finalize(); 
        cookie->items.push_back(elems[i]);
    }
    return cookie;
}
/**
 * 
 * @param finish
 * @return 
 */
bool  qt4DiaFactoryFinish(void *finish)
{
  bool  r=false;
  factoryCookie *cookie=(factoryCookie *)  finish;
  QSpacerItem *spacer = new QSpacerItem(20, 16, QSizePolicy::Minimum, QSizePolicy::Fixed);
  QDialogButtonBox *buttonBox = new QDialogButtonBox();
  
  // Add buttons
   buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

   QObject::connect(buttonBox, SIGNAL(accepted()), cookie->dialog, SLOT(accept()));
   QObject::connect(buttonBox, SIGNAL(rejected()), cookie->dialog, SLOT(reject()));

   if (cookie->layout)
           cookie->vboxlayout->addLayout(cookie->layout);

   cookie->vboxlayout->addItem(spacer);
   cookie->vboxlayout->addWidget(buttonBox);

   cookie->dialog->setLayout(cookie->vboxlayout);

  if(cookie->dialog->exec()==QDialog::Accepted)
  {
     int nb=cookie->items.size();
     for(int i=0;i<nb;i++)
     {
        ADM_assert(cookie->items[i]);
        cookie->items[i]->getMe();     
      }
     r=true;
  }
  delete cookie;
  return r;
}

/**
    \fn shortkey(const char *in)
    \brief translate gtk like accelerator (_) to QT4 like accelerator (&)
*/
const char *shortkey(const char *in)
{
        QString escaped = QString::fromUtf8(in);

        escaped.replace("&", "&&");
        escaped.replace("_", "&");

        return ADM_strdup(escaped.toUtf8().constData());
}


/**
 *         \fn qt4DiaFactoryRunTabs
 */
void  *qt4DiaFactoryTabsPrepare(const char *title,uint32_t nb,diaElemTabs **tabs)
{
    factoryCookie *cookie=new factoryCookie(title);
  
    ADM_assert(title);
    ADM_assert(nb);
    ADM_assert(tabs);
  
    cookie->layout  = new QGridLayout();
    cookie->tabWidget = new QTabWidget();
    

    for(int i=0;i<nb;i++)
    {
        ADM_assert(tabs[i]);
        insertTab(i,tabs[i],cookie->tabWidget);
         for(int j=0;j<tabs[i]->nbElems;j++)
             cookie->items.push_back(tabs[i]->dias[j]);
    }        
    return cookie;
}
 /**
  * 
  * @param f
  * @return 
  */ 
bool qt4DiaFactoryTabsFinish(void *f)
{
    bool r=false;
    factoryCookie *cookie=(factoryCookie *)f;
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(); 
    QObject::connect(buttonBox, SIGNAL(accepted()), cookie->dialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), cookie->dialog, SLOT(reject()));    
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QSpacerItem *spacer = new QSpacerItem(20, 16, QSizePolicy::Minimum, QSizePolicy::Fixed);
    cookie->vboxlayout->addLayout(cookie->layout);
    cookie->vboxlayout->addWidget(cookie->tabWidget,0,0);
    cookie->vboxlayout->addItem(spacer);
    cookie->vboxlayout->addWidget(buttonBox,1,0);

     cookie->dialog->setLayout(cookie->vboxlayout);

     // Expand to see all tabs but still allow the window to be resized smaller
     cookie->tabWidget->setUsesScrollButtons(false);
     cookie->dialog->adjustSize();
     cookie->tabWidget->setUsesScrollButtons(true);

    if(cookie->dialog->exec()==QDialog::Accepted)
    {
        // Read tabs
        int n=cookie->items.size();
        for(int i=0;i<n;i++)
            cookie->items[i]->getMe();
        r=true;
    }    
    delete cookie;
    return r;
}
void insertTab(uint32_t index, diaElemTabs *tab, QTabWidget *wtab)
{

  QWidget *wid=new QWidget;  
  QSpacerItem *spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
  int currentLayout = 0;
  QVBoxLayout *vboxLayout = new QVBoxLayout(wid);
  QLayout *layout = NULL;
  
  /* First compute the size of our window */
  int vsize=0;
  for(int i=0;i<tab->nbElems;i++)
  {
    ADM_assert(tab->dias[i]);
     vsize+=tab->dias[i]->getSize(); 
  }

    int  v=0;

    for(int i=0;i<tab->nbElems;i++)
    {
        ADM_assert(tab->dias[i]);

        if (tab->dias[i]->getRequiredLayout() != currentLayout)
        {
            if (layout)
                vboxLayout->addLayout(layout);

            switch (tab->dias[i]->getRequiredLayout())
            {
                case FAC_QT_GRIDLAYOUT:
                    layout = new QGridLayout();
                    break;
                case FAC_QT_VBOXLAYOUT:
                    layout = new QVBoxLayout();
                    break;
            }

            currentLayout = tab->dias[i]->getRequiredLayout();
            v = 0;
        }

        tab->dias[i]->setMe( wid,layout,v); 
        v+=tab->dias[i]->getSize();
    }
  
  wtab->addTab(wid,QString::fromUtf8(tab->title));
  for(int i=0;i<tab->nbElems;i++)
  {
    tab->dias[i]->finalize(); 
  }

  if (layout)
        vboxLayout->addLayout(layout);

  vboxLayout->addItem(spacerItem);
}
/**
 * 
 * @param title
 * @param nb
 * @param elems
 * @return 
 */
uint8_t qt4DiaFactoryRun(const char *title,uint32_t nb,diaElem **elems)
{
    void *cookie=qt4DiaFactoryPrepare(title,nb,elems);
    return qt4DiaFactoryFinish(cookie);
}

/**
 *         \fn qt4DiaFactoryRunTabs
 */
uint8_t qt4DiaFactoryRunTabs(const char *title,uint32_t nb,diaElemTabs **tabs)
{
    void *cookie=qt4DiaFactoryTabsPrepare(title,nb,tabs);
    return qt4DiaFactoryTabsFinish(cookie);
}
/**
 * 
 */
/**
 *  \fn gtkFactoryGetVersion
 *         \brief returns the version this has been compiled with
 */
void      qt4FactoryGetVersion(uint32_t *maj,uint32_t *minor,uint32_t *patch)
{
        *maj=ADM_COREUI_MAJOR;
        *minor=ADM_COREUI_MINOR;
        *patch=ADM_COREUI_PATCH;
        
}
extern CREATE_BITRATE_T     qt4CreateBitrate;
extern DELETE_DIA_ELEM_T    qt4DestroyBitrate;
extern CREATE_BAR_T         qt4CreateBar;
extern DELETE_DIA_ELEM_T    qt4DestroyBar;
extern CREATE_FLOAT_T       qt4CreateFloat;
extern DELETE_DIA_ELEM_T    qt4DestroyFloat;
extern CREATE_FRAME_T       qt4CreateFrame;
extern DELETE_DIA_ELEM_T    qt4DestroyFrame;
extern CREATE_HEX_T         qt4CreateHex;
extern DELETE_DIA_ELEM_T    qt4DestroyHex;
extern CREATE_INTEGER_T     qt4CreateInteger;
extern CREATE_UINTEGER_T    qt4CreateUInteger;
extern DELETE_DIA_ELEM_T    qt4DestroyInteger;
extern DELETE_DIA_ELEM_T    qt4DestroyUInteger;
extern CREATE_MATRIX_T      qt4CreateMatrix;
extern DELETE_DIA_ELEM_T    qt4DestroyMatrix;
extern CREATE_NOTCH_T       qt4CreateNotch;
extern DELETE_DIA_ELEM_T    qt4DestroyNotch;
extern CREATE_READONLYTEXT_T qt4CreateRoText;
extern CREATE_TEXT_T       qt4CreateText;
extern DELETE_DIA_ELEM_T   qt4DestroyRoText;
extern DELETE_DIA_ELEM_T   qt4DestroyText;
extern CREATE_BUTTON_T     qt4CreateButton;
extern DELETE_DIA_ELEM_T   qt4DestroyButton;
extern CREATE_FILE_T       qt4CreateFile;
extern DELETE_DIA_ELEM_T   qt4DestroyFile;
extern CREATE_DIR_T        qt4CreateDir;
extern DELETE_DIA_ELEM_T   qt4DestroyDir;
extern CREATE_MENUDYNAMIC_T     qt4CreateMenuDynamic;
extern CREATE_MENU_T       qt4CreateMenu;
extern DELETE_DIA_ELEM_T   qt4DestroyMenu;
extern DELETE_DIA_ELEM_T   qt4DestroyMenuDynamic;
extern CREATE_USLIDER_T    qt4CreateUSlider;
extern DELETE_DIA_ELEM_T   qt4DestroyUSlider;
extern         CREATE_SLIDER_T     qt4CreateSlider;
extern         DELETE_DIA_ELEM_T   qt4DestroySlider;
extern CREATE_THREADCOUNT_T qt4CreateThreadCount;
extern DELETE_DIA_ELEM_T    qt4DestroyThreadCount;
extern CREATE_TOGGLE_UINT   qt4CreateToggleUint;
extern DELETE_DIA_ELEM_T    qt4DestroyToggleUint;
extern CREATE_TOGGLE_INT    qt4CreateToggleInt;
extern DELETE_DIA_ELEM_T    qt4DestroyToggleInt;
extern CREATE_TOGGLE_T      qt4CreateToggle;
extern DELETE_DIA_ELEM_T    qt4DestroyToggle;
extern CREATE_TIMESTAMP_T   qt4CreateTimeStamp;
extern DELETE_DIA_ELEM_T    qt4DestroyTimeStamp;

//************
static FactoryDescriptor Qt4FactoryDescriptor=
{
        &qt4FactoryGetVersion,
        &qt4DiaFactoryRun,
        &qt4DiaFactoryRunTabs,        
        &qt4DiaFactoryTabsPrepare,
        &qt4DiaFactoryTabsFinish,
        &qt4DiaFactoryPrepare,
        &qt4DiaFactoryFinish,

        // Buttons
        &qt4CreateButton,
        &qt4DestroyButton,
        // Bar
        &qt4CreateBar,
        &qt4DestroyBar,
        // Float
        &qt4CreateFloat,
        &qt4DestroyFloat,
        // Integer
        &qt4CreateInteger,
        &qt4DestroyInteger,
        // UInteger
        &qt4CreateUInteger,
        &qt4DestroyUInteger,
        // Notch
        &qt4CreateNotch,
        &qt4DestroyNotch,
        // RoText
        &qt4CreateRoText,
        &qt4DestroyRoText,
        // Text
        &qt4CreateText,
        &qt4DestroyText,
        // Hex
        &qt4CreateHex,
        &qt4DestroyHex,
        // Matrix
        &qt4CreateMatrix,
        &qt4DestroyMatrix,
        // Menu
        &qt4CreateMenu,
        &qt4DestroyMenu,
        &qt4CreateMenuDynamic,
        &qt4DestroyMenuDynamic,
        // ThreadCount
        &qt4CreateThreadCount,
        &qt4DestroyThreadCount,
        // Bitrate
        &qt4CreateBitrate,
        &qt4DestroyBitrate,
        // File
        &qt4CreateFile,
        &qt4DestroyFile,
        // Dir
        &qt4CreateDir,
        &qt4DestroyDir,
        // Frame
        &qt4CreateFrame,
        &qt4DestroyFrame,
    // Toggle uint/int
        &qt4CreateToggleUint,
        &qt4DestroyToggleUint,
        &qt4CreateToggleInt,
        &qt4DestroyToggleInt,
        // Regular toggle
        &qt4CreateToggle,
        &qt4DestroyToggle,
        // Slider
        &qt4CreateUSlider,
        &qt4DestroyUSlider,
        &qt4CreateSlider,
        &qt4DestroySlider,
        // Timestamp
        &qt4CreateTimeStamp,
        &qt4DestroyTimeStamp
};

/**
 *         \fn InitFactory
 *  \brief Install our factory hooks
 */
void InitFactory(void)
{
        DIA_factoryInit(&Qt4FactoryDescriptor);
        
        
}

//EOF
