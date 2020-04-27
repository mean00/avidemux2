/** *************************************************************************
    \file    dmx_io.cpp
    \brief   handle several files as one
    \author  mean (c) 2005/2010 fixounet@free.fr

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
#include "ADM_cpp.h"
#include <math.h>

#include "ADM_default.h"
#include "dmx_io.h"
#include "ADM_coreUtils.h"
#if 0
    #define aprintf(...) {}
#else
    #define aprintf printf
#endif

fileParser::fileParser(uint32_t cacheSize)
{
                _off=0;
                _curFd=0;
                _bufferSize=cacheSize;
                _buffer=(uint8_t *)ADM_alloc(_bufferSize);
                ADM_assert(_buffer);
                _head=_tail=0;
                _size=0;
}

fileParser::~fileParser()
{
        int nb=listOfFd.size();
        for(int i=0;i<nb;i++)
        {
                if(listOfFd[i].file)
                {
                    fclose(listOfFd[i].file);
                    listOfFd[i].file=NULL;
                }
        }
        listOfFd.clear();
        ADM_dealloc(_buffer);
        _buffer=NULL;
}

/**
 *  \fn    open
 *  \brief Open one file, probe to see if there is several file with contiguous name
 *         and handle them as one big file if that's the case
 *  \param filename The path to the file to open
 *  \param[in] multi Control file size pattern to match
 *      0: don't append files even if they exist.
 *      1: use default values
 *      negative values: skip size check
 *      else fragment size in MiB to match
 *  \param[out] multi Size of the first file in MiB if there are several files, 0 if only one
 *      do not use when size check was skipped
 *  \return 1 on success, 0 on failure
 */
