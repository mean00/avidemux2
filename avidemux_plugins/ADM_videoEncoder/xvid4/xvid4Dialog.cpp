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

/**
    \fn xvidConfigure
    \brief UI configuration for xvid4 encoder
*/
extern xvid4_encoder xvid4Settings;
bool         xvid4Configure(void)
{

diaMenuEntry meE[]={
  {0,QT_TRANSLATE_NOOP("xvid4","None"),NULL},
  {1,QT_TRANSLATE_NOOP("xvid4","Low"),NULL},
  {2,QT_TRANSLATE_NOOP("xvid4","Medium"),NULL},
  {3,QT_TRANSLATE_NOOP("xvid4","Full"),NULL}
};

diaMenuEntry qzE[]={
  {0,QT_TRANSLATE_NOOP("xvid4","H.263"),NULL},
  {1,QT_TRANSLATE_NOOP("xvid4","MPEG"),NULL},
  {2,QT_TRANSLATE_NOOP("xvid4","Custom"),NULL}
};
diaMenuEntry profileE[]={
  { XVID_PROFILE_S_L0  ,"Simple Level0", NULL},
  { XVID_PROFILE_S_L1  ,"Simple Level1", NULL},
  { XVID_PROFILE_S_L2  ,"Simple Level2", NULL},
  { XVID_PROFILE_S_L3  ,"Simple Level3", NULL},
  { XVID_PROFILE_AS_L0 ,"Adv. Simple Level0", NULL},
  { XVID_PROFILE_AS_L1 ,"Adv. Simple Level1", NULL},
  { XVID_PROFILE_AS_L2 ,"Adv. Simple Level2", NULL},
  { XVID_PROFILE_AS_L3 ,"Adv. Simple Level3", NULL},
  { XVID_PROFILE_AS_L4 ,"Adv. Simple Level4", NULL}
};

diaMenuEntry rdE[]={
  {0,QT_TRANSLATE_NOOP("xvid4","None"),NULL},
  {1,QT_TRANSLATE_NOOP("xvid4","DCT"),NULL},
  {2,QT_TRANSLATE_NOOP("xvid4","Qpel16"),NULL},
  {3,QT_TRANSLATE_NOOP("xvid4","Qpel8"),NULL},
  {4,QT_TRANSLATE_NOOP("xvid4","Square"),NULL}
};

diaMenuEntry threads[]={
  {1,QT_TRANSLATE_NOOP("xvid4","One thread"),NULL},
  {2,QT_TRANSLATE_NOOP("xvid4","Two threads)"),NULL},
  {3,QT_TRANSLATE_NOOP("xvid4","Three threads"),NULL},
  {99,QT_TRANSLATE_NOOP("xvid4","Auto (#cpu)"),NULL}
};

diaMenuEntry arModeE[]={
  {XVID_PAR_11_VGA,QT_TRANSLATE_NOOP("xvid4","1:1 (PC)"),NULL},
  {XVID_PAR_43_PAL,QT_TRANSLATE_NOOP("xvid4","4:3 (PAL)"),NULL},
  {XVID_PAR_43_NTSC,QT_TRANSLATE_NOOP("xvid4","4:3 (NTSC)"),NULL},
  {XVID_PAR_169_PAL,QT_TRANSLATE_NOOP("xvid4","16:9 (PAL)"),NULL},
  {XVID_PAR_169_NTSC,QT_TRANSLATE_NOOP("xvid4","16:9 (NTSC)"),NULL}
};


#define PX(x) &(xvid4Settings.x)

         diaElemBitrate   bitrate(&(xvid4Settings.params),NULL);
         diaElemMenu      meM(PX(motionEstimation),QT_TRANSLATE_NOOP("xvid4","Motion Estimation"),4,meE);

         diaElemMenu      threadM(PX(nbThreads),QT_TRANSLATE_NOOP("xvid4","Threading"),4,threads);

         diaElemUInteger  qminM(PX(qMin),QT_TRANSLATE_NOOP("xvid4","Mi_n. quantizer:"),1,31);
         diaElemUInteger  qmaxM(PX(qMax),QT_TRANSLATE_NOOP("xvid4","Ma_x. quantizer:"),1,31);
/*
         diaElemUInteger  qdiffM(PX(max_qdiff),QT_TRANSLATE_NOOP("xvid4","Max. quantizer _difference:"),1,31);
*/
         diaElemToggle    trellis(PX(trellis),QT_TRANSLATE_NOOP("xvid4","_Trellis quantization"));         
         diaElemUInteger  max_b_frames(PX(maxBFrame),QT_TRANSLATE_NOOP("xvid4","_Number of B frames:"),0,32);

         diaElemToggle    fdrop(PX(enableFrameDrop),QT_TRANSLATE_NOOP("xvid4","_Drop identical frames (this disables B-frames)"),0);
         diaElemUInteger  fdRatio(PX(frameDropRatio),QT_TRANSLATE_NOOP("xvid4","Framedrop _Ratio:"),1,100);

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
        
        // DIVX or XVID ?
        diaElemToggle fcc(PX(useXvidFCC),QT_TRANSLATE_NOOP("xvid4","Use XVID fcc (else DIVX)"));
        
          /* First Tab : encoding mode */
        diaElemFrame frameBr(QT_TRANSLATE_NOOP("xvid4","Encoding Mode"));
        frameBr.swallow(&profileM);
        frameBr.swallow(&bitrate);

        diaElemFrame frameGop(QT_TRANSLATE_NOOP("xvid4","Frame Settings"));
        frameGop.swallow(&max_b_frames);
        frameGop.swallow(&fdrop);
        frameGop.swallow(&fdRatio);
        frameGop.swallow(&GopSize);

        fdrop.link(0,&max_b_frames);
        fdrop.link(1,&fdRatio);

        diaElemFrame frameMisc(QT_TRANSLATE_NOOP("xvid4","Miscellaneous"));
        frameMisc.swallow(&fcc);

        diaElem *diaEM[]={&frameBr,&frameGop,&frameMisc};
        diaElemTabs tabEM(QT_TRANSLATE_NOOP("xvid4","Encoding Mode"),3,diaEM);

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
        diaElemTabs *tabs[]={&tabEM,&tabQz,&tabThread,&tabAR};
        if( diaFactoryRunTabs(QT_TRANSLATE_NOOP("xvid4","Xvid4 MPEG-4 ASP configuration"),4,tabs))
        {
            if(xvid4Settings.enableFrameDrop)
                xvid4Settings.maxBFrame=0;
            return true;
        }
         return false;
}
