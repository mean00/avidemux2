/**
    \file audio_encoderWrapper.h
    \brief Do the opposite of the bridge. Transform an ADM_audioEncoder stream to a ADMGenericVideoStream

*/
#ifndef audio_encoderWrapper_H
#define audio_encoderWrapper_H
#include "ADM_audioStream.h"
class ADM_audioEncoderWrapper :  public ADM_audioStream
{
protected:
        AUDMEncoder *_encoder;

public:
                            ADM_audioEncoderWrapper( AUDMEncoder *coder);
        virtual             ~ADM_audioEncoderWrapper();
        virtual	uint8_t		getPacket(uint8_t *dest, uint32_t *len, uint32_t *samples);
		virtual uint8_t     packetPerFrame( void);

        virtual uint8_t		goTo(uint32_t offset) {ADM_assert(0);return 0;} // Not supposed to seek..
        virtual uint32_t	read(uint32_t size,uint8_t *ptr);
        virtual uint8_t		extraData(uint32_t *l,uint8_t **d);
};

#endif

//EOF
