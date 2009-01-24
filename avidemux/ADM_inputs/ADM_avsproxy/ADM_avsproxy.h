/*

(c) Mean 2006
*/

#ifndef AVS_PROXY_H

#include "avifmt.h"
#include "avifmt2.h"

#include "ADM_Video.h"
#include "ADM_audio/aviaudio.hxx"


class avsHeader         :public vidHeader
{
    protected:
        int         mySocket;
        uint8_t     bindMe(uint32_t port);
        uint8_t     sendData(uint32_t cmd,uint32_t frame, uint32_t payload_size,uint8_t *payload);
        uint8_t     receiveData(uint32_t *cmd, uint32_t *frame,uint32_t *payload_size,uint8_t *payload);
        uint8_t     askFor(uint32_t cmd,uint32_t frame, uint32_t payloadsize,uint8_t *payload);
        uint8_t     rxData(uint32_t howmuch, uint8_t *where);
        uint8_t     txData(uint32_t howmuch, uint8_t *where);
    public:


        virtual   void 				Dump(void) {};

        avsHeader( void );
        ~avsHeader(  );
// AVI io
        virtual 	uint8_t			open(const char *name);
        virtual 	uint8_t			close(void) ;
  //__________________________
  //				 Info
  //__________________________

  //__________________________
  //				 Audio
  //__________________________

        virtual 	WAVHeader *getAudioInfo(void ) { return NULL ;} ;
        virtual 	uint8_t			getAudioStream(AVDMGenericAudioStream **audio)
        {  *audio=NULL;return 1;};

// Frames
  //__________________________
  //				 video
  //__________________________

        virtual 	uint8_t  setFlag(uint32_t frame,uint32_t flags) {return 1;}
        virtual 	uint32_t getFlags(uint32_t frame,uint32_t *flags) 
                            {*flags=AVI_KEY_FRAME;return AVI_KEY_FRAME;}
        virtual 	uint8_t  getFrameNoAlloc(uint32_t framenum,ADMCompressedImage *img);

};
#endif
//EOF
