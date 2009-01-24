/***************************************************************************
                          ADMMP4p.cpp  -  description
                             -------------------
	
		Read quicktime/mpeg4 file format found in 3gpp file.
		They are limited to SQCIF/QCIF video size and can
		only contains
			video : h263 or mpeg4 
			audio : AMR or AAC


		For the mpeg4, the VOL headers are stored in esds atom
		and not in the first image
		Idem for MJPG and SVQ3

		The usual tree structure of a 3gp file is

		- ftyp
		- mdat
		- moov
			xxx
			trak
				tkhd (duration / ...)
			mdia
				hdlr (type)
				minf
					stsd header for audio/video
					stbl index to datas

	We ignore other chunk as they are not vital for our aim
	and just keep moov/mdia/minf/stsd/stbl stuff

Generic
*********
http://developer.apple.com/documentation/QuickTime/QTFF/QTFFChap2/chapter_3_section_5.html#//apple_ref/doc/uid/DontLinkBookID_69-CH204-BBCJEIIA

version 2 media descriptor :
****************************** http://developer.apple.com/documentation/QuickTime/Conceptual/QT7Win_Update_Guide/Chapter03/chapter_3_section_1.html#//apple_ref/doc/uid/TP40002476-CH314-BBCDGGBB



    begin                : Tue Jul  2003
    copyright            : (C) 2003/2006 by mean
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
#include "ADM_editor/ADM_Video.h"

#include "fourcc.h"
#include "ADM_mp4.h"

#include "ADM_codecs/ADM_codec.h"

#include "ADM_video/ADM_videoInfoExtractor.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_3GP
#include "ADM_osSupport/ADM_debug.h"

//#define MP4_VERBOSE
#define MAX_CHUNK_SIZE (3*1024)

uint32_t ADM_UsecFromFps1000(uint32_t fps1000);
//****************************************************
MP4Track::MP4Track(void)
{
    extraDataSize=0;
    extraData=NULL;
    index=NULL;
    nbIndex=0;
    id=0;
    memset(&_rdWav,0,sizeof(_rdWav));

}
MP4Track::~MP4Track()
{
    if(extraData) delete [] extraData;
    if(index)   delete [] index;
 
    index=NULL;
    extraData=NULL;
    
}
//****************************************************
uint8_t MP4Header::setFlag(uint32_t frame,uint32_t flags)
{
    UNUSED_ARG(frame);
    UNUSED_ARG(flags);
    
    VDEO.index[frame].intra=flags;
    return 0;
}

uint32_t MP4Header::getFlags(uint32_t frame,uint32_t *flags)
{
        if(frame>= (uint32_t)_videostream.dwLength) return 0;
        *flags=VDEO.index[frame].intra;
  
        return 1;
}


uint8_t  MP4Header::getFrameNoAlloc(uint32_t framenum,ADMCompressedImage *img)
{
    if(framenum>=VDEO.nbIndex)
    {
      return 0; 
    }
    uint32_t offset=VDEO.index[framenum].offset; //+_mdatOffset;


    fseeko(_fd,offset,SEEK_SET);
    fread(img->data, VDEO.index[framenum].size, 1, _fd);
    img->dataLength=VDEO.index[framenum].size;
    return 1;
}
MP4Header::~MP4Header()
{
    close();

}
uint8_t    MP4Header::close( void )
{
      if(_fd)
              {
              fclose(_fd);
              }
            _fd=NULL;
      return 1;
}
//
//	Set default save value
//

MP4Header::MP4Header(void)
{
        _fd=NULL;
        nbAudioTrack=0;
        _currentAudioTrack=0;
        _reordered=0;
        _videoScale=1;
        _videoFound=0;
}
uint8_t	MP4Header::getAudioStream(AVDMGenericAudioStream **audio)
{  
    if(nbAudioTrack) 
    {
        *audio=_audioTracks[_currentAudioTrack];
    }  else 
        *audio=NULL;
    return 1;
};
WAVHeader 	*MP4Header::getAudioInfo(void )
{ 	
	if(!nbAudioTrack)
		return NULL; 
       
        return _audioTracks[_currentAudioTrack]->getInfo();
} ;
uint8_t   MP4Header::getExtraHeaderData(uint32_t *len, uint8_t **data)
{
uint32_t old;
        *len=0;*data=NULL;
        if(_tracks[0].extraDataSize)
        {
            *len= VDEO.extraDataSize;
            *data=VDEO.extraData;
        }
        return 1;
}
//______________________________________
//
// Open and recursively read the atoms
// until we got the information we want
// i.e. :
//	index for audio and video track
//	esds for mpeg4
//	size / codec used 
//
// We don't care about sync atom and all
// other stuff which are pretty useless on
// 3gp file anyway.
//______________________________________
uint8_t    MP4Header::open(const char *name)
{
        printf("** opening 3gpp files **");	
        _fd=fopen(name,"rb");
        if(!_fd)
        {
                printf("\n cannot open %s \n",name);
                return 0;
        }
#define CLR(x)              memset(& x,0,sizeof(  x));

        CLR( _videostream);
        CLR(  _mainaviheader);

        _videostream.dwScale=1000;
        _videostream.dwRate=10000;
        _mainaviheader.dwMicroSecPerFrame=100000;;     // 10 fps hard coded
        
        adm_atom *atom=new adm_atom(_fd);
        // Some mp4/mov files have the data at the end but do start properly
        // detect and workaround...
        // Check it is not mdat start(ADM_memcpy_0)     
        uint8_t check[4];
        fseeko(_fd,4,SEEK_SET);
        fread(check,4,1,_fd);
        fseeko(_fd,0,SEEK_SET);
        if(check[0]=='m' && check[1]=='d' &&check[2]=='a' && check[3]=='t')
        {
                        uint32_t of;
                                        printf("Data first, header later...\n");
                                        of=atom->read32();
                                        if(of==1)
                                        {
                                          atom->read32();
                                          atom->read32();
                                          of=atom->read32();
                                        }
                                        fseeko(_fd,of,SEEK_SET);        
                                        printf("Header starts at %x\n",of);
                                        delete atom;
                                        atom=new adm_atom(_fd);
        }
        //**************
        
        if(!lookupMainAtoms((void*) atom))
        {
          printf("Cannot find needed atom\n");
          fclose(_fd);
		  delete atom;
          return 0;
        }
        
        delete atom;

	      _isvideopresent=1;
	      _isaudiopresent=0;
    	     
              _videostream.fccType=fourCC::get((uint8_t *)"vids");
              _video_bih.biBitCount=24;
              _videostream.dwInitialFrames= 0;
              _videostream.dwStart= 0;

	printf("\n");
	
       
        if(!VDEO.index) 
        {
                printf("No index!\n");
                return 0;
        }

        // If it is mpeg4 and we have extra data
        // Decode vol header to get the real width/height
        // The mpeg4/3GP/Mov header is often misleading

        if(fourCC::check(_videostream.fccHandler,(uint8_t *)"DIVX"))
        {
            if(VDEO.extraDataSize)
            {
                uint32_t w,h,ti;
                if(extractMpeg4Info(VDEO.extraData,VDEO.extraDataSize,&w,&h,&ti))
                {
                    printf("MP4 Corrected size : %lu x %lu\n",w,h);
                    _video_bih.biWidth=_mainaviheader.dwWidth=w ;
                    _video_bih.biHeight=_mainaviheader.dwHeight=h;                               
                }
            }else { printf("No extradata to probe\n");}
        
        }
        else
        {

        /*
            Same story for H263 : Analyze 1st frame to get the real width/height
        */
            if(fourCC::check(_videostream.fccHandler,(uint8_t *)"H263"))
            {
                uint32_t w,h,sz;
                uint8_t *bfer=NULL;
                sz=VDEO.index[0].size;
                if(sz)
                {
                        bfer=new uint8_t[sz];
                        ADMCompressedImage img;
                        img.data=bfer;
                        if(getFrameNoAlloc(0,&img))
                        {
                        if(extractH263Info(bfer,sz,&w,&h))
                        {
                            printf("H263 Corrected size : %lu x %lu\n",w,h);
                            _video_bih.biWidth=_mainaviheader.dwWidth=w ;
                            _video_bih.biHeight=_mainaviheader.dwHeight=h;                               
                        }
                        else
                        {
                                  printf("H263 COULD NOT EXTRACT SIZE, using : %lu x %lu\n",
                                      _video_bih.biWidth,  _video_bih.biHeight);
                        }
                        }
                        delete [] bfer;
                }
            }
        }
        /*
                Now build audio tracks
        */
        if(nbAudioTrack) _isaudiopresent=1; // Still needed ?
        for(int audio=0;audio<nbAudioTrack;audio++)
        {
            _audioTracks[audio]=new MP4Audio(name,&(_tracks[1+audio]));   
            
        }
        fseek(_fd,0,SEEK_SET);
        printf("3gp/mov file successfully read..\n");
        return 1;
}

