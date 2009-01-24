/***************************************************************************
                          ADM_ompages.cpp  -  description
                             -------------------

			Low level page handler

    begin                : Tue Apr 28 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr
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
#include "ADM_assert.h"

#include "ADM_default.h"
#include "fourcc.h"

#include "ADM_ogm.h"
#include "ADM_ogmpages.h"

OGMDemuxer::OGMDemuxer(void )
{
		memset(&_page,0,sizeof(_page));
		_fd=NULL;
		_payload=0;
		_filesize=0;

}

uint8_t OGMDemuxer::open(const char *name)
{
		_fd=fopen(name,"rb");
		if(!_fd) return 0;
		fseeko(_fd,0,SEEK_END);
		_filesize=ftello(_fd);
		fseeko(_fd,0,SEEK_SET);
		return 1;

}
OGMDemuxer::~OGMDemuxer()
{
	if(_fd)
		{
			fclose(_fd);
			_fd=NULL;
		}
}
uint64_t OGMDemuxer::getFileSize( void )
{
	return _filesize;
}
uint64_t OGMDemuxer::getPos( void )
{
	return _hdrpos;
}
uint8_t OGMDemuxer::setPos( uint64_t pos )
{
	_payload=0;
	fseeko(_fd,pos,SEEK_SET);
	ADM_assert(pos<_filesize);
	_payload=0;
	return 1;
}
uint8_t		OGMDemuxer::readHeaderOfType(uint8_t type,uint32_t *paySize, uint32_t *flags, uint64_t *frame)
{
uint8_t id;
	while(1)
	{
		id=0xff;
		if(!readHeader(paySize,flags,frame,&id)) return 0;
		if(id==type) return 1;

	}

}
uint8_t OGMDemuxer::dumpHeaders(uint8_t *ptr,uint32_t *size)
{
	memcpy(ptr,&_page,sizeof(_page));
	ptr+=sizeof(_page);
	for(uint32_t i=0;i<_page.nb_segment;i++)
	{
		*ptr++=_lace[i];
	}
	*size=sizeof(_page)+_page.nb_segment;
	return 1;


}

uint8_t		OGMDemuxer::readHeader(uint32_t *paySize, uint32_t *flags, uint64_t *frame,uint8_t *id)
{
uint8_t gotcha=0,c=0;
uint32_t total=0,failed=0;
uint64_t seq;


	*frame=0;

	if(_payload) fseeko(_fd,_payload,SEEK_CUR);
	_payload=0;
	while(1)
	{
		_hdrpos=ftello(_fd);
		if(fread(&_page,sizeof(_page),1,_fd)!=1) return 0;

		if(!fourCC::check(_page.sig,(uint8_t *)"OggS"))
		{
			printf("Bad at offset :%lu 0x %x\n",_hdrpos,_hdrpos);
			ADM_assert(0);
		}
		*id=fourCC::get(_page.serial);
		//
		
		seq=_page.page_sequence[0]+(_page.page_sequence[1]<<8);
		//printf("seq:%x \n",seq);
		uint64_t *ll;
		ll=(uint64_t *)_page.abs_pos;		
		//printf("abs:%llx \n",*ll);
		
		//
		//if(fourCC::check(_trackId,_page.serial)) // got it
		{
			gotcha=1;
		}
		total=0;
		_nbLace=_page.nb_segment;
		_nbFrag=0;
		for(uint32_t i=0;i<_page.nb_segment;i++)
		{

			c=fgetc(_fd);
			_lace[i]=c;
			if(c!=0xff) _nbFrag++;
			total+=c;
		}


		if(gotcha)
		{
#if 0		
			*frame=*(unsigned long long *)_page.abs_pos;
#else			
			*frame+=_page.abs_pos[4]+(_page.abs_pos[5]<<8)+(_page.abs_pos[6]<<16)+
					(_page.abs_pos[7]<<24);
			*frame=(*frame)<<32;
			*frame=_page.abs_pos[0]+(_page.abs_pos[1]<<8)+(_page.abs_pos[2]<<16)+
					(_page.abs_pos[3]<<24);
			if(*frame>48000*3 && seq==3)
			{
				printf("Fixing stupid abs_pos\n");
				*frame=0;
			}
			
			
#endif			
			*flags=_page.header_type;
			*paySize=total;
			_payload=total;
			return 1;
		}
		// skipt it
		fseek(_fd,total,SEEK_CUR);
		failed++;
		if(failed>10)
		{
			_payload=0;
			return 0; // we allow 10 fails
		}
	}
	return 0;
}
uint8_t		OGMDemuxer::readPayload( uint8_t *data)
{

		ADM_assert(_payload);
		fread(data,1,_payload,_fd);
		_payload=0;
		return 1;


}
uint8_t		OGMDemuxer::readBytes(uint32_t size, uint8_t *data)
{

		if(size>_payload)
		{
			printf("Oops: %lu size, %lu payload\n",size,_payload);
			exit(0);
		
		}
		fread(data,1,size,_fd);
		_payload-=size;
		return 1;

}
uint8_t		OGMDemuxer::skipBytes(uint32_t size)
{

		ADM_assert(size<=_payload);
		fseeko(_fd,size,SEEK_CUR);
		_payload-=size;
		return 1;

}


