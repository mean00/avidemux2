//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//

/*
* MODIFIED BY GMV 30.1.05: prepared for ODML
*/
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_fileio.h"
#include "ADM_quota.h"

#define ADM_FILE_BUFFER 4*256*1024 // 256 kB
//#define ADMF_DEBUG
ADMFile::ADMFile( void)
{
        _out=NULL;
        _fill=0;
        _curPos=0;
        _buffer=new uint8_t[ADM_FILE_BUFFER];
        ADM_assert(_buffer);
        
}
ADMFile::~ADMFile()
{
        flush();
        if(_buffer) 
                delete [] _buffer;
        _buffer=NULL;
}
uint8_t ADMFile::open(FILE *in)
{
        ADM_assert(!_out);
        ADM_assert(in);
        _out=in;
        _curPos=ftello(_out);
        _fill=0;
        return 1;
}
uint8_t ADMFile::flush(void)
{
 ADM_assert(_fill<=ADM_FILE_BUFFER);
        if(_fill)
        {
                
                qfwrite(_buffer,_fill,1,_out);
                
                _curPos+=_fill;
#ifdef ADMF_DEBUG                
                printf("Flushing %lu bytes, now at :%lu\n",_fill,_curPos);
#endif                
                _fill=0;
        }
        return 1;
}
uint64_t ADMFile::tell(void)
{
 ADM_assert(_fill<ADM_FILE_BUFFER);
	// MOD Feb 2005 by GMV
	// uint32_t c;
	uint64_t c;
	// END MOD Feb 2005 by GMV
        
     
        flush();
#ifdef ADMF_DEBUG           
        c=ftello(_out);
        ADM_assert(c==_curPos);
        printf("[%lu] Ftell :%lu \n",this,c);
#endif        
        return _curPos+_fill;  
}
uint8_t ADMFile::seek(uint64_t where)
{
 ADM_assert(_fill<ADM_FILE_BUFFER);
        flush();
        fseeko(_out,where,SEEK_SET);
        _curPos=where;
#ifdef ADMF_DEBUG        
        printf("[%lu] Fseek :%lu \n",this,where);
#endif        
        return 1;
}
//
// Buffered write
//
uint8_t ADMFile::write(const uint8_t *data,uint32_t how)
{
        uint32_t oneshot;
#ifdef ADMF_DEBUG        
        printf("[%lu]Fwrite : %lu\n",this,how);
        printf("Curpos:%lu ",_curPos);
        printf("Tell  :%lu ",ftello(_out));
        printf("Fill :%lu\n",_fill);
#endif        
        while(1)
        {
                ADM_assert(_fill<ADM_FILE_BUFFER);
                oneshot=_fill+how;
                if(oneshot<ADM_FILE_BUFFER)
                {
                        memcpy(_buffer+_fill,data,how);
                        _fill+=how;
                        return 1;
                }
                // copy what's possible
                oneshot=ADM_FILE_BUFFER-_fill;
                memcpy(_buffer+_fill,data,oneshot);
                _fill+=oneshot;
                flush();
                data+=oneshot;
                how-=oneshot;
        }

        return 1;
}
