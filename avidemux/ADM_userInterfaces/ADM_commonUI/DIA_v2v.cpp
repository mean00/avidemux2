/***************************************************************************
                          GUI_mux.h  -  description
                             -------------------
    begin                : Wed Jul 3 2002
    copyright            : (C) 2002 by mean
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

#include "config.h"
#include "ADM_default.h"

#include "DIA_coreToolkit.h"
#include "DIA_fileSel.h"
#include "DIA_factory.h"

/**
      \fn DIA_v2v
      \brief  handle vob2vobsub dialog 
*/
uint8_t  DIA_v2v(char **vobname, char **ifoname,char **vobsubname)
{
uint8_t ret=0;
char *tmp=NULL,*tmp2=NULL,*tmp3=NULL;

        
        diaElemFile eVob(0,&tmp,QT_TR_NOOP("_VOB file(s):"), NULL, QT_TR_NOOP("Select VOB file(s)"));
        diaElemFile eIfo(0,&tmp2,QT_TR_NOOP("_IFO file:"), NULL, QT_TR_NOOP("Select IFO file"));
        diaElemFile eVsub(1,&tmp3,QT_TR_NOOP("VobSub file:"), NULL, QT_TR_NOOP("Select VobSub file"));
        
        while(1)
        {
           diaElem *tabs[]={&eVob,&eIfo,&eVsub};
          if( diaFactoryRun(QT_TR_NOOP("VOB to VobSub"),3,tabs))
	  {
              if(!ADM_fileExist(tmp))
              {
                GUI_Error_HIG(QT_TR_NOOP("The selected vobfile does not exist"), NULL); 
                continue;
              }
              if(!ADM_fileExist(tmp2))
              {
                GUI_Error_HIG(QT_TR_NOOP("The selected vobfile does not exist"), NULL); 
                continue;
              }
              if(strlen(tmp3)<3)
              {
                 GUI_Error_HIG(QT_TR_NOOP("Please select a correct VobSub path/dir"), NULL); 
                 continue;
              }
                  if(*vobname) ADM_dealloc(*vobname);
                  if(*ifoname) ADM_dealloc(*ifoname);
                  if(*vobsubname) ADM_dealloc(*vobsubname);

                    *vobname=*ifoname=*vobsubname=NULL;

                  *vobname=ADM_strdup(tmp);
                  *ifoname=ADM_strdup(tmp2);
                  *vobsubname=(char *)ADM_alloc(strlen(tmp3)+5); //ADM_strdup(tmp3);
                  strcpy(*vobsubname,tmp3);
                  if(tmp3[strlen(tmp3)-1]!='x'|| tmp3[strlen(tmp3)-2]!='d') 
                          strcat(*vobsubname,".idx");
                  ADM_dealloc(tmp);
                  ADM_dealloc(tmp2);
                  ADM_dealloc(tmp3);
                  return 1;
          } 
          else return 0;
        }
        return 0;
}
//EOF

