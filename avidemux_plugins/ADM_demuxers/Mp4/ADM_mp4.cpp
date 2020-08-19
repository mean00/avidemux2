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
#include "DIA_processing.h"

#include "ADM_videoInfoExtractor.h"
#include "ADM_codecType.h"
#include "ADM_a52info.h"
#include "ADM_mp3info.h"
#include "ADM_dcainfo.h"
#include "ADM_audioXiphUtils.h"

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
    delay=0;
    totalDataSize=0;
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

#ifdef DERIVE_TB_FROM_MINIMUM_DELTA
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
        uint32_t fps1000=floor(f+0.49);
        ADM_info("MinDelta=%d us\n",(int)minDelta);
        ADM_info("Computed fps1000=%d\n",fps1000);
        if(fps1000 == _videostream.dwRate)
        {
            ADM_info("Computed fps1000 matches the average one.\n");
            return true;
        }
        int score=0;
        uint64_t avgDelta=_mainaviheader.dwMicroSecPerFrame;
        int64_t halfway=avgDelta-minDelta+1;
        halfway/=2;
        halfway+=minDelta;
        for(int i=0;i<n-1;i++)
        {
            MP4Index *dex=&(_tracks[0].index[i]);
            MP4Index *next=&(_tracks[0].index[i+1]);
            if(dex->dts==ADM_NO_PTS) continue;
            if(next->dts==ADM_NO_PTS) continue;
            uint64_t delta=next->dts-dex->dts;
            if(delta==minDelta) score++;
            if(delta<halfway) score++;
        }
        float weighted=score*1000.;
        weighted/=n;
        ADM_info("Original fps1000 = %d, score = %d, weighted score = %d\n",_videostream.dwRate,score,(int)weighted);
        // require that at least 10% of video better or 5% perfectly matches max fps
        if(fps1000 > _videostream.dwRate && (int)weighted > 100)
        {
            ADM_info("Adjusting fps, the computed is higher than average, dropped frames ?\n");
           _videostream.dwRate=fps1000;
           _mainaviheader.dwMicroSecPerFrame=ADM_UsecFromFps1000(_videostream.dwRate);
        }
    }
    return true;
    
}
#endif

