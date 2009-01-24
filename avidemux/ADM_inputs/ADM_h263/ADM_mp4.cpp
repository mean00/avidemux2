/*
 *
 *
 * Simple mpeg4 parser
 *
 *
 * SHORT HEADER NOT DONE
 *
 * NEWPRED NOT CODED
 * SHORT HEADER NOT CODED
 *
 *
 *
 */
 #include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>

#include "ADM_default.h"
#include "ADM_editor/ADM_Video.h"
#include "ADM_assert.h"

#include "fourcc.h"
#include "ADM_h263.h"

#include "bitsRead.h"


typedef struct cb
{
	uint8_t code_min;
	uint8_t code_max;
	const char *comment;
	void (*func)(void);


}cb;

static void dummy_cb(void );
static void vo_start(void );
static void vop_start(void );
static void vol_start(void );
static void dummy_cb(void ) {};
static void user_data(void ) ;
extern void mixDump(uint8_t *ptr,uint32_t len);

static uint32_t time_inc;
static uint32_t time_bits=15;
static uint32_t x_width, x_height;
static bitsReader *parser=NULL;


cb cb_decode[]={
	{ 0x00,0x1f,"Video obj start",vo_start},
	{ 0x20,0x2f,"Video obj Layer start",vol_start},
	{ 0x30,0xAf,"RESERVED",dummy_cb},
	{ 0xB0,0xB0,"Visual obj Sequence Start",dummy_cb},
	{ 0xB1,0xB1,"Visual obj Sequence End",dummy_cb},
	{ 0xB2,0xB2,"User Data",user_data},
	{ 0xB3,0xB3,"Group of VOP start ",dummy_cb},
	{ 0xB4,0xB4,"Video session error ",dummy_cb},
	{ 0xB5,0xB5,"Visual obj start",dummy_cb},
	{ 0xB6,0xB6,"VOP start",vop_start},
	{ 0xB7,0xB9,"RESERVED",dummy_cb},
	{ 0xBA,0xFF,"RESERVED",dummy_cb}

};

#define ONEOPT(x) {if(parser->read1bit()) printf("\t"x"\n");			}
#define NOT_ONEOPT(x) {if(!parser->read1bit()) printf("\t"x"\n");			}
#define TWOPT(x,y) {if(parser->read1bit()) printf(x);	else printf(y);		}

//_________________________________
//_________________________________
uint8_t bitsNeeded(uint32_t in)
{
	uint8_t i=1;
	in--;
	while(in)
	{
		in>>=1;
		if(in) i++;

	}
	// ???
	//if(i<8) i--;
	return i;
}
void user_data(void )
{
	char buffer[512];
	uint16_t k=0;
	uint8_t c;
	do
			{
				c=buffer[k++]=parser->readByte();
			}while(c && (k<500));
	mixDump((uint8_t *)buffer,k);
	printf("\n");


}
void vol_start(void )
{
	const char *tt[]={"Reserved","Simple Object type","Simple scalable object type",
		   "Core object type","Main object type","N bit","Basic 2d","2D mesh",
		   "Simple face","Still text","Adv RT Simple","core scalable",
		   "Adv Coding efficiency","Adv Scalable Text","simple FBA"};

	const char *tar[]={"Forbidden","1:1","12:11","10:11","16:11","40:33","","","","","","","","","","Extended"};
		   
	uint32_t info,verid,pri;
		ONEOPT("Random access vol");	
		parser->read(8,&info);
		printf("\t: %s (%ld)\n",tt[info&15],info);
		info=parser->read1bit(); // is object layer
		if(info)
		{

				printf("\tis_vo_id\n");
				parser->read(4,&verid);
				parser->read(3,&pri);
				printf("\t\tverid: %ld\n",verid);
				printf("\t\tpri  : %ld\n",pri);
		}
		uint32_t ar;
		parser->read(4,&ar);
		printf("\t AR: %s (%ld)\n",tar[ar],ar);
		if(ar==15)
				{
					printf("\t\t %u %% %u\n",parser->readByte(),parser->readByte());

				}
		

		info=parser->read1bit(); // Vol control paramater
		if(info) {
			printf("\t Vol control parameter\n");
			parser->read(2,&info);
			printf("\tChroma %ld (1=yv12)\n",info);
			ONEOPT("Low Delay");
			info=parser->read1bit(); // VBV Stuff 
			if(info) {
					printf("\tVBV \n");
			parser->read(16,&info);
			parser->read(16,&info);
			parser->read(16,&info);
			parser->read(16,&info);
			parser->read(16,&info);
				}

			}
		parser->read(2,&info);
		printf("\tShape: %ld ",info);
		if(!info) printf(" rectangular");
		printf("\n");
		parser->read1bit(); // marker

		uint32_t u1,u2;
		
		//parser->read(16,&time_inc);
		parser->read(8,&u1);
		parser->read(8,&u2);
		time_inc=(u1<<8)+u2;

		printf("\t u1u2 %ld %ld",u1,u2);

		parser->read1bit(); // marker
		printf("\tTime inc:%ld",time_inc);
		
		time_bits=bitsNeeded(time_inc);
		printf("\n warning tims_bits forced to 12\n");
		//time_bits=12;
		
//		printf(" time coded on %u bits \n",time_bits);
		info=parser->read1bit(); // fixed_vop_rate
		if(info)
		{
			printf("\t\tFixed vop rate\n");
			parser->read(time_bits,&info); // time inc
			printf("\tFixed vop time inc:%ld\n",info);
		}
		parser->read1bit(); // marker
		parser->read(13,&info); // width 
		printf("\twidth:%ld\n",info);
		x_width=info;
		parser->read1bit(); // marker
		parser->read(13,&info); // height 
		x_height=info;
		printf("\theight:%ld\n",info);
		parser->read1bit(); // marker
		ONEOPT("Interlaced");	
		NOT_ONEOPT("ODBMC DISABLED");	
		if(verid==1)
		{
			ONEOPT("sprite enable");
		}
		else
		{
			parser->read(2,&info);
			printf("\tsprite enable : %ld\n",info);
		}
		if(parser->read1bit())  // not 8 bits
		{
			parser->read(4,&info);
			printf("\t quant precision :%ld\n",info);
			parser->read(4,&info);
			printf("\t bits per pixel  :%ld\n",info);

		}
		info=parser->read1bit(); // quant type
		printf("\t Quant type :%ld\n",info);
		if(info) 
		{
			printf("\t Extra quant matrix info--> NOT DECODED!!\n");
			info=parser->read1bit(); // load quant matrix
			if(info)
			{
				printf("\t\t Load intra Quant :%ld\n",info);
				while(parser->readByte());
			}
			info=parser->read1bit(); // load quant matrix
			if(info)
			{
				printf("\t\t Load non intra Quant :%ld\n",info);
				while(parser->readByte());
			}

		}
		if(verid!=1)
		{
			ONEOPT("quarter sample");
		}
		ONEOPT("Complexity estimation disable ");
		ONEOPT("Resync marker disable ");
	//	("Data partitionned ");
		info=parser->read1bit(); // quant type
		if(info)
		{
			printf("\tData partitionned :%ld\n",info);
			ONEOPT("Reversible VLC ");
		}
		ONEOPT("Newpred enable ");
		ONEOPT("Reduced resolution enable ");
}