//
//	That tag are coded like this
//	Each 8 bits is in fact a 7 Bits part while b7=1
// 	So we concanate them while possible
uint32_t MP4Header::readPackedLen(adm_atom *tom )
{
	uint32_t len=0;
	uint8_t	 b=0;

	do
	{
	b=tom->read();
	len=len<<7;
	len+=b&0x7f;
	}while(b&0x80);
	return len;
}
uint32_t              MP4Header::ptsDtsDelta(uint32_t frame)
{
   if(frame >= _videostream.dwLength) return 0;
   return VDEO.index[frame].deltaPtsDts;
}
uint8_t MP4Header::getFrameSize (uint32_t frame, uint32_t * size){
  if(frame >= _videostream.dwLength) return 0;
  *size = VDEO.index[frame].size;
  return 1;
}
  uint8_t   MP4Header::changeAudioStream(uint32_t newstream)
{
        if(newstream>nbAudioTrack) return 0;
        _currentAudioTrack=newstream;
        return 1;
}
uint32_t     MP4Header::getCurrentAudioStreamNumber(void) 
{ 
    return _currentAudioTrack;
}
 uint8_t   MP4Header::getAudioStreamsInfo(uint32_t *nbStreams, audioInfo **infos)
{
        *nbStreams=nbAudioTrack;
        if(nbAudioTrack)
        {
            *infos=new audioInfo[nbAudioTrack];
            for(int i=0;i<nbAudioTrack;i++)
            {
                WAV2AudioInfo(&(_tracks[i+1]._rdWav),&((*infos)[i]));
            }
                
             //   (*infos)[i]=_tracks[i+1]._rdWav.encoding;
        }
        return 1;
}
uint8_t                 MP4Header::isReordered( void )
{ 
        return _reordered;
}
/***************************************/
uint8_t MP4Header::reorder( void )
{

        if( _reordered) return 1;
        printf("Reordering...\n");
#define INDEX_TMPL        MP4Index
#define INDEX_ARRAY_TMPL  (VDEO.index)
#define FRAMETYPE_TMPL    intra
  
#include "ADM_video/ADM_reorderTemplate.cpp"

#undef INDEX_TMPL       
#undef INDEX_ARRAY_TMPL 
#undef FRAMETYPE_TMPL   
        VDEO.nbIndex= _mainaviheader.dwTotalFrames;
        // last frame cannot be B frame
        index[last].intra&=~AVI_B_FRAME;
          _reordered=ret;
        return ret;

}
//EOF 
