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
#include "ADM_audiofilter/audiofilter_normalize_param.h"
#include "ADM_audiofilter/audioeng_buildfilters.h"
/**
    \fn DIA_getAudioFilter
    \brief Dialog to manage audio filters
*/
int DIA_getAudioFilter(GAINparam *gain,
                        RESAMPLING *downsamplingmethod, 
                        int *tshifted,
                        int *shiftvalue, 
                        int *drc,
                        int *freqvalue,
                        FILMCONV *filmconv,
                        CHANNEL_CONF *channel)
{
  
  uint32_t vDRC=*drc;
  uint32_t vTshift=*tshifted;
  int32_t vTshiftValue=*shiftvalue;
  uint32_t vFreq=*freqvalue;
  uint32_t vChan=(uint32_t)*channel;
  uint32_t vDownsample=(uint32_t )*downsamplingmethod;
  uint32_t vFilm=(uint32_t )*filmconv;
  uint32_t vGainMode=(uint32_t)gain->mode;
  ELEM_TYPE_FLOAT vGainValue=gain->gain10;
  vGainValue/=10.;
  //**********************************
       
   diaElemToggle    eDRC(&vDRC,QT_TR_NOOP("_Dynamic range compression"));
   
   diaElemToggleInt eTimeShift(&vTshift,QT_TR_NOOP("_Time shift (ms):"),&vTshiftValue,QT_TR_NOOP("Time shift value (ms)"),-1000*100,1000*100);
   
//    diaElemToggle    eTimeShift(&vTshift,QT_TR_NOOP("Enable _time shift"));
//    diaElemInteger  eShift(&vTshiftValue,QT_TR_NOOP("Time shift _value (ms):"),-10000,10000);
//    
//    eTimeShift.link(1,&eShift);
//   
    //**********************************
   diaElemToggleUint eResample(&vDownsample,QT_TR_NOOP("R_esampling (Hz):"),&vFreq,QT_TR_NOOP("Resampling frequency (Hz)"),6000,64000);
    //diaElemToggle      eResample(&vDownsample,QT_TR_NOOP("R_esampling (Hz)"));
    //diaElemUInteger  eResampleValue(&vFreq,QT_TR_NOOP("_Resampling frequency (Hz):"),6000,64000);
  
//    eResample.link(1,&eResampleValue);
    
    //**********************************
    diaMenuEntry menuFPS[]={
  {FILMCONV_NONE,     QT_TR_NOOP("None")},
  {FILMCONV_FILM2PAL, QT_TR_NOOP("Film to PAL")},
  {FILMCONV_PAL2FILM, QT_TR_NOOP("PAL to Film")}
    };
  
   diaElemMenu      eFPS(&vFilm,QT_TR_NOOP("_Frame rate change:"),3,menuFPS);

   //**********************************
    diaMenuEntry menuGain[]={
  {ADM_NO_GAIN,       QT_TR_NOOP("None")},
  {ADM_GAIN_AUTOMATIC,QT_TR_NOOP("Automatic (max -3 dB)")},
  {ADM_GAIN_MANUAL,   QT_TR_NOOP("Manual")}};
  
   diaElemMenu      eGain(&vGainMode,QT_TR_NOOP("_Gain mode:"),3,menuGain);
   
    diaElemFloat  eGainValue(&vGainValue,QT_TR_NOOP("G_ain value:"),-10,10);
     eGain.link(&(menuGain[2]),1,&eGainValue);
   diaElemFrame frameGain(QT_TR_NOOP("Gain"));   
    frameGain.swallow(&eGain);
    frameGain.swallow(&eGainValue);
  //********************************
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
 diaElem *elems[]={&eFPS, &eMixer, &eTimeShift,  &eResample, &eDRC, &frameGain};
  if( diaFactoryRun(QT_TR_NOOP("Audio Filters"),6,elems))
    {
        *drc=vDRC;
        *tshifted=vTshift;
        *shiftvalue=vTshiftValue;
        *freqvalue=vFreq;
        *channel=(CHANNEL_CONF)vChan;
        *downsamplingmethod=(RESAMPLING)vDownsample;
        *filmconv=(FILMCONV)vFilm;
        gain->mode=(ADM_GAINMode)vGainMode;
        gain->gain10=(uint32_t)(10.*vGainValue);
      return 1;
    }
    
    return 0;
 
}



// EOF
