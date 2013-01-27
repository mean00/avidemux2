/***************************************************************************
                          ADM_OpenDML.cpp  -  description
                             -------------------
	
		OpenDML Demuxer
		
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
#include "DIA_coreToolkit.h"
#include "ADM_odml_audio.h"
#include "ADM_coreUtils.h"

#ifdef ADM_DEBUG
	#define OPENDML_VERBOSE
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif
#define QT_TR_NOOP(x) x

uint8_t OpenDMLHeader::setFlag(uint32_t frame,uint32_t flags)
{
    UNUSED_ARG(frame);
    UNUSED_ARG(flags);
    if(frame>= (uint32_t)_videostream.dwLength) return 0;
	_idx[frame].intra=flags;    
	return 1;
}
/**
        \fn getFlags
*/
uint32_t OpenDMLHeader::getFlags(uint32_t frame,uint32_t *flags)
{
*flags=0;
	if(frame>= (uint32_t)_videostream.dwLength) return 0;

	if(fourCC::check(_videostream.fccHandler,(uint8_t *)"MJPG"))
	{
		*flags=AVI_KEY_FRAME;
	}
	else
	{
		if(_idx[frame].intra & AVI_KEY_FRAME) 		*flags=AVI_KEY_FRAME;
		else if(_idx[frame].intra & AVI_B_FRAME)	*flags=AVI_B_FRAME;
	}
	if(!frame) *flags=AVI_KEY_FRAME;
	return 1;
}
/**
        \fn getFrameSize
*/
 uint8_t  OpenDMLHeader::getFrameSize(uint32_t frame,uint32_t *size) 
{
	*size=0;
	if(frame>= (uint32_t)_videostream.dwLength) return 0;
	*size=_idx[frame].size;
	return 1;
}
/**
    \fn getExtraHeaderData
*/
uint8_t OpenDMLHeader::getExtraHeaderData(uint32_t *len, uint8_t **data)
{
	if(_videoExtraLen)
	{
		*len=_videoExtraLen;
		*data=_videoExtraData;
		return 1;
	}
	*len=0;
	*data=NULL;
	return 0;

}
/**
    \fn frameToUs
    \brief convert a framenumber to us
*/
uint64_t OpenDMLHeader::frameToUs(uint32_t frame)
{
    float f=frame;
    f*=_videostream.dwScale;
    f/=_videostream.dwRate;
    f*=1000000.;
    aprintf("[openDML]%u->%f\n",frame,f);
    return (uint64_t)f;
}
/**
    \fn getFrame
    \return frame # framenum in img
*/
uint8_t  OpenDMLHeader::getFrame(uint32_t framenum,ADMCompressedImage *img)
{
    if(framenum>= (uint32_t)_videostream.dwLength) return 0;
uint64_t offset=_idx[framenum].offset; //+_mdatOffset;
	
 	fseeko(_fd,offset,SEEK_SET);
 	fread(img->data, _idx[framenum].size, 1, _fd);
  	img->dataLength=_idx[framenum].size;
        img->flags=_idx[framenum].intra;
        img->demuxerDts=_idx[framenum].dts; // FIXME
        img->demuxerPts=_idx[framenum].pts;
        
	aprintf("Size: %lu\n",_idx[framenum].size);
//	if(offset & 1) printf("odd!\n");
 	return 1;
}
/**
    \fn getFrame
    \brief Return PTS of given frame. It is only approximate...
*/
uint64_t OpenDMLHeader::getTime(uint32_t frameNum)
{
    if(frameNum>= (uint32_t)_videostream.dwLength) return ADM_NO_PTS;
    return _idx[frameNum].pts; 

}
/**
    \fn getVideoDuration
*/
uint64_t  OpenDMLHeader::getVideoDuration(void)
{
    if(!_videostream.dwLength) return 0;
    return _idx[_videostream.dwLength-1].dts; 
}

OpenDMLHeader::~OpenDMLHeader()
{
	close();

}
/**
    \fn close
*/
uint8_t    OpenDMLHeader::close( void )
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
	if(_videoExtraData)
	{
		delete [] _videoExtraData;
		_videoExtraData=NULL;
	}
	if(_audioTracks)
	{
		delete [] _audioTracks;
		_audioTracks=NULL;
	}
    if(myName)
    {
      ADM_dealloc(myName);
      myName=NULL; 
    }
    // Destroy audioStreams
    if(_audioStreams)
    {
        for(int i=0;i<_nbAudioTracks;i++)
        {
            delete _audioStreams[i];
        }
        delete [] _audioStreams;
        _audioStreams=NULL;
    }
 	return 1;
}
//
//	Set default save value
//

