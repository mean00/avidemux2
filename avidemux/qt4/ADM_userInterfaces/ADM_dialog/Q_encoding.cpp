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
#include "ADM_inttype.h"
#include "Q_encoding.h"

#include "prefs.h"
#include "DIA_working.h"
#include "DIA_encoding.h"
#include "DIA_coreToolkit.h"
#include "avidemutils.h"
#include "ADM_vidMisc.h"

extern void UI_purge(void);
static int stopReq=0;

encodingWindow::encodingWindow() : QDialog()
 {
	ui.setupUi(this);

#ifndef __WIN32
	//check for root privileges
	if (getuid() == 0)
	{
		// set priority to normal, regardless of preferences
		ui.comboBoxPriority->setCurrentIndex(2);
	}
#endif

	connect(ui.checkBoxShutdown, SIGNAL(stateChanged(int)), this, SLOT(shutdownChanged(int)));
	connect(ui.pushButton, SIGNAL(pressed()), this, SLOT(buttonPressed()));
	connect(ui.comboBoxPriority, SIGNAL(currentIndexChanged(int)), this, SLOT(priorityChanged(int)));

	// set priority
	uint32_t priority;

	prefs->get(PRIORITY_ENCODING,&priority);	

#ifndef __WIN32
	// check for root privileges
	if (getuid() == 0)
	{
		ui.comboBoxPriority->setCurrentIndex(priority);
	}
#else
	ui.comboBoxPriority->setCurrentIndex(priority);
#endif
 }

void encodingWindow::buttonPressed(void)
{
	printf("StopReq\n");
	stopReq=1;
}

void encodingWindow::priorityChanged(int priorityLevel)
{
#ifndef __WIN32
	if (getuid() != 0)
	{
		ui.comboBoxPriority->disconnect(SIGNAL(currentIndexChanged(int)));
		ui.comboBoxPriority->setCurrentIndex(2);
		connect(ui.checkBoxShutdown, SIGNAL(currentIndexChanged(int)), this, SLOT(priorityChanged(int)));

		GUI_Error_HIG(QT_TR_NOOP("Privileges Required"), QT_TR_NOOP( "Root privileges are required to perform this operation."));

		return;
	}
#endif

	setpriority(PRIO_PROCESS, 0, ADM_getNiceValue(priorityLevel));
}

void encodingWindow::shutdownChanged(int state)
{
#ifndef __WIN32
	if (getuid() != 0)
	{
		ui.checkBoxShutdown->disconnect(SIGNAL(stateChanged(int)));
		ui.checkBoxShutdown->setCheckState(Qt::Unchecked);
		connect(ui.checkBoxShutdown, SIGNAL(stateChanged(int)), this, SLOT(shutdownChanged(int)));

		GUI_Error_HIG(QT_TR_NOOP("Privileges Required"), QT_TR_NOOP( "Root privileges are required to perform this operation."));
	}
#endif
}

//*******************************************
#define WIDGET(x) (window->ui.x)
#define WRITEM(x,y) window->ui.x->setText(y)
#define WRITE(x) WRITEM(x,string)
/*************************************/
static char string[80];
static encodingWindow *window=NULL;
DIA_encodingQt4::DIA_encodingQt4( uint64_t duration) : DIA_encodingBase(duration)
{
        ADM_assert(window==NULL);
        stopReq=0;
        window=new encodingWindow();
        window->setModal(TRUE);
        window->show();

}
/**
    \fn setFps(uint32_t fps)
    \brief Memorize fps, it will be used later for bitrate computation
*/

void DIA_encodingQt4::setFps(uint32_t fps)
{
    
        
}

void DIA_stop( void)
{
        printf("Stop request\n");
        stopReq=1;
}
/**
    \fn dtpor
*/
DIA_encodingQt4::~DIA_encodingQt4( )
{
	bool shutdownRequired = (window->ui.checkBoxShutdown->checkState() == Qt::Checked);

	if(window) delete window;
	window=NULL;
#if 0
	if (shutdownRequired && !stopReq)
	{
		DIA_working *work=new DIA_working(QT_TR_NOOP("Shutting down"));
		bool performShutdown=true;

		for(int i = 0; i <= 30; i++)
		{
			if (work->isAlive())
			{
				GUI_Sleep(1000);
				work->update(i, 30);
			}
			else
			{
				performShutdown=false;
				break;
			}
		}

		if (performShutdown && shutdown())
		{
			GUI_Sleep(5000);
		}

		delete work;
	}
#endif
}
/**
    \fn setPhasis(const char *n)
    \brief Display parameters as phasis
*/

void DIA_encodingQt4::setPhasis(const char *n)
{
          ADM_assert(window);
          WRITEM(labelPhasis,n);

}
/**
    \fn    setPercent
    \brief display percent of saved file
*/

void DIA_encodingQt4::setPercent(uint32_t p)
{
          ADM_assert(window);
}
/**
    \fn setAudioCodec(const char *n)
    \brief Display parameters as audio codec
*/

void DIA_encodingQt4::setAudioCodec(const char *n)
{
          ADM_assert(window);
          WRITEM(labelAudCodec,n);
}
/**
    \fn setCodec(const char *n)
    \brief Display parameters as video codec
*/

void DIA_encodingQt4::setVideoCodec(const char *n)
{
          ADM_assert(window);
          WRITEM(labelVidCodec,n);
}
/**
    \fn setBitrate(uint32_t br,uint32_t globalbr)
    \brief Display parameters as instantaneous bitrate and average bitrate
*/

void DIA_encodingQt4::setBitrate(uint32_t br,uint32_t globalbr)
{
          ADM_assert(window);
          snprintf(string,79,"%"LU" kB/s",br,globalbr);
          WRITE(labelVidBitrate);

}
/**
    \fn setContainer(const char *container)
    \brief Display parameter as container field
*/

void DIA_encodingQt4::setContainer(const char *container)
{
        ADM_assert(window);
        WRITEM(labelContainer,container);
}

/**
    \fn setQuantIn(int size)
    \brief display parameter as quantizer
*/

void DIA_encodingQt4::setQuantIn(int size)
{
          ADM_assert(window);
          sprintf(string,"%"LU,size);
          WRITE(labelQz);

}
/**
    \fn setSize(int size)
    \brief display parameter as total size
*/

void DIA_encodingQt4::setTotalSize(uint64_t size)
{
          ADM_assert(window);
          uint64_t mb=size>>20;
          sprintf(string,"%"LU" MB",(int)mb);
          WRITE(labelTotalSize);

}

/**
    \fn setAudioSizeIn(int size)
    \brief display parameter as audio size
*/

void DIA_encodingQt4::setAudioSize(uint64_t size)
{
          ADM_assert(window);
          uint64_t mb=size>>20;
          sprintf(string,"%"LU" MB",(int)mb);
          WRITE(labelAudioSize);

}
/**
    \fn isAlive( void )
    \brief return 0 if the window was killed or cancel button press, 1 otherwisearchForward
*/
bool DIA_encodingQt4::isAlive( void )
{

        if(stopReq)
        {
          if(GUI_Alternate((char*)QT_TR_NOOP("The encoding is paused. Do you want to resume or abort?"),
                              (char*)QT_TR_NOOP("Resume"),(char*)QT_TR_NOOP("Abort")))
                 {
                         stopReq=0;
                 }
        }

        if(!stopReq) return true;		

        return false;
}
/**
        \fn createEncoding
*/
namespace ADM_Qt4CoreUIToolkit
{
DIA_encodingBase *createEncoding(uint64_t duration)
{
        return new DIA_encodingQt4(duration);
}
}
//********************************************
//EOF
