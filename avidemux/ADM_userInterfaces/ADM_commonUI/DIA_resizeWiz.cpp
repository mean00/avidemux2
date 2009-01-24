
/***************************************************************************
                         DIA_resizeWiz
                             -------------------

                           Ui for hue & sat
    copyright            : (C) 2004/5 by mean
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

#include "DIA_resizeWiz.h"
#include "DIA_factory.h"


uint8_t DIA_resizeWiz(RESWIZ_FORMAT *format, RESWIZ_AR *source, RESWIZ_AR *destination)
{
uint8_t r=0;
#define ONELINE(x,y) {RESWIZ_##x,y,NULL}
            diaMenuEntry menuFTarget[]={
                             ONELINE(VCD,QT_TR_NOOP("VCD")),
                             ONELINE(SVCD,QT_TR_NOOP("SVCD")),
                             ONELINE(DVD,QT_TR_NOOP("DVD")),
                             ONELINE(DVD_HD1,QT_TR_NOOP("DVD half D1")),
                             ONELINE(PSP,QT_TR_NOOP("PSP 480*272")),
                             ONELINE(PSP_FULLRES,QT_TR_NOOP("PSP full res (720*480")),
                             ONELINE(IPOD,"IPOD 320*240"),
                             ONELINE(IPOD640,"IPOD 640*480"),
                          };
             diaMenuEntry menuFAspect[3]={
               {RESWIZ_AR_1_1,QT_TR_NOOP("1:1"),NULL},
               {RESWIZ_AR_4_3,QT_TR_NOOP("4:3"),NULL},
               {RESWIZ_AR_16_9,QT_TR_NOOP("16:9"),NULL}
                          };
  
  
                          
    uint32_t tformat=(uint32_t )*format;
    uint32_t tsource=(uint32_t )*source;
    uint32_t tdestination=(uint32_t )*destination;
                          
    diaElemMenu     menu1(&tformat,QT_TR_NOOP("_Target type:"), sizeof(menuFTarget) / sizeof(diaMenuEntry),menuFTarget);
    diaElemMenu     menu2(&tsource,QT_TR_NOOP("_Source aspect ratio:"), 3,menuFAspect);
    diaElemMenu     menu3(&tdestination,QT_TR_NOOP("_Destination aspect ratio:"), 3,menuFAspect);
    
    
    
      diaElem *elems[3]={&menu1,&menu2,&menu3};
    if(diaFactoryRun(QT_TR_NOOP("Auto Wizard"),3,elems))
    {
       *format=(RESWIZ_FORMAT) tformat;
       *source=(RESWIZ_AR) tsource;
       *destination=(RESWIZ_AR) tdestination;
      return 1; 
    }
    return 0;
}
