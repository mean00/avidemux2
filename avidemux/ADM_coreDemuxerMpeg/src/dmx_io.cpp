/***************************************************************************
                          dmx_io.cpp  -  description
                             -------------------

    copyright            : (C) 2005 by mean
    email                : fixounet@free.fr

    This just handles mpeg sync search and little endian/big endin integer reading
    It also handle multiple files seen as one logical file and buffering
        to speed up ios

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <math.h>

#include "ADM_default.h"

#include "dmx_io.h"


fileParser::fileParser( void )
{
                _fd=NULL;
                _sizeFd=NULL;
                _off=0;
                _nbFd=0;
                _curFd=0;
                _buffer=new uint8_t[DMX_BUFFER];
                _head=_tail=0;
                _size=0;
}

fileParser::~fileParser()
{
        if(_nbFd)
        {
                 for(uint32_t i=0;i<_nbFd;i++)
                 {
                                if(_fd[i])
                                        fclose(_fd[i]);
                 }
                delete [] _fd;
                delete [] _sizeFd;
                delete [] _sizeFdCumul;


        }
        if(_buffer) delete [] _buffer;
        _buffer=NULL;
}

/*
        Open one file, probe to see if there is several file with contiguous name
        and handle them as one big file if that's the case

        If multi is set to probe, return value will be APPEND if there is several files, dont_append if one
        if multi is set to dont_append, file won't be auto appended even if they exist
*/
uint8_t fileParser::open( const char *filename,FP_TYPE *multi )
{
        char *dot = NULL;                   // pointer to the last dot in filename
        uint8_t decimals = 0;               // number of decimals
        char *left = NULL, *number = NULL, *right = NULL; // parts of filename (after splitting)

        char *followup = new char[ strlen(filename) + 1 ]; // possible follow-up filename
        uint8_t first_followup = 1;         // indicates first follow-up
        uint8_t last_followup = 0;          // indicates last follow-up (by number: 99)
        uint8_t count = 0;                  // number of follow-ups

        FILE **buffer_fd = NULL;            // _fd buffer
        uint64_t *buffer_sizeFd = NULL;     // _sizeFd buffer

        int i = 0;                          // index (general use)


        // find the last dot
        dot = strrchr( filename, '.' );

        // count the decimals before the dot
        decimals = 1;
        while( (dot != NULL) && ((dot - decimals) != filename) &&
               (dot[0 - decimals] >= '0') && (dot[0 - decimals] <= '9') )
                { decimals++; }
        decimals--;

        // Nuv files can have 20 decimals
        // Keep it down to 10000
        if(decimals>4) decimals=4;
        if(*multi==FP_DONT_APPEND)
        {
                if(decimals) printf("There was several files, but dont append was forced\n");
                decimals=0;
        }
        // no number sequence
        if( decimals == 0 )
        {
                printf( "\nSimple loading: \n" );
                delete [] followup;
                _nbFd = 1;
                _curFd = 0;
                _fd = new FILE * [_nbFd];
                _sizeFd = new uint64_t [_nbFd];
                _sizeFdCumul = new uint64_t [_nbFd];

                // open file
                if(! (_fd[0] = fopen(filename, "rb")) )
                  { return 0; }

                // calculate file-size
                fseeko( _fd[0], 0, SEEK_END );
                _sizeFd[0] = ftello( _fd[0] );
                fseeko( _fd[0], 0, SEEK_SET );
                _sizeFdCumul[0]=0;
                _size=_sizeFd[0];
                printf( " file: %s, size: %"LLU"\n", filename, _sizeFd[0] );
                printf( " found 1 files \n" );
                printf( "Done \n" );
                return 1;
        }

        // possible number sequence
        else
        {
                // split the filename in <left>, <number> and <right>
                // -----

                // <left> part
                left = new char[(dot - filename - decimals) + 1];
                strncpy( left, filename, (dot - filename - decimals) );
                left[(dot - filename - decimals)] = '\0';

                // <number> part
                number = new char[decimals + 1];
                strncpy( number, (dot - decimals), decimals );
                number[decimals] = '\0';

                // <right> part
                right = new char[ strlen(dot) ];
                strcpy( right, dot );

                // add the file, and all existing follow-ups
                // -----
                uint32_t tabSize;

                tabSize=(uint32_t)pow(10,decimals);
                buffer_fd = new FILE * [tabSize];
                buffer_sizeFd = new uint64_t [tabSize];

                printf( "\nAuto adding: \n" );
                while( last_followup == 0 )
                {
                        strcpy( followup, left );
                        strcat( followup, number );
                        strcat( followup, right );

                        // open file
                        buffer_fd[count] = fopen(followup, "rb");
                        if(! buffer_fd[count] )
                        {
                                // we need at least one file!
                                if( first_followup == 1 )
                                  { return 0; }
                                else
                                  { printf( " file: %s not found. \n", followup ); break; }
                        }

                        // calculate file-size
                        fseeko( buffer_fd[count], 0, SEEK_END );
                        buffer_sizeFd[count] = ftello( buffer_fd[count] );
                        fseeko( buffer_fd[count], 0, SEEK_SET );

                        printf( " file %d: %s, size: %"LLU"\n", (count + 1), followup, buffer_sizeFd[count] );

                        // increase number
                        number[decimals - 1] = number[decimals - 1] + 1;
                        for( i = decimals - 1; i >= 0; i-- )
                        {
                                if( number[i] > '9' )
                                {
                                        if( i == 0 )
                                          { last_followup = 1; break; }
                                        number[i] = '0';
                                        number[i - 1] = number[i - 1] + 1;
                                }
                        }

                        first_followup = 0;
                        count++;
                } // while( last_followup == 0 )

                // copy from buffer
                _nbFd = count;
                _curFd = 0;
                _fd = new FILE * [_nbFd];
                _sizeFd = new uint64_t [_nbFd];
                _sizeFdCumul = new uint64_t [_nbFd];
                uint64_t total=0;
                for( i = 0; i < count; i++ )
                {
                        _fd[i] = buffer_fd[i];
                        _sizeFd[i] = buffer_sizeFd[i];
                        _sizeFdCumul[i]=total;
                        total+=buffer_sizeFd[i];
                }
                _size=total;
                // clean up
                delete [] followup;
                delete [] left;
                delete [] number;
                delete [] right;
                delete [] buffer_fd;
                delete [] buffer_sizeFd;
                if(*multi==FP_PROBE)
                {
                        if(count>1)
                                *multi=FP_APPEND;       //
                        else
                                *multi=FP_DONT_APPEND;
                }

                printf( " found %d files \n", count );
                printf( "Done \n" );
        } // if( decimals == 0 )
                return 1;
} // fileParser::open()