OpenDMLHeader::OpenDMLHeader(void)
{
    _fd=NULL;
	_idx=NULL;	
	_nbTrack=0;
	memset(&(_Tracks[0]),0,sizeof(_Tracks));
	memset(&_regularIndex,0,sizeof(_regularIndex));
	_videoExtraData=NULL;
	_videoExtraLen=0;
	_reordered=0;
	_recHack=0;
    // Audio
    _audioTracks=NULL;
    _audioStreams=NULL;
    _nbAudioTracks=0;
    _currentAudioTrack=0;
    myName=NULL;
    ptsAvailable=false;
}

/**
    \fn getAudioInfo

*/
WAVHeader              *OpenDMLHeader::getAudioInfo(uint32_t i ) 
{
    if(_nbAudioTracks)
		return _audioStreams[i]->getInfo();
	else
		return NULL;
}
/**
    \fn getAudioStream

*/
uint8_t                 OpenDMLHeader::getAudioStream(uint32_t i,ADM_audioStream  **audio)
{
        if(_nbAudioTracks)
        {

                ADM_assert(i<_nbAudioTracks);
                *audio=_audioStreams[i];
                ADM_assert(*audio);
                return 1;
        }
        *audio=NULL;
        return 0;	
}
/**
    \fn getNbAudioStreams

*/
uint8_t                 OpenDMLHeader::getNbAudioStreams(void)
{
    return _nbAudioTracks;
}

/**
    \fn open

*/
uint8_t    OpenDMLHeader::open(const char *name)
{
uint8_t badAvi=0;
uint32_t rd;

	printf("** opening OpenDML files **");	
        
	_fd=ADM_fopen(name,"rb");
	if(!_fd)
	{
		printf("\n cannot open %s \n",name);
		return 0;
	}
        myName=ADM_strdup(name);
#define CLR(x)              memset(& x,0,sizeof(  x));

          CLR( _videostream);
          CLR( _mainaviheader);
	      _isvideopresent=1;
	      _isaudiopresent=0;    	     	      	 	      
	      
		_nbTrack=0;
		riffParser *parser=new riffParser(name);
		
		if(MKFCC('R','I','F','F')!=(rd=parser->read32()))
			{
				printf("Not riff\n");badAvi=1;
				printf("%x != %x\n",rd,MKFCC('R','I','F','F'));
			}
		parser->read32();
		if(MKFCC('A','V','I',' ')!=parser->read32())
			{
				printf("Not Avi\n");badAvi=1;
			}
		
		if(!badAvi)
			{
				walk(parser);	
			
			}					
		delete parser;
		aprintf("Found %d tracks\n:-----------\n",_nbTrack);
		// check if it looks like a correct avi
		if(!_nbTrack) badAvi=1;
		
		// if we are up to here -> good avi :)
		if(badAvi)
		{
			printf("FAIL\n");
			return 0;
		}
		// now read up each parts...
		//____________________________
		                
#define DUMP_TRACK(i) aprintf(" at %"PRIu64" (%"PRIx64") size : %"PRIu64" (%"PRIx64")\n", \
				_Tracks[i].strh.offset,\
				_Tracks[i].strh.offset,\
				_Tracks[i].strh.size,\
				_Tracks[i].strh.size);

		for(uint32_t i=0;i<_nbTrack;i++)
		{
			DUMP_TRACK(i);		
		}		
		
		uint32_t vidTrack=0xff;
		// search wich track is the video one
		// and load it to _videoheader
		
		for(uint32_t i=0;i<_nbTrack;i++)
		{
			fseeko(_fd,_Tracks[i].strh.offset,SEEK_SET);
			if(_Tracks[i].strh.size!=sizeof(_videostream))
			{
				printf("[AVI]Mmm(1) we have a bogey here, size mismatch : %"PRIu64"\n",_Tracks[i].strh.size);
				printf("[AVI]expected %d\n",(int)sizeof(_videostream));
				if(_Tracks[i].strh.size<sizeof(_videostream)-8) // RECT is not mandatory
				{
                                  GUI_Error_HIG(QT_TR_NOOP("Malformed header"), NULL);
					return 0;
				}		
				printf("[AVI]Trying to continue anyway\n");			
			}
			fread(&_videostream,sizeof(_videostream),1,_fd);
#ifdef ADM_BIG_ENDIAN
				Endian_AviStreamHeader(&_videostream);
#endif
			if(_videostream.fccType==MKFCC('v','i','d','s'))
				{
					vidTrack=i;
					printf("Video track is %u\n",i);
					break;
				}		
		}
		if(0xff==vidTrack)
		{
			printf("Could not identify video track!");
			return 0;
		}
		
		// then bih stuff
		int32_t extra;
