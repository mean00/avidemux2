/***************************************************************************
                          ADM_h263.cpp  -  description
                             -------------------

			Rebuild a pseudo avi from a raw h263 stream.
			Very basic /crude hack but enought for my needs.

    begin                : Tue Jun 4 2002
    copyright            : (C) 2002 by mean
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

#include "ADM_default.h"
#include "ADM_editor/ADM_Video.h"
#include "ADM_assert.h"

#include "fourcc.h"
#include "ADM_h263.h"
#include "bitsRead.h"
#include "DIA_coreToolkit.h"

#ifdef H263_VERBOSE
#define ONEOPT(x) {if(parser->read1bit()) printf(x);			}
#define TWOPT(x,y) {if(parser->read1bit()) printf(x);	else printf(y);		}
#else
#define ONEOPT(x) {}
#define TWOPT(x,y) {}
#endif
#define MAX_DELTA 15
static const char *resolution[8]={
		"\tForbidden\n",
		"\tSQcif\n",
		"\tQcif\n",
		"\tCIF\n",
		"\t4CIF\n",
		"\t16CIF\n",
		"\tReserved\n",
		"\tExtended\n"};
static const char *frameType[8]={
			"\t\t Intra\n",
			"\t\t Inter\n",
			"\t\t Imp PB\n",
			"\t\t B-frame\n",
			"\t\t EI\n",
			"\t\t EI\n",
			"\t\t Error\n",
			"\t\t Error\n"
		  };

uint8_t h263Header::setFlag(uint32_t frame,uint32_t flags){
    UNUSED_ARG(frame);
    UNUSED_ARG(flags);
	return 0;
}

uint32_t h263Header::getFlags(uint32_t frame,uint32_t *flags){
	if(frame>= (uint32_t)_videostream.dwLength) return 0;
	*flags=AVI_KEY_FRAME*_idx[frame].intra;
	return 1;
}


uint8_t  h263Header::getFrameNoAlloc(uint32_t framenum,ADMCompressedImage *img)
{
uint32_t offset=_idx[framenum].offset;
        if(framenum>= (uint32_t)_videostream.dwLength) return 0;
 	fseek(_fd,offset,SEEK_SET);
 	fread(img->data, _idx[framenum].size, 1, _fd);
  	img->dataLength=_idx[framenum].size;
 	return 1;
}

uint8_t    h263Header::close( void )
{
	if(_fd)
 		{
               	fclose(_fd);
             	}
              _fd=NULL;
	if(_idx)
	{
		delete [] _idx;
		_idx=NULL;
	}
 	return 1;
}
//______________________________________
//
// Open and index the (small) h263
//
//______________________________________
uint8_t    h263Header::open(const char *name)
{
	uint32_t w,h,res=255,nbImg=0;
	uint32_t word;
	uint32_t delta;
	uint32_t pts;
	uint32_t last_pts=0;
	uint32_t i=0;
	uint32_t pos=0;
	uint8_t intra=0;//,size;

	_fd=fopen(name,"rb");
	if(!_fd) return 0;

	// first pass to scan  # of frame
	// ___________________________________
	bitsReader *parser=new bitsReader();

	if(!parser->open(name))
			{
				printf("\n error parsing h263\n");
				delete parser;
				return 0;
			}
	while( parser->sync())
	{
		parser->read(8,&pts);
		parser->read(8,&word);
		delta=256+pts-last_pts;
		delta &=0xff;
		if(!nbImg) delta=0;
		if(((word&0xc0)==0x80) && ( delta<MAX_DELTA))
		{
			last_pts=pts;
			nbImg++;
		}
	}

	printf("\n Pass1 over \n\n");
	printf("\n Found : %ld frames \n",nbImg);
	delete parser;parser=NULL;

	parser=new bitsReader();
	if(!parser->open(name))
			{
				printf("\n error parsing h263\n");
				delete parser;
				return 0;
			}
	_idx=new h263Entry[nbImg+1];
	ADM_assert(_idx);

	last_pts=0;
	nbImg=0;
	// second pass, index it
	//___________________
	while( parser->sync())
	{

		parser->read(8,&pts);
		parser->read(8,&word);
		delta=256+pts-last_pts;
		delta &=0xff;
		if(!nbImg) delta=0;
		//++++++++++++++PTYPE++++++++++++
		if(((word&0xc0)==0x80) && ( abs(delta)<MAX_DELTA))
		{
		last_pts=pts;
		nbImg++;
		pos=parser->getPos()-5;
#ifdef H263_VERBOSE
		printf("\n[+PSC+] at 0x%x\n",pos);
		printf("[-TR--] %x\n",pts);
		printf("[PTYPE] %04x \n",word);
#endif	
		if(word&0x40) printf("\t Split Screen\n");
		if(word&0x20) printf("\t Document Camera indicator \n");
		if(word&0x10) printf("\t Full Picture Freeze Release \n");
		word&=7;
		printf ("%s",resolution[word]);
		res=word;
		if(word!=7) // no extended
			{
	
				if(parser->read1bit()) 
				{
					printf("\tInter\n");intra=0;
				}
				else
				{
					printf("\tIntra\n");intra=1;
				}
			//	TWOPT("\tINTER\n","\tINTRA\n");
				ONEOPT("\tUnrestricted MV\n");
				ONEOPT("\tSAC MV\n");
				ONEOPT("\tAdv prediction mode (4mv)\n");
				ONEOPT("\tPB frames\n");
			}	
		else
			{ // ------------------OPPTYPE---------------------
		  	parser->read(3,&word);
		  	printf("\t\t UFEP: %lx\n",word);
		  	if(word==1)
		  	{
		  		parser->read(3,&word);
				printf ("%s",resolution[word]);
				res=word;
				ONEOPT("\t\t custom PCF\n");
				ONEOPT("\t\t UMV \n");
				ONEOPT("\t\t SAC \n");
				ONEOPT("\t\t ADV Pred \n");
				ONEOPT("\t\t AIC \n");
				ONEOPT("\t\t Deblocking \n");
				ONEOPT("\t\t Slice \n");
				ONEOPT("\t\t RPS \n");
				ONEOPT("\t\t ISD \n");
				ONEOPT("\t\t Altern Intra VLC \n");
				ONEOPT("\t\t Modified Qz  \n");
				parser->read(4,&word);
				if(word!=0x8) printf("\t\t **OOPS\n");
	
		   	}
		  	parser->read(3,&word);
		  	printf("%s",frameType[word]);
				ONEOPT("\t\t RPR \n");
				ONEOPT("\t\t RRU \n");
				ONEOPT("\t\t RType \n");
				ONEOPT("\t\t error (1) \n");
				ONEOPT("\t\t error (3) \n");
				if(!parser->read1bit()) printf("\t\t error (2) \n");
		  	}
	
			if(i)
					{
						_idx[i-1].size=pos-_idx[i-1].offset;
					}
				_idx[i].offset=pos;
				_idx[i].intra=intra;
				printf(" Frame %ld at %lx intra :%d\n",i,pos,intra);
				i++;
		}
	}
	//
	//		Now build header info
	//
	switch(res)
	{
		default:
			printf("\n incorrect size !\n");
                        GUI_Error_HIG(QT_TR_NOOP("Size is not (s)QCIF"), NULL);
		
		case 1:  w=128;h=96;break;
		case 2:  w=176;h=144;break;
			
	}
	_idx[0].intra=1;
 	_isaudiopresent=0; // Remove audio ATM
       	_isvideopresent=1; // Remove audio ATM
		
#define CLR(x)              memset(& x,0,sizeof(  x));

              CLR( _videostream);
              CLR(  _mainaviheader);

    	      _videostream.dwScale=1;
              _videostream.dwRate=25;
              _mainaviheader.dwMicroSecPerFrame=40000;;     // 25 fps hard coded
              _videostream.fccType=fourCC::get((uint8_t *)"vids");
              _video_bih.biBitCount=24;
              _videostream.fccHandler=0;
              _videostream.dwLength= _mainaviheader.dwTotalFrames=nbImg;
              _videostream.dwInitialFrames= 0;
              _videostream.dwStart= 0;
              _video_bih.biWidth=_mainaviheader.dwWidth=w ;
              _video_bih.biHeight=_mainaviheader.dwHeight=h;
              _videostream.fccHandler=fourCC::get((uint8_t *)"H263"); 
	      _video_bih.biCompression=_videostream.fccHandler;

	
	return 1;
}








