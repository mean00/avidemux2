
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
#ifndef AUDMaudioLavcodec
#define AUDMaudioLavcodec
#include "lavcodec_encoder.h"
 //_____________________________________________
class AUDMEncoder_Lavcodec : public ADM_AudioEncoder
{
  protected:

typedef enum {
    asInt16,
    asFloat,
    asFloatPlanar,
    unsupported
} ADM_outputFlavor;

typedef enum {
    normal,
    flushing,
    flushed
} ADM_encoderState;

    void                *_context;
    AVFrame             *_frame;
    AVPacket            *_pkt;
    uint32_t             _chunk;
    bool                 _globalHeader;
    ADM_outputFlavor     outputFlavor;
    ADM_encoderState     encoderState;
    float                *planarBuffer;
    int                  planarBufferSize;
    CHANNEL_TYPE        channelMapping[8];

    float               *i2p(int count);
    bool                fillFrame(int count);
    bool                computeChannelLayout(void);
    void                printError(const char *s,int er);

  public:
            lav_encoder _config;
            bool        initialize(void);
   virtual             ~AUDMEncoder_Lavcodec();
                        AUDMEncoder_Lavcodec(AUDMAudioFilter *instream,bool globalHeader,CONFcouple *c);
   virtual bool  	encode(uint8_t *dest, uint32_t *len, uint32_t *samples);
   uint8_t              extraData(uint32_t *l,uint8_t **d);
};

#endif
