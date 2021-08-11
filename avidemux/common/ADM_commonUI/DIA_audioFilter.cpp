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


#include "ADM_default.h"
#include "audiofilter_conf.h"
#include "DIA_factory.h"
/**
    \fn DIA_getAudioFilter
    \brief Dialog to manage audio filters
*/
int DIA_getAudioFilter(ADM_AUDIOFILTER_CONFIG *config)
{
  uint32_t vChan=config->mixerConf;
  uint32_t vFilm=config->film2pal;
  uint32_t vGainMode=(uint32_t)config->gainParam.mode;
  int32_t  vShift=config->shiftInMs;
  uint32_t bShiftEnabled=config->shiftEnabled;
  ELEM_TYPE_FLOAT vGainValue=config->gainParam.gain10/10.;
  ELEM_TYPE_FLOAT vGainMaxLevel=config->gainParam.maxlevel10/10.;

#define PX(x) (&(config->x))
   diaElemToggleUint eResample(PX(resamplerEnabled),QT_TRANSLATE_NOOP("adm","R_esampling (Hz):"),PX(resamplerFrequency),QT_TRANSLATE_NOOP("adm","Resampling frequency (Hz)"),6000,64000);
    
    //**********************************
    diaMenuEntry menuFPS[]={
  {FILMCONV_NONE,     QT_TRANSLATE_NOOP("adm","None"), NULL},
  {FILMCONV_FILM2PAL, QT_TRANSLATE_NOOP("adm","Film to PAL"), NULL},
  {FILMCONV_PAL2FILM, QT_TRANSLATE_NOOP("adm","PAL to Film"), NULL}
    };
  
   diaElemMenu      eFPS(&vFilm,QT_TRANSLATE_NOOP("adm","_Frame rate change:"),3,menuFPS);

   //**********************************
    diaMenuEntry menuMixer[]={
  {CHANNEL_INVALID,     QT_TRANSLATE_NOOP("adm","No change"), NULL},
  {CHANNEL_MONO,        QT_TRANSLATE_NOOP("adm","Mono"), NULL},
  {CHANNEL_STEREO,      QT_TRANSLATE_NOOP("adm","Stereo"), NULL},
  {CHANNEL_2F_1R,       QT_TRANSLATE_NOOP("adm","Stereo+surround"), NULL},
  {CHANNEL_3F,          QT_TRANSLATE_NOOP("adm","Stereo+center"), NULL},
  {CHANNEL_3F_1R,           QT_TRANSLATE_NOOP("adm","Stereo+center+surround"), NULL},
  {CHANNEL_2F_2R,           QT_TRANSLATE_NOOP("adm","Stereo front+stereo rear"), NULL},
  {CHANNEL_3F_2R,           QT_TRANSLATE_NOOP("adm","5 channels"), NULL},
  {CHANNEL_3F_2R_LFE,       QT_TRANSLATE_NOOP("adm","5.1"), NULL},
  {CHANNEL_DOLBY_PROLOGIC,  QT_TRANSLATE_NOOP("adm","Dolby Pro Logic"), NULL},
  {CHANNEL_DOLBY_PROLOGIC2, QT_TRANSLATE_NOOP("adm","Dolby Pro Logic II"), NULL}
    };
  //*************************
    diaElemToggle    tDRC(PX(drcEnabled),QT_TRANSLATE_NOOP("adm","DRC"));
//*************************
  diaMenuEntry menuGain[]={
  {ADM_NO_GAIN,       QT_TRANSLATE_NOOP("adm","None"), NULL},
  {ADM_GAIN_AUTOMATIC,QT_TRANSLATE_NOOP("adm","Automatic"), NULL},
  {ADM_GAIN_MANUAL,   QT_TRANSLATE_NOOP("adm","Manual (dB)"), NULL}
    };

  diaElemFrame  frameGain(QT_TRANSLATE_NOOP("adm","Gain"));
  diaElemMenu   eGain(&vGainMode,QT_TRANSLATE_NOOP("adm","_Gain mode:"),3,menuGain);   
  diaElemFloat  eGainValue(&vGainValue,QT_TRANSLATE_NOOP("adm","G_ain value:"),-10,40);
  diaElemFloat  eGainMaxLevel(&vGainMaxLevel,QT_TRANSLATE_NOOP("adm","_Maximum value:"),-10,0);
  
  eGain.link(&(menuGain[2]),1,&eGainValue);
  eGain.link(&(menuGain[1]),1,&eGainMaxLevel);
  frameGain.swallow(&eGain);
  frameGain.swallow(&eGainValue);
  frameGain.swallow(&eGainMaxLevel);
  //****************************

 diaElemMenu      eMixer(&vChan,QT_TRANSLATE_NOOP("adm","_Mixer:"),11,menuMixer);
 bool bMixer=config->mixerEnabled;
 diaElemToggle    tMixer(&bMixer,QT_TRANSLATE_NOOP("adm","Remix:"));
 tMixer.link(1,&eMixer);
 
 diaElemFrame frameMixer(QT_TRANSLATE_NOOP("adm","Mixer"));
 frameMixer.swallow(&tMixer);
 frameMixer.swallow(&eMixer);
 
 //****************************
 diaElemToggleInt eShift(&bShiftEnabled,QT_TRANSLATE_NOOP("adm","Shift audio:"),&vShift, QT_TRANSLATE_NOOP("adm","Shift Value (ms):"),-30000,30000);
 /************************************/
 diaElem *elems[]={&eFPS, &tDRC, &eResample,&eShift,&frameMixer,&frameGain};
  if( diaFactoryRun(QT_TRANSLATE_NOOP("adm","Audio Filters"),4+2,elems))
    {
        config->mixerConf=(CHANNEL_CONF)vChan;
        config->film2pal=(FILMCONV)vFilm;
        config->gainParam.mode=(ADM_GAINMode)vGainMode;
        config->gainParam.gain10=vGainValue*10;
        config->gainParam.maxlevel10=vGainMaxLevel*10;
        config->mixerEnabled=bMixer;
	config->shiftInMs=vShift;
        config->shiftEnabled=bShiftEnabled;
        
      return true;
    }
    
    return false;
}



// EOF