/*----------------------------------------

------------------------------------------*/

uint8_t fileParser::forward(uint64_t jmp)
{
                // still in the buffer ?
                if((_off+jmp)<_tail)
                {
                        _off+=jmp;
                        return 1;
                }

               // locate the new file

               if(_off+jmp>=_size)
                        {
                               _off=_size-1;
                               _head=_off;
                               _tail=_off;
                               return 0;
                        }
                _off+=jmp;  // final location
                for(uint32_t i=_curFd;i<_nbFd;i++)
                {
                        if(_off>=_sizeFdCumul[i] && _off<(_sizeFdCumul[i]+_sizeFd[i]))
                        {
                                _curFd=i;
                                fseeko(_fd[i],_off-_sizeFdCumul[i],SEEK_SET);
                                _head=_tail=_off;
                                return 1;
                        }

                }

                return 0;
}

uint8_t fileParser::setpos(uint64_t o)
{

                if(o>=_head && o<_tail)
                {
                        _off=o;
                        return 1;
                }
                for(uint32_t i=0;i<_nbFd;i++)
                        {
                                if( (o>=_sizeFdCumul[i]) && o<(_sizeFdCumul[i]+_sizeFd[i]))
                                        {
                                                        _curFd=i;
                                                        _off=o;
                                                        fseeko(_fd[_curFd],_off-_sizeFdCumul[i],SEEK_SET);
                                                        _head=_tail=_off; // Flush
                                                  return 1;
                                        }
                        }
                        printf("\n cannot seek to %"LLU"\n",o);
                        return 0;
}
//
//      Search packet signature and return packet type
//_______________________________________
uint8_t fileParser::sync(uint8_t *stream)
{
uint32_t val,hnt;

        val=0;
        hnt=0;
        // preload
        if((4+_off)>=_size)
        {
                printf("Dmx IO: End of file met (%"LLU" / %"LLU" seg%"LU")\n",_off,_size,_nbFd);
                return 0;
        }
        hnt=(read8i()<<16) + (read8i()<<8) +read8i();


        while((hnt!=0x00001))
        {
                hnt<<=8;
                val=read8i();
                hnt+=val;
                hnt&=0xffffff;
                if(_curFd==_nbFd-1)
                {
                                if((4+_off)>=_size) return 0;
                }
        }

        *stream=read8i();
        return 1;
}
//
//      Search packet signature and return packet type
//_______________________________________
uint8_t fileParser::syncH264(uint8_t *stream)
{
uint32_t val,hnt;

        val=0;
        hnt=0;
        // preload
        if((5+_off)>=_size)
        {
                printf("Dmx IO: End of file met (%"LLU" / %"LLU" seg%"LU")\n",_off,_size,_nbFd);
                return 0;
        }
        hnt=(read8i()<<24)+(read8i()<<16) + (read8i()<<8) +read8i();


        while((hnt!=0x1))
        {
                hnt<<=8;
                val=read8i();
                hnt+=val;
                if(_curFd==_nbFd-1)
                {
                                if((5+_off)>=_size) return 0;
                }
        }

        *stream=read8i();
        return 1;
}

