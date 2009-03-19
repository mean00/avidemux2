 
/***************************************************************************
    copyright            : (C) 2002-6 by mean
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
#include "ADM_coreAudio.h"
#include "ADM_audioFilter.h"
#include "audioencoder.h"
#include "ADM_audioCodecEnum.h"
AUDMEncoder::AUDMEncoder(AUDMAudioFilter *in) // :AVDMGenericAudioStream  ()
{
  _wavheader = new WAVHeader;
  _incoming=in;
  memcpy(_wavheader, _incoming->getInfo(), sizeof(WAVHeader));
  _wavheader->encoding=WAV_AAC;
  _incoming->rewind();	// rewind
  _extraData=NULL;
  _extraSize=0;
  tmpbuffer=new float[_wavheader->frequency*_wavheader->channels];
  tmphead=tmptail=0;
  eof_met=0;
  
};
/********************/
AUDMEncoder::~AUDMEncoder()
{
  cleanup();
};
/********************/
uint8_t AUDMEncoder::cleanup(void)
{
  if(_wavheader) delete(_wavheader);
  _wavheader=NULL;

  if(_extraData) delete [] _extraData;
  _extraData=NULL;
  
  if(tmpbuffer) delete [] tmpbuffer;
  tmpbuffer=NULL;
};
/********************/

uint8_t AUDMEncoder::refillBuffer(int minimum)
{
  uint32_t filler=_wavheader->frequency*_wavheader->channels;
  uint32_t nb;
  AUD_Status status;
  if(eof_met) return 0;
  while(1)
  {
    ADM_assert(tmptail>=tmphead);
    if((tmptail-tmphead)>=minimum) return 1;
  
    if(tmphead && tmptail>filler/2)
    {
      memmove(&tmpbuffer[0],&tmpbuffer[tmphead],(tmptail-tmphead)*sizeof(float)); 
      tmptail-=tmphead;
      tmphead=0;
    }
    ADM_assert(filler>tmptail);
    nb=_incoming->fill( (filler-tmptail)/2,&tmpbuffer[tmptail],&status);
    if(!nb)
    {
      if(status!=AUD_END_OF_STREAM) ADM_assert(0);
      
      if((tmptail-tmphead)<minimum)
      {
        memset(&tmpbuffer[tmptail],0,sizeof(float)*(minimum-(tmptail-tmphead)));
        tmptail=tmphead+minimum;
        eof_met=1;  
        return minimum;
      }
      else continue;
    } else
      tmptail+=nb;
  }
}


/**
 * 	\fn reorderChannels
 *  \brief Reorder the channels
 */
void AUDMEncoder::reorderChannels(float *data, uint32_t nb,CHANNEL_TYPE *input,CHANNEL_TYPE *output)
{
	float tmp [_wavheader->channels];
	static uint8_t reorder[MAX_CHANNELS];
	static bool reorder_on;
	
	
		reorder_on = 0;
		int j = 0;
		// Should we reorder the channels (might be needed for encoder ?
		if (_wavheader->channels > 2) 
		{
			CHANNEL_TYPE *p_ch_type;
			for (int i = 0; i < _wavheader->channels; i++) 
			{
				for (int c = 0; c < _wavheader->channels; c++) 
				{
					if (input[c] == output[i]) 
					{
						if (j != c)
							reorder_on = 1;
						reorder[j++] = c;
					}
				}
			}
		}
	

	if (reorder_on)
		for (int i = 0; i < nb; i++) {
			memcpy(tmp, data, sizeof(tmp));
			for (int c = 0; c < _wavheader->channels; c++)
				*data++ = tmp[reorder[c]];
		}

}

uint32_t AUDMEncoder::read(uint32_t len,uint8_t *buffer)
{
  ADM_assert(0);
  return 0; 
}

uint32_t AUDMEncoder::grab(uint8_t * obuffer)
{
  uint32_t len,sam;
  if(getPacket(obuffer,&len,&sam))
    return len;
  return MINUS_ONE;
}

//EOF