//		_fd=fopen(name,"rb");
		
		fseeko(_fd,_Tracks[vidTrack].strf.offset,SEEK_SET);		
		extra=_Tracks[vidTrack].strf.size-sizeof(_video_bih);
		if(extra<0)
		{	
			printf("[AVI]bih is not big enough (%"PRIu64"/%d)!\n",_Tracks[vidTrack].strf.size,(int)sizeof(_video_bih));
			return 0;
		}
		fread(&_video_bih,sizeof(_video_bih),1,_fd);
#ifdef ADM_BIG_ENDIAN
		Endian_BitMapInfo(&_video_bih);
#endif
		if(extra>0)
		{				
			_videoExtraLen=extra;		
			_videoExtraData=new uint8_t [extra];
			fread(_videoExtraData,extra,1,_fd);
		}
		_isvideopresent=1;
		//--------------------------------------------------
		//	Read audio trak info, select if there is
		//	several
		//--------------------------------------------------
		// and audio track
		if(_mainaviheader.dwStreams>=2)
		{
			// which one is the audio track, is there several ?
			if(!(_nbAudioTracks=countAudioTrack()))
                        {
                                printf("Weird, there is no audio track, but more than one stream...\n");
                        }			
                        else
                        {
                          uint32_t run=0,audio=0;
                          odmlAudioTrack *track;

                          _audioTracks=new odmlAudioTrack[_nbAudioTracks]; 
                          _audioStreams=new ADM_audioStream *[_nbAudioTracks]; 
                          while(audio<_nbAudioTracks)
                          {
                                        ADM_assert(run<_nbTrack);

                                        track=&(_audioTracks[audio]);
                                        fseeko(_fd,_Tracks[run].strh.offset,SEEK_SET);
                                        if(_Tracks[run].strh.size != sizeof(_audiostream))
                                        {
                                                printf("[AVI]Mmm(2) we have a bogey here, size mismatch : %"PRIu64"\n",_Tracks[run].strh.size);
                                                printf("[AVI]expected %d\n",(int)sizeof(_audiostream));
                                                if(_Tracks[run].strh.size<sizeof(_audiostream)-8)
                                                {
                                                  GUI_Error_HIG(QT_TR_NOOP("Malformed header"), NULL);
                                                        return 0;
                                                }
                                                printf("[AVI]Trying to continue anyway\n");			
                                        }
                                        fread(track->avistream,sizeof(_audiostream),1,_fd);
#ifdef ADM_BIG_ENDIAN
                                        Endian_AviStreamHeader(track->avistream);
#endif
                                        if(track->avistream->fccType!=MKFCC('a','u','d','s'))
                                        {	
                                                printf("Not an audio track!\n");
                                                run++;
                                                continue;
                                        }
                                        // now read extra stuff
                                        fseeko(_fd,_Tracks[run].strf.offset,SEEK_SET);		
                                        extra=_Tracks[run].strf.size-sizeof(WAVHeader);
                                        if(extra<0)
                                        {	
                                                printf("[AVI]WavHeader is not big enough (%"PRIu64"/%d)!\n",
                                                _Tracks[run].strf.size,(int)sizeof(WAVHeader));
                                                return 0;
                                        }
                                        fread(track->wavHeader,sizeof(WAVHeader),1,_fd);				
#ifdef ADM_BIG_ENDIAN
                                        Endian_WavHeader(track->wavHeader);
#endif
                                        if(extra>2)
                                        {
                                                fgetc(_fd);fgetc(_fd);
                                                extra-=2;
                                                track->extraDataLen=extra;		
                                                track->extraData=new uint8_t [extra];
                                                fread(track->extraData,extra,1,_fd);
                                        }
                                        track->trackNum=run;
                                        audio++;
                                        run++;
                           }	
                        }
                }
		
		// now look at the index stuff
		// there could be 3 cases:
		// 1- It is a openDML index, meta index  + several smaller index
		// 2- It is a legacy index (type 1 , most common)
		// 3- It is a broken index or no index at all
		//
		// If it is a openDML index we will find a "indx" field in the Tracks
		// Else we will find it in _regularIndex Track
		// Since openDML often also have a regular index we will try open DML first
		
		uint8_t ret=0;
		Dump();
		
		// take the size of riff header and actual file size
		uint64_t riffSize;
		fseeko(_fd,0,SEEK_END);		
		_fileSize=ftello(_fd);
		fseeko(_fd,0,SEEK_SET);
		read32();
		riffSize=(uint64_t )read32();
				
		
		// 1st case, we have an avi < 4 Gb
		// potentially avi type 1	
