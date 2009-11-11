//
//
// C++ Implementation: ADM_riff
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "ADM_default.h"

#include "ADM_riff.h"

#define aprintf(...) {}

#ifdef ADM_DEBUG
	//#define ODML_RIFF_VERBOSE
#endif

uint64_t riffParser::getPos( void ) 
{ 
#ifdef __WIN32
        aprintf("pos : %I64u / %I64u \n",curPos,ftello(fd));
#else
	aprintf("pos : %llu / %llu \n",curPos,ftello(fd));
#endif        
	return curPos;
//	return ftello(fd);
};

uint8_t riffParser::read(uint32_t len, uint8_t *data)
{
	if(len!=fread(data,1,len,fd)) return 0;
	curPos+=len;
	return 1;

}
uint8_t riffParser::skip(uint32_t s)
{
	if(s&1) s++;
	fseeko(fd,s,SEEK_CUR);
	curPos+=s;
	if(curPos>endPos)
		{
			printf("chunk : Going out of bound!\n");
		}
	return 1;
}
uint8_t riffParser::endReached(void)
{
#ifdef __WIN32
        aprintf("Cur : %I64u end : %I64u left: %I64u\n",curPos,endPos,endPos-curPos);
#else
	aprintf("Cur : %llu end : %llu left: %llu\n",curPos,endPos,endPos-curPos);
#endif        
	if(curPos<endPos) 
		return 0;
	fseeko(fd,endPos,SEEK_SET);
	return 1;
}
riffParser::~riffParser()
{
	fseeko(fd,endPos,SEEK_SET);
	if(_root)
	{
		fclose(fd);
		
	}
}
riffParser::riffParser(const char *name)
{
	ADM_assert(fd=ADM_fopen(name,"rb"));
	startPos=0;
	fseeko(fd,0,SEEK_END);
	endPos=ftello(fd);
	fseeko(fd,0,SEEK_SET);
	curPos=0;
	_root=1;
}
riffParser::riffParser(riffParser  *father, uint32_t size)
{
	fd=father->fd;
	startPos=father->curPos;
	curPos=startPos;
	endPos=curPos+size;
	_root=0;
}
uint32_t riffParser::read32( void )
{
	uint32_t out;
	uint8_t in[4]={0,0,0,0};
	fread(in,1,4,fd);
#if 0//def ADM_BIG_ENDIAN
	out=in[3]+(in[2]<<8)+(in[1]<<16)+(in[0]<<24);
#else
	out=in[0]+(in[1]<<8)+(in[2]<<16)+(in[3]<<24);
#endif
	curPos+=4;
	return out;

}
