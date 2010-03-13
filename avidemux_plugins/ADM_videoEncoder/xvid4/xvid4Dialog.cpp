/***************************************************************************
                          \fn ADM_Xvid4
                          \brief Front end for xvid4 Mpeg4 asp encoder
                             -------------------
    
    copyright            : (C) 2002/2009 by mean/gruntster
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
#include "ADM_default.h"
#include "ADM_xvid4.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif
/**
    \fn jpegConfigure
    \brief UI configuration for jpeg encoder
*/
extern xvid4_encoder xvid4Settings;
bool         xvid4Configure(void)
{         

diaMenuEntry meE[]={
  {0,QT_TR_NOOP("None")},
  {1,QT_TR_NOOP("Low")},
  {2,QT_TR_NOOP("Medium")},
  {3,QT_TR_NOOP("Full")}
};       

diaMenuEntry qzE[]={
  {0,QT_TR_NOOP("H.263")},
  {1,QT_TR_NOOP("MPEG")},
  {2,QT_TR_NOOP("Custom")}
};       
/*
diaMenuEntry rdE[]={
  {0,QT_TR_NOOP("MB comparison")},
  {1,QT_TR_NOOP("Fewest bits (vhq)")},
  {2,QT_TR_NOOP("Rate distortion")}
};     
diaMenuEntry threads[]={
  {0,QT_TR_NOOP("One thread")},
  {2,QT_TR_NOOP("Two threads)")},
  {3,QT_TR_NOOP("Three threads")},
  {99,QT_TR_NOOP("Auto (#cpu)")}
};     


        FFcodecSettings *conf=&Mp4Settings;

uint32_t me=(uint32_t)conf->lavcSettings.me_method;  
*/
#define PX(x) &(xvid4Settings.x)

         diaElemBitrate   bitrate(&(xvid4Settings.params),NULL);
         diaElemMenu      meM(PX(motionEstimation),QT_TR_NOOP("MotionEstimation"),4,meE);
/*
         diaElemMenu      threadM(PX(MultiThreaded),QT_TR_NOOP("Threading"),4,threads);
         diaElemUInteger  qminM(PX(qmin),QT_TR_NOOP("Mi_n. quantizer:"),1,31);
         diaElemUInteger  qmaxM(PX(qmax),QT_TR_NOOP("Ma_x. quantizer:"),1,31);
         diaElemUInteger  qdiffM(PX(max_qdiff),QT_TR_NOOP("Max. quantizer _difference:"),1,31);
         
         diaElemToggle    fourMv(PX(_4MV),QT_TR_NOOP("4_MV"));
*/
         uint32_t trelBol=*PX(trellis);
         diaElemToggle    trellis(&trelBol,QT_TR_NOOP("_Trellis quantization"));
/*         
         diaElemToggle    qpel(PX(_QPEL),QT_TR_NOOP("_Quarter pixel"));
         diaElemToggle    gmc(PX(_GMC),QT_TR_NOOP("_GMC"));
*/
         
         diaElemUInteger  max_b_frames(PX(maxBFrame),QT_TR_NOOP("_Number of B frames:"),0,32);

         diaElemMenu     qzM(PX(cqmMode),QT_TR_NOOP("_Quantization type:"),2,qzE);
/*         
         diaElemMenu     rdM(PX(mb_eval),QT_TR_NOOP("_Macroblock decision:"),3,rdE);
         
         diaElemUInteger filetol(PX(vratetol),QT_TR_NOOP("_Filesize tolerance (kb):"),0,100000);
         
         diaElemFloat    qzComp(PX(qcompress),QT_TR_NOOP("_Quantizer compression:"),0,1);
         diaElemFloat    qzBlur(PX(qblur),QT_TR_NOOP("Quantizer _blur:"),0,1);
         */
        diaElemUInteger GopSize(PX(maxKeyFrameInterval),QT_TR_NOOP("_Gop Size:"),1,500); 
          /* First Tab : encoding mode */
        diaElem *diamode[]={&bitrate,&meM,&trellis,&max_b_frames,&GopSize};
        diaElemTabs tabMode(QT_TR_NOOP("User Interface"),5,diamode);
        
        /* 2nd Tab : ME */
        diaElemFrame frameMe(QT_TR_NOOP("Advanced Simple Profile"));
        
        
        frameMe.swallow(&meM);
        frameMe.swallow(&trellis);
        frameMe.swallow(&max_b_frames);
        frameMe.swallow(&GopSize);
        frameMe.swallow(&bitrate);
        frameMe.swallow(&qzM);
        
        diaElem *diaME[]={&frameMe};
        diaElemTabs tabME(QT_TR_NOOP("Motion Estimation"),1,diaME);
        /* 3nd Tab : Qz */
       #if 0 
         diaElem *diaQze[]={&qzM,&rdM,&qminM,&qmaxM,&qdiffM,&trellis};
        diaElemTabs tabQz(QT_TR_NOOP("Quantization"),6,diaQze);
        
        /* 4th Tab : RControl */
        
         diaElem *diaRC[]={&filetol,&qzComp,&qzBlur};
        diaElemTabs tabRC(QT_TR_NOOP("Rate Control"),3,diaRC);
        #endif
         diaElemTabs *tabs[]={&tabME};
        if( diaFactoryRunTabs(QT_TR_NOOP("libavcodec MPEG-4 configuration"),1,tabs))
        {
            *PX(trellis)= trelBol;
            return true;
        }
         return false;
}