void vop_start(void )
{
const char *cd[4]={"I Frame","P-frame","B Frame","Sprite"};	
//uint8_t is_vo_id;
uint32_t coding_type;
uint32_t info;

	parser->read(2,&coding_type);
	printf("\t%s\n",cd[coding_type]);
	while(parser->read1bit());
	parser->read1bit(); // marker
	parser->read(time_bits,&info);
	parser->read1bit(); // marker
	if(!parser->read1bit())
			{
				printf("\tno vop!\n");return;
			}
	if(coding_type==1) // P Frame
	{
		printf("\t Vop rounding :%d\n",parser->read1bit() );
	}



}

void vo_start(void )
{
	
	uint8_t is_vo_id;
	uint32_t verid;
	uint32_t pri;
	uint32_t objtype;
			is_vo_id=parser->read1bit();
			if(is_vo_id)
			{
				printf("\tis_vo_id\n");
				parser->read(4,&verid);
				parser->read(3,&pri);
				printf("\t\tverid: %ld\n",verid);
				printf("\t\tpri  : %ld\n",pri);


			}
			parser->read(4,&objtype);
			printf("\tvo type: %ld\n",objtype);
}
//
//
// Interface with avidemux
//
//
//______________________________________
//
// Open and index the (small) h263
//
//______________________________________
uint8_t    mp4Header::open(char *name)
{
	// uint32_t w,h,res=255;
	uint32_t nbImg=0;
	// uint32_t word;
	// uint32_t i=0;
	// uint32_t lastpos=0;
	uint32_t pos=0;
	// uint8_t intra=0;
	uint8_t sc=0;


	x_width=x_height=0;
	
	_fd=fopen(name,"rb");
	if(!_fd) return 0;

	// first pass to scan  # of frame
	// ___________________________________
	parser=new bitsReader();

	if(!parser->open(name))
			{
				printf("\n error parsing mp4\n");
				delete parser;
				return 0;
			}
	while( parser->syncMpeg(&sc))
	{
		if(sc==0xB6) // vop start
		{
			nbImg++;
		}
	}

	printf("\n Pass1 over \n\n");
	printf("\n Found : %ld frames \n",nbImg);
	delete parser;parser=NULL;

	parser=new bitsReader();
	if(!parser->open(name))
			{
				printf("\n error parsing mp4\n");
				delete parser;
				return 0;
			}
	_idx=new h263Entry[nbImg+1];
	ADM_assert(_idx);
	//
	// Second pass, collect as much information as possible
	//

	for(uint32_t img=0;img<nbImg;)
	{
		if(!parser->syncMpeg(&sc))
		{
			printf("\n cannot sync : %ld img\n",img);
			return 0;

		}
		if(sc==0xb6)
		{
		 if(!img)
		 {
			 _idx[0].offset=0;
			 _idx[0].intra=1;
			img++;
		 }
		 else
		 {
			_idx[img-1].offset=pos;
			pos=parser->getPos()-5;
			_idx[img-1].size=pos-_idx[img-1].offset;
			printf("Frame: %03ld At : %lx len %lx\n",img,(uint32_t)_idx[img-1].offset,(uint32_t)_idx[img-1].size);
			img++;
		 }

		}
		// needed to have size
	for(uint32_t i=0;i<sizeof(cb_decode)/sizeof(cb);i++)
		{
			if((sc>=cb_decode[i].code_min) &&
			(sc<=cb_decode[i].code_max) )
			{

				printf("%s\n",cb_decode[i].comment);
				printf("_______________________\n");
				cb_decode[i].func();
			}

		}


	}
	//
	//		Now build header info
	//
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
              _video_bih.biWidth=_mainaviheader.dwWidth=x_width ;
              _video_bih.biHeight=_mainaviheader.dwHeight=x_height;
              _videostream.fccHandler=fourCC::get((uint8_t *)"DIVX"); 
	      _video_bih.biCompression=_videostream.fccHandler;

	printf("\n Mp4 opened successfully\n");	
	printf("\n %ld x %ld , 25 fps hardcoded\n",x_width,x_height);	
	return 1;
}


