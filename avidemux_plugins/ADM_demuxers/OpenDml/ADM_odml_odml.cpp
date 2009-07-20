/***************************************************************************
                          ADM_OpenDML.cpp  -  description
                             -------------------

		OpenDML index reader
		Read the opendml type index

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"
#include "ADM_openDML.h"


#define aprintf(...) {}

//#define OPENDML_VERBOSE

#if defined( __WIN32) || defined(ADM_CPU_X86_64)
	#define PPACKED __attribute__ ((packed, gcc_struct))
#else
	#define PPACKED
#endif

typedef struct  //
{
	//uint32_t	fcc;
	//uint32_t 	cb: // ?
	uint16_t 	longsPerEntry;
	uint8_t		indexSubType;
	uint8_t		indexType;
	uint32_t	nbEntryInUse;
	uint32_t	chunkId;
	uint32_t	reserver[3];
}PPACKED OPENDML_INDEX;


typedef struct
{
	uint64_t 	offset;
	uint32_t	size;
	uint32_t	duration;
}PPACKED OPENDML_ENTRY ;


typedef struct
{
	uint16_t 	longsPerEntry;
	uint8_t		indexSubType;
	uint8_t		indexType;
	uint32_t	nbEntryInUse;
	uint32_t	chunkId;
	uint64_t	base;
	uint32_t	reserver;
} PPACKED  OPENML_SECONDARY_INDEX ;


 static int readMasterIndex(OPENDML_INDEX *index,FILE *fd);
 static int readSuperEntries(OPENDML_ENTRY *entries,int count,FILE *fd);
 static int readSecondary(OPENML_SECONDARY_INDEX *index,FILE *fd);
 /*
     Go here to solve endianness issues
 */
 int readMasterIndex(OPENDML_INDEX *index,FILE *fd)
 {
     if(1!=fread(index,sizeof(OPENDML_INDEX),1,fd)) return 0;
 #ifdef    ADM_BIG_ENDIAN

 #define REV16(x)   index->x=R16(index->x)
 #define REV32(x)   index->x=R32(index->x)
 #define REV64(x)   index->x=R64(index->x)
         REV16(longsPerEntry);
         REV32(nbEntryInUse);
         REV32(chunkId);
 #endif
     return 1;
 }
 int readSuperEntries(OPENDML_ENTRY *entries,int count,FILE *fd)
 {
     if(1!=fread(entries,sizeof(OPENDML_ENTRY)*count,1,fd)) return 0;
 #ifdef ADM_BIG_ENDIAN
     OPENDML_ENTRY *index;
     for(int i=0;i<count;i++)
     {
         index=&(entries[i]);
         REV64(offset);
         REV32(size);
         REV32(duration);
     }
 #endif

     return 1;
 }
 int readSecondary(OPENML_SECONDARY_INDEX *index,FILE *fd)
 {
     if(1!=fread(index,sizeof(OPENML_SECONDARY_INDEX),1,fd)) return 0;
 #ifdef ADM_BIG_ENDIAN
         REV16(longsPerEntry);
         REV32(nbEntryInUse);
         REV32(chunkId);
         REV64(base);
         REV32(reserver);
 #endif
     return 1;
 }


/*
	Try to index if it is/was an openDML file
	with super Index

	audTrack is the index in Tracks of the audio track
	audioTrackNumber is either 0-> First track
				   1-> Second track

	In case of openml file, audioTrack is enough
	In case of avi file, audioTrackNumber is used.

*/
uint8_t		OpenDMLHeader::indexODML(uint32_t vidTrack)
{
uint32_t total;

	printf("Building odml video track\n");
	if(!scanIndex(vidTrack,&_idx,&total))
        {
                printf("Odml video index failed\n");
		return 0;
        }
 	_videostream.dwLength= _mainaviheader.dwTotalFrames=total;
	printf("\nBuilding odm audio tracks\n");
	for(int i=0;i<_nbAudioTracks;i++)
        {
		printf("\nDoing track %d of %d\n",i,_nbAudioTracks);
                if(!scanIndex(     _audioTracks[i].trackNum,
                                &(_audioTracks[i].index),
                                &(_audioTracks[i].nbChunks)))
                {
                        printf("Odml audio %d tracknum %d, index failed\n",i,_audioTracks[i].trackNum);
                        return 0;
                }
        }
        printf("Odml indexing succeeded\n");
	return 1;
}