#if 0	
		if((_fileSize<4*1024*1024*1024LL)&&
                	// if riff size is ~ fileSize try regular index
			 (abs(riffSize-_fileSize)<1024*1024))
#endif

#define HAS(x) if(x) printf(#x" : yes\n"); else printf(#x" : no\n");
                // If there is no openDML index
                HAS( _regularIndex.offset);
                HAS( _Tracks[vidTrack].indx.offset);
                if(!ret && _regularIndex.offset &&!_Tracks[vidTrack].indx.offset) 
        // try regular avi if a idx1 field is there (avi index)
                        ret=indexRegular(vidTrack);

                if (!ret && _Tracks[vidTrack].indx.offset)	// Try openDML if a index field is there (openDML)
                        ret=indexODML(vidTrack);
                if(!ret) 
                {
                        printf("Could not index it properly...\n");
                        return 0;

                }
                if(!_nbAudioTracks)
                {
                         _isaudiopresent=0;
                }
                else
                {
                        odmlAudioTrack *track;
                        // Check it is not a weird DV file
                        if(fourCC::check(_video_bih.biCompression,(uint8_t *)"dvsd"))
                        {
                             for(int i=0;i<_nbAudioTracks;i++)
                             {
                                    track=&(_audioTracks[i]);
                                    WAVHeader *hdr=  track->wavHeader;
                                    if(!hdr->frequency)
                                    {
                                            ADM_warning("Fixing audio track to be PCM\n");
                                            hdr->frequency=48000;
                                            //hdr->channels=2;
                                            hdr->byterate=48000*hdr->channels*2;
                                            hdr->blockalign=2*hdr->channels;
                                    }
                             }

                        }
                        // build audio stream
                        
                        for(int i=0;i<_nbAudioTracks;i++)
                        {
                                track=&(_audioTracks[i]);
                                ADM_aviAudioAccess *access=new ADM_aviAudioAccess(track->index,track->wavHeader,
                                            track->nbChunks,
                                            myName,
                                            track->extraDataLen,track->extraData);
                                _audioStreams[i]= ADM_audioCreateStream((track->wavHeader), access);
                        }
                }
                if(!_video_bih.biCompression && fourCC::check(_videostream.fccHandler,(uint8_t*)"DIB "))
                  {
                        _videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t*)"DIB ");
                  }
                else
                _videostream.fccHandler=_video_bih.biCompression;
                printf("\nOpenDML file successfully read..\n");
                if(ret==1) 
                {
                    computePtsDts();
                    removeEmptyFrames();
                }
                ADM_info("PtsAvailable : %d\n",(int)ptsAvailable);
                return ret;
}
/**
    \fn removeEmptyFrames
    \brief Remove the null padding frames, they are not needed and indeed harmful
*/
bool OpenDMLHeader::removeEmptyFrames(void)
{
    
    odmlIndex *nwIdx=new odmlIndex[_videostream.dwLength];
    int index=0;
    for(int i=0;i<_videostream.dwLength;i++)
    {
        if(_idx[i].size)
        {
            nwIdx[index++]=_idx[i];
        }
    }
    if(index==_videostream.dwLength)
    {
        delete [] nwIdx;
        printf("[openDml] No empty frames\n");
        return true;
    }
    int delta=_videostream.dwLength-index;

    printf("[openDml] Removed %d empty frames\n",(int)delta);
    _mainaviheader.dwTotalFrames=_videostream.dwLength=index;
    delete [] _idx;
    _idx=nwIdx;
    if(index)
    {   // make sure the first frame is ok (Intra + have PTS)
         _idx[0].intra |=AVI_KEY_FRAME;
         if(_idx[0].pts==ADM_NO_PTS)
         {
             if(_idx[0].dts!=ADM_NO_PTS) _idx[0].pts=_idx[0].dts;
             else  _idx[0].pts=0;
         }
    }
    return true;
}
/**
    \fn computePtsDts
    \brief Compute PtsDts
*/
extern uint8_t isMpeg4Compatible (uint32_t fourcc);
uint8_t OpenDMLHeader::computePtsDts(void)
{
    // if it is mpeg4-sp, removed packet bitstream & reindex
    if(isMpeg4Compatible(_videostream.fccHandler))  OpenDMLHeader::unpackPacked(  );
    // Now if we have B frames, it is properly tagged
    // Begin by putting PTS=DTS i.e. no B-frames
    for(int i=0;i<_videostream.dwLength;i++)
    {
       odmlIndex *idx=&( _idx[i]);
       idx->pts=ADM_COMPRESSED_NO_PTS;
       idx->dts=frameToUs(i);
    }
    _idx[0].pts=0;
    return 1;
}
/**
    \fn mpegReorder
    \brief Compute PTS when handling mpeg 1/2/4sp type of b frames

*/
uint8_t OpenDMLHeader::mpegReorder(void)
{
    int last=0;
    int nbBframe=0;
    int maxBframe=0;
    for(int i=1;i<_videostream.dwLength;i++)
    {
        if(_idx[i].intra & AVI_B_FRAME) nbBframe++;
        else        
            {
                if(nbBframe>maxBframe) maxBframe=nbBframe;
                nbBframe=0;
            }
    }
    ADM_info("Found max %d sequential bframes\n",maxBframe);
    if(!maxBframe)
    {
        ADM_info("No b frame, pts=dts\n");
        for(int i=1;i<_videostream.dwLength;i++)
            _idx[i].pts=_idx[i].dts;
        ptsAvailable=1;
        return true;
    }

    for(int i=1;i<_videostream.dwLength;i++)
    {
        if(_idx[i].intra & AVI_B_FRAME)
        {
            _idx[i].pts=_idx[i].dts;
            nbBframe++;
        }
        else
        {
            _idx[last].pts=_idx[nbBframe+last+1].dts;
            nbBframe=0;
            last=i;
        }
    }
    ptsAvailable=1;
    return 1;
}
/*
	Count how many audio track is found

*/
uint32_t OpenDMLHeader::countAudioTrack( void )
{
AVIStreamHeader tmp;
uint32_t count=0;
	for(uint32_t i=0;i<_nbTrack;i++)
	{		
			fseeko(_fd,_Tracks[i].strh.offset,SEEK_SET);
			if(_Tracks[i].strh.size!=sizeof(tmp))
			{
				
				printf("[AVI]Mmm(3) we have a bogey here, size mismatch : %"PRIu64"\n",_Tracks[i].strh.size);
				printf("[AVI]expected %d\n",(int)sizeof(tmp));
				if(_Tracks[i].strh.size<sizeof(tmp)-8)
				{
                                  GUI_Error_HIG(QT_TR_NOOP("Malformed header"), NULL);
					return 0;
				}		
				printf("[AVI]Trying to continue anyway\n");			
			}
			
			
			fread(&tmp,sizeof(tmp),1,_fd);
#ifdef ADM_BIG_ENDIAN
			Endian_AviStreamHeader(&tmp);
#endif
			if(tmp.fccType==MKFCC('a','u','d','s'))
				{
					count++;
					printf("Track %u/%u is audio\n",i,_nbTrack);	
				}
			else
			{
                                if(tmp.fccType==MKFCC('v','i','d','s') && tmp.fccHandler==MKFCC('D','X','S','B'))
                                {

                                        printf("Track %u/%u is subs\n",i,_nbTrack);  
                                }
                                else
                                {
                                        printf("Track %u/%u :\n",i,_nbTrack);
                                        fourCC::print(tmp.fccType);
                                        fourCC::print(tmp.fccHandler);
                                        printf("\n");
                                }
					
			}			
	}
	return count;
}

