
#include "ADM_default.h"
#include "ADM_threads.h"
#include "interact.hpp"

#undef malloc
#undef realloc
#undef free



#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_LAVFORMAT
#include "ADM_osSupport/ADM_debug.h"

#include "cpu_accel.h"
#include "mjpeg_types.h"
#include "mjpeg_logging.h"
#include "mpegconsts.h"

#include "bits.hpp"
#include "outputstrm.hpp"
#include "multiplexor.hpp"

#include "ADM_inout.h"
#include "ADM_transfert.h"


/******************************************************************************/
/********************************
 *
 * IFileBitStream - Input bit stream class for bit streams sourced
 * from standard file I/O (this of course *includes* network sockets,
 * fifo's, et al).
 *
 * OLAF: To hook into your PES reader/reconstructor you need to define
 * a class like this one, where 'ReadStreamBytes' calls you code to
 * generate the required number of bytes of ES data and transfer it 
 * to the specified buffer.  The logical way to do this would be to
 * inherit IBitStream as a base class of the top-level classes for the ES
 * reconstructors.
 *
 ********************************/

bool IFileBitStream::EndOfStream(void) 
{
        if(queue->isEof()) return true;
        return false;

}

IFileBitStream::IFileBitStream(PacketQueue *q, mplexStreamDescriptor *desc, unsigned int buf_size) : IBitStream(desc) //MEANX
{
        queue=q;        
        SetBufSize(buf_size);
        eobs = false;
        byteidx = 0;
        
        if (!ReadIntoBuffer())
        {
                        ADM_assert(buffered);
                
        }
        
}


/**
   Destructor: close the device containing the bit stream after a read
   process
*/
IFileBitStream::~IFileBitStream()
{
        
        
        Release();
}
/**

*/
 size_t IFileBitStream::ReadStreamBytes( uint8_t *buf, size_t number )
 {
uint32_t s,z;
        if(!queue->Pop(buf,&z,&s)) return 0;
      ADM_assert(z);
      ADM_assert(z<=number);
        return z;
 }
 //EOF
 
