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
#include "ADM_prettyPrint.h"

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif
namespace ADM_Qt4CoreUIToolkit
{
/**
 * \fn ctor
 * @param title
 * @param fps1000
 * @param duration
 */
DIA_processingQt4::DIA_processingQt4(const char *title,uint64_t totalToProcess) : DIA_processingBase(title,totalToProcess)
{
        _totalToProcess=totalToProcess;

        if(!_totalToProcess) _totalToProcess=1000000;
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
        _first=true;
        _slotIndex=0;
        setWindowModality(Qt::ApplicationModal);        
        
        connect( ui->cancelButton,SIGNAL(clicked(bool)),this,SLOT(stop(bool)));
        ui->labelTimeLeft->setText(QString("Unknown"));
        ui->progressBar->setValue((int)0);        
        show();
}

/**
 * \fn update
 * @param percent
 * @return true if processing should be stopped
 */
bool DIA_processingQt4::update(uint32_t frame,uint64_t currentProcess)
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
        _clock.reset();
        _nextUpdate=REFRESH_RATE_IN_MS;
        if(_first)
        {
            _first=false;
            for(int i=0;i<PROC_NB_SLOTS;i++)
                _slots[i]=_currentFrames;
        }
        _slots[_slotIndex]=_currentFrames;
        _slotIndex%=PROC_NB_SLOTS;
        
        // compute time left
        double percent=((double)(currentProcess))/((double)_totalToProcess);
        
        
        double dElapsed=_totalTime.getElapsedMS(); // in dElapsed time, we have made percent percent
        
        double totalTimeNeeded=dElapsed/percent;
        double remaining=totalTimeNeeded-dElapsed;
        if(remaining<0) remaining=1;
        
        std::string r;
        ADM_durationToString(remaining, r);
        QString qRemaining=QString(r.c_str());
        
        percent=100.*percent;
        aprintf("Percent=%d,cur=%d,tot=%d\n",(int)percent,_lastFrames,_totalFrame);
        aprintf("time %llx/ total time %llx\n",currentProcess,_totalToProcess);
        if(percent>100.) percent=99.9;
        if(percent<0.1) percent=0.1;
        _lastFrames+=_currentFrames;
        _currentFrames=0;
        
        int avg=0;
        for(int i=0;i<PROC_NB_SLOTS;i++)
            avg+=_slots[i];
        avg/=PROC_NB_SLOTS;
        
        if(ui)
        {
                        ui->labelImages->setText( QString::number(_lastFrames));
                        ui->labelTimeLeft->setText(qRemaining);
                        ui->progressBar->setValue((int)percent);
                        ui->labelSpeed->setText(QString::number(avg)+QString(" fps"));
        }
        return false;
}

/**
 * \fn dtor
 */
DIA_processingQt4::~DIA_processingQt4()
{
     qtUnregisterDialog(this);
}


/**
    \fn createWorking
*/
DIA_processingBase *createProcessing(const char *title,uint64_t totalToProcess)
{
    return new DIA_processingQt4(title,totalToProcess);
}   
/**
 * \fn stop
 * @param a
 */
void            DIA_processingQt4::stop(bool a)
{
    ADM_info("Stop Request\n");
     if (GUI_Confirmation_HIG(QT_TR_NOOP("_Resume"), 
                                 QT_TR_NOOP("The processing is paused."), 
                                 QT_TR_NOOP("Cancel it ?")))
        {
                _stopRequest=true;
        }

}

}// Namespace
//********************************************
//EOF