/*
	Dump in human readable form the content of all headers
		
*/

void OpenDMLHeader::Dump( void )
{
        printf(  "Main header\n" );
        printf(  "______________________\n" );  

#define X_DUMP(x) printf("[Avi] "#x":\t\t:%d\n",_mainaviheader.x);
    	
    	X_DUMP(dwMicroSecPerFrame) ;
	X_DUMP(dwMaxBytesPerSec);
    	X_DUMP(dwPaddingGranularity);
    	X_DUMP(dwFlags);
    	X_DUMP(dwTotalFrames);
    	X_DUMP(dwInitialFrames);
        X_DUMP(dwStreams);
        X_DUMP(dwSuggestedBufferSize);
    	X_DUMP(dwWidth);
    	X_DUMP(dwHeight);
 	printf("\n");
#undef X_DUMP
#define X_DUMP(x) printf("[Avi]  "#x":\t\t:%d\n",_videostream.x);

				
	printf(  "[Avi] video stream attached:\n" );
	printf(  "[Avi] ______________________\n" );	
	printf(  "[Avi] Extra Data  : %u",_videoExtraLen);
	if(_videoExtraLen)
	{
		mixDump( _videoExtraData, _videoExtraLen);
        printf("\n");
	}

	printf("[Avi]  fccType     :");
	fourCC::print(_videostream.fccType);printf("\n");
	printf("[Avi]  fccHandler :");
	fourCC::print(_videostream.fccHandler);printf("\n");

	X_DUMP(dwFlags);
	X_DUMP(	wPriority);	/* dwPriority - splited for audio */
	X_DUMP(	wLanguage);
	
	X_DUMP(dwInitialFrames);
        X_DUMP(dwScale);
	X_DUMP(dwRate);
        X_DUMP(dwStart);
        X_DUMP(dwLength); 
        X_DUMP(dwSuggestedBufferSize);
        X_DUMP(dwQuality);
	X_DUMP(dwSampleSize);
	
	
    	
	X_DUMP(rcFrame.left);
        X_DUMP(rcFrame.right);
        X_DUMP(rcFrame.top);
        X_DUMP(rcFrame.bottom);
	printf("\n");
	  	  
        printBih(&_video_bih);

        /*****************************************************************
                Dump infos about all audio tracks found

        ******************************************************************/
        for(int i=0;i<_nbAudioTracks;i++)
     	{
#undef X_DUMP
#define X_DUMP(x) printf("[Avi] "#x":\t\t:%d\n",_audioTracks[i].avistream->x);



	   printf(  "[Avi] audio stream attached:\n" );
	   printf(  "[Avi] ______________________\n" );


	  printf("[Avi]  fccType     :");
	  fourCC::print(_audioTracks[i].avistream->fccType);printf("\n");
	  printf("[Avi]  fccHandler :");
	  fourCC::print(_audioTracks[i].avistream->fccHandler);printf("\n");
	  printf("[Avi]  fccHandler :0x%x\n", _audioTracks[i].avistream->fccHandler);


	  X_DUMP(dwFlags);
	  X_DUMP(dwInitialFrames);
	  X_DUMP(dwRate);
	  X_DUMP(dwScale);
	  X_DUMP(dwStart);
	  X_DUMP(dwLength);
	  X_DUMP(dwSuggestedBufferSize);
	  X_DUMP(dwQuality);
	  X_DUMP(dwSampleSize);
	  
	      	
	  
#undef X_DUMP

        printWavHeader(_audioTracks[i].wavHeader);
        printf("[Avi]  Extra Data  : %u\n",_audioTracks[i].extraDataLen);
	if(_audioTracks[i].extraDataLen)
	{
		mixDump( _audioTracks[i].extraData, _audioTracks[i].extraDataLen);
	}
	printf("\n");

      }
}
#define PAD	for(uint32_t j=0;j<nest;j++) aprintf("\t");
/*
	Recursively climb a riff tree and not where are
	the interesting informations to be read later on

*/
void OpenDMLHeader::walk(riffParser *p)
{
	uint32_t fcc,len;
	static uint32_t nest=0;

	nest++;
	while(!p->endReached())
	{
	fcc=p->read32();
	len=p->read32();
#ifdef OPENDML_VERBOSE	
	for(uint j=0;j<nest;j++) aprintf("\t");
	PAD;
	aprintf("Entry  :");fourCC::print(fcc);aprintf(" Size: %lu (%lx)\n",len,len);
#endif	
	switch(fcc)
	{
		case MKFCC('a','v','i','h'):
				aprintf("main header found \n");
				if(len!=sizeof(_mainaviheader))
				{
					printf("[AVI]oops : %"PRIu32" / %d\n",len,(int)sizeof(_mainaviheader));
				}
				p->read(len,(uint8_t *)&_mainaviheader);

#ifdef ADM_BIG_ENDIAN
			Endian_AviMainHeader(&_mainaviheader);
#endif
				printf("[Avi]  Main avi header :\n");				
				break;
		case MKFCC('i','d','x','1'):
                                _regularIndex.offset=p->getPos();
                                printf("[Avi] Idx1 found at offset %"PRIx64"\n",_regularIndex.offset);
                                _regularIndex.size=len;
				return;
				break;				
		case MKFCC('R','I','F','F'):
				uint32_t sublen;
				p->read32();
				{

							riffParser *n;
							n=new riffParser(p,len-4);
							walk(n);
							delete n;
				}
				p->curPos=ftello(p->fd);
				break;
		case MKFCC('s','t','r','f'):
					_Tracks[_nbTrack].strf.offset=p->getPos();				
					_Tracks[_nbTrack].strf.size=len;
					p->skip(len);
					DUMP_TRACK(_nbTrack);
					break;
		case MKFCC('s','t','r','h'):
					_Tracks[_nbTrack].strh.offset=p->getPos();				
					_Tracks[_nbTrack].strh.size=len;
					p->skip(len);
					DUMP_TRACK(_nbTrack);
					break;
		case MKFCC('i','n','d','x'):
					printf("[Avi] Indx found for track %d\n",_nbTrack);
					_Tracks[_nbTrack].indx.offset=p->getPos();
					_Tracks[_nbTrack].indx.size=len;
					p->skip(len);
					break;					
							
		case MKFCC('L','I','S','T'):		
				{
				uint32_t sub,l;
					PAD;
					aprintf("\tType:");
					sub=p->read32();
					len-=4;
#ifdef OPENDML_VERBOSE					
					fourCC::print(sub);printf(" (%d bytes to go)\n",len);
#endif					
					if(sub==MKFCC('m','o','v','i'))
					{
						_movi.offset=p->getPos();
						p->skip(len);	
					}
					else
						{
							riffParser *n;
							n=new riffParser(p,len);
							walk(n);
							delete n;
						}
						p->curPos=ftello(p->fd);
					if(MKFCC('s','t','r','l')==sub)
					{
 						_nbTrack++;
					}
					
				}
				break;
		default:
			PAD;
			aprintf("[Avi] \tskipping %lu bytes\n");
			p->skip(len);
			break;
		
	}
	}
	nest--;
}

