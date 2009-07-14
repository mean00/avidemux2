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
  
  uint32_t vFreq=config->resamplerFrequency;
  uint32_t vChan=config->mixerConf;
  uint32_t vDownsample=config->mixerEnabled;
  uint32_t vFilm=config->film2pal;

   diaElemToggleUint eResample(&vDownsample,QT_TR_NOOP("R_esampling (Hz):"),&vFreq,QT_TR_NOOP("Resampling frequency (Hz)"),6000,64000);
    
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

 diaElemMenu      eMixer(&vChan,QT_TR_NOOP("_Mixer:"),11,menuMixer);
 
 /************************************/
 diaElem *elems[]={&eFPS, &eMixer, &eResample};
  if( diaFactoryRun(QT_TR_NOOP("Audio Filters"),3,elems))
    {
        config->resamplerFrequency=vFreq;
        config->mixerConf=(CHANNEL_CONF)vChan;
        config->mixerEnabled=vDownsample;
        config->film2pal=(FILMCONV)vFilm;
      return true;
    }
    
    return false;
}



// EOF
