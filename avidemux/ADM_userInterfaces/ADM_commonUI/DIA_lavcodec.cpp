//
// C++ Implementation: ADM_vidForcedPP
//
// Description: 
//
//	Force postprocessing assuming constant quant & image type
//	Uselefull on some badly authored DVD for example
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "config.h"
#include "ADM_default.h"


#include "DIA_factory.h"
#include "ADM_lavcodec.h"
#include "ADM_codecs/ADM_ffmpegConfig.h"
#include "ADM_encoder/ADM_vidEncode.hxx"
/**
      \fn getFFCompressParams
      \brief Dialog for lavcodec mpeg4/... codec settings
*/
uint8_t getFFCompressParams(COMPRES_PARAMS *incoming)
{
int ret=0;	
	FFcodecSetting *conf=(FFcodecSetting *)incoming->extraSettings;
	ADM_assert(incoming->extraSettingsLen==sizeof(FFcodecSetting));
#define PX(x) &(conf->x)
         
         
         
diaMenuEntry meE[]={
  {1,QT_TR_NOOP("None")},
  {2,QT_TR_NOOP("Full")},
  {3,QT_TR_NOOP("Log")},
  {4,QT_TR_NOOP("Phods")},
  {5,QT_TR_NOOP("EPZS")},
  {6,QT_TR_NOOP("X1")}
};       

diaMenuEntry qzE[]={
  {0,QT_TR_NOOP("H.263")},
  {1,QT_TR_NOOP("MPEG")}
};       

diaMenuEntry rdE[]={
  {0,QT_TR_NOOP("MB comparison")},
  {1,QT_TR_NOOP("Fewest bits (vhq)")},
  {2,QT_TR_NOOP("Rate distortion")}
};     

uint32_t me=(uint32_t)conf->me_method;  

         diaElemBitrate bitrate(incoming,NULL);
         diaElemMenu      meM(&me,QT_TR_NOOP("Matrices"),4,meE);
         diaElemUInteger  qminM(PX(qmin),QT_TR_NOOP("Mi_n. quantizer:"),1,31);
         diaElemUInteger  qmaxM(PX(qmax),QT_TR_NOOP("Ma_x. quantizer:"),1,31);
         diaElemUInteger  qdiffM(PX(max_qdiff),QT_TR_NOOP("Max. quantizer _difference:"),1,31);
         
         diaElemToggle    fourMv(PX(_4MV),QT_TR_NOOP("4_MV"));
         diaElemToggle    trellis(PX(_TRELLIS_QUANT),QT_TR_NOOP("_Trellis quantization"));
         
         diaElemToggle    qpel(PX(_QPEL),QT_TR_NOOP("_Quarter pixel"));
         diaElemToggle    gmc(PX(_GMC),QT_TR_NOOP("_GMC"));

         
         diaElemUInteger  max_b_frames(PX(max_b_frames),QT_TR_NOOP("_Number of B frames:"),0,32);
         diaElemMenu     qzM(PX(mpeg_quant),QT_TR_NOOP("_Quantization type:"),2,qzE);
         
         diaElemMenu     rdM(PX(mb_eval),QT_TR_NOOP("_Macroblock decision:"),3,rdE);
         
         diaElemUInteger filetol(PX(vratetol),QT_TR_NOOP("_Filesize tolerance (kb):"),0,100000);
         
         diaElemFloat    qzComp(PX(qcompress),QT_TR_NOOP("_Quantizer compression:"),0,1);
         diaElemFloat    qzBlur(PX(qblur),QT_TR_NOOP("Quantizer _blur:"),0,1);
         
         
          /* First Tab : encoding mode */
        diaElem *diamode[]={&bitrate};
        diaElemTabs tabMode(QT_TR_NOOP("User Interface"),1,diamode);
        
        /* 2nd Tab : ME */
        diaElemFrame frameMe(QT_TR_NOOP("Advanced Simple Profile"));
        
        frameMe.swallow(&max_b_frames);
        frameMe.swallow(&qpel);
        frameMe.swallow(&gmc);
        
        diaElem *diaME[]={&fourMv,&frameMe};
        diaElemTabs tabME(QT_TR_NOOP("Motion Estimation"),2,diaME);
        /* 3nd Tab : Qz */
        
         diaElem *diaQze[]={&qzM,&rdM,&qminM,&qmaxM,&qdiffM,&trellis};
        diaElemTabs tabQz(QT_TR_NOOP("Quantization"),6,diaQze);
        
        /* 4th Tab : RControl */
        
         diaElem *diaRC[]={&filetol,&qzComp,&qzBlur};
        diaElemTabs tabRC(QT_TR_NOOP("Rate Control"),3,diaRC);
        
         diaElemTabs *tabs[]={&tabMode,&tabME,&tabQz,&tabRC};
        if( diaFactoryRunTabs(QT_TR_NOOP("libavcodec MPEG-4 configuration"),4,tabs))
	{
          conf->me_method=(Motion_Est_ID)me;
          return 1;
        }
         return 0;
}
// EOF
