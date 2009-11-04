/***************************************************************************
                          DIA_dmx.cpp  -  description
                             -------------------     
Indexer progress dialog                             
    
    copyright            : (C) 2005 by mean
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

#include <QtGui/QApplication>

#include "T_index_pg.h"
#include "ADM_default.h"
#include "ADM_videoFilter.h"
//#include "ADM_encoderConf.h"
//#include "ADM_encoder/adm_encoder.h"
#include "DIA_idx_pg.h"
#include "ADM_vidMisc.h"

extern void UI_purge( void );

void Ui_iDialog::setupUi(QDialog *Dialog)
{
	Dialog->setObjectName(QString::fromUtf8("Dialog"));
	verticalLayout = new QWidget(Dialog);
	verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
	verticalLayout->setGeometry(QRect(9, 9, 215, 89));
	vboxLayout = new QVBoxLayout(verticalLayout);
	vboxLayout->setSpacing(6);
	vboxLayout->setMargin(0);
	vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
	labelTimeLeft = new QLabel(verticalLayout);
	labelTimeLeft->setObjectName(QString::fromUtf8("labelTimeLeft"));
	vboxLayout->addWidget(labelTimeLeft);

	labelImages = new QLabel(verticalLayout);
	labelImages->setObjectName(QString::fromUtf8("labelImages"));
	vboxLayout->addWidget(labelImages);

	progressBar = new QProgressBar(verticalLayout);
	progressBar->setObjectName(QString::fromUtf8("progressBar"));
	progressBar->setValue(0);
	progressBar->setOrientation(Qt::Horizontal);

	vboxLayout->addWidget(progressBar);

	QSize size(236, 111);
	size = size.expandedTo(Dialog->minimumSizeHint());
	Dialog->resize(size);

	QMetaObject::connectSlotsByName(Dialog);
}

void Ui_iDialog::retranslateUi(QDialog *Dialog)
{
	Dialog->setWindowTitle(QApplication::translate("Dialog", "Indexing", 0, QApplication::UnicodeUTF8));
	labelTimeLeft->setText(QApplication::translate("Dialog", "Time Left : Infinity", 0, QApplication::UnicodeUTF8));
	labelImages->setText(QApplication::translate("Dialog", "# Images :", 0, QApplication::UnicodeUTF8));

	Q_UNUSED(Dialog);
}

static Ui_indexingDialog *dialog=NULL; 

#if 0
static gint on_destroy_abort(GtkObject * object, gpointer user_data)
{
DIA_progressIndexing *pf;

        UNUSED_ARG(object);
        UNUSED_ARG(user_data);

        pf=(Ui_indexingDialog *)user_data;
        if(!GUI_Confirmation_HIG(QT_TR_NOOP("Continue indexing"),QT_TR_NOOP("Abort Requested"),QT_TR_NOOP("Do you want to abort indexing ?")))
        {
         //       pf->abortRequest();
                abted=1;
        }

        return TRUE;

};
#endif

DIA_progressIndexing::DIA_progressIndexing(const char *name)
{
        dialog=new Ui_indexingDialog(name);
        clock.reset();
        aborted=0;
	_nextUpdate=0;
        dialog->show();
        UI_purge();

}
DIA_progressIndexing::~DIA_progressIndexing()
{
        ADM_assert(dialog);
        delete dialog;
        dialog=NULL;
}
uint8_t       DIA_progressIndexing::isAborted(void) 
{
        ADM_assert(dialog);
        return dialog->abted;
}
uint8_t DIA_progressIndexing::abortRequest(void)
{
        ADM_assert(dialog);
        aborted=1;
        dialog->abted=1;
        return 1;
}
uint8_t       DIA_progressIndexing::update(uint32_t done,uint32_t total, uint32_t nbImage, uint32_t hh, uint32_t mm, uint32_t ss)
{
        uint32_t tim;
	#define  GUI_UPDATE_RATE 1000

	tim=clock.getElapsedMS();
	if(tim>_nextUpdate)
	{
        char string[256];
        double f;
        	uint32_t   tom,zhh,zmm,zss,zmms;

		_nextUpdate=tim+GUI_UPDATE_RATE;
        sprintf(string,"%02d:%02d:%02d",hh,mm,ss);
        dialog->setTime(string);
        

        sprintf(string,QT_TR_NOOP("# Images :%0"LU),nbImage);
        dialog->setImage(string);

        f=done;
        f/=total;

        dialog->setPercent(f);

        /* compute ETL */
       // Do a simple relation between % and time
        // Elapsed time =total time*percent
        if(f<0.01) return 1;
        f=tim/f;
        // Tom is total time
        tom=(uint32_t)floor(f);
        if(tim>tom) return 1;
        tom=tom-tim;
        ms2time(tom,&zhh,&zmm,&zss,&zmms);
        sprintf(string,QT_TR_NOOP("Time Left :%02d:%02d:%02d"),zhh,zmm,zss);
        dialog->setETA(string);
        UI_purge();
        }
        return 1;
}
//****************************** CLASS ***********************
Ui_indexingDialog::Ui_indexingDialog(const char *name)
{
      abted=0;
      ui.setupUi(this);
}
Ui_indexingDialog::~Ui_indexingDialog()
{
    
}
void Ui_indexingDialog::setTime(const char *f)
{
    //printf("Time:%s\n",f);
}
void Ui_indexingDialog::setImage(const char *f)
{
	dialog->ui.labelImages->setText(QString::fromUtf8(f));
}

void Ui_indexingDialog::setETA(const char *f)
{
   // printf("Eta:%s\n",f);
  dialog->ui.labelTimeLeft->setText(QString::fromUtf8(f));
}
void Ui_indexingDialog::setPercent(float f)
{
    dialog->ui.progressBar->setValue((int)(100.*f));
    
}



// EOF
