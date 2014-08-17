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
#include <math.h>

#include "Q_processing.h"

#include "ADM_vidMisc.h"

#include "ADM_toolkitQt.h"
#include "DIA_coreToolkit.h"

namespace ADM_Qt4CoreUIToolkit
{
/**
 * \fn ctor
 * @param title
 * @param fps1000
 * @param duration
 */
DIA_processingQt4::DIA_processingQt4(const char *title,uint32_t fps1000,uint64_t duration) : DIA_processingBase(title,fps1000,duration)
{
        _fps1000=fps1000;
        _duration=duration;
        ui=new Ui_DialogProcessing();
        ui->setupUi(this);
	qtRegisterDialog(this);

        setWindowTitle(title);
        postCtor(); 
}
#define REFRESH_RATE_IN_MS 1000
/**
 * \fn postCtor
 */
void DIA_processingQt4 :: postCtor( void )
{
        _priv=NULL;
        _stopRequest=false;
      
        _nextUpdate=_clock.getElapsedMS()+REFRESH_RATE_IN_MS; // every one sec
        _lastFrames=0;
        _currentFrames=0;      
      
        setWindowModality(Qt::ApplicationModal);        
        
        connect( ui->cancelButton,SIGNAL(clicked(bool)),this,SLOT(stop(bool)));
        show();
}

/**
 * \fn update
 * @param percent
 * @return 
 */
bool DIA_processingQt4::update(uint32_t frame)
{
        UI_purge();

        if(_stopRequest) return true;
        if(!frame) return false;
        
        _currentFrames+=frame;
        uint32_t elapsed=_clock.getElapsedMS();
        if(elapsed<_nextUpdate) 
        {
          return false;
        }
        uint32_t delta;
        delta=elapsed-_nextUpdate;
        if(delta>REFRESH_RATE_IN_MS) _nextUpdate=0;
        else _nextUpdate=REFRESH_RATE_IN_MS-delta;

        uint32_t hh,mm,ss,mms;
        char string[25];

        ms2time(elapsed,&hh,&mm,&ss,&mms);
        sprintf(string,"%02d:%02d:%02d",hh,mm,ss);

        
        _lastFrames+=_currentFrames;
        _currentFrames=0;
        if(ui)
        {
            ui->labelImages->setText( QString::number(_lastFrames));
            ui->labelTime->setText(QString(string));
        }
        return false;
}


DIA_processingQt4::~DIA_processingQt4()
{
     qtUnregisterDialog(this);
}


/**
    \fn createWorking
*/
DIA_processingBase *createProcessing(const char *title,uint32_t fps1000,uint64_t duration)
{
    return new DIA_processingQt4(title,fps1000,duration);
}   
/**
 * \fn stop
 * @param a
 */
void            DIA_processingQt4::stop(bool a)
        {
            ADM_info("Stop Request\n");
            _stopRequest=true;
        }
}
//********************************************
//EOF