uint8_t fileParser::open( const char *filename, int *multi )
{

        uint32_t decimals = 0;               // number of decimals
        char *left = NULL, *right = NULL; // parts of filename (after splitting)

        int nbFollowUps = 0;
        uint32_t base=0;
        if(*multi)
        {
            aprintf("Checking if there are several files...\n");
            if(ADM_splitSequencedFile(filename, &left, &right, &decimals, &base))
            {
                aprintf("left:<%s>, right=<%s>,base=%" PRIu32",digit=%" PRIu32"\n",left,right,base,decimals);
                nbFollowUps = ADM_probeSequencedFile(filename,multi);
                if(nbFollowUps<0) return 0;
            }else
            {
                aprintf("No.\n");
            }
        }
        // ____________________
        // Single loading
        // ____________________

        if(!nbFollowUps)
        {
                fdIo newFd;
                aprintf( "\nSimple loading: \n" );
                _curFd = 0;
                FILE *f=NULL;
                // open file
                if(! (f = ADM_fopen(filename, "rb")) )
                  { return 0; }
                newFd.file=f;
                // calculate file-size
                fseeko( f, 0, SEEK_END );
                 newFd.fileSize = ftello( f );
                fseeko( f, 0, SEEK_SET );
                 newFd.fileSizeCumul=0;
                _size=newFd.fileSize;
                listOfFd.append(newFd);
                aprintf( " file: %s, size: %" PRIu64"\n", filename, newFd.fileSize );
                aprintf( " found 1 files \n" );
                aprintf( "Done \n" );
                *multi=0;
                return 1;
        }
        // ____________________
        // Multi loading
        // ____________________
        std::string leftPart(left);
        std::string rightPart(right);
        delete [] left;
        delete [] right;
        left=NULL;
        right=NULL;
    
        aprintf( "\nAuto adding: \n" );
        _curFd = 0;
        uint64_t total=0;

        // build match string
        char match[16];
        match[0]='%';
        match[1]='0';
        sprintf(match+2,"%d",decimals); // snprintf instead ...
        strcat(match,"d");
        match[15]=0;
        aprintf("Using %s as match string\n",match);
        char number[16];
        for(int i=0; i<nbFollowUps+1; i++)
        {
                sprintf(number,match,base+i);
                std::string middle(number);
                std::string outName=leftPart+middle+rightPart;
                aprintf("Checking %s\n",outName.c_str());

                // calculate file-size
                int64_t sz=ADM_fileSize(outName.c_str());
                if(sz<=0)
                {
                    if(!i) return 0;
                    printf(" file: %s not found.\n",outName.c_str());
                    nbFollowUps=i-1;
                    break;
                }
                // open file
                FILE *f= ADM_fopen(outName.c_str(), "rb");
                if(!f)
                {
                        // we need at least one file!
                        if(!i)
                          { return 0; }
                        else
                          { 
                                printf( " file: %s not found. \n", outName.c_str() ); 
                                break; 
                          }
                }

                fdIo myFd;
                myFd.file=f;
                myFd.fileSize=sz;
                myFd.fileSizeCumul = total;
                total+=  myFd.fileSize;

                aprintf( " file %d: %s, size: %" PRIu64"\n", i + 1, outName.c_str(),
                                            myFd.fileSize );
                if(*multi>0 && !i)
                    *multi=myFd.fileSize>>20;
                listOfFd.append(myFd);
        } 

        _size=total;
        if(nbFollowUps<1)
                *multi=0;

        aprintf( " found %d files \n", nbFollowUps+1 );
        aprintf( "Done \n" );
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
                int nb=listOfFd.size();
                for(uint32_t i=_curFd;i<nb;i++)
                {
                        if(_off>=listOfFd[i].fileSizeCumul && _off<(listOfFd[i].fileSizeCumul
                                                                    +listOfFd[i].fileSize))
                        {
                                _curFd=i;
                                fseeko(listOfFd[i].file,_off-listOfFd[i].fileSizeCumul,SEEK_SET);
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
                int nb=listOfFd.size();
                for(uint32_t i=0;i<nb;i++)
                        {
                                if( (o>=listOfFd[i].fileSizeCumul) && o<(listOfFd[i].fileSizeCumul+listOfFd[i].fileSize))
                                        {
                                                        _curFd=i;
                                                        _off=o;
                                                        fseeko(listOfFd[_curFd].file,_off-listOfFd[i].fileSizeCumul,SEEK_SET);
                                                        _head=_tail=_off; // Flush
                                                  return 1;
                                        }
                        }
                        printf("\n cannot seek to %" PRIu64"\n",o);
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
                printf("Dmx IO: End of file met (%" PRIu64" / %" PRIu64" seg:%u)\n",_off,_size,(unsigned int)listOfFd.size());
                return 0;
        }
        hnt=(read8i()<<16) + (read8i()<<8) +read8i();


        while((hnt!=0x00001))
        {
                hnt<<=8;
                val=read8i();
                hnt+=val;
                hnt&=0xffffff;
                if(_curFd==listOfFd.size()-1)
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
                printf("Dmx IO: End of file met (%" PRIu64" / %" PRIu64" seg:%u)\n",_off,_size,(unsigned int)listOfFd.size());
                return 0;
        }
        hnt=(read8i()<<24)+(read8i()<<16) + (read8i()<<8) +read8i();


        while((hnt!=0x1))
        {
                hnt<<=8;
                val=read8i();
                hnt+=val;
                if(_curFd==listOfFd.size()-1)
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
uint64_t remain,begin,mx,last;

        ADM_assert(_off>=_head);
        ADM_assert(_off<=_tail);

        if(_head>=_size-1)
        {
            memset(buffer,0,len);
            return 0;
        }

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
        mx=listOfFd[_curFd].fileSize+listOfFd[_curFd].fileSizeCumul-_off;
        // Do we need more, if so jump over it
        if(len>mx)
        {
                fread(buffer,mx,1,listOfFd[_curFd].file);
                len-=mx;
                _off+=mx;
                buffer+=mx;
                _head=_tail=_off;
                _curFd++;
                if(_curFd>=listOfFd.size()) return 0;
                fseeko(listOfFd[_curFd].file,0,SEEK_SET);
                return mx+read32(len,buffer);
        }
        if(len>_bufferSize)
        {
                // Read what is available in file, store leftover in the buffer
                fread(buffer,len,1,listOfFd[_curFd].file);
                _off+=len;
                // available in that file
                mx-=len;
                if(mx>_bufferSize) mx=_bufferSize;
                fread(_buffer,mx,1,listOfFd[_curFd].file);
                _head=_off;
                _tail=_head+mx;

                return len;
        }
        // Fill the buffer first
        if(mx>_bufferSize) mx=_bufferSize;
        fread(_buffer,mx,1,listOfFd[_curFd].file);
        _head=_off;
        _tail=_head+mx;
        return read32(len,buffer);
}
/**
 *  \fn setBufferSize
 */
uint8_t fileParser::setBufferSize(uint32_t size)
{
    if(size>DMX_BUFFER_MAX)
        return 0;
    setpos(0);
    ADM_dealloc(_buffer);
    _buffer=(uint8_t *)ADM_alloc(size);
    ADM_assert(_buffer);
    _bufferSize=size;
    return 1;
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
/**
    \fn peek8i
    \brief Return the next bytes in the stream without advancing index
    * BEWARE: Use with caution!
*/
uint8_t  fileParser::peek8i(void)
{
uint8_t r;
    if(_off+1<_tail)
    {
        r= _buffer[_off-_head];
    }
    else    
    {
        uint64_t mx=listOfFd[_curFd].fileSize+listOfFd[_curFd].fileSizeCumul-_off;
        // If nothing is left, switch to the next file
        if(!mx)
        {
            _head=_tail=_off;
            _curFd++;
            if(_curFd>=listOfFd.size()) return 0;
            fseeko(listOfFd[_curFd].file,0,SEEK_SET);
            mx=listOfFd[_curFd].fileSize;
        }
        if(mx>_bufferSize) mx=_bufferSize;
        // Fill the buffer
        fread(_buffer,mx,1,listOfFd[_curFd].file);
        _head=_off;
        _tail=_head+mx;
        r=_buffer[0];
    }
    return r;

}

void fileParser::hexDump(uint8_t *buf, int size)
{
	int len, i, j, c;

	for(i=0;i<size;i+=16)
	{
		len = size - i;

		if (len > 16)
			len = 16;

		printf("%08x ", i);

		for(j=0;j<16;j++)
		{
			if (j < len)
				printf(" %02x", buf[i+j]);
			else
				printf("   ");
		}

		printf(" ");

		for(j=0;j<len;j++)
		{
			c = buf[i+j];

			if (c < ' ' || c > '~')
				c = '.';

			printf("%c", c);
		}

		printf("\n");
	}
}
