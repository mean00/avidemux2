/***************************************************************************
  Small scanner for avi/openDML streams
    
            
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define uint8_t unsigned char
#define uint32_t unsigned long int
#define uint64_t unsigned long long int
//#define QUIET

#ifdef QUIET
	#define Q(c) NULL
#else
	#define Q(x) x
#endif
int nesting=0;
int parse(int maxSize);
void riff(int maxSize);
void skip(int maxSize);
void list(int maxSize);
void indx(int maxSize);
void ix(int maxSize);
void idx1(int maxSize);

#define xprintf for(int i=0;i<nesting;i++) printf("\t");printf

FILE *fd=NULL;
//
typedef struct
{
	const char *tag;
	bool       recurse;
	void       (*func)(int maxSize);
}chunkDesc;
//
chunkDesc allChunks[]=
{
	{"RIFF",false,riff},
	{"AVI ",true,NULL},
	{"LIST",false,list},
	{"avih",false,NULL},
	{"strh",false,NULL},
	{"strf",false,NULL},
	{"indx",false,Q(indx)},
	{"ix00",false,Q(ix)},
	{"ix01",false,Q(ix)},
	{"00dc",false,NULL},
	{"01wd",false,NULL},
	{"JUNK",false,NULL},
	{"idx1",false,Q(idx1)},

	{NULL,true,NULL}

};
uint32_t read32()
{
	uint32_t r=0;
	for(int i=0;i<4;i++)
	{
		int x=fgetc(fd);
		r=r+(x<<(8*i));
	}
	return r;
}
uint64_t read64()
{
	uint64_t r=0;
	for(int i=0;i<8;i++)
	{
		int x=fgetc(fd);
		r=r+(x<<(8*i));
		//r=(r<<8)+x;
	}
	return r;
}
//
chunkDesc *lookupChunk(const char *n)
{
	chunkDesc *c=allChunks;
	while(1)
	{
		if(!c->tag) return NULL;
		if(!strcmp(c->tag,n)) return c;
		c++;
	}
	return NULL;	
}
//
void ix(int maxSize)
{
	uint64_t base;
	uint32_t pos=ftello(fd);
	skip(4);
	uint32_t n,fcc;
	n=read32();
        fcc=read32();
	char xfcc[5];
	*(uint32_t *)xfcc=fcc;
	xfcc[4]=0;
	base=read64();
	read32();
	xprintf("At 0x%lx, found regular index for %s,nbEntries=%d base=0x%llx\n",pos,xfcc,(int)n,base);
	for(int i=0;i<n;i++)
	{
		uint64_t off=read32();
		uint32_t size=read32();
		xprintf("at offset 0x%llx, absolute offset=0x%llx size=%d\n",
				off,off+base,(int)(size&0x7fffffff));
	}	
	fseeko(fd,pos+maxSize,SEEK_SET);

}
void idx1(int maxSize)
{
	int n=maxSize/16;
	for(int i=0;i<n;i++)
	{
		uint32_t fcc=read32();
		char xfcc[5];
		*(uint32_t *)xfcc=fcc;
		xfcc[4]=0;
		uint32_t y=read32();
		uint32_t z=read32();
		uint32_t t=read32();
		xprintf("\t %s : off=0x%x size=%d\n",xfcc,z,t);
	}
}
	//
void indx(int maxSize)
{
	uint32_t pos=ftello(fd);
	skip(4);
	uint32_t n,fcc;
	n=read32();
        fcc=read32();
	char xfcc[5];
	*(uint32_t *)xfcc=fcc;
	xfcc[4]=0;
	read32();
	read32();
	read32();
	xprintf("At 0x%lx, found index for %s,nbEntries=%d\n",pos,xfcc,(int)n);
	for(int i=0;i<n;i++)
	{
		uint64_t off=read64();
		uint32_t size=read32();
		uint32_t duration=read32();
		xprintf("at offset 0x%llx, size=%d, duration=%d\n",off,(int)size,(int)duration);
	}	
	fseeko(fd,pos+maxSize,SEEK_SET);

}
void skip(int maxSize)
{
	fseeko(fd,maxSize,SEEK_CUR);
}
//---
void riff(int maxSize)
{
	skip(4);
	maxSize-=4;
	uint64_t tail=ftello(fd)+maxSize;
	while(1)
	{
		if(ftello(fd)==tail) return;
		int size=tail-ftello(fd);
		parse(size);
	}

}
void list(int maxSize)
{
	char y[5];
	fread(y,4,1,fd);
	maxSize-=4;
	y[4]=0;
	xprintf("\t of type %s\n",y);
	if(!strcmp(y,"index"))
		return skip(maxSize);
#ifdef QUIET
	if(!strcmp(y,"movi"))
		return skip(maxSize);
#endif
	//
	uint64_t tail=ftello(fd)+maxSize;
	while(1)
	{
		if(ftello(fd)==tail) return;
		int size=tail-ftello(fd);
		parse(size);
	}

}
//_____________________________
//_____________________________
int main(int argc, char **argv)
{
	if(!argv[1]) exit(0);
	
	fd=fopen(argv[1],"rb");
	if(!fd)
	{
		
			xprintf("Cannot open %s\n",argv[1]);
	}
	fseeko(fd,0,SEEK_END);
	uint64_t fileSize=ftello(fd);
	fseeko(fd,0,SEEK_SET);
	xprintf("Scanning file %s, size=%llu\n",argv[1],fileSize);
	parse(fileSize);
	fclose(fd);
	xprintf("\nDone.\n");
}

// Parse until we find a PES header to identify the stream
//
int parse(int maxSize)
{
	uint8_t fcc[5],len[5];
	uint64_t pos=ftell(fd);
	fread(fcc,4,1,fd);
	fread(len,4,1,fd);	
	nesting++;
	fcc[4]=0;
	uint64_t l=len[0]+(len[1]<<8)+(len[2]<<16)+(len[3]<<24);
	if(l&1) l++;
	xprintf("Found tag %s len=%llu at pos %llx\n",fcc,l,pos);
	const chunkDesc *chunk=lookupChunk((const char *)fcc);
	bool recurse=false;
	if(!chunk)
	{
		xprintf("Unknown chunk\n");
		xprintf("Skipping\n");
		fseeko(fd,l,SEEK_CUR);	
	}else
	{
		if(chunk->func)
			chunk->func(l);
		else
			if(chunk->recurse==true)
				parse(l);
			else
			{
				xprintf("Skipping\n");
				skip(l);
			}
	}
	uint64_t pos2=ftell(fd);
	if(pos+l+8!=pos2)
	{
		xprintf("Mismatch : 0x%llx != 0x%llx + 0x%llx +8\n",pos2,pos,l);
		xprintf("Mismatch : %lld != %lld + %lld +8\n",pos2,pos,l);
		exit(-1);
	}
	nesting--;
	return 0;
}
