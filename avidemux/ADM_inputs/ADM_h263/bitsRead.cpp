/***************************************************************************
                          bitsRead.cpp  -  description
                             -------------------

                           Handle bit oriented stream 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ADM_default.h"
#include "ADM_editor/ADM_Video.h"
#include "ADM_assert.h"

#include "fourcc.h"
#include "ADM_h263.h"
#include "bitsRead.h"

const int mask[9]={
0,
1,
3,
7,
15,
31,
63,
127,
255
	

};
bitsReader::bitsReader(void )
{

	_rdIndex=_wrIndex;
	_rdBits=0;
	_fd=0;
	memset(_buffer,0,BR_BUFFER_SIZE);
	_size=_pos=0;
}
uint32_t bitsReader::getPos(void )
{
//	return _pos;
uint32_t pos;
		pos=ftell(_fd);
		return pos;
}
uint8_t	bitsReader::syncMpeg( uint8_t *sc )
{
uint32_t sync=0;
uint8_t c;
	sync=readByte();
	sync=(sync<<8) | readByte();

	while(1)
	{
		c=readByte();
		if(_pos>=_size) return 0;
		sync &=0xffff;
		sync<<=8;
		sync|=c;
		//printf("%x sync\n",sync);
		if(sync  ==0x1)
		{
			*sc=readByte();
			_curByte=readByte();
			_rdBits=0;
			return 1;
		}


	}
	return 0;

}

uint8_t bitsReader::sync(void )
{
	uint32_t sync=0;
	uint8_t c;
	//printf("--starting sync--\n");
	sync=readByte();
	sync=(sync<<8) | readByte();
	while(1)
	{
		c=readByte();
		if(_pos>=_size) return 0;
		sync &=0xffff;
		sync<<=8;
		sync|=c;
		//printf("%x sync\n",sync);
		if((sync & 0xffff80) ==0x80)
		{
			_curByte=c;
			_rdBits=6;
			return 1;
		}


	}
	return 0;

}

bitsReader::~bitsReader()
{
	if(_fd)
			{

				fclose(_fd);
				_fd=0;

			}
}
uint8_t bitsReader::open(const char *name)
{
	_fd=fopen(name,"rb");
	if(!_fd) return 0;

	fseek(_fd,0,SEEK_END);
	_size=ftell(_fd);
	fseek(_fd,0,SEEK_SET);
	//_curByte=readByte();
	return 1;
}

uint8_t bitsReader::readByte(void)
{
uint8_t c;
	fread(&c,1,1,_fd);
	//printf("%x read\n",c);
	_pos++;
	_rdBits=0;
	return c;

}
uint8_t bitsReader::forward(uint32_t nbBits)
{
        UNUSED_ARG(nbBits);
		ADM_assert(0);
}
uint8_t	bitsReader::read1bit( void )
{
uint32_t o=0;
	read(1,&o);
	if(o) return 1;
	return 0;



}
uint8_t bitsReader::read(uint32_t nbBits,uint32_t *val)
{
	uint32_t r=0;
	uint8_t left=0;
	uint8_t w=0; 
	ADM_assert(nbBits<32);
	while(1)
	{
		left=8-_rdBits;
		w=_curByte;
		if(left>nbBits)
		{
			w>>=left-nbBits;
			w&=mask[nbBits];
			_rdBits+=nbBits;
			r<<=nbBits;
			r+=w;
			
			*val=r;
			return 1;

		}
		// we take all that is left
		r<<=left;
		r+=w & mask[left];
		nbBits-=left;
		// refill 
		_curByte=readByte();
	}
	return 0;
}
// EOF
//