/*
	Build index for the given track
	Returns also the number of chunk/frame found

*/
uint8_t		OpenDMLHeader::scanIndex(uint32_t track,odmlIndex **index,uint32_t *nbElem)
{
OPENDML_INDEX 	masterIndex;
uint32_t 	i,j;

	// Jump to index of vidTrack
	printf("[AVI]Trying ODML super index..\n");
#define szeof(x) printf("Sizeof "#x":%d\n",(int)sizeof(x));
        szeof(OPENDML_INDEX);
        szeof(OPENDML_ENTRY);
        szeof(OPENML_SECONDARY_INDEX);


	if(!_Tracks[track].indx.offset)
	{
		printf("[AVI]No indx field.\n");
		return 0;
	}
	fseeko(_fd,_Tracks[track].indx.offset,SEEK_SET);
        if(!readMasterIndex(&masterIndex,_fd))   //if(1!=fread(&masterIndex,sizeof(masterIndex),1,_fd))
		{
			printf("[AVI]Problem reading master index\n");
			return 0;
		}
	if(masterIndex.indexType) // not a super index ?
		{
			printf("[AVI]Not a super index!\n");
			return 0;
		}
	printf("[AVI]Master index of "),fourCC::print(masterIndex.chunkId);printf(" found\n");
	printf("[AVI]SubType : %"LU"\n",masterIndex.indexSubType);



	OPENDML_ENTRY superEntries[masterIndex.nbEntryInUse];
	printf("[AVI]We have %"LU" indeces\n",masterIndex.nbEntryInUse);
        if(!readSuperEntries(superEntries,masterIndex.nbEntryInUse,_fd)) //if(1!=fread(superEntries,sizeof(OPENDML_ENTRY)*masterIndex.nbEntryInUse,1,_fd))
	{
		printf("[AVI]Problem reading indices\n");
		return 0;
	}
	// now we have the master index complete
	// time to scan each index and create
	// the final index
	uint32_t 		fcc,len,total=0;;
	OPENML_SECONDARY_INDEX 	second;
	for( i=0;i<masterIndex.nbEntryInUse;i++)
	{
		fseeko(_fd,superEntries[i].offset,SEEK_SET);
		fread(&fcc,4,1,_fd);
		fread(&len,4,1,_fd);
                if(!readSecondary(&second,_fd)) //if(1!=fread(&second,sizeof(second),1,_fd))
		{
			printf("[AVI]Problem reading secondary index (%u/%u) trying to continue \n",i,masterIndex.nbEntryInUse);
			goto _cntue;
		}
		total+=second.nbEntryInUse;
	}
_cntue:
	printf("Found a grand total of %"LU" frames\n",total);
	*nbElem=total;

	// second pass, actually assign them
	*index=new odmlIndex[total];
	uint32_t count=0;
	for( i=0;i<masterIndex.nbEntryInUse;i++)
	{
		fseeko(_fd,superEntries[i].offset,SEEK_SET);
		fcc=read32();
		len=read32();
                aprintf("subindex : %"LU" size %"LU" (%lx)",i,len,len);

                aprintf("Seeking to %"LLX"\n",superEntries[i].offset);
		fourCC::print(fcc);aprintf("\n");
		//if(1!=fread(&second,sizeof(second),1,_fd))
                if(!readSecondary(&second,_fd))
		{
			printf("Problem reading secondary index (%u/%u) trying to continue \n",i,masterIndex.nbEntryInUse);
			return 1;
		}

                aprintf("Base : %"LLX"\n",second.base);
		uint32_t sizeflag;
		for( j=0;j<second.nbEntryInUse;j++)
		{
			if(second.indexSubType) // field
			{
				aprintf("Field.\n");
			}
			else
			{

				(*index)[count].offset=read32();
				(*index)[count].offset+=second.base;
				sizeflag=read32();
				(*index)[count].size=sizeflag&0x7fffffff;
				if(sizeflag & 0x80000000)
					(*index)[count].intra=0;
				else
					(*index)[count].intra=AVI_KEY_FRAME;

                                aprintf("Frame.off : %"LLX", size %"LLX"\n",
                                        _idx[count].offset,
                                        _idx[count].size);

				count++;

			}

		}
	}

	return 1;
}

