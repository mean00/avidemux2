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


#include <string.h>
#include <math.h>

#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"
#include "ADM_mp4.h"

#include "ADM_vidMisc.h"

#include "ADM_videoInfoExtractor.h"
#include "ADM_codecType.h"
#include "ADM_a52info.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

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
/**
    \fn getTime
*/
uint64_t                   MP4Header::getTime(uint32_t frameNum)
{
    ADM_assert(frameNum<VDEO.nbIndex);
    // Assume if not PTS, PTS=DTS (non mpeg4/non h264 streams)
    if(VDEO.index[frameNum].pts==ADM_COMPRESSED_NO_PTS) return VDEO.index[frameNum].dts;
    return VDEO.index[frameNum].pts;
}
/**
    \fn getVideoDuration
*/
uint64_t                   MP4Header::getVideoDuration(void)
{
    return _movieDuration*1000LL; //VDEO.index[VDEO.nbIndex-1].time;

}
/**
 * \fn compute the minimum us delta = maximum fps
 * \brief average fps is not good enough, it might be too high
 * @return 
 */
bool MP4Header::refineFps(void)
{
    int n=VDEO.nbIndex;
    uint64_t minDelta=60*1000*1000;
    for(int i=0;i<n-1;i++)
    {
        MP4Index *dex=&(_tracks[0].index[i]);
        MP4Index *next=&(_tracks[0].index[i+1]);
        if(dex->dts==ADM_NO_PTS) continue;
        if(next->dts==ADM_NO_PTS) continue;
        uint64_t delta=next->dts-dex->dts;
        if(delta<minDelta) minDelta=delta;
    }
    if(minDelta>1000)
    {
        double f=1000000./(double)minDelta;
        f*=1000.;
        ADM_info("MinDelta=%d us\n",(int)minDelta);
        ADM_info("Computed fps1000=%d\n",(int)f);
        uint32_t fps1000=floor(f+0.49);
        if(fps1000>  _videostream.dwRate)
        {
            ADM_info("Adjusting fps, the computed is higher than average, dropped frames ?\n");
           _videostream.dwRate=fps1000;
           _mainaviheader.dwMicroSecPerFrame=ADM_UsecFromFps1000(_videostream.dwRate);
        }
    }
    
}
uint8_t  MP4Header::getFrame(uint32_t framenum,ADMCompressedImage *img)
{
    aprintf("[MP4] frame %d requested (nbFrame=%d)\n",framenum,VDEO.nbIndex);
    if(framenum>=VDEO.nbIndex)
    {
      return 0;
    }

MP4Index *idx=&(VDEO.index[framenum]);

    uint64_t offset=idx->offset; //+_mdatOffset;


    fseeko(_fd,offset,SEEK_SET);
    fread(img->data, idx->size, 1, _fd);
    img->dataLength=idx->size;
	img->flags = idx->intra;

    img->demuxerDts=idx->dts;
    img->demuxerPts=idx->pts;
    aprintf("[MP4] Pts=%s\n",ADM_us2plain(idx->pts));
    /*
    if(img->demuxerPts==ADM_COMPRESSED_NO_PTS)
        img->demuxerPts=img->demuxerDts;
    */
    return 1;
}
MP4Header::~MP4Header()
{
    close();

	for (int audio = 0; audio < nbAudioTrack; audio++)
	{
		delete audioStream[audio];
		delete audioAccess[audio];
	}
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
        delayRelativeToVideo=0;
        _flavor=Mp4Regular;
}
/**
    \fn getAudioInfo
    \brief
*/
WAVHeader    *MP4Header::getAudioInfo(uint32_t i )
{
    if(nbAudioTrack)
    {
        ADM_assert(i<nbAudioTrack);
        return &(_tracks[i+1]._rdWav);
    }

    return NULL;

}
/**
    \fn getAudioStream
*/

