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
#include "ADM_x264.h"
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
extern x264_encoder x264Settings;
bool         x264Configure(void)
{         
#if 0
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
diaMenuEntry profileE[]={
{ XVID_PROFILE_S_L0  ,"Simple Level0"},
{ XVID_PROFILE_S_L1  ,"Simple Level1"},
{ XVID_PROFILE_S_L2  ,"Simple Level2"},
{ XVID_PROFILE_S_L3  ,"Simple Level3"},
{ XVID_PROFILE_AS_L0 ,"Adv. Simple Level0"},
{ XVID_PROFILE_AS_L1 ,"Adv. Simple Level1"},
{ XVID_PROFILE_AS_L2 ,"Adv. Simple Level2"},
{ XVID_PROFILE_AS_L3 ,"Adv. Simple Level3"},
{ XVID_PROFILE_AS_L4 ,"Adv. Simple Level4"},
}; 

diaMenuEntry rdE[]={
  {0,QT_TR_NOOP("None")},
  {1,QT_TR_NOOP("DCT")},
  {2,QT_TR_NOOP("Qpel16")},
  {3,QT_TR_NOOP("Qpel8")},
  {4,QT_TR_NOOP("Square")}
};    

diaMenuEntry threads[]={
  {1,QT_TR_NOOP("One thread")},
  {2,QT_TR_NOOP("Two threads)")},
  {3,QT_TR_NOOP("Three threads")},
  {99,QT_TR_NOOP("Auto (#cpu)")}
};     


#define PX(x) &(xvid4Settings.x)

         diaElemBitrate   bitrate(&(xvid4Settings.params),NULL);
         diaElemMenu      meM(PX(motionEstimation),QT_TR_NOOP("MotionEstimation"),4,meE);

         diaElemMenu      threadM(PX(nbThreads),QT_TR_NOOP("Threading"),4,threads);
/*
         diaElemUInteger  qminM(PX(qmin),QT_TR_NOOP("Mi_n. quantizer:"),1,31);
         diaElemUInteger  qmaxM(PX(qmax),QT_TR_NOOP("Ma_x. quantizer:"),1,31);
         diaElemUInteger  qdiffM(PX(max_qdiff),QT_TR_NOOP("Max. quantizer _difference:"),1,31);
*/
         uint32_t trelBol=*PX(trellis);
         diaElemToggle    trellis(&trelBol,QT_TR_NOOP("_Trellis quantization"));         
         diaElemUInteger  max_b_frames(PX(maxBFrame),QT_TR_NOOP("_Number of B frames:"),0,32);

         diaElemMenu     qzM(PX(cqmMode),QT_TR_NOOP("_Quantization type:"),2,qzE);
         
         diaElemMenu     rdM(PX(rdMode),QT_TR_NOOP("_Macroblock decision:"),5,rdE);

         diaElemMenu     profileM(PX(profile),QT_TR_NOOP("Profile:"),9,profileE);
         /*
         diaElemUInteger filetol(PX(vratetol),QT_TR_NOOP("_Filesize tolerance (kb):"),0,100000);
         
         diaElemFloat    qzComp(PX(qcompress),QT_TR_NOOP("_Quantizer compression:"),0,1);
         diaElemFloat    qzBlur(PX(qblur),QT_TR_NOOP("Quantizer _blur:"),0,1);
         */
        diaElemUInteger GopSize(PX(maxKeyFrameInterval),QT_TR_NOOP("_Gop Size:"),1,500); 
          /* First Tab : encoding mode */
       
        diaElemFrame frameMe(QT_TR_NOOP("Advanced Simple Profile"));
        frameMe.swallow(&profileM);
        frameMe.swallow(&max_b_frames);
        frameMe.swallow(&GopSize);
        frameMe.swallow(&bitrate);
       
        
        diaElem *diaME[]={&frameMe};
        diaElemTabs tabME(QT_TR_NOOP("Motion Estimation"),1,diaME);

        /* 2nd Tab : Qz */
       
         diaElem *diaQze[]={&qzM,&rdM,&meM,&trellis};
        diaElemTabs tabQz(QT_TR_NOOP("Quantization"),4,diaQze);
        
        /* 3th Tab : thread */
         diaElem *diaThread[]={&threadM};
         diaElemTabs tabThread(QT_TR_NOOP("Threads"),1,diaThread);
        #if 0
         diaElem *diaRC[]={&filetol,&qzComp,&qzBlur};
        diaElemTabs tabRC(QT_TR_NOOP("Rate Control"),3,diaRC);
        #endif
         diaElemTabs *tabs[]={&tabME,&tabQz,&tabThread};
        if( diaFactoryRunTabs(QT_TR_NOOP("Xvid4 MPEG-4 ASP configuration"),3,tabs))
        {
            *PX(trellis)= trelBol;
            return true;
        }

         return false;
#endif
    return true;
}