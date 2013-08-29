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
  {0,QT_TRANSLATE_NOOP("xvid4","None")},
  {1,QT_TRANSLATE_NOOP("xvid4","Low")},
  {2,QT_TRANSLATE_NOOP("xvid4","Medium")},
  {3,QT_TRANSLATE_NOOP("xvid4","Full")}
};       

diaMenuEntry qzE[]={
  {0,QT_TRANSLATE_NOOP("xvid4","H.263")},
  {1,QT_TRANSLATE_NOOP("xvid4","MPEG")},
  {2,QT_TRANSLATE_NOOP("xvid4","Custom")}
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
  {0,QT_TRANSLATE_NOOP("xvid4","None")},
  {1,QT_TRANSLATE_NOOP("xvid4","DCT")},
  {2,QT_TRANSLATE_NOOP("xvid4","Qpel16")},
  {3,QT_TRANSLATE_NOOP("xvid4","Qpel8")},
  {4,QT_TRANSLATE_NOOP("xvid4","Square")}
};    

diaMenuEntry threads[]={
  {1,QT_TRANSLATE_NOOP("xvid4","One thread")},
  {2,QT_TRANSLATE_NOOP("xvid4","Two threads)")},
  {3,QT_TRANSLATE_NOOP("xvid4","Three threads")},
  {99,QT_TRANSLATE_NOOP("xvid4","Auto (#cpu)")}
};  

diaMenuEntry arModeE[]={
  {XVID_PAR_11_VGA,QT_TRANSLATE_NOOP("xvid4","1:1 (PC)")},
  {XVID_PAR_43_PAL,QT_TRANSLATE_NOOP("xvid4","4:3 (PAL))")},
  {XVID_PAR_43_NTSC,QT_TRANSLATE_NOOP("xvid4","4:3 (NTSC))")},
  {XVID_PAR_169_PAL,QT_TRANSLATE_NOOP("xvid4","16:9 (PAL))")},
  {XVID_PAR_169_NTSC,QT_TRANSLATE_NOOP("xvid4","16:9 (NTSC))")},
};  


#define PX(x) &(xvid4Settings.x)

         diaElemBitrate   bitrate(&(xvid4Settings.params),NULL);
         diaElemMenu      meM(PX(motionEstimation),QT_TRANSLATE_NOOP("xvid4","MotionEstimation"),4,meE);

         diaElemMenu      threadM(PX(nbThreads),QT_TRANSLATE_NOOP("xvid4","Threading"),4,threads);

         diaElemUInteger  qminM(PX(qMin),QT_TRANSLATE_NOOP("xvid4","Mi_n. quantizer:"),1,31);
         diaElemUInteger  qmaxM(PX(qMax),QT_TRANSLATE_NOOP("xvid4","Ma_x. quantizer:"),1,31);
/*
         diaElemUInteger  qdiffM(PX(max_qdiff),QT_TRANSLATE_NOOP("xvid4","Max. quantizer _difference:"),1,31);
*/
         diaElemToggle    trellis(PX(trellis),QT_TRANSLATE_NOOP("xvid4","_Trellis quantization"));         
         diaElemUInteger  max_b_frames(PX(maxBFrame),QT_TRANSLATE_NOOP("xvid4","_Number of B frames:"),0,32);

         diaElemMenu     qzM(PX(cqmMode),QT_TRANSLATE_NOOP("xvid4","_Quantization type:"),2,qzE);
         
         diaElemMenu     rdM(PX(rdMode),QT_TRANSLATE_NOOP("xvid4","_Macroblock decision:"),5,rdE);

         diaElemMenu     profileM(PX(profile),QT_TRANSLATE_NOOP("xvid4","Profile:"),9,profileE);
         
         diaElemMenu    par(PX(arMode),QT_TRANSLATE_NOOP("xvid4","Aspect Ratio:"),sizeof(arModeE)/sizeof(diaMenuEntry),arModeE);
         
         /*
         diaElemUInteger filetol(PX(vratetol),QT_TRANSLATE_NOOP("xvid4","_Filesize tolerance (kb):"),0,100000);
         
         diaElemFloat    qzComp(PX(qcompress),QT_TRANSLATE_NOOP("xvid4","_Quantizer compression:"),0,1);
         diaElemFloat    qzBlur(PX(qblur),QT_TRANSLATE_NOOP("xvid4","Quantizer _blur:"),0,1);
         */
        diaElemUInteger GopSize(PX(maxKeyFrameInterval),QT_TRANSLATE_NOOP("xvid4","_Gop Size:"),1,500); 
          /* First Tab : encoding mode */
       
        diaElemFrame frameMe(QT_TRANSLATE_NOOP("xvid4","Advanced Simple Profile"));
        frameMe.swallow(&profileM);
        frameMe.swallow(&max_b_frames);
        frameMe.swallow(&GopSize);
        frameMe.swallow(&bitrate);
       
        
        diaElem *diaME[]={&frameMe};
        diaElemTabs tabME(QT_TRANSLATE_NOOP("xvid4","Motion Estimation"),1,diaME);

        /* 2nd Tab : Qz */
       
         diaElem *diaQze[]={&qzM,&qminM,&qmaxM,&rdM,&meM,&trellis};
        diaElemTabs tabQz(QT_TRANSLATE_NOOP("xvid4","Quantization"),6,diaQze);
        
        /* 3th Tab : thread */
         diaElem *diaThread[]={&threadM};
         diaElemTabs tabThread(QT_TRANSLATE_NOOP("xvid4","Threads"),1,diaThread);
         
         /**
          * 4th tab : aspect ratio
          * @return 
          */
         diaElem *diaAR[]={&par};
         diaElemTabs tabAR(QT_TRANSLATE_NOOP("xvid4","Aspect Ratio"),1,diaAR);

         
        #if 0
         diaElem *diaRC[]={&filetol,&qzComp,&qzBlur};
        diaElemTabs tabRC(QT_TRANSLATE_NOOP("xvid4","Rate Control"),3,diaRC);
        #endif
         diaElemTabs *tabs[]={&tabME,&tabQz,&tabThread,&tabAR};
        if( diaFactoryRunTabs(QT_TRANSLATE_NOOP("xvid4","Xvid4 MPEG-4 ASP configuration"),4,tabs))
        {
            return true;
        }
         return false;
}
