#ifndef ADM_ODML_AUDIO_H
#define ADM_ODML_AUDIO_H
#include <vector>
using std::vector;
#include "ADM_audioStream.h"
class odmlIndex;
/**
    \class ADM_aviAudioAccess
    \brief provided audio access
*/
class ADM_aviAudioAccess : public ADM_audioAccess
{
protected:
               
                uint32_t length;
                uint32_t pos;
                FILE     *fd;
                uint32_t currentIndex;
                vector <odmlIndex> myIndex;
                uint32_t   nbIndex;
                WAVHeader *wavHeader;
public: 
                ADM_aviAudioAccess(odmlIndex *idx,WAVHeader *hdr,
						uint32_t nbchunk,
						const char *name,
						uint32_t extraLen,
						uint8_t  *extraData);

                virtual ~ADM_aviAudioAccess();

                
                virtual bool      canSeekTime(void) {return false;};
                virtual bool      canSeekOffset(void) {return true;};
                virtual bool      canGetDuration(void) {return false;};
                virtual uint64_t  getDurationInUs(void) {return 0;};
                virtual uint32_t  getLength(void) {return length;}
                virtual bool      goToTime(uint64_t timeUs) {ADM_assert(0);return true;}
                virtual bool      isCBR(void) ;
                virtual uint64_t  getPos(void);
                
                

                virtual bool   getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts);
                virtual bool   setPos(uint64_t pos);
};


#endif
//EOF
