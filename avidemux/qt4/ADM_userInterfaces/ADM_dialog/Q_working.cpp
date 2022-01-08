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

#include "Q_working.h"
#include "ADM_default.h"
#include "ADM_vidMisc.h"
#include "DIA_working.h"
#include "ADM_toolkitQt.h"
#include "DIA_coreToolkit.h"

workWindow::workWindow(QWidget *parent) : QDialog(parent)
 {
     ui=new Ui_workingDialog();
     ui->setupUi(this);
     active=true;
     setWindowModality(Qt::ApplicationModal);
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
     connect(ui->buttonBox,SIGNAL(rejected()),this,SLOT(reject()));
#else
     connect(ui->buttonBox,&QDialogButtonBox::rejected,this,&QDialog::reject);
#endif
 }

workWindow::~workWindow()
{
    delete ui;
    ui = NULL;
}

void workWindow::reject(void)
{
    ADM_info("Stop Request\n");
    active=false;
}
//*******************************************

namespace ADM_Qt4CoreUIToolkit
{
/**
 * \class DIA_workingQt4
 */
class DIA_workingQt4 : public DIA_workingBase
{

protected:
        virtual void         postCtor( void ) ;
public:
                             DIA_workingQt4( const char *title=NULL );
        virtual              ~DIA_workingQt4();
            
        virtual uint8_t      update(uint32_t percent);  // If returns 1 -> Means aborted
        virtual uint8_t      update(uint32_t current,uint32_t total); // If returns 1 -> Means aborted
        virtual uint8_t      isAlive (void );
        virtual void         reuseAs( const char *title=NULL );
                void         closeDialog(void);
        
};
/**
 * 
 * @param title
 */
DIA_workingQt4::DIA_workingQt4(const char *title) : DIA_workingBase(title)
{
    workWindow *wind = new workWindow(qtLastRegisteredDialog());
    qtRegisterDialog(wind);
    _priv=(void *)wind;
    wind->setWindowTitle(QString::fromUtf8(title));
    postCtor();  
}
/**
 * 
 */
void DIA_workingQt4 :: postCtor( void )
{
    workWindow *wind=(workWindow *)_priv; 
    ADM_assert(wind);
    wind->show();
    lastper=0;
    _nextUpdate=0;
}
/**
 * 
 * @param percent
 * @return 
 */
uint8_t DIA_workingQt4::update(uint32_t percent)
{
#define GUI_UPDATE_RATE 1000

    UI_purge();
    if(!_priv) 
        return 1;
    workWindow *wind=(workWindow *)_priv; 
    ADM_assert(wind);
    if(!wind->active)
    {
        return true;
    }
    if(!percent) 
        return 0;
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
    char string[32]; // keep margin

    ms2time(elapsed,&hh,&mm,&ss,&mms);
    sprintf(string,"%02d:%02d:%02d",hh,mm,ss);



    if(percent>=1)
    {
        double totalTime=(100*elapsed)/percent;
        double remaining=totalTime-elapsed;
        if(remaining<0)
            remaining=0;
        uint32_t remainingMs=(uint32_t)remaining;
        wind->ui->labelTimeLeft->setText(ms2timedisplay(remainingMs));
    }


    wind->ui->labelElapsed->setText(string);
    wind->ui->progressBar->setValue(percent);

    return 0;
}
/**
 * 
 * @param cur
 * @param total
 * @return 
 */
uint8_t DIA_workingQt4::update(uint32_t cur, uint32_t total)
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
    if(percent>100) percent=100;
    return update(percent);
}
/**
 * 
 * @return 
 */
uint8_t DIA_workingQt4::isAlive (void )
{
    if(!_priv) return 0;
    workWindow *wind=(workWindow *)_priv; 
    ADM_assert(wind);
    return wind->active;
}
/**
 * 
 */
void DIA_workingQt4::reuseAs( const char *title )
{
    lastper=0;
    _nextUpdate=0; 
    _clock.reset();
    workWindow *wind=(workWindow *)_priv;
    ADM_assert(wind);
    ADM_assert(title!=NULL);
    wind->setWindowTitle(QString::fromUtf8(title));    
}
/**
 * 
 */
DIA_workingQt4::~DIA_workingQt4()
{
    closeDialog();
}
/**
 * 
 */
void DIA_workingQt4::closeDialog(void)
{
    workWindow *wind = (workWindow*)_priv;
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
DIA_workingBase *createWorking(const char *title)
{
    return new DIA_workingQt4(title);
}   


}
//********************************************
//EOF
