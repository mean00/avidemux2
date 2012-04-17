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
#include "Q_audioTrackClass.h"
#include "ADM_audioFilterInterface.h"
#include "DIA_fileSel.h"
extern const char *getStrFromAudioCodec( uint32_t codec);
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
        for(int i=_srcActive->size() ;i<NB_MENU;i++)
        {
            EditableAudioTrack *copy=new EditableAudioTrack();
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
        {
            QObject::connect( window->filters[i],SIGNAL(clicked(bool)),
                            this,SLOT(filtersClicked(bool)));
            QObject::connect( window->codecConf[i],SIGNAL(clicked(bool)),
                            this,SLOT(codecConfClicked(bool)));
            QObject::connect( window->enabled[i],SIGNAL(stateChanged(int)),
                            this,SLOT(enabledStateChanged(int)));

        }
        QObject::connect(window->ui.pushButtonAddTrack,SIGNAL(clicked(bool)),
                            this,SLOT(addTrack(bool)));
        // 
        window->show();
                                    
};

/**
     \fn addTrack
*/
bool  audioTrackQt4::addTrack(bool state)
{
        ADM_info("Adding external audio track\n");
        // start fileselector
        #define MAX_SOURCE_LENGTH 1024
        char fileName[MAX_SOURCE_LENGTH];
        if(FileSel_SelectRead("Select audio file",fileName,MAX_SOURCE_LENGTH-1,NULL))
        {
            ADM_edAudioTrackExternal *ext=create_edAudioExternal(fileName);
            if(!ext)
            {
                GUI_Error_HIG("Error","Cannot use that file as audio track");
                return false;
            }else
            {
                _pool->addInternalTrack(ext);
                for(int i=0;i<NB_MENU;i++)
                    setupMenu(i);

            }
        }
        //
        return true;

}
/**
     \fn enabledStateChanged
*/
bool  audioTrackQt4::enabledStateChanged(int state)
{
        int dex=-1;
        QObject *ptr=sender();

        for(int i=0;i<NB_MENU;i++) if(ptr==window->enabled[i]) dex=i;
        if(dex==-1)
        {
            ADM_warning("No track found matching that enabling\n");
            return true;
        }
        if(Qt::Checked==state)
        {
            enable(dex);
        }
        else        
        {
            disable(dex);
        }
        return true;

}
/**
    \fn codecConfClicked
*/
bool  audioTrackQt4::codecConfClicked(bool a)
{
 QObject *ptr=sender();
        int dex=-1;
        for(int i=0;i<NB_MENU;i++) if(ptr==window->codecConf[i]) dex=i;
        if(dex==-1)
        {
            ADM_warning("No track found matching that codec\n");
            return true;
        }
        printf("codec at %d\n",dex);
        // get index
        int codecIndex=window->codec[dex]->currentIndex();
        if(codecIndex<=0) return true; // no configure

        EditableAudioTrack *ed=active.atEditable(dex);
        if(!ed)
        {
            ADM_warning("No track found at index %d\n",dex);
            return true;
        }
        audioCodecConfigureCodecIndex( codecIndex,&(ed->encoderConf));
        return true;
}
/**
    \fn filtersClicked
*/
bool       audioTrackQt4::filtersClicked(bool a)
{
        QObject *ptr=sender();
        int dex=-1;
        for(int i=0;i<NB_MENU;i++) if(ptr==window->filters[i]) dex=i;
        if(dex==-1)
        {
            ADM_warning("No track found matching that filter\n");
            return true;
        }
        printf("Editable at %d\n",dex);
        EditableAudioTrack *ed=active.atEditable(dex);
        ADM_assert(ed);
        ed->audioEncodingConfig.audioFilterConfigure();
        return true;
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
    // 1 - check for duplicates 
    int map[NB_MENU];
    memset(map,0,sizeof(map));
    for(int i=0;i<NB_MENU;i++)
    {
        if(window->enabled[i]->checkState()==Qt::Checked)
        {
            int trackIndex=window->inputs[i]->currentIndex();
            if(map[trackIndex])
            {
                GUI_Error_HIG("Error","Some tracks are used multiple times");
                return false;
            }
            map[trackIndex]++;
        }
    }

    // 2 - recreate audio tracks from menu
    _srcActive->clear();
    int done=0;
    for(int i=0;i<NB_MENU;i++)
    {
        if(window->enabled[i]->checkState()==Qt::Checked)
        {
            ADM_info("Processing input %d for track %d\n",i,done);
            int trackIndex=window->inputs[i]->currentIndex();
            _srcActive->addTrack(trackIndex,_pool->at(trackIndex));
            //
            EditableAudioTrack *dest=_srcActive->atEditable(done);
            // set codec
            dest->encoderIndex=window->codec[i]->currentIndex();
            // conf
            EditableAudioTrack *src=active.atEditable(i);
            if(src)
            {
                dest->encoderConf=CONFcouple::duplicate(src->encoderConf);
                // filters
                dest->audioEncodingConfig=src->audioEncodingConfig;
            }
            // next
            done++;
        }
    }
    _srcActive->dump();
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
    ADM_edAudioTrack *edTrack;
    
    // 
    window->inputs[dex]->clear();


    for(int i=0;i<_pool->size();i++)
    {
        QString str;
        //
        edTrack=_pool->at(i);
        switch(edTrack->getTrackType())
        {
                case ADM_EDAUDIO_FROM_VIDEO:
                                {
                                QString num;
                                ADM_edAudioTrackFromVideo *fromVideo=edTrack->castToTrackFromVideo() ;
                                num.setNum(fromVideo->getMyTrackIndex());
                                str=QString("Track ")+num+QString(" from video");
                                }
                                break;
                case ADM_EDAUDIO_EXTERNAL:
                                {
                                ADM_edAudioTrackExternal *ext=edTrack->castToExternal() ;
                                ADM_assert(ext);
                                std::string name=ext->getMyName();
                                str=QString("File ")+QString(name.c_str());
                                }
                                break;
                default:
                                ADM_assert(0);
                                break;
        }
        // Get info about that track
        WAVHeader *hdr=_pool->at(i)->getInfo();
        if(hdr)
        {
            int bitrate=hdr->byterate;
            QString sBitrate;
            QString sChan;
            switch(hdr->channels)
            {
                case 1: sChan=QString("Mono");break;
                case 2: sChan=QString("Stereo");break;
                default: sChan.setNum(hdr->channels);sChan+=QString(" chan");break;
            }
            bitrate*=8;
            bitrate/=1000;
            sBitrate.setNum(bitrate);
            str+=QString(" (")+QString(getStrFromAudioCodec(hdr->encoding))+QString(",");
            str+=sChan+QString(",");
            str+=sBitrate+QString("kbps)");
        }
        window->inputs[dex]->addItem(str); 
    }
    // set index if possible
    EditableAudioTrack *ed;
    ed=active.atEditable(dex);

    // set current track if it exists
    if(ed)
    {
        if(ed->edTrack)
        {
            int poolIndex=ed->poolIndex;
            window->inputs[dex]->setCurrentIndex(poolIndex);             
        }
     }
    // now add codecs
    int nbAud=audioEncoderGetNumberOfEncoders();
    window->codec[dex]->addItem(QString("copy"));
	for(uint32_t i=1;i<nbAud;i++)
	{
		QString name=QString(audioEncoderGetDisplayName(i));
		window->codec[dex]->addItem(name);
	}
    if(active.atEditable(dex)->edTrack)
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