uint8_t  MP4Header::getFrame(uint32_t framenum,ADMCompressedImage *img)
{
    aprintf("[MP4] frame %d requested (nbFrame=%d)\n",framenum,VDEO.nbIndex);
    if(framenum>=VDEO.nbIndex)
    {
      return 0;
    }

    MP4Index *idx=&(VDEO.index[framenum]);
    uint64_t sz = idx->size;
    if(sz > ADM_COMPRESSED_MAX_DATA_LENGTH)
    {
        ADM_warning("Frame %u size %llu exceeds max %u, truncating.\n",framenum,sz,ADM_COMPRESSED_MAX_DATA_LENGTH);
        sz = ADM_COMPRESSED_MAX_DATA_LENGTH;
    }

    uint64_t offset=idx->offset; //+_mdatOffset;


    if(fseeko(_fd,offset,SEEK_SET))
    {
        ADM_error("Seeking past the end of the file! Broken index?\n");
        return 0;
    }
    if(!fread(img->data, (size_t)sz, 1, _fd))
    {
        ADM_error("Incomplete frame %" PRIu32". Broken index?\n",framenum);
        return 0;
    }
    img->dataLength=sz;
    img->flags = idx->intra;

    img->demuxerDts=idx->dts;
    img->demuxerPts=idx->pts;
    aprintf("[MP4] Pts=%s\n",ADM_us2plain(idx->pts));
    aprintf("[MP4] Dts=%s\n",ADM_us2plain(idx->dts));
    /*
    if(img->demuxerPts==ADM_COMPRESSED_NO_PTS)
        img->demuxerPts=img->demuxerDts;
    */
    return 1;
}
MP4Header::~MP4Header()
{
    close();

    for(int audio = 0; audio < nbAudioTrack; audio++)
    {
        delete audioStream[audio];
        delete audioAccess[audio];
    }
    for(int i = 0; i < nbTrex; i++)
    {
        delete _trexData[i];
        _trexData[i] = NULL;
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
        nbTrex=0;
        for(int i=0;i<_3GP_MAX_TRACKS;i++)
            _trexData[i]=NULL;
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
        printf("** opening 3gpp files **\n");
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
                                        printf("Header starts at %" PRIx64"\n",of);
                                        delete atom;
                                        atom=new adm_atom(_fd);
        }
        //**************

        if(!lookupMainAtoms((void*) atom))
        {
          printf("Cannot find needed atom\n");   
          if(!_tracks[0].fragments.size() || !indexVideoFragments(0))
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
                  if(_tracks[i].index==NULL)
                      nbAudioTrack--;
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
                    printf("MP4 Corrected size : %" PRIu32" x %" PRIu32"\n",w,h);
                    _video_bih.biWidth=_mainaviheader.dwWidth=w ;
                    _video_bih.biHeight=_mainaviheader.dwHeight=h;
                }
            }else { printf("No extradata to probe\n");}

        }else if(fourCC::check(_videostream.fccHandler,(uint8_t *)"H263"))
        { // Same story for H263 : Analyze 1st frame to get the real width/height
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
                        printf("H263 Corrected size : %" PRIu32" x %" PRIu32"\n",w,h);
                        _video_bih.biWidth=_mainaviheader.dwWidth=w ;
                        _video_bih.biHeight=_mainaviheader.dwHeight=h;
                    }else
                    {
                        printf("H263 COULD NOT EXTRACT SIZE, using : %" PRIu32" x %" PRIu32"\n",
                        _video_bih.biWidth,  _video_bih.biHeight);
                    }
                }
                delete [] bfer;
            }
        }else if(isH264Compatible(_videostream.fccHandler) && VDEO.extraDataSize)
        { // Get frame type from H.264 slice headers, this is essential for handling of field encoded streams
            ADM_SPSInfo info;
            if(extractSPSInfo_mp4Header(VDEO.extraData,VDEO.extraDataSize,&info))
            {
                uint32_t nalSize = ADM_getNalSizeH264(VDEO.extraData,VDEO.extraDataSize);
                uint32_t prevSpsLen=0;
                uint8_t *prevSps=NULL, *curSps=NULL;
#define MAX_SPS_SIZE 1024
#define MAX_FRAME_LENGTH (1920*1080*3) // ~7 MiB, should be enough even for 4K
                uint8_t *bfer=new uint8_t[MAX_FRAME_LENGTH];
                ADMCompressedImage img;
                img.data=bfer;
                uint32_t i,fields=0,nb=VDEO.nbIndex;
                uint64_t processed=0;
                DIA_processingBase *work=createProcessing(QT_TRANSLATE_NOOP("mp4demuxer","Decoding frame type"),nb);
                for(i=0;i<nb;i++)
                {
                    if(work && work->update(1,processed++))
                        break; // cancelling frame type decoding is non-fatal
                    if(!getFrame(i,&img))
                    {
                        ADM_warning("Could not get frame %u while decoding H.264 frame type.\n",i);
                        continue;
                    }
                    if(img.flags & AVI_KEY_FRAME)
                    {
                        // Check for presence of SPS in the stream. If it changes on-the-fly, we are in trouble.
                        if(!curSps)
                            curSps=new uint8_t[MAX_SPS_SIZE];
                        memset(curSps,0,MAX_SPS_SIZE);
                        uint32_t curSpsLen = getRawH264SPS(img.data, img.dataLength, nalSize, curSps, MAX_SPS_SIZE);
                        bool match=true;
                        if(curSpsLen)
                        {
                            if(prevSps)
                            {
                                if(prevSpsLen)
                                    match=!memcmp(prevSps,curSps,(prevSpsLen>curSpsLen)? curSpsLen : prevSpsLen);
                            }else
                            {
                                prevSps=new uint8_t[MAX_SPS_SIZE];
                            }
                            if(!match)
                            {
                                ADM_warning("Codec parameters change at frame %u.\n",i);
                                printf("\nOld SPS:\n");
                                mixDump(prevSps,prevSpsLen);
                                printf("\nNew SPS:\n");
                                mixDump(curSps,curSpsLen);
                            }
                            prevSpsLen=curSpsLen;
                            memset(prevSps,0,MAX_SPS_SIZE);
                            memcpy(prevSps,curSps,prevSpsLen);
                        }
                        if(!match && curSps)
                        {
                            ADM_info("SPS mismatch? Checking deeper...\n");
                            ADM_SPSInfo info2;
                            if(extractSPSInfoFromData(curSps,curSpsLen,&info2))
                            { // check only fields we actually use
#define MATCH(x) if(info.x != info2.x) { ADM_warning("%s value does not match.\n",#x); info.x = info2.x; match=false; }
                                match=true;
                                // FIXME: dimensions mismatch should be fatal
                                MATCH(CpbDpbToSkip)
                                MATCH(hasPocInfo)
                                MATCH(log2MaxFrameNum)
                                MATCH(log2MaxPocLsb)
                                MATCH(frameMbsOnlyFlag)
                                MATCH(refFrames)
                                if(!match)
                                    ADM_warning("Codec parameters change on the fly, expect problems.\n");
                            }
                        }
                    }
                    uint32_t flags;
                    if(extractH264FrameType(img.data,img.dataLength,nalSize,&flags,NULL,&info))
                    {
                        if(flags & AVI_FIELD_STRUCTURE)
                        {
                            if(!fields)
                                printf("First field at frame %u\n",i);
                            fields++;
                        }else if(fields==1)
                        { // discard a single field immediately followed by a frame, probably damaged stream
                            printf("Resetting fields counter at frame %u\n",i);
                            fields=0;
                        }
                        setFlag(i,flags);
                    }
                }
                if(work) delete work;
                work=NULL;
                delete [] bfer;
                bfer=NULL;
                if(curSps) delete [] curSps;
                curSps=NULL;
                if(prevSps) delete [] prevSps;
                prevSps=NULL;
                if(fields)
                    ADM_info("Field encoded H.264 stream detected, # fields: %u\n",fields);
                else
                    ADM_info("Probably a frame encoded H.264 stream.\n");
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
                  break;
            }
            case WAV_MP3: // same for mp3
            {
                MP4Index *dex=_tracks[1+audio].index;
                int size=dex[0].size;
                uint8_t *buffer=new uint8_t[size];
                fseeko(_fd,dex[0].offset,SEEK_SET);
                if(fread(buffer,1,size,_fd))
                {
                    uint32_t off;
                    MpegAudioInfo mpeg;
                    if(getMpegFrameInfo(buffer, size, &mpeg, NULL, &off) && size >= off+mpeg.size)
                    {
                        if(mpeg.mode == 3 && _tracks[1+audio]._rdWav.channels!=1)
                        {
                            uint32_t fq=mpeg.samplerate;
                            uint32_t br=(mpeg.bitrate*1000)>>3; //
                            ADM_info("Updating MP3 info : Fq=%u, br=%u, chan=%u\n",fq,br,1);
                            _tracks[1+audio]._rdWav.channels=1;
                            _tracks[1+audio]._rdWav.frequency=fq;
                            _tracks[1+audio]._rdWav.byterate=br;
                        }
                    }else
                    {
                        ADM_warning("Cannot get MP3 info from the first sample.\n");
                    }
                }
                delete [] buffer;
                break;
            }
            case WAV_DTS:
            {
                MP4Index *dex=_tracks[1+audio].index;
                int size=dex[0].size;
                uint8_t *buffer=(uint8_t *)ADM_alloc(size);
                ADM_assert(buffer);
                fseeko(_fd,dex[0].offset,SEEK_SET);
                if(fread(buffer,1,size,_fd))
                {
                    uint32_t off;
                    ADM_DCA_INFO dca;
                    if(ADM_DCAGetInfo(buffer,size,&dca,&off)) // just for DTS core
                    {
                        uint32_t fq=dca.frequency;
                        uint32_t br=dca.bitrate>>3;
                        ADM_info("Updating DTS info: fq = %u, br = %u, chan = %u\n",fq,br,dca.channels);
                        _tracks[1+audio]._rdWav.channels=dca.channels;
                        _tracks[1+audio]._rdWav.frequency=fq;
                        _tracks[1+audio]._rdWav.byterate=br;
                    }else
                    {
                        ADM_warning("Cannot get DCA info from the first sample.\n");
                    }
                }
                ADM_dealloc(buffer);
                break;
            }
            case WAV_OGG_VORBIS:
            {
                ADM_info("[MP4] Reformatting vorbis header for track %u\n",1+audio);
                uint8_t *newExtra=NULL;
                int newExtraSize=0;
                if(false==ADMXiph::xiphExtraData2Adm(_tracks[1+audio].extraData, _tracks[1+audio].extraDataSize, &newExtra, &newExtraSize))
                {
                    ADM_warning("Cannot reformat vorbis extra data, faking audio format to avoid crash.\n");
                    _tracks[1+audio]._rdWav.encoding=WAV_UNKNOWN;
                }
                // Destroy old extradata
                delete [] _tracks[1+audio].extraData;
                _tracks[1+audio].extraData=newExtra;
                _tracks[1+audio].extraDataSize=newExtraSize;
            }
                break;
            default:
                break;
            }
            audioAccess[audio]=new ADM_mp4AudioAccess(name,&(_tracks[1+audio]));
            audioStream[audio]=ADM_audioCreateStream(&(_tracks[1+audio]._rdWav), audioAccess[audio]);
        }
        fseeko(_fd,0,SEEK_SET);
        uint64_t duration1=_movieDuration*1000LL;
        uint64_t duration2=0;
        uint32_t lastFrame=0;
        for(int i=nb-32;i<nb;i++)
        {
            if(i<0) continue;
            if(_tracks[0].index[i].pts==ADM_NO_PTS) continue;
            if(duration2<_tracks[0].index[i].pts)
            {
                duration2=_tracks[0].index[i].pts;
                lastFrame=i;
            }
        }
        int64_t increment=_mainaviheader.dwMicroSecPerFrame;
        if(!increment) // perfectly regular stream
        {
            ADM_assert(_videostream.dwRate);
            double f=_videostream.dwScale;
            f*=1000.*1000.;
            f/=_videostream.dwRate;
            f+=0.49;
            increment=(int64_t)f;
        }
        duration2+=increment;
        ADM_info("3gp/mov file successfully read..\n");
        if(duration2!=ADM_NO_PTS && duration2>=duration1)
        { // video duration must be > max PTS, otherwise we drop the last frame
            ADM_warning("Last PTS is at or after movie duration, increasing movie duration\n");
            _movieDuration=(duration2+499)/1000;
#ifdef DERIVE_TB_FROM_MINIMUM_DELTA
            // adjust calculated average FPS and time increment
            double f=_movieDuration;
            f=1000.*_tracks[0].nbIndex/f;
            f*=1000.;
            _videostream.dwRate=(uint32_t)floor(f+0.49);
            _mainaviheader.dwMicroSecPerFrame=ADM_UsecFromFps1000(_videostream.dwRate);
            ADM_info("Adjusted fps1000: %d = %" PRIu64" us per frame.\n",_videostream.dwRate,_mainaviheader.dwMicroSecPerFrame);
#endif
        }