//****************************************
odmlAudioTrack::odmlAudioTrack(void)
{
        index=NULL;
        extraData=NULL;

        wavHeader=new WAVHeader;
        nbChunks=0;
        extraDataLen=0;
        trackNum=0;
        totalLen=0;
        nbChunks=0;
        avistream=new AVIStreamHeader;
}
odmlAudioTrack::~odmlAudioTrack()
{
        if(index) delete [] index;
        if(wavHeader) delete wavHeader;
        if(extraData) delete [] extraData;
        if(avistream) delete avistream;
}
/**
    \fn getPtsDts
*/
bool    OpenDMLHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
    if(frame>_videostream.dwLength)
    {
        printf("[Odml] %"PRIu32" exceeds nb of video frames %"PRIu32"\n",frame,_videostream.dwLength);
        return false;
    }
    *dts=_idx[frame].dts; // FIXME
    *pts=_idx[frame].pts;
    return true;
}
/**
        \fn setPtsDts
*/
bool    OpenDMLHeader::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
    if(frame>_videostream.dwLength)
    {
        printf("[Odml] %"PRIu32" exceeds nb of video frames %"PRIu32"\n",frame,_videostream.dwLength);
        return false;
    }
    _idx[frame].dts=dts;
    _idx[frame].pts=pts;
    return true;

}
