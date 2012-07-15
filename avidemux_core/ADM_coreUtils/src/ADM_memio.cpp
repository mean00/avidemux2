/**
    \file ADM_memmio
    \brief base class to write little endian to memory buffer
*/
#include "ADM_default.h"
#include "ADM_memio.h"

/**
    \ffn ctor
*/
ADMMemio::ADMMemio(int size)
{
    buffer=new uint8_t[size];
    cur=buffer;
    tail=buffer+size;
}
/**
    \ffn ctor
*/
ADMMemio::~ADMMemio()
{
    if(buffer)
        {
            delete [] buffer;
            buffer=cur=tail=NULL;
        }
}
/**
    \fn write
*/
void            ADMMemio::write(int len, const uint8_t *data)
{
    ADM_assert(buffer+len<=tail)
    memcpy(cur,data,len);
    cur+=len;
}

#ifdef ADM_X86
void            ADMMemio::write32(uint32_t w)
{
    ADM_assert(buffer+4<=tail);
    memcpy(cur,&w,4);
    cur+=4;
}
void            ADMMemio::write16(uint16_t w)
{
    ADM_assert(buffer+4<=tail);
    memcpy(cur,&w,2);
    cur+=2;
}
void            ADMMemio::write8(uint8_t w)
{
    ADM_assert(buffer<tail)
    *cur++=w;
}
#else
void            ADMMemio::write32(uint32_t w)
{
    ADM_assert(buffer+4<=tail);
    *cur++=w&0xff; w>>=8;
    *cur++=w&0xff; w>>=8;
    *cur++=w&0xff; w>>=8;
    *cur++=w&0xff; w>>=8;
}
void            ADMMemio::write16(uint16_t w)
{
    ADM_assert(buffer+4<=tail);
    *cur++=w&0xff; w>>=8;
    *cur++=w&0xff; w>>=8;
}
void            ADMMemio::write8(uint8_t w)
{
    ADM_assert(buffer<tail)
    *cur++=w;
}

#endif

       