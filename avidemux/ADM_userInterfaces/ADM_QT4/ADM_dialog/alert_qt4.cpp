/**
    \file alert_qt4
   copyright            : (C) 2007 by mean
    email                : Mean/fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QDialog>
#include <QtGui/QMessageBox>
#include "DIA_coreToolkit.h"
#include "DIA_coreUI_internal.h"
#include "ADM_default.h"
#include "prefs.h"

static int beQuiet=0;
extern QWidget *QuiMainWindows;
namespace ADM_Qt4CoreUIToolkit
{
//****************************************************************************************************
void            GUI_Alert(const char *alertstring)
{
    QMessageBox::StandardButton reply;
              reply = QMessageBox::critical(QuiMainWindows, QString::fromUtf8(QT_TR_NOOP("Alert")),
                                        QString::fromUtf8(alertstring),
                                        QMessageBox::Ok );
}
//****************************************************************************************************
void            GUI_Info(const char *alertstring)
{
    QMessageBox::StandardButton reply;
              reply = QMessageBox::information(QuiMainWindows, QString::fromUtf8(QT_TR_NOOP("Info")),
                                        QString::fromUtf8(alertstring),
                                        QMessageBox::Ok );
}

//****************************************************************************************************
void            GUI_Info_HIG(const ADM_LOG_LEVEL level,const char *primary, const char *secondary_format)
{
  uint32_t msglvl=2;
  char *string;
  char alertstring[1024];

        prefs->get(MESSAGE_LEVEL,&msglvl);

        if(msglvl<level)
        {
                printf("Info : %s \n",primary);
                return;
        }

        if(! secondary_format)
        {
            snprintf(alertstring,1024,"<big><b>%s</b></big>",primary);
         }else
         {
        	 snprintf(alertstring,1024,"<big><b>%s</b></big><br><br>%s",primary,secondary_format);
         }
        QMessageBox::StandardButton reply;
          reply = QMessageBox::information(QuiMainWindows, QString::fromUtf8(QT_TR_NOOP("Info")),
                                    QString::fromUtf8(alertstring),
                                    QMessageBox::Ok );

}
//****************************************************************************************************
void            GUI_Error_HIG(const char *primary, const char *secondary_format)
{
  uint32_t msglvl=2;
  char *string;
  char alertstring[1024];
  
  
        prefs->get(MESSAGE_LEVEL,&msglvl);

        if(msglvl==ADM_LOG_NONE) 
        {
                printf("Error :%s\n",primary);
                return;
        }

        if(! secondary_format)
        {
            snprintf(alertstring,1024,"<big><b>%s</b></big>",primary);
         }else
         {
        	 snprintf(alertstring,1024,"<big><b>%s</b></big><br><br>%s",primary,secondary_format);
         }
          QMessageBox::StandardButton reply;
          reply = QMessageBox::critical(QuiMainWindows, QString::fromUtf8(QT_TR_NOOP("Info")),
                                    QString::fromUtf8(alertstring),
                                    QMessageBox::Ok );
}
//****************************************************************************************************
int             GUI_Confirmation_HIG(const char *button_confirm, const char *primary, const char *secondary_format)
{
uint32_t msglvl=2;
char alertstring[1024];

        if (beQuiet)
              {
                      printf("Info: %s\n", primary);
                      return 0;
              }
        
        if (!secondary_format)
        {
              snprintf(alertstring,1024,"<big><b>%s</b></big>",primary);
        }
        else
        {	
        	snprintf(alertstring,1024,"<big><b>%s</b></big><br><br>%s",primary,secondary_format);
        }
          QMessageBox::StandardButton reply;
          reply = QMessageBox::question(QuiMainWindows, QString::fromUtf8(QT_TR_NOOP("Confirmation")),
                                    QString::fromUtf8(alertstring),
                                    QMessageBox::Yes | QMessageBox::No );
          if(reply==QMessageBox::Yes) return 1;
        return 0; 
}
//****************************************************************************************************
int             GUI_YesNo(const char *primary, const char *secondary_format)
{
uint32_t msglvl=2;
char alertstring[1024];
char alertstring2[1024];
        if (beQuiet)
              {
                      printf("Info: %s\n", primary);
                      return 0;
              }
        
        if (!secondary_format)
        {
              snprintf(alertstring,1024,"<big><b>%s</b></big>",primary);
        }
        else
        {	
        	snprintf(alertstring,1024,"<big><b>%s</b></big><br><br>%s",primary,secondary_format);
        }
          QMessageBox::StandardButton reply;
          reply = QMessageBox::question(QuiMainWindows, QString::fromUtf8(QT_TR_NOOP("Confirmation")),
                                    QString::fromUtf8(alertstring),
                                    QMessageBox::Yes | QMessageBox::No );
          if(reply==QMessageBox::Yes) return 1;
        return 0; 
}
//****************************************************************************************************
int             GUI_Question(const char *alertstring)
{
 QMessageBox::StandardButton reply;
          if (beQuiet)
              {
                      printf("Question: %s\n", alertstring);
                      return 0;
              }
          reply = QMessageBox::question(QuiMainWindows, QString::fromUtf8(QT_TR_NOOP("Question")),
                                    QString::fromUtf8(alertstring),
                                    QMessageBox::Yes | QMessageBox::No );
          if(reply==QMessageBox::Yes) return 1;
          return 0;
}
/**
    \fn GUI_Alternate(char *title,char *choice1,char *choice2)
    \brief Popup a dialog box name title with 2 buttons (choice1/choice2)
    \return 0 if first is selected, 1 if second
*/
int      GUI_Alternate(char *title,char *choice1,char *choice2)
{
int reply;
QMessageBox *box;
    if (beQuiet)
              {
                      printf("Alternate<%s>: %s or %s\n", title,choice1,choice2);
                      return 0;
              }

char alertstring[1024];

        box=new QMessageBox(QuiMainWindows);
        box->setWindowTitle(QString::fromUtf8(QT_TR_NOOP("Question ?")));
        box->addButton(choice1,QMessageBox::YesRole);
        box->addButton(choice2,QMessageBox::NoRole);
         if (title)
        {
              snprintf(alertstring,1024,"%s",title);
        }else
        {
            snprintf(alertstring,1024,QT_TR_NOOP("Question"));
        }
        box->setText(QString::fromUtf8(alertstring));
        box->setIcon(QMessageBox::Question);
        reply = box->exec();
        delete box;
        printf("Reply:%d\n",reply);
        if(reply==0) return 1;
        return 0;

}
//****************************************************************************************************
uint8_t  GUI_getDoubleValue(double *valye, float min, float max, const char *title)
{
  
  
  return 0; 
}
//****************************************************************************************************
uint8_t		GUI_isQuiet(void)
{
    return beQuiet;
}
//****************************************************************************************************
void            GUI_Verbose(void)
{
    beQuiet=0;
}
//****************************************************************************************************
void            GUI_Quiet(void)
{
  beQuiet=1;
}
//****************************************************************************************************
}

static CoreToolkitDescriptor Qt4CoreToolkitDescriptor=
{
		&ADM_Qt4CoreUIToolkit::GUI_Info_HIG,
		&ADM_Qt4CoreUIToolkit::GUI_Error_HIG,
		&ADM_Qt4CoreUIToolkit::GUI_Confirmation_HIG,
		&ADM_Qt4CoreUIToolkit::GUI_YesNo,
		&ADM_Qt4CoreUIToolkit::GUI_Question,
		&ADM_Qt4CoreUIToolkit::GUI_Alternate,
		&ADM_Qt4CoreUIToolkit::GUI_Verbose,
		&ADM_Qt4CoreUIToolkit::GUI_Quiet,
		&ADM_Qt4CoreUIToolkit::GUI_isQuiet
};

void InitCoreToolkit(void )
{
	DIA_toolkitInit(&Qt4CoreToolkitDescriptor);
	
}
//EOF