#ifdef DERIVE_TB_FROM_MINIMUM_DELTA
        refineFps();
#endif
        if(nb>1 && !lastFrame)
            lastFrame=nb-1;
        ADM_info("Nb images       : %d\n",nb);
        ADM_info("Movie duration  : %s\n",ADM_us2plain(_movieDuration*1000LL));
        ADM_info("Last video PTS  : %s\n",ADM_us2plain(_tracks[0].index[lastFrame].pts));
        ADM_info("Last video DTS  : %s\n",ADM_us2plain(_tracks[0].index[nb-1].dts));

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
bool MP4Header::adjustElstDelay(void)
{
    int xmin=10000000;
    int xscaledDelay[_3GP_MAX_TRACKS];
    for(int i=0;i<1+nbAudioTrack;i++)
    {
        double scaledDelay=_tracks[i].delay;
        double scaledStartOffset=_tracks[i].startOffset;
        scaledDelay/=_movieScale;
        scaledStartOffset/=_tracks[i].scale;
        scaledDelay*=1000000;
        scaledStartOffset*=1000000;
        ADM_info("Delay for track %d : raw = %d, scaled  = %d with scale = %d\n",i,_tracks[i].delay,(int)scaledDelay,_movieScale);
        ADM_info("Start offset for track %d : raw = %d, scaled = %d with scale = %d\n",i,_tracks[i].startOffset,(int)scaledStartOffset,_tracks[i].scale);
        scaledDelay-=scaledStartOffset;
        xscaledDelay[i]=scaledDelay;
        if(scaledDelay<xmin)
            xmin=scaledDelay;
    }
    ADM_info("Elst minimum = %d us\n",xmin);
    for(int i=0;i<1+nbAudioTrack;i++)
    {
        int d=xscaledDelay[i]-xmin;
        if(d)
        {
            ADM_info("    Shifting track %d by %s\n",i,ADM_us2plain(d));
            shiftTrackByTime(i,d);
        }
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
        for(int i=1;i<nbAudioTrack;i++)            
            shiftTrackByTime(i,shift);
        return true;        
}
/**
 * 
 * @param dex
 * @param shift
 * @return 
 */
bool MP4Header::shiftTrackByTime(int dex,uint64_t shift)
{
    int nb=(int)_tracks[dex].nbIndex;
    MP4Index *myIndex=_tracks[dex].index;
    for(int i=0;i<nb;i++)
    {
             uint64_t dts,pts;
                dts=myIndex[i].dts;
                pts=myIndex[i].pts;
                if(dts!=ADM_COMPRESSED_NO_PTS)
                {
                    dts+=shift;
                }
                if(pts!=ADM_COMPRESSED_NO_PTS)
                {
                    pts+=shift;
                }
                myIndex[i].dts=dts;
                myIndex[i].pts=pts;
     }

    return true;
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
      printf("[MP4] Frame %" PRIu32" exceeds # of frames %" PRIu32"\n",frame,VDEO.nbIndex);
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
      printf("[MP4] Frame %" PRIu32" exceeds # of frames %" PRIu32"\n",frame,VDEO.nbIndex);
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