uint8_t      MP4Header::getAudioStream(uint32_t i,ADM_audioStream  **audio)
{
    if(nbAudioTrack)
    {
        ADM_assert(i<nbAudioTrack);
        *audio=audioStream[i];
    }  else
        *audio=NULL;
    return 1;
}
/**
    \fn getNbAudioStreams
*/
uint8_t      MP4Header::getNbAudioStreams(void)
{
    return nbAudioTrack;

}


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
        _fd=ADM_fopen(name,"rb");
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
        uint64_t fileSize;
        fseeko(_fd,0,SEEK_END);
        fileSize=ftello(_fd);
        fseeko(_fd,4,SEEK_SET);
        fread(check,4,1,_fd);
        fseeko(_fd,0,SEEK_SET);
        if(check[0]=='m' && check[1]=='d' &&check[2]=='a' && check[3]=='t')
        {
                        uint64_t of;
                        uint64_t hi,lo;
                                        printf("Data first, header later...\n");
                                        of=atom->read32();
                                        if(of==1)
                                        {
                                          atom->read32();	// size
                                          atom->read32();	// fcc
                                          hi=atom->read32();
                                          lo=atom->read32();
                                          of=(hi<<32)+lo;
                                          if(of>fileSize) of=hi;
                                        }
                                        fseeko(_fd,of,SEEK_SET);
                                        printf("Header starts at %"PRIx64"\n",of);
                                        delete atom;
                                        atom=new adm_atom(_fd);
        }
        //**************

        if(!lookupMainAtoms((void*) atom))
        {
          printf("Cannot find needed atom\n");          
          if(!_tracks[0].fragments.size() || !indexVideoFragments(0)) // fixme audio
          {
            fclose(_fd);
            _fd=NULL;
            delete atom;
            return 0;
          }else
          { // do other tracks as well
              for(int i=1;i<=nbAudioTrack;i++)
              {
                  if(_tracks[i].fragments.size())
                      indexAudioFragments(i);
              }
          }
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
                    printf("MP4 Corrected size : %"PRIu32" x %"PRIu32"\n",w,h);
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
                        if(getFrame(0,&img))
                        {
                        if(extractH263Info(bfer,sz,&w,&h))
                        {
                            printf("H263 Corrected size : %"PRIu32" x %"PRIu32"\n",w,h);
                            _video_bih.biWidth=_mainaviheader.dwWidth=w ;
                            _video_bih.biHeight=_mainaviheader.dwHeight=h;
                        }
                        else
                        {
                                  printf("H263 COULD NOT EXTRACT SIZE, using : %"PRIu32" x %"PRIu32"\n",
                                      _video_bih.biWidth,  _video_bih.biHeight);
                        }
                        }
                        delete [] bfer;
                }
            }
        }
        /*
         * Veryfy DTS<=PTS
         */
        int nb=(int)_tracks[0].nbIndex;
        uint64_t delta,maxDelta=0;
        for(int i=0;i<nb;i++)
        {
            uint64_t pts,dts;
            dts=VDEO.index[i].dts;
            pts=VDEO.index[i].pts;
            if(pts==ADM_COMPRESSED_NO_PTS || dts==ADM_COMPRESSED_NO_PTS) continue;
            if(dts>=pts)
            {
                uint64_t delta=(uint64_t)(dts-pts);
                if(delta>maxDelta) maxDelta=delta;
            }
        }
        if(maxDelta)
        {
            shiftTimeBy(maxDelta);
             _movieDuration+=(maxDelta+999)/1000;
        }
        /*
                Now build audio tracks
        */
        if(nbAudioTrack) _isaudiopresent=1; // Still needed ?
        
        adjustElstDelay();
        
        //
        for(int audio=0;audio<nbAudioTrack;audio++)
        {
            switch(_tracks[1+audio]._rdWav.encoding)
            {
                
            // Lookup if AAC is lying about # of channels
            case WAV_AAC:
                {
                    if(_tracks[1+audio].extraDataSize==2)
                    {
                        // Channels
                        uint32_t word=(_tracks[1+audio].extraData[0]<<8)+_tracks[1+audio].extraData[1];
                        uint32_t chan=(word>>3)&0xf;
                        uint32_t fqIndex=(word>>7)&0xf;
                        printf("0x%x word, Channel : %d, fqIndex=%d\n",word,chan,fqIndex);
                    }
                }
                break;
            case WAV_AC3: // same for ac3
            {
                 // read First chunk
                
                MP4Index *dex=_tracks[1+audio].index;
                int size=dex[0].size;
                uint8_t *buffer=new uint8_t[size];
                  fseeko(_fd,dex[0].offset,SEEK_SET);
                  if(fread(buffer,1,size,_fd))
                  {
                      uint32_t fq,  br,  chan, syncoff;
                      if(ADM_AC3GetInfo(buffer,size, &fq, &br, &chan,&syncoff))
                      {
                          ADM_info("Updating AC3 info : Fq=%d, br=%d, chan=%d\n",fq,br,chan);
                          _tracks[1+audio]._rdWav.channels=chan;
                          _tracks[1+audio]._rdWav.byterate=br;
                      }
                  }
                  delete [] buffer;
            }
                break;
            default:
                break;
            }
            audioAccess[audio]=new ADM_mp4AudioAccess(name,&(_tracks[1+audio]));
            audioStream[audio]=ADM_audioCreateStream(&(_tracks[1+audio]._rdWav), audioAccess[audio]);
        }
        fseeko(_fd,0,SEEK_SET);
        refineFps();
        uint64_t duration1=_movieDuration*1000LL;
        uint64_t duration2=_tracks[0].index[nb-1].pts;
        
        ADM_info("3gp/mov file successfully read..\n");
        if(duration2!=ADM_NO_PTS && duration2>duration1)
        {
            ADM_warning("Last PTS is after movie duration, increasing movie duration\n");
            _movieDuration=(_tracks[0].index[nb-1].pts/1000)+1;
        }
        ADM_info("Nb images      : %d\n",nb);
        ADM_info("Movie duration : %s\n",ADM_us2plain(_movieDuration*1000LL));
        ADM_info("Last video PTS : %s\n",ADM_us2plain(_tracks[0].index[nb-1].pts));
        ADM_info("Last video DTS : %s\n",ADM_us2plain(_tracks[0].index[nb-1].dts));

        checkDuplicatedPts();
        
        return 1;
}
bool MP4Header::checkDuplicatedPts(void)
{
        int nb=(int)_tracks[0].nbIndex;
        for(int i=0;i<nb;i++)
        {
            int mn,mx;
            mn=i-10;
            if(mn<0) mn=0;
            mx=i+10;
            if(mx>=nb-1) mx=nb-1;
            for(int j=mn;j<mx;j++)
            {
                if(j==i) continue;
                if(_tracks[0].index[i].pts==_tracks[0].index[j].pts)
                {
                    ADM_warning("Duplicate pts %s at %d and %d\n",ADM_us2plain(_tracks[0].index[i].pts),i,j);
                    _tracks[0].index[j].pts+=1000; // add 1 ms
                }
            }
                
        }
        return true;
}
/**
 * \fn adjustElstDelay
 * @return 
 */
