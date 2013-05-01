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
  ELEM_TYPE_FLOAT vGainValue=config->gainParam.gain10/10;

#define PX(x) (&(config->x))
   diaElemToggleUint eResample(PX(resamplerEnabled),QT_TRANSLATE_NOOP("adm","R_esampling (Hz):"),PX(resamplerFrequency),QT_TRANSLATE_NOOP("adm","Resampling frequency (Hz)"),6000,64000);
    
    //**********************************
    diaMenuEntry menuFPS[]={
  {FILMCONV_NONE,     QT_TRANSLATE_NOOP("adm","None")},
  {FILMCONV_FILM2PAL, QT_TRANSLATE_NOOP("adm","Film to PAL")},
  {FILMCONV_PAL2FILM, QT_TRANSLATE_NOOP("adm","PAL to Film")}
    };
  
   diaElemMenu      eFPS(&vFilm,QT_TRANSLATE_NOOP("adm","_Frame rate change:"),3,menuFPS);

   //**********************************
    diaMenuEntry menuMixer[]={
  {CHANNEL_INVALID,     QT_TRANSLATE_NOOP("adm","No change")},
  {CHANNEL_MONO,        QT_TRANSLATE_NOOP("adm","Mono")},
  {CHANNEL_STEREO,      QT_TRANSLATE_NOOP("adm","Stereo")},
  {CHANNEL_2F_1R,       QT_TRANSLATE_NOOP("adm","Stereo+surround")},
  {CHANNEL_3F,          QT_TRANSLATE_NOOP("adm","Stereo+center")},
  {CHANNEL_3F_1R,           QT_TRANSLATE_NOOP("adm","Stereo+center+surround")},
  {CHANNEL_2F_2R,           QT_TRANSLATE_NOOP("adm","Stereo front+stereo rear")},
  {CHANNEL_3F_2R,           QT_TRANSLATE_NOOP("adm","5 channels")},
  {CHANNEL_3F_2R_LFE,       QT_TRANSLATE_NOOP("adm","5.1")},
  {CHANNEL_DOLBY_PROLOGIC,  QT_TRANSLATE_NOOP("adm","Dolby Pro Logic")},
  {CHANNEL_DOLBY_PROLOGIC2, QT_TRANSLATE_NOOP("adm","Dolby Pro Logic II")}
    };
  //*************************
    diaElemToggle    tDRC(PX(drcEnabled),QT_TRANSLATE_NOOP("adm","DRC"));
//*************************
  diaMenuEntry menuGain[]={
  {ADM_NO_GAIN,       QT_TRANSLATE_NOOP("adm","None")},
  {ADM_GAIN_AUTOMATIC,QT_TRANSLATE_NOOP("adm","Automatic (max -3 dB)")},
  {ADM_GAIN_MANUAL,   QT_TRANSLATE_NOOP("adm","Manual (dB)")}};
  
   diaElemMenu      eGain(&vGainMode,QT_TRANSLATE_NOOP("adm","_Gain mode:"),3,menuGain);
   
    diaElemFloat  eGainValue(&vGainValue,QT_TRANSLATE_NOOP("adm","G_ain value:"),-10,40);
     eGain.link(&(menuGain[2]),1,&eGainValue);
  //****************************

 diaElemMenu      eMixer(&vChan,QT_TRANSLATE_NOOP("adm","_Mixer:"),11,menuMixer);
 bool bMixer=config->mixerEnabled;
 diaElemToggle    tMixer(&bMixer,QT_TRANSLATE_NOOP("adm","Remix:"));
 tMixer.link(1,&eMixer);
 //****************************
 diaElemToggleInt eShift(&bShiftEnabled,QT_TRANSLATE_NOOP("adm","Shift audio:"),&vShift, QT_TRANSLATE_NOOP("adm","Shift Value (ms):"),-30000,30000);
 /************************************/
 diaElem *elems[]={&eFPS, &tDRC,&tMixer,&eMixer, &eResample,&eGain,&eGainValue,&eShift};
  if( diaFactoryRun(QT_TRANSLATE_NOOP("adm","Audio Filters"),4+4,elems))
    {
        config->mixerConf=(CHANNEL_CONF)vChan;
        config->film2pal=(FILMCONV)vFilm;
        config->gainParam.mode=(ADM_GAINMode)vGainMode;
        config->gainParam.gain10=vGainValue*10;
        config->mixerEnabled=bMixer;
	config->shiftInMs=vShift;
        config->shiftEnabled=bShiftEnabled;
        
      return true;
    }
    
    return false;
}



// EOF
