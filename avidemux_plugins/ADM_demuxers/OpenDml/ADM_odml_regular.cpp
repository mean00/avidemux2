//
//
// C++ Implementation: ADM_odml_regular
//
// Description: This file tries to read a type 1 avi index, building both audio and video 
//			index all along
//
//
// Author: mean <fixounet@free.fr>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include <string.h>
#include <math.h>

#include "ADM_default.h"

#ifdef ADM_DEBUG
	//#define ODML_INDEX_VERBOSE
#endif

#include "ADM_Video.h"

#include "fourcc.h"
#include "ADM_openDML.h"


#define aprintf(...) {}

uint8_t		OpenDMLHeader::indexRegular(uint32_t vidTrack)
{
uint32_t fcc,flags,offset,len;
uint32_t count,total=0;
uint32_t trackId,ccType;
uint32_t audioCount[9]={0,0,0,0, 0,0,0,0,0};
uint32_t forward[9];
uint32_t backward[9];
int64_t startOfData;
  
	printf("Trying avi type 1 index\n");
	if(!_regularIndex.offset)
	{
		printf("No regular index\n");
		return 0;
	}
	fseeko(_fd,_regularIndex.offset,SEEK_SET);
	
	// first pass : count how much
	count=_regularIndex.size >> 4;
	while(count)
	{
		fcc=len=0;
		fcc=read32();
		flags=read32();
		offset=read32();
		len=read32();	
		
		if(fcc==MKFCC('r','e','c',' '))
		{
			_recHack=1;
			count--;
			continue;
		}
		
		trackId=((fcc>>8) & 0xFF) -'0';;
		if(trackId>9) trackId=0;
	  	ccType=fcc >>16;
		switch(ccType)
		{
		
		case MKFCC('d','c',0,0):	// video tracks
		case MKFCC('d','b',0,0):
			total++;
			break;
		case MKFCC('w','b',0,0):					
			audioCount[trackId]++;
			break;	
		default:
			printf("Unknown fcc:");fourCC::print(fcc);printf("\n");
		}				
		count--;
		
	}
	
	printf("Found %u videos chunk\n",total);
        for(int i=0;i<9;i++)
                printf("Audio track :%d, %u audio chunk\n",i,audioCount[i]);
	fseeko(_fd,_regularIndex.offset,SEEK_SET);

	_idx=new odmlIndex[total];
	memset(_idx,0,sizeof(odmlIndex)*total);
	count=0;

        int run=0;
        // Pack tracks
        for(int i=0;i<9;i++)
        {
                if(audioCount[i])
                {
                         forward[run]=i;
                         backward[i]=run;
                         run++;
                }
        }
        ADM_assert(run==_nbAudioTracks);

        // Create index
        uint32_t nb;
        for(int i=0;i<_nbAudioTracks;i++)
        {
                nb=audioCount[forward[i]];
                _audioTracks[i].index=new odmlIndex[nb+1];
                memset(_audioTracks[i].index,0,(sizeof(odmlIndex)*(nb+1)));
                _audioTracks[i].nbChunks=0;
        }

        uint32_t audiocount=0,videocount=0;
        uint32_t audiototal=0;
        uint32_t audioSize=0;
        odmlIndex *track;
        int     Achunk;
        count=_regularIndex.size >> 4;
        while(count)
	{
_again:
                fcc=len=0;
                fcc=read32();
                flags=read32();
                offset=read32();
                len=read32();
                count--;	
                trackId=((fcc>>8) & 0xFF) -'0';;
                if(trackId>9) trackId=0;
                ccType=fcc >>16;
                if(videocount==0 && audiocount==0) // first chunk met
                {
                        // the first one is at movie_offset-4+8
                        // Its offset + startoff data should be there
                        startOfData=_movi.offset+8;
                        startOfData-=offset;
                        if(_recHack)
                        {
                                startOfData+=4+4+4;
                        }	
                }
                if(fcc==MKFCC('r','e','c',' '))
                {
                        _recHack=1;
                        continue;
                }
                switch(ccType)
                {

                case MKFCC('d','c',0,0):	// video tracks
                case MKFCC('d','b',0,0):
                        _idx[videocount].offset=offset;
                        _idx[videocount].offset+=startOfData;
                        _idx[videocount].size=len;
                        _idx[videocount].intra=flags;
#ifdef ODML_INDEX_VERBOSE			
                        printf("Vid:at %llx size %lu (%lu/%lu) |",_idx[videocount].offset,len,videocount,total);fourCC::print(fcc);printf("|\n");
                        if(offset & 1) printf("!!!! ODD !!!");	
#endif			
                        videocount++;
                        break;
                case MKFCC('w','b',0,0):
                        nb=backward[trackId];
                        ADM_assert(nb<_nbAudioTracks);
                        track=_audioTracks[nb].index;

                        Achunk=_audioTracks[nb].nbChunks;
                        _audioTracks[nb].nbChunks++;

                        track[Achunk].offset=offset;
                        track[Achunk].offset+=startOfData;
                        track[Achunk].size=len;
                        track[Achunk].intra=flags;
                        _audioTracks[nb].totalLen+=len;

                        audioSize+=len;
                        if(offset & 1) printf("!!!! ODD !!!");	
                        audiocount++;	
                        break;	

                default:
                        printf("Idx Regulat: Unknown fcc:");fourCC::print(fcc);printf("\n");
                }	

        }
        _videostream.dwLength= _mainaviheader.dwTotalFrames=total;
        if(total)
                _idx[0].intra=AVI_KEY_FRAME;
        printf("Found %u video \n",total);
        return 1;
}