bool MP4Header::adjustElstDelay()
{
    if(this->delayRelativeToVideo)
    {
        ADM_info("Compensating for a/v delay\n");
        shiftAudioTimeBy(this->delayRelativeToVideo);
    }
    return true;

}
        
    

/**
 * \fn shiftTimeBy
 * \brief increase pts by shift, fix some mp4 where dts is too low
 * @param shift
 * @return 
 */
bool MP4Header::shiftTimeBy(uint64_t shift)
{
        
        ADM_warning("MP4, Must increase pts by %d us\n",(int)shift);
        int nb=(int)_tracks[0].nbIndex;
        for(int i=0;i<nb;i++)
        {
           uint64_t pts;
            pts=VDEO.index[i].pts;
            if(pts==ADM_COMPRESSED_NO_PTS) continue;
            pts+=shift;
            VDEO.index[i].pts=pts;
        }

       shiftAudioTimeBy(shift);
        return true;        
}
bool MP4Header::shiftAudioTimeBy(uint64_t shift)
{
    int nb;
      for(int audioTrack=0;audioTrack<nbAudioTrack;audioTrack++)
        {
            nb=(int)_tracks[1+audioTrack].nbIndex;
            for(int i=0;i<nb;i++)
            {
                     uint64_t dts;
                        dts=_tracks[audioTrack+1].index[i].dts;
                        if(dts==ADM_COMPRESSED_NO_PTS) continue;
                        dts+=shift;
                        _tracks[audioTrack+1].index[i].dts=dts;
             }
        }
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

/**
    \fn getPtsDts
*/
bool    MP4Header::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{

    if(frame>=VDEO.nbIndex)
    {
      printf("[MP4] Frame %"PRIu32" exceeds # of frames %"PRIu32"\n",frame,VDEO.nbIndex);
      return false;
    }

    MP4Index *idx=&(VDEO.index[frame]);

    *dts=idx->dts; // FIXME
    *pts=idx->pts;
    return true;
}
/**
        \fn setPtsDts
*/
bool    MP4Header::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
    if(frame>=VDEO.nbIndex)
    {
      printf("[MP4] Frame %"PRIu32" exceeds # of frames %"PRIu32"\n",frame,VDEO.nbIndex);
      return 0;
    }

    MP4Index *idx=&(VDEO.index[frame]);

    idx->dts=dts; // FIXME
    idx->pts=pts;
    return true;
}
/**
 * \fn unreliableBFramePts
 * \brief with mp4+h264, bframe PTS are unreliable
 * @return 
 */
bool         MP4Header::unreliableBFramePts (void)
{
    if(isH264Compatible(_videostream.fccHandler))
        return true;
    return false;
}
//EOF
