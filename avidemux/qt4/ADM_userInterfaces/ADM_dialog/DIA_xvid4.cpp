
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
        MKTOGGLE(QT_TRANSLATE_NOOP("adm","_Interlaced"),	    interlaced);
        MKTOGGLE(QT_TRANSLATE_NOOP("adm","Ca_rtoon mode"),      cartoon);
        MKTOGGLE(QT_TRANSLATE_NOOP("adm","_Greyscale"),         greyscale);
        MKTOGGLE(QT_TRANSLATE_NOOP("adm","Turbo mode"),      turbo);
        MKTOGGLE(QT_TRANSLATE_NOOP("adm","C_hroma optimizer"),         chroma_opt);
        diaElem *main[]={&bitrate,&t_interlaced,&t_cartoon,&t_greyscale,&t_turbo,&t_chroma_opt};
        diaElemTabs tabMain(QT_TRANSLATE_NOOP("adm","Main"),6,main);
         /* Tab 2 motion */
         diaMenuEntry motionMenu[] = {
                             {0,       QT_TRANSLATE_NOOP("adm","None"),NULL}
                            ,{1,      QT_TRANSLATE_NOOP("adm","Very Low"),NULL}
                            ,{2,      QT_TRANSLATE_NOOP("adm","Low"),NULL}
                            ,{3,      QT_TRANSLATE_NOOP("adm","Medium"),NULL}
                            ,{4,      QT_TRANSLATE_NOOP("adm","High"),NULL}
                            ,{5,      QT_TRANSLATE_NOOP("adm","Very High"),NULL}
                            ,{6,      QT_TRANSLATE_NOOP("adm","Ultra High"),NULL}};
        diaElemMenu motion(PX(guiLevel),QT_TRANSLATE_NOOP("adm","Motion Search Precision"),7,motionMenu);
        
         diaMenuEntry vhqMenu[] = {
                             {0,       QT_TRANSLATE_NOOP("adm","Off"),NULL}
                            ,{1,      QT_TRANSLATE_NOOP("adm","Mode Decision"),NULL}
                            ,{2,      QT_TRANSLATE_NOOP("adm","Limited Search"),NULL}
                            ,{3,      QT_TRANSLATE_NOOP("adm","Medium Search"),NULL}
                            ,{4,      QT_TRANSLATE_NOOP("adm","Wide Search"),NULL} };
         diaElemMenu vhq(PX(vhqmode),QT_TRANSLATE_NOOP("adm","VHQ Mode"),5,vhqMenu);
        
        
        /* Tab2-ASP */
          diaElemUInteger  bframe(PX(bframes),QT_TRANSLATE_NOOP("adm","Max B Frames"),0,3);
          diaElemToggle    qpel(PX(qpel),QT_TRANSLATE_NOOP("adm","Quarter Pixel"));
          diaElemToggle    gmc(PX(gmc),QT_TRANSLATE_NOOP("adm","GMC"));
          diaElemToggle    bvhq(PX(bvhq),QT_TRANSLATE_NOOP("adm","BVHQ"));
          diaElemFrame  frameASP(QT_TRANSLATE_NOOP("adm","Advanced Simple Profile"));
          frameASP.swallow(&bframe);
          frameASP.swallow(&qpel);
          frameASP.swallow(&gmc);
          frameASP.swallow(&bvhq);
        
         
          /* Tab 2 motion extra */
            diaElemToggle    inter4mv(PX(inter4mv),QT_TRANSLATE_NOOP("adm","4MV"));
            diaElemToggle    chroma_me(PX(chroma_me),QT_TRANSLATE_NOOP("adm","Chroma ME"));
            diaElemToggle    hqac(PX(chroma_me),QT_TRANSLATE_NOOP("adm","HQ AC"));
          diaElemFrame  frameMore(QT_TRANSLATE_NOOP("adm","More Search"));
          frameMore.swallow(&inter4mv);
          frameMore.swallow(&chroma_me);
          frameMore.swallow(&hqac);
          /* Tab 2 gop size */
          diaElemUInteger  min_key_interval(PX(min_key_interval),QT_TRANSLATE_NOOP("adm","Min Gop Size"),1,900);
          diaElemUInteger  max_key_interval(PX(max_key_interval),QT_TRANSLATE_NOOP("adm","Max Gop Size"),1,900);
            diaElemFrame  frameGop(QT_TRANSLATE_NOOP("adm","GOP Size"));
              frameGop.swallow(&min_key_interval);
              frameGop.swallow(&max_key_interval);
            
           diaElem *motions[]={&motion,&vhq,&frameMore,&frameGop,&frameASP};
          diaElemTabs tabMotion(QT_TRANSLATE_NOOP("adm","Motion"),5,motions);
        /* Tab 3 Qz*/
          diaMenuEntry qzMenu[] = {
                             {0,       QT_TRANSLATE_NOOP("adm","H263"),NULL}
                            ,{1,      QT_TRANSLATE_NOOP("adm","Mpeg"),NULL}}                            ;
           diaElemMenu h263(PX(mpegQuantizer),QT_TRANSLATE_NOOP("adm","Quantization Matrix"),2,qzMenu);
           diaElemToggle    trellis(PX(trellis),QT_TRANSLATE_NOOP("adm","Trellis Quantization"));
           
           
          diaElem *qz[]={&h263,&trellis};
          diaElemTabs tabQz(QT_TRANSLATE_NOOP("adm","Quantization"),2,qz);
          
          /* Tab 4 : 2nd pass */
#define MKENTRY(y,x) diaElemUInteger x(PX(x),y,0,100); frameOne.swallow(&x);
        diaElemFrame  frameOne(QT_TRANSLATE_NOOP("adm","Two Pass Tuning")); 
         
        MKENTRY(QT_TRANSLATE_NOOP("adm","Key Frame Boost(%)"), keyframe_boost);
        
        MKENTRY(QT_TRANSLATE_NOOP("adm","I-frames closer than..."), kfthreshold);
        MKENTRY(QT_TRANSLATE_NOOP("adm",".. are reduced by(%)"), kfreduction);
        MKENTRY(QT_TRANSLATE_NOOP("adm","Max Overflow Improvement(%)"), max_overflow_improvement);
        MKENTRY(QT_TRANSLATE_NOOP("adm","Max Overglow Degradation(%)"), max_overflow_degradation);

#undef MKENTRY
#define MKENTRY(y,x) diaElemUInteger  x(PX(x),y,0,100);frameTwo.swallow(&x);
        diaElemFrame  frameTwo(QT_TRANSLATE_NOOP("adm","Curve Compression"));  

        MKENTRY(QT_TRANSLATE_NOOP("adm","High Bitrate Scenes (%)"), curve_compression_high);
        MKENTRY(QT_TRANSLATE_NOOP("adm","Low Bitrate Scenes (%)"), curve_compression_low);
        MKENTRY(QT_TRANSLATE_NOOP("adm","Overflow Control Strength"), overflow_control_strength);

         diaElem *twopass[]={&frameOne,&frameTwo};
          diaElemTabs tabPass(QT_TRANSLATE_NOOP("adm","Two Pass"),2,twopass);
        /**/
        
        
          
          /* End of tabs */
        diaElemTabs *tabs[4]={&tabMain,&tabMotion,&tabQz,&tabPass};
        if( diaFactoryRunTabs(QT_TRANSLATE_NOOP("adm","Xvid4 Configuration"),4,tabs))
	{
           memcpy(incoming->extraSettings,&localParam,sizeof(localParam));
           return 1;
        }
         return 0;
}
#endif
// EOF
