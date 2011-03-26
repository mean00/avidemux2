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
#include "x264.h"

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


diaMenuEntry threads[]={
  {1,QT_TR_NOOP("One thread")},
  {2,QT_TR_NOOP("Two threads)")},
  {3,QT_TR_NOOP("Three threads")},
  {99,QT_TR_NOOP("Auto (#cpu)")}
};     
#define PX(x) &(x264Settings.x)
        // Main
        int nbPreset=sizeof(x264_preset_names)/sizeof(const char *);
        int nbTunes=sizeof(x264_tune_names)/sizeof(const char *);
        

#define MKMENUENTRY(N,out,array) diaMenuEntry out[N]; \
        for(int i=0;i<N;i++) {out[i].val=i;out[i].text=array[i];out[i].desc=NULL;}

        MKMENUENTRY(nbPreset,presetEntries,x264_preset_names);
        MKMENUENTRY(nbTunes,tuneEntries,x264_tune_names);
        
        
        diaElemMenu      presetMenu(PX(preset),QT_TR_NOOP("Preset"),nbPreset,presetEntries);
        diaElemMenu      tuneMenu(PX(tune),QT_TR_NOOP("Tune"),nbTunes,tuneEntries);
        
        uint32_t presetToggle=(uint32_t)x264Settings.usePreset;
        uint32_t tuneToggle=(uint32_t)x264Settings.useTune;
        diaElemToggle    usePreset(&presetToggle,QT_TR_NOOP("Use preset"));         
        diaElemToggle    useTune(&tuneToggle,    QT_TR_NOOP("Use tune"));         

         uint32_t trelBol=*PX(Trellis);
         uint32_t cabacBol=*PX(CABAC);

         diaElemToggle    trellis(&trelBol,QT_TR_NOOP("_Trellis quantization"));         
         diaElemToggle    cabac(&cabacBol,QT_TR_NOOP("Cabac encoding"));         

         diaElemUInteger  GopSize(PX(MaxIdr),QT_TR_NOOP("_Gop Size:"),1,500); 
         diaElemUInteger  max_b_frames(PX(MaxBFrame),QT_TR_NOOP("Max B Frames:"),0,5); 
         diaElemBitrate   bitrate(&(x264Settings.params),NULL);
         diaElemMenu      threadM(PX(threads),QT_TR_NOOP("Threading"),4,threads);

         diaElemUInteger  profile(PX(profile),QT_TR_NOOP("Profile:"),10,50); 

        //-------------------

          /* First Tab : encoding mode */
        diaElem *diaMain[]={&usePreset,&presetMenu,&useTune,&tuneMenu,&profile};
        diaElemTabs tabMain(QT_TR_NOOP("Main"),5,diaMain);

        
         diaElemFrame frameMe(QT_TR_NOOP("Main"));
        frameMe.swallow(&max_b_frames);
        frameMe.swallow(&GopSize);
        frameMe.swallow(&bitrate);
       
        
        diaElem *diaME[]={&frameMe};
        diaElemTabs tabME(QT_TR_NOOP("Encoding mode"),1,diaME);

        /* 2nd Tab : Qz */
       
        diaElem *diaQze[]={&trellis,&cabac};
        diaElemTabs tabQz(QT_TR_NOOP("Quantization"),2,diaQze);
        
        /* 3th Tab : thread */
         diaElem *diaThread[]={&threadM};
         diaElemTabs tabThread(QT_TR_NOOP("Threads"),1,diaThread);
      
        diaElemTabs *tabs[]={&tabMain,&tabME,&tabQz,&tabThread};

       // usePreset.link(0,&trellis);
        //usePreset.link(0,&cabac);

        usePreset.link(1,&presetMenu);
        useTune.link(1,&tuneMenu);

        if( diaFactoryRunTabs(QT_TR_NOOP("X264 MPEG-4 AVC configuration"),4,tabs))
        {
            *PX(Trellis)= trelBol;
            *PX(CABAC)= cabacBol;
            *PX(usePreset)= presetToggle;
            *PX(useTune)= tuneToggle;
            return true;
        }

         return false;
}