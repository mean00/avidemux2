/***************************************************************************
  \file ADM_aacadts.h
  \brief wrapper around libavcodec bitstream filter
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_AAC_ADTS
#define ADM_AAC_ADTS
/**
    \class ADM_adts2aac
*/
#define ADTS_BUFFER_SIZE 16000
class ADM_adts2aac
{
private:
        bool    hasExtra;
        uint8_t extra[2];
        uint8_t buffer[ADTS_BUFFER_SIZE*2];
        int     head,tail;
		
public:
		bool getExtraData(uint32_t *len,uint8_t **data);
		bool convert(int incomingLen,uint8_t *intData,int *outLen,uint8_t *out);
        int  getFrequency(void);
        int  getChannels(void);
             ADM_adts2aac();
             ~ADM_adts2aac();

};
#endif

