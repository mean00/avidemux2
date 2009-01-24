
// Author: mean <fixounet@free.fr>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "config.h"

#ifdef USE_XVID_4
#include "ADM_default.h"

#include "DIA_factory.h"
#include "ADM_encoder/ADM_vidEncode.hxx"
#include "ADM_codecs/ADM_xvid4param.h"
/**
      \fn getFFCompressParams
      \brief Dialog for lavcodec mpeg4/... codec settings
*/
uint8_t DIA_xvid4(COMPRES_PARAMS *incoming)
{
int b;
int ret=0;
int code;
      xvid4EncParam localParam;
      ADM_assert(incoming->extraSettingsLen==sizeof(localParam));
      memcpy(&localParam,incoming->extraSettings,sizeof(localParam));
#define PX(x) &(localParam.x)
         // Our tabs
         /* Tab 1 main */
           diaElemBitrate bitrate(incoming,NULL);
		   bitrate.setMinQz(1);
#define MKTOGGLE(y,x)           diaElemToggle  t_##x(PX(x),y)
        MKTOGGLE(QT_TR_NOOP("_Interlaced"),	    interlaced);
        MKTOGGLE(QT_TR_NOOP("Ca_rtoon mode"),      cartoon);
        MKTOGGLE(QT_TR_NOOP("_Greyscale"),         greyscale);
        MKTOGGLE(QT_TR_NOOP("Turbo mode"),      turbo);
        MKTOGGLE(QT_TR_NOOP("C_hroma optimizer"),         chroma_opt);
        diaElem *main[]={&bitrate,&t_interlaced,&t_cartoon,&t_greyscale,&t_turbo,&t_chroma_opt};
        diaElemTabs tabMain(QT_TR_NOOP("Main"),6,main);
         /* Tab 2 motion */
         diaMenuEntry motionMenu[] = {
                             {0,       QT_TR_NOOP("None"),NULL}
                            ,{1,      QT_TR_NOOP("Very Low"),NULL}
                            ,{2,      QT_TR_NOOP("Low"),NULL}
                            ,{3,      QT_TR_NOOP("Medium"),NULL}
                            ,{4,      QT_TR_NOOP("High"),NULL}
                            ,{5,      QT_TR_NOOP("Very High"),NULL}
                            ,{6,      QT_TR_NOOP("Ultra High"),NULL}};
        diaElemMenu motion(PX(guiLevel),QT_TR_NOOP("Motion Search Precision"),7,motionMenu);
        
         diaMenuEntry vhqMenu[] = {
                             {0,       QT_TR_NOOP("Off"),NULL}
                            ,{1,      QT_TR_NOOP("Mode Decision"),NULL}
                            ,{2,      QT_TR_NOOP("Limited Search"),NULL}
                            ,{3,      QT_TR_NOOP("Medium Search"),NULL}
                            ,{4,      QT_TR_NOOP("Wide Search"),NULL} };
         diaElemMenu vhq(PX(vhqmode),QT_TR_NOOP("VHQ Mode"),5,vhqMenu);
        
        
        /* Tab2-ASP */
          diaElemUInteger  bframe(PX(bframes),QT_TR_NOOP("Max B Frames"),0,3);
          diaElemToggle    qpel(PX(qpel),QT_TR_NOOP("Quarter Pixel"));
          diaElemToggle    gmc(PX(gmc),QT_TR_NOOP("GMC"));
          diaElemToggle    bvhq(PX(bvhq),QT_TR_NOOP("BVHQ"));
          diaElemFrame  frameASP(QT_TR_NOOP("Advanced Simple Profile"));
          frameASP.swallow(&bframe);
          frameASP.swallow(&qpel);
          frameASP.swallow(&gmc);
          frameASP.swallow(&bvhq);
        
         
          /* Tab 2 motion extra */
            diaElemToggle    inter4mv(PX(inter4mv),QT_TR_NOOP("4MV"));
            diaElemToggle    chroma_me(PX(chroma_me),QT_TR_NOOP("Chroma ME"));
            diaElemToggle    hqac(PX(chroma_me),QT_TR_NOOP("HQ AC"));
          diaElemFrame  frameMore(QT_TR_NOOP("More Search"));
          frameMore.swallow(&inter4mv);
          frameMore.swallow(&chroma_me);
          frameMore.swallow(&hqac);
          /* Tab 2 gop size */
          diaElemUInteger  min_key_interval(PX(min_key_interval),QT_TR_NOOP("Min Gop Size"),1,900);
          diaElemUInteger  max_key_interval(PX(max_key_interval),QT_TR_NOOP("Max Gop Size"),1,900);
            diaElemFrame  frameGop(QT_TR_NOOP("GOP Size"));
              frameGop.swallow(&min_key_interval);
              frameGop.swallow(&max_key_interval);
            
           diaElem *motions[]={&motion,&vhq,&frameMore,&frameGop,&frameASP};
          diaElemTabs tabMotion(QT_TR_NOOP("Motion"),5,motions);
        /* Tab 3 Qz*/
          diaMenuEntry qzMenu[] = {
                             {0,       QT_TR_NOOP("H263"),NULL}
                            ,{1,      QT_TR_NOOP("Mpeg"),NULL}}                            ;
           diaElemMenu h263(PX(mpegQuantizer),QT_TR_NOOP("Quantization Matrix"),2,qzMenu);
           diaElemToggle    trellis(PX(trellis),QT_TR_NOOP("Trellis Quantization"));
           
           
          diaElem *qz[]={&h263,&trellis};
          diaElemTabs tabQz(QT_TR_NOOP("Quantization"),2,qz);
          
          /* Tab 4 : 2nd pass */
#define MKENTRY(y,x) diaElemUInteger x(PX(x),y,0,100); frameOne.swallow(&x);
        diaElemFrame  frameOne(QT_TR_NOOP("Two Pass Tuning")); 
         
        MKENTRY(QT_TR_NOOP("Key Frame Boost(%)"), keyframe_boost);
        
        MKENTRY(QT_TR_NOOP("I-frames closer than..."), kfthreshold);
        MKENTRY(QT_TR_NOOP(".. are reduced by(%)"), kfreduction);
        MKENTRY(QT_TR_NOOP("Max Overflow Improvement(%)"), max_overflow_improvement);
        MKENTRY(QT_TR_NOOP("Max Overglow Degradation(%)"), max_overflow_degradation);

#undef MKENTRY
#define MKENTRY(y,x) diaElemUInteger  x(PX(x),y,0,100);frameTwo.swallow(&x);
        diaElemFrame  frameTwo(QT_TR_NOOP("Curve Compression"));  

        MKENTRY(QT_TR_NOOP("High Bitrate Scenes (%)"), curve_compression_high);
        MKENTRY(QT_TR_NOOP("Low Bitrate Scenes (%)"), curve_compression_low);
        MKENTRY(QT_TR_NOOP("Overflow Control Strength"), overflow_control_strength);

         diaElem *twopass[]={&frameOne,&frameTwo};
          diaElemTabs tabPass(QT_TR_NOOP("Two Pass"),2,twopass);
        /**/
        
        
          
          /* End of tabs */
        diaElemTabs *tabs[4]={&tabMain,&tabMotion,&tabQz,&tabPass};
        if( diaFactoryRunTabs(QT_TR_NOOP("Xvid4 Configuration"),4,tabs))
	{
           memcpy(incoming->extraSettings,&localParam,sizeof(localParam));
           return 1;
        }
         return 0;
}
#endif
// EOF
