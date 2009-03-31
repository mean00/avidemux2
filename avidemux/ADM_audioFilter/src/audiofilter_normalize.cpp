/***************************************************************************
                          \file  audiofilter_normalize
                          \brief absolute or automatic gain filter
                             -------------------
    
    copyright            : (C) 2002/2009 by mean
    email                : fixounet@free.fr
    
    Compute the ratio so that the maximum is at -3db
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <math.h>

#include "ADM_default.h"
#include "ADM_audioFilter.h"
#include "audiofilter_normalize_param.h"
#include "audiofilter_normalize.h"
#include "audiofilter_dolby.h"

#include "ADM_coreAudio.h"
//#include "ADM_dialog/DIA_busy.h" //CANNOT USE IT! We are in another thread!

//extern AVDMGenericAudioStream *currentaudiostream;

#ifdef __WIN32
#define POW10(x)   pow(10,x)
#elif defined(ADM_BSD_FAMILY)
#define POW10(x) powf(10.0,x)
#else
#define POW10(x)  pow10f(x)
#endif

#define LINEAR_TO_DB(x) (20.*log10(x))
#define DB_TO_LINEAR(x) (POW10((x/20.)))

// Ctor
//__________

AUDMAudioFilterNormalize::AUDMAudioFilterNormalize(AUDMAudioFilter * instream,GAINparam *param):AUDMAudioFilter (instream)
{
  float db_out;
    // nothing special here...
  switch(param->mode)
  {
    case ADM_NO_GAIN: _ratio=1;_scanned=1;printf("[Gain] Gain of 1.0\n");break; 
    case ADM_GAIN_AUTOMATIC: _ratio=1;_scanned=0;printf("[Gain] Automatic gain\n");break;
    case ADM_GAIN_MANUAL: 
                _scanned=1;
                db_out =  param->gain10/10.0; // Dbout is in 10*DB (!)
                _ratio = DB_TO_LINEAR(db_out);
                printf("[Gain] %f db (p=%d)\n", (float)(param->gain10)/10.,param->gain10);
                printf("[Gain] Linear ratio of : %03.3f\n", _ratio);
  }
    _previous->rewind();
};

AUDMAudioFilterNormalize::~AUDMAudioFilterNormalize()
{

}
//
// For normalize, we scan the input stream
// to check for maximum value
//___________________________________________
uint8_t AUDMAudioFilterNormalize::preprocess(void)
{

    int16_t *ptr;
    uint32_t scanned = 0, ch = 0;
    AUD_Status status;
    _ratio = 0;

    uint32_t percent=0;
    uint32_t current=0,llength=0;
    float max[_wavHeader.channels];
    _previous->rewind();
    DolbySkip(1);
    printf("\n Seeking for maximum value, that can take a while\n");

    llength=_length ;
    

      for(int i=0;i<_wavHeader.channels;i++) max[i]=0;
      while (1)
      {
          int ready=_previous->fill(AUD_PROCESS_BUFFER_SIZE>>2,_incomingBuffer,&status);
          if(!ready)
          {
            if(status==AUD_END_OF_STREAM) 
            {
              break; 
            }
           else 
            {
              printf("Unknown cause : %d\n",status);
              ADM_assert(0); 
            }
          }
          ADM_assert(!(ready %_wavHeader.channels));
          
          int index=0;
          float current;
          
        //  printf("*\n");
          int sample= ready /_wavHeader.channels;
          for(int j=0;j<sample;j++)
            for(int chan=0;chan<_wavHeader.channels;chan++)
          {
            current=fabs(_incomingBuffer[index++]);
            if(current>max[chan]) max[chan]=current;
          }
          scanned+=ready;
      }
      
      
      

    _previous->rewind();
    float mx=0;
    for(int chan=0;chan<_wavHeader.channels;chan++)
    {
        if(max[chan]>mx) mx=max[chan];
        printf("[Normalize] maximum found for channel %d : %f\n", chan,max[chan]);
    }
    printf("[Normalize] Using : %0.4f as max value \n", mx);
    double db_in, db_out=-3;

    if (mx>0.001)
      db_in = LINEAR_TO_DB(mx);
    else
      db_in = -20; // We consider -20 DB to be noise

    printf("--> %2.2f db / %2.2f \n", db_in, db_out);

    // search ratio
    _ratio=1;

    float db_delta=db_out-db_in;
    printf("[Normalize]Gain %f dB\n",db_delta);
    _ratio = DB_TO_LINEAR(db_delta);
    printf("\n Using ratio of : %f\n", _ratio);

    _scanned = 1;
    DolbySkip(0);
    _previous->rewind();
    return 1;
}
//
//___________________________________________
uint32_t AUDMAudioFilterNormalize::fill( uint32_t max, float * buffer,AUD_Status *status)
{
    uint32_t rd, i, j,rd2;

    *status=AUD_OK;
    if(!_scanned) preprocess();
    rd = _previous->fill(max, _incomingBuffer,status);
    if(!rd)
    {
      if(*status==AUD_END_OF_STREAM) return 0;
      ADM_assert(0);
    }
    float *in,*out,tmp;
    for (i = 0; i < rd; i++)
    {
      tmp=_incomingBuffer[i];
      tmp*=_ratio;
      buffer[i]=tmp;
    }
    return rd;
};
//EOF

