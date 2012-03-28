/***************************************************************************
    copyright            : (C) 2001 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_inttype.h"
#include "Q_audioTracks.h"

#include "DIA_coreToolkit.h"
#include "ADM_vidMisc.h"
#include "ADM_toolkitQt.h"
#include "ADM_edit.hxx"
#include "DIA_audioTracks.h"
extern void UI_purge(void);
uint32_t audioEncoderGetNumberOfEncoders(void);
const char  *audioEncoderGetDisplayName(int i);
/**
    \fn audioTrackQt4
*/
class audioTrackQt4: public DIA_audioTrackBase,QObject
{
protected:
            audioTrackWindow *window;
            ActiveAudioTracks active;
            void            setupMenu(int dex);
            void            enable(int i);
            void            disable(int i);
public:
       
                            audioTrackQt4( PoolOfAudioTracks *pool, ActiveAudioTracks *xactive );
            virtual		~audioTrackQt4();
                          
                       bool  updateActive(void);
            virtual   bool  run(void);
public slots:
            void      filtersClicked(bool a);
};
/**
    \fn audioTrackQt4
    \brief ctor
*/  
audioTrackQt4::audioTrackQt4( PoolOfAudioTracks *pool, ActiveAudioTracks *xactive ) : 
        DIA_audioTrackBase( pool, xactive )
{
        // Duplicate _srcActive to active
        for(int i=0;i<_srcActive->size() && i<NB_MENU;i++)
        {
            EditableAudioTrack *copy=
                    new EditableAudioTrack(*(_srcActive->atEditable(i)));
            active.addTrack(copy);
        }
        // create windows
        window=new audioTrackWindow();
        qtRegisterDialog(window);
        window->setModal(TRUE);
        
        for(int i=0;i<NB_MENU;i++)
            setupMenu(i);

        // bind it
        for(int i=0;i<NB_MENU;i++)
            QObject::connect( window->filters[i],SIGNAL(clicked(bool)),
                            this,SLOT(filtersClicked(bool)));

        // 
        window->show();
                                    
};
/**
    \fn filtersClicked
*/
void      audioTrackQt4::filtersClicked(bool a)
{
        QObject *ptr=sender();
        int dex=-1;
        for(int i=0;i<NB_MENU;i++) if(ptr=window->filters[i]) dex=i;
        if(dex==-1)
        {
            ADM_warning("No track found matching that filers\n");
            return;
        }
        EditableAudioTrack *ed=active.atEditable(dex);
        ADM_assert(ed);
        ed->audioEncodingConfig.audioFilterConfigure();
}
/**
    \fn audioTrackQt4
    \brief dtor
*/
audioTrackQt4::		~audioTrackQt4()
{
  
        if(window) delete window;
        window=NULL;
  
}
/**
    \fn run
    \brief run the dialog 
*/
bool      audioTrackQt4::run(void)
{
    ADM_info("Running QT4 audioTrack GUI\n");
    bool r=false;
    again:
    if(window->exec()==QDialog::Accepted)
    {
        r=true;
        if(false==updateActive()) 
        {
            goto again;
        }
    }
    qtUnregisterDialog(window);
    ADM_info("/Running QT4 audioTrack GUI\n");
    return r;
}
              
/**
    \fn updateActive
*/
bool  audioTrackQt4::updateActive(void)
{
    return true;
}

/**
    \fn enable
*/
void audioTrackQt4::enable(int i)
{
#define ONOFF(x)  window->x[i]->setEnabled(true)
    window->enabled[i]->setCheckState(Qt::Checked);
    ONOFF(inputs);
    ONOFF(codec);
    ONOFF(codecConf);
    ONOFF(filters);
}
/**
    \fn disable
*/
#undef ONOFF
#define ONOFF(x)  window->x[i]->setEnabled(false)

void audioTrackQt4::disable(int i)
{
    window->enabled[i]->setCheckState(Qt::Unchecked);
    ONOFF(inputs);
    ONOFF(codec);
    ONOFF(codecConf);
    ONOFF(filters);

}

/**
    \fn setupMenu
*/
void audioTrackQt4::setupMenu(int dex)
{
    for(int i=0;i<_pool->size();i++)
    {
        QString num;
        num.setNum(i);
        QString str=QString("Track ")+num;
        window->inputs[dex]->addItem(str);
            
    }
    // now add codecs
    int nbAud=audioEncoderGetNumberOfEncoders();
    window->codec[dex]->addItem(QString("copy"));
	for(uint32_t i=1;i<nbAud;i++)
	{
		QString name=QString(audioEncoderGetDisplayName(i));
		window->codec[dex]->addItem(name);
	}
    if(dex<active.size())
    {
        int selected=active.atEditable(dex)->encoderIndex;
        enable(dex);
        window->codec[dex]->setCurrentIndex(selected);
    }
    else    
        disable(dex);

}
/**
        \fn createEncoding
*/
namespace ADM_Qt4CoreUIToolkit
{
DIA_audioTrackBase *createAudioTrack( PoolOfAudioTracks *pool, ActiveAudioTracks *active )
{
        return new audioTrackQt4(pool,active);
}
}
//********************************************
//EOF
