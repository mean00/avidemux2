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
  ELEM_TYPE_FLOAT vGainValue=config->gainParam.gain10/10;

#define PX(x) (&(config->x))
   diaElemToggleUint eResample(PX(resamplerEnabled),QT_TR_NOOP("R_esampling (Hz):"),PX(resamplerFrequency),QT_TR_NOOP("Resampling frequency (Hz)"),6000,64000);
    
    //**********************************
    diaMenuEntry menuFPS[]={
  {FILMCONV_NONE,     QT_TR_NOOP("None")},
  {FILMCONV_FILM2PAL, QT_TR_NOOP("Film to PAL")},
  {FILMCONV_PAL2FILM, QT_TR_NOOP("PAL to Film")}
    };
  
   diaElemMenu      eFPS(&vFilm,QT_TR_NOOP("_Frame rate change:"),3,menuFPS);

   //**********************************
    diaMenuEntry menuMixer[]={
  {CHANNEL_INVALID,     QT_TR_NOOP("No change")},
  {CHANNEL_MONO,        QT_TR_NOOP("Mono")},
  {CHANNEL_STEREO,      QT_TR_NOOP("Stereo")},
  {CHANNEL_2F_1R,       QT_TR_NOOP("Stereo+surround")},
  {CHANNEL_3F,          QT_TR_NOOP("Stereo+center")},
  {CHANNEL_3F_1R,           QT_TR_NOOP("Stereo+center+surround")},
  {CHANNEL_2F_2R,           QT_TR_NOOP("Stereo front+stereo rear")},
  {CHANNEL_3F_2R,           QT_TR_NOOP("5 channels")},
  {CHANNEL_3F_2R_LFE,       QT_TR_NOOP("5.1")},
  {CHANNEL_DOLBY_PROLOGIC,  QT_TR_NOOP("Dolby Pro Logic")},
  {CHANNEL_DOLBY_PROLOGIC2, QT_TR_NOOP("Dolby Pro Logic II")}
    };
  //*************************
  diaMenuEntry menuGain[]={
  {ADM_NO_GAIN,       QT_TR_NOOP("None")},
  {ADM_GAIN_AUTOMATIC,QT_TR_NOOP("Automatic (max -3 dB)")},
  {ADM_GAIN_MANUAL,   QT_TR_NOOP("Manual (dB)")}};
  
   diaElemMenu      eGain(&vGainMode,QT_TR_NOOP("_Gain mode:"),3,menuGain);
   
    diaElemFloat  eGainValue(&vGainValue,QT_TR_NOOP("G_ain value:"),-10,40);
     eGain.link(&(menuGain[2]),1,&eGainValue);
  //****************************
 diaElemMenu      eMixer(&vChan,QT_TR_NOOP("_Mixer:"),11,menuMixer);
 bool bMixer=config->mixerEnabled;
 diaElemToggle    tMixer(&bMixer,QT_TR_NOOP("Remix:"));
 tMixer.link(1,&eMixer);
 /************************************/
 diaElem *elems[]={&eFPS, &tMixer,&eMixer, &eResample,&eGain,&eGainValue};
  if( diaFactoryRun(QT_TR_NOOP("Audio Filters"),4+2,elems))
    {
        config->mixerConf=(CHANNEL_CONF)vChan;
        config->film2pal=(FILMCONV)vFilm;
        config->gainParam.mode=(ADM_GAINMode)vGainMode;
        config->gainParam.gain10=vGainValue*10;
        config->mixerEnabled=bMixer;
      return true;
    }
    
    return false;
}



// EOF
