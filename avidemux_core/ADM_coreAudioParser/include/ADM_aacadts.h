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
class ADM_adts2aac
{
private:
		void * cookie;
        void * codec;
		
public:
		bool getExtraData(uint32_t *len,uint8_t **data);
		bool convert(int incomingLen,uint8_t *intData,int *outLen,uint8_t *out);
        int  getFrequency(void);
        int  getChannels(void);
             ADM_adts2aac();
             ~ADM_adts2aac();

};
#endif

