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
#include "ADM_default.h"
#include "ADM_vidMisc.h"
#include "DIA_processing.h"
#include "ADM_toolkitQt.h"
#include "DIA_coreToolkit.h"

processingWindow::processingWindow(QWidget *parent) : QDialog(parent)
 {
     ui=new Ui_DialogProcessing();
     ui->setupUi(this);
     active=true;
     setWindowModality(Qt::ApplicationModal);
     connect( ui->cancelButton,SIGNAL(clicked(bool)),this,SLOT(stop(bool)));
 }
 void processingWindow::stop(bool a) 
{
    ADM_info("Stop Request\n");
    active=false;
    delete ui;
}
//*******************************************

namespace ADM_Qt4CoreUIToolkit
{

class DIA_processingQt4 : public DIA_processingBase
{

protected:
        virtual void 	    postCtor( void ) ;
public:
                            DIA_processingQt4( const char *title=NULL );
        virtual		        ~DIA_processingQt4();
            // If returns 1 -> Means aborted
        virtual uint8_t  	update(uint32_t percent);
        virtual uint8_t 	update(uint32_t current,uint32_t total);
        virtual uint8_t  	isAlive (void );
                void            closeDialog(void);
        
};
//********************************************************

DIA_processingQt4::DIA_processingQt4(const char *title) : DIA_processingBase(title)
{
	processingWindow *wind = new processingWindow(qtLastRegisteredDialog());
	qtRegisterDialog(wind);
        _priv=(void *)wind;
	wind->setWindowTitle(title);
	postCtor();  
}

void DIA_processingQt4 :: postCtor( void )
{
      processingWindow *wind=(processingWindow *)_priv; ADM_assert(wind);
      wind->show();
      lastper=0;
      _nextUpdate=0;
}
uint8_t DIA_processingQt4::update(uint32_t percent)
{
		#define GUI_UPDATE_RATE 1000

        UI_purge();

        if(!_priv) return 1;
        if(!percent) return 0;
        if(percent==lastper)
        {

            return 0;
        }

        elapsed=_clock.getElapsedMS();

        if(elapsed<_nextUpdate) 
        {
          return 0;
        }

        _nextUpdate=elapsed+1000;
        lastper=percent;

		uint32_t hh,mm,ss,mms;
		char string[9];

		ms2time(elapsed,&hh,&mm,&ss,&mms);
		sprintf(string,"%02d:%02d:%02d",hh,mm,ss);

        processingWindow *wind=(processingWindow *)_priv; ADM_assert(wind);
        
        if(percent>=1)
        {
            double totalTime=(100*elapsed)/percent;
            double remaining=totalTime-elapsed;
            if(remaining<0)
                remaining=0;
            uint32_t remainingMs=(uint32_t)remaining;
            //wind->ui->labelTimeLeft->setText(ms2timedisplay(remainingMs));
        }
        
        
      //  wind->ui->labelElapsed->setText(string);
        wind->ui->progressBar->setValue(percent);
       
        return 0;
}

uint8_t DIA_processingQt4::update(uint32_t cur, uint32_t total)
{
        double d,n;
        uint32_t percent;
        UI_purge();
        if(!_priv) return 1;
        if(!total) return 0;

        d=total;
        n=cur;
        n=n*100.;

        n=n/d;

        percent=(uint32_t )floor(n);
        return update(percent);

}

uint8_t DIA_processingQt4::isAlive (void )
{
	if(!_priv) return 0;
    processingWindow *wind=(processingWindow *)_priv; ADM_assert(wind);
	return wind->active;
}

DIA_processingQt4::~DIA_processingQt4()
{
    closeDialog();
}

void DIA_processingQt4::closeDialog(void)
{
	processingWindow *wind = (processingWindow*)_priv;
	ADM_assert(wind);

	if (wind)
	{
		qtUnregisterDialog(wind);
		delete wind;
	}

	wind = NULL;
	_priv = NULL;
}

/**
    \fn createWorking
*/
DIA_processingBase *createProcessing(const char *title)
{
    return new DIA_processingQt4(title);
}   


}
//********************************************
//EOF
