/**
    \file ADM_audioAccessFileAACADTS
    \brief Source is a AAC audio file, wrapped in ADTS container

*/
#include "ADM_default.h"
#include "ADM_audioStream.h"
#include "ADM_audioAccessFileAACADTS.h"

/**
 * 
 * @return 
 */
bool ADM_audioAccessFileAACADTS::init(void)
{
    return false;
}

/**
    \fn
    \brief
*/

ADM_audioAccessFileAACADTS::ADM_audioAccessFileAACADTS(const char *fileName,int offset)
{
        fileSize=ADM_fileSize(fileName)-offset;
        _fd=ADM_fopen(fileName,"rb");
        ADM_assert(_fd);
        clock=NULL;
        inited=init();;
        
        
}
/**
    \fn
    \brief
*/

ADM_audioAccessFileAACADTS::~ADM_audioAccessFileAACADTS()
{
        if(_fd) ADM_fclose(_fd);
        _fd=NULL;
        if(clock) delete clock;
        clock=NULL;
}

/**
    \fn
    \brief
*/

bool    ADM_audioAccessFileAACADTS::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
{
    if(!inited) return false;
    if(!getPos()) *dts=0;
        else      *dts=ADM_NO_PTS;
    int n=fread(buffer,1,maxSize,_fd);
    *size=n;
 //   ADM_info("ExternalTrack : Request for %d bytes, maxSize=%d\n",*size,maxSize);
    if(n>0) return true;
        return false;
}
/**
 * 
 * @param timeUs
 * @return 
 */
bool      ADM_audioAccessFileAACADTS::goToTime(uint64_t timeUs)
{
    if(!inited)
        return false;
    return false;
}

// EOF

