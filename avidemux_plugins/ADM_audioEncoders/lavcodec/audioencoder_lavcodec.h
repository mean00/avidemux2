
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
     typedef enum 
            {
                asInt16,asFloat,asFloatPlanar
            }ADM_outputFlavor;
  protected:
    bool                 _closeCodec;
    void                *_context;
    uint32_t             _chunk;
    bool                 _globalHeader;
    ADM_outputFlavor     outputFlavor;
    float                *planarBuffer;
    int                  planarBufferSize;
    float               *i2p(int count);
    bool                encodeBlock(int count, uint8_t *dest,int &encoded);
    bool                encodeBlockSimple(int count, uint8_t *dest,int &encoded);
    bool                encodeBlockMultiChannels(int count, uint8_t *dest,int &encoded);
    bool                lastBlock(AVPacket *p,int &encoded);
    bool                computeChannelLayout(void);
    CHANNEL_TYPE        channelMapping[8];
    bool                needChannelRemapping;
    AVFrame             *_frame;
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