uint8_t fileParser::getpos(uint64_t *o)
{
         *o=_off;
         return 1;

}


 uint64_t fileParser::getSize( void )
{
        return  _size;

}
/*--------------------------------------------------
                Read l bytes from file
----------------------------------------------------*/
uint32_t fileParser::read32(uint32_t len, uint8_t *buffer)
{
uint32_t r;

uint64_t remain,begin,mx,last;

        ADM_assert(_off>=_head);
        ADM_assert(_off<=_tail);

        if(_head>=_size-1) return 0;

// Check we do not go out of bound
        if(_off+len>=_size)
        {
                len=_size-_off;
        }

        remain=_tail-_off;
        begin=_off-_head;



        // everything in cache ?
        if(len<=remain)
        {
                memcpy(buffer,_buffer+begin,len);
                _off+=len;
                return len;
        }

        // No enough data, purge cache
        if(remain)
        {
                memcpy(buffer,_buffer+begin,remain);
                _off+=remain;
                len-=remain;
                buffer+=remain;
                return remain+read32(len,buffer);
        }

        // Reload ?
        // What is left in that file ?
        mx=_sizeFd[_curFd]+_sizeFdCumul[_curFd]-_off;
        // Do we need more, if so jump over it
        if(len>mx)
        {
                fread(buffer,mx,1,_fd[_curFd]);
                len-=mx;
                _off+=mx;
                buffer+=mx;
                _head=_tail=_off;
                _curFd++;
                if(_curFd>=_nbFd) return 0;
                fseeko(_fd[_curFd],0,SEEK_SET);
                return mx+read32(len,buffer);
        }
        // Read what is available in file, store leftover in the buffer
        fread(buffer,len,1,_fd[_curFd]);
        _off+=len;
        // available in that file
        mx-=len;
        if(mx>DMX_BUFFER) mx=DMX_BUFFER;
        fread(_buffer,mx,1,_fd[_curFd]);
        _head=_off;
        _tail=_head+mx;

         return len;

}
#ifdef NO_INLINE_FP
uint32_t fileParser::read32i(void )
{
       uint32_t v;
       uint8_t c[4];
       uint8_t *p;
        // case one, it fits in the buffer
        //
        if(_off+4<_tail)
        {
                p=&(_buffer[_off-_head]);
                _off+=4;
        }
        else
        {
               read32(4,c);
               p=c;
        }
       v= (p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
       return v;
}
uint16_t fileParser::read16i(void )
{
  uint32_t v;
       uint8_t c[4];
       uint8_t *p;
        // case one, it fits in the buffer
        //
        if(_off+2<_tail)
        {
                p=&(_buffer[_off-_head]);
                _off+=2;
        }
        else
        {
               read32(2,c);
               p=c;
        }
       v= (p[0]<<8)+p[1];
       return v;
}
uint8_t fileParser::read8i(void )
{
uint8_t r;
        if(_off+1<_tail)
        {
                r= _buffer[_off-_head];
                _off++;
        }
        else
        {
                read32(1,&r);
        }
        return r;
}
#endif
