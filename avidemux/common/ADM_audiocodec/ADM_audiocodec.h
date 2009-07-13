/***************************************************************************
                          ADM_audiocodec.h  -  description
                             -------------------
    begin                : Fri May 31 2002
    copyright            : (C) 2002 by mean
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
#ifndef ADMAUDIOCODEC
#define ADMAUDIOCODEC
#include "ADM_audiodef.h"
#define SCRATCH_PAD_SIZE (100*1000*2)
extern uint8_t scratchPad[];
//#define  ADMAC_BUFFER (48000*4)
class ADM_Audiocodec
{
    protected:
        uint8_t	_init;
        WAVHeader *_wavHeader;
    public:
        ADM_Audiocodec(uint32_t fourcc)
        {
            UNUSED_ARG(fourcc);
            _init=0;
        };

        virtual	~ADM_Audiocodec() {};
        virtual	void purge(void) {}
        virtual	uint8_t beginDecompress(void)=0;
        virtual	uint8_t endDecompress(void)=0;
        virtual	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut)=0;
        virtual	uint8_t isCompressed(void)=0;
        virtual	bool    isDummy(void) {return false;};
        // Channel mapping, only input is used by the decoders..
        CHANNEL_TYPE channelMapping[MAX_CHANNELS];
 };

ADM_Audiocodec	*getAudioCodec(uint32_t fourcc, WAVHeader *info, uint32_t extra=0, uint8_t *extraData=NULL);

class ADM_AudiocodecWav : public     ADM_Audiocodec
{
	public:
		ADM_AudiocodecWav(uint32_t fourcc);
		virtual	~ADM_AudiocodecWav();
		virtual	uint8_t beginDecompress(void);
		virtual	uint8_t endDecompress(void);
		virtual	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t * nbOut);
		virtual	uint8_t isCompressed(void);
};

class ADM_AudiocodecWavSwapped : public     ADM_Audiocodec
{
	public:
		ADM_AudiocodecWavSwapped(uint32_t fourcc);
		virtual	~ADM_AudiocodecWavSwapped();
		virtual	uint8_t beginDecompress(void);
		virtual	uint8_t endDecompress(void);
		virtual	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
		virtual	uint8_t isCompressed(void);

   };

class ADM_AudiocodecUnknown : public     ADM_Audiocodec
{
	public:
		ADM_AudiocodecUnknown(uint32_t fourcc) : ADM_Audiocodec(fourcc) {}
		~ADM_AudiocodecUnknown() {}
		uint8_t beginDecompress(void) {return 0;}
		uint8_t endDecompress(void) {return 0;}
		uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut) {*nbOut=0;return 0;}
		uint8_t isCompressed(void) {return 1;}
        bool    isDummy(void) {return true;}
};



class ADM_Audiocodec8Bits : public     ADM_Audiocodec
{
	protected:
		uint8_t _unsign;

	public:
		ADM_Audiocodec8Bits(uint32_t fourcc);
		virtual	~ADM_Audiocodec8Bits();
		virtual	uint8_t beginDecompress(void) {return 1;}
		virtual	uint8_t endDecompress(void) {return 1;}
		virtual	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
		virtual	uint8_t isCompressed(void) {return 1;}
};



#endif
