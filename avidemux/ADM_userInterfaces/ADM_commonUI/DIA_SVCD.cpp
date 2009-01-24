//
//
// C++ Implementation: DIA_SVCD
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "config.h"
//#include "ADM_lavcodec.h"


#include <string.h>
#include <stdio.h>

# include <math.h>

#include "ADM_default.h"

#include "ADM_codecs/ADM_codec.h"
#include "ADM_encoder/ADM_vidEncode.hxx"
#include "ADM_assert.h" 
#include "DIA_factory.h"
#include "../../ADM_libraries/ADM_libmpeg2enc/ADM_mpeg2enc.h"

/**
      \fn DIA_SVCDParam
      \brief Dialog to set encoding options for SVCD/DVD mpeg2enc based
*/
uint8_t DIA_SVCDParam(COMPRES_PARAMS *incoming)
{	

Mpeg2encParam *conf=(Mpeg2encParam *)incoming->extraSettings;
ADM_assert(incoming->extraSettingsLen==sizeof(Mpeg2encParam));

diaMenuEntry wideM[]={
  {0,QT_TR_NOOP("4:3")},
  {1,QT_TR_NOOP("16:9")}};
diaMenuEntry matrixM[]={
  {0,QT_TR_NOOP("Default")},
  {1,QT_TR_NOOP("TMPGEnc")},
  {2,QT_TR_NOOP("Anime")},
  {3,QT_TR_NOOP("KVCD")}
};
diaMenuEntry interM[]={
  {0,QT_TR_NOOP("Progressive")},
  {1,QT_TR_NOOP("Interlaced TFF")},
  {2,QT_TR_NOOP("Interlaced BFF")}
};
                      
         diaElemBitrate bitrate(incoming,NULL);
         diaElemMenu      widescreen(&(conf->widescreen),QT_TR_NOOP("Aspect _ratio:"),2,wideM);
         diaElemMenu      matrix(&(conf->user_matrix),QT_TR_NOOP("_Matrices:"),4,matrixM);
         diaElemUInteger gop(&(conf->gop_size),QT_TR_NOOP("_GOP size:"),1,30);
         diaElemUInteger maxb(&(conf->maxBitrate),QT_TR_NOOP("Ma_x. bitrate:"),100,9000);

uint32_t inter;
          if(!conf->interlaced) inter=0;
            else if(conf->bff) inter=2;
                else inter=1;
         diaElemMenu      interW(&inter,QT_TR_NOOP("_Interlacing:"),3,interM);
  
      diaElem *elems[6]={&bitrate,&widescreen,&interW,&matrix,&gop,&maxb};
    
  if( diaFactoryRun(QT_TR_NOOP("MPEG-2 Configuration"),6,elems))
  {
    switch(inter)
    {
      case 0: conf->interlaced=0;conf->bff=0;break;
      case 1: conf->interlaced=1;conf->bff=0;break;
      case 2: conf->interlaced=1;conf->bff=1;break;
      default: ADM_assert(0);
    }
    return 1;
  }
  return 0;
}	
// EOF
