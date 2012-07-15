/** *************************************************************************
        \file op_aviwrite.cpp
        \brief low level avi muxer

		etc...

LAll
  LMain
   LHeader
   ( in indexer : LMovie)
   


    copyright            : (C) 2002 by mean
                           (C) Feb 2005 by GMV: ODML write support
    GPL V2.0
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_cpp.h"
#include "ADM_default.h"
#include <math.h>
#include "ADM_muxer.h"

#include "avifmt.h"
#include "avifmt2.h"
#include "fourcc.h"

#include "avilist_avi.h"
#include "op_aviwrite.hxx"

#include "ADM_quota.h"
#include "ADM_fileio.h"

#include "muxerAvi.h"

#if 1
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif

#include "aviIndex.h"
#include "aviIndexAvi.h"
#include "aviIndexOdml.h"

//------------
typedef struct VBRext
    {
  uint16_t   	    cbsize ;
  uint16_t          wId ;
  uint32_t          fdwflags ;
  uint16_t          nblocksize ;
  uint16_t          nframesperblock  ;
  uint16_t          ncodecdelay ;
} VBRext;


//________________________________________________
//   Beginning of the write process
//   We fill-in the headers
//	3- Write audio headers
//   That one can be used several times so we pass stuff
//   as parameter
//_______________________________________________
static 	uint32_t aacBitrate[16]=
{
	96000, 88200, 64000, 48000,
	44100, 32000, 24000, 22050,
	16000, 12000, 11025,  8000,
	0,     0,     0,     0
};
//

/**
    \fn aviWrite
*/
aviWrite::aviWrite( void )
{
	_out=NULL;
	LAll=NULL;
    _file=NULL;

    memset(&(audioTracks),0,sizeof(audioTracks));
    memset(openDmlHeaderPosition,0,sizeof(openDmlHeaderPosition));
    
}
/**
    \fn ~ aviWrite
*/

aviWrite::~aviWrite()
{
	if (LAll)
		delete LAll;

    if(indexMaker)
        delete indexMaker;

	LAll = NULL;
    indexMaker=NULL;
}

/**
    \fn updateHeader
    \brief when writing is done, we need to update the headers with what we actually wrote
*/
uint8_t aviWrite::updateHeader (MainAVIHeader * mainheader,
			AVIStreamHeader  *videostream)
{
        
        ADM_assert(_file);
        printf("[Avi] Updating headers...\n");
        _file->seek(32);
        AviListAvi tmpList("dummy",_file);
        tmpList.writeMainHeaderStruct(_mainheader);
        _file->seek(0x6c);
        tmpList.writeStreamHeaderStruct(_videostream);
        return 1;
}

/**
    \fn writeVideoHeader
//   Beginning of the write process
//   We fill-in the headers
//	2- Write video headers
*/
uint8_t aviWrite::writeVideoHeader( uint8_t *extra, uint32_t extraLen )
{

      ADM_assert (_file);
      _videostream.fccType = fourCC::get ((uint8_t *) "vids");
      _bih.biSize=sizeof(_bih)+extraLen;
      setVideoStreamInfo(_file,_videostream,_bih, 
                    extra,extraLen, 0x1000);
	return 1;
}

/**
        \fn WriteAudioHeader
        \brief update *header with info taken from stream then write it to file
*/
uint8_t aviWrite::writeAudioHeader(ADM_audioStream *stream, AVIStreamHeader *header,int	trackNumber)
{

uint32_t odml_audio_super_idx_size;;
WAVHeader wav;
// pre compute some headers with extra data in...
uint8_t wmaheader[12];
VBRext  mp3vbr;
uint8_t aacHeader[12];
uint8_t *extra=NULL;
uint32_t extraLen=0;

	if(!stream) return 1;
	memset(wmaheader,0,12);
	memset(&mp3vbr,0,sizeof(mp3vbr));

    memcpy(&wav,stream->getInfo(),sizeof(wav));

	wmaheader[16-16]=0x0a;
	wmaheader[19-16]=0x08;
	wmaheader[22-16]=0x01;
	wmaheader[24-16]=0x74;
	wmaheader[25-16]=01;


      memset (header, 0, sizeof (AVIStreamHeader));
      header->fccType = fourCC::get ((uint8_t *) "auds");
      header->dwInitialFrames = 0;
      header->dwStart = 0;
      header->dwRate = wav.byterate;
      header->dwSampleSize = 1;
      header->dwQuality = 0xffffffff;
      header->dwSuggestedBufferSize = 8000;

      printf("[ODML/Audio] Encoding 0x%x\n",wav.encoding);
	switch(wav.encoding)
	{
        case WAV_IMAADPCM:
                wav.blockalign=1024;
                header->dwScale         = wav.blockalign;
                header->dwSampleSize    = 1;
                header->dwInitialFrames =1;
                header->dwSuggestedBufferSize=2048;
                break;
		case WAV_AAC:
		{
            // nb sample in stream
            // since it is vbr, assume N packet of 1024 samples
			double len;
			len=_videostream.dwLength;
			len/=_videostream.dwRate;
			len*=_videostream.dwScale;
			len*=wav.frequency;
			len/=1024;
            header->dwFlags=1;
            header->dwInitialFrames=0;
            header->dwRate=wav.frequency;
            header->dwScale=1024; //sample/packet 1024 seems good for aac
            header->dwSampleSize = 0;
            header->dwSuggestedBufferSize=8192;
            header->dwInitialFrames = 0;

            wav.blockalign=1024;
            wav.bitspersample = 0;

            uint32_t aLen;
            uint8_t  *aData;
            stream->getExtraData(&aLen,&aData);

            aacHeader[0]=0x2;
            aacHeader[1]=0x0;
            if(2==aLen)
            {
                aacHeader[2]=aData[0];
                aacHeader[3]=aData[1];
            }else
            {
                int SRI=4;	// Default 44.1 khz
                for(int i=0;i<16;i++) if(wav.frequency==aacBitrate[i]) SRI=i;
            
                aacHeader[2]=(2<<3)+(SRI>>1); // Profile LOW
                aacHeader[3]=((SRI&1)<<7)+((wav.channels)<<3);
            }


            extra=&(aacHeader[0]);
            extraLen=4;
            }
            break;

        case WAV_DTS:
        case WAV_AC3: // Vista compatibility
                      extra=(uint8_t *)wmaheader;
                      extra[0]=0;
                      extra[1]=0;
                      extraLen=2;
                      header->dwScale = 1;
                      wav.blockalign=1;
                break;
        case WAV_MP3:
           {
          int samplePerFrame=1152; // see http://msdn.microsoft.com/en-us/library/ms787304(VS.85).aspx
		  // then update VBR fields
		  mp3vbr.cbsize = R16(12);
		  mp3vbr.wId = R16(1);
		  mp3vbr.fdwflags = R32(2);
	      mp3vbr.nframesperblock = R16(1);
		  mp3vbr.ncodecdelay = 0;

		  wav.bitspersample = 0;
		  mp3vbr.nblocksize=samplePerFrame; //384; // ??

		  header->dwScale = 1;
	  	  header->dwInitialFrames = 1;
          extra=(uint8_t *)&mp3vbr;
		  extraLen=sizeof(mp3vbr);
          if (1) // FIXME stream->isVBR()) //wav->blockalign ==1152)	// VBR audio
		  {			// We do like nandub do
		  	//ADM_assert (audiostream->asTimeTrack ());
            if(wav.frequency>=32000)  // mpeg1
            {
                samplePerFrame=1152;
            }else                       // Mpeg2 , we assume layer3
            {
                samplePerFrame=576;
            }
		  	wav.blockalign = samplePerFrame;	// just a try
            wav.bitspersample = 16;
            header->dwRate 	= wav.frequency;	//wav->byterate;
			header->dwScale = wav.blockalign;
			header->dwLength= _videostream.dwLength;
  			header->dwSampleSize = 0;
			mp3vbr.nblocksize=samplePerFrame;

		   }
		   else
           {
             wav.blockalign=1;
           }
           }
		  break;


	case WAV_WMA:
			header->dwScale 	= wav.blockalign;
			header->dwSampleSize 	= wav.blockalign;
			header->dwInitialFrames =1;
			header->dwSuggestedBufferSize=10*wav.blockalign;
			extra=(uint8_t *)&wmaheader;
			extraLen=12;
			break;
    case WAV_PCM:
    case WAV_LPCM:
            header->dwScale=header->dwSampleSize=wav.blockalign=2*wav.channels; // Realign
            header->dwLength/=header->dwScale;
            break;
    case WAV_8BITS_UNSIGNED:
            wav.encoding=WAV_PCM;
			header->dwScale=header->dwSampleSize=wav.blockalign=wav.channels;
			header->dwLength/=header->dwScale;
            wav.bitspersample=8;
            break;


	default:
			header->dwScale = 1;
			wav.blockalign=1;
			break;
    }
	setAudioStreamInfo (_file,
			*header,
	 		wav,
			trackNumber,
			extra,extraLen, 0x1000);

  return 1;
}

//_______________________________________________________
//
//   Begin to save, built header and prepare structure
//   The nb frames is indicative but the real value
//   must be smaller than this parameter
//
//_______________________________________________________
uint8_t aviWrite::saveBegin (
             const char         *name,
		     ADM_videoStream    *video,
             uint32_t           nbAudioStreams,
             ADM_audioStream 	*audiostream[])
{



//  Sanity Check
        ADM_assert (_out == NULL);
        if (!(_out = qfopen (name, "wb")))
        {
                printf("Problem writing : %s\n",name);
                return 0;
        }
        _file=new ADMFile();
        if(!_file->open(_out))
        {
                printf("Cannot create ADMfileio\n");
                delete _file;
                _file=NULL;
                return 0;
        }
        vframe = 0;
        nb_audio=0;
    // Build avi like structure from the one from vstream 
    //----------------------------------------------------
        memset (&_mainheader, 0, sizeof (MainAVIHeader));
        mx_mainHeaderFromVideoStream(&_mainheader,video);
        _mainheader.dwStreams = 1+nbAudioStreams;
        nb_audio=nbAudioStreams;
        _mainheader.dwTotalFrames = 0;

        memset (&_videostream, 0, sizeof (AVIStreamHeader));
        mx_streamHeaderFromVideo(&_videostream,video);
        _videostream.dwLength = 0;


        mx_bihFromVideo(&_bih,video);

  //___________________
  // Prepare header
  //___________________
    uint32_t videoextraLen;
    uint8_t  *videoextra;
    video->getExtraData(&videoextraLen, &videoextra);


      _file->seek(0);
      LAll = new AviListAvi ("RIFF", _file);
      LAll->Begin();
      LAll->Write32("AVI ");


      // Header chunk
      AviListAvi *LMain = new AviListAvi ("LIST", _file);
      LMain->Begin();
      LMain->Write32("hdrl");
      LMain->Write32 ("avih");
      LMain->Write32 (sizeof (MainAVIHeader));
      LMain->writeMainHeaderStruct(_mainheader);
 	  writeVideoHeader(videoextra,videoextraLen );
      for(int i=0;i<nb_audio;i++)
        {
            writeAudioHeader(audiostream[i],&(audioTracks[i].header),i);
        }

	LMain->End();
	delete LMain;
	LMain=NULL;
    for(int i=0;i<3;i++)
        ADM_info("SuperIndex position so far %d : %"LLD"\n",i,openDmlHeaderPosition[i]);
  //___________________________________
  // Write the beginning of the movie part
  //___________________________________
    switch(muxerConfig.odmlType)
    {
        case AVI_MUXER_TYPE1:
        case AVI_MUXER_AUTO:
                indexMaker=new aviIndexAvi(this);   
                break;
        case AVI_MUXER_TYPE2:
                indexMaker=new aviIndexOdml(this);   
                break;
        default:
                ADM_assert(0);
                break;
    }
    
  
  vframe = 0;
  return 1;
}

/**
    \fn    saveVideoFrame
    \brief Write video frames and update index accordingly
*/
uint8_t aviWrite::saveVideoFrame (uint32_t len, uint32_t flags, uint8_t * data)
{
    vframe++;   
    return indexMaker->addVideoFrame(len, flags,data);
}
/**
       \fn saveAudioFrame
*/
uint8_t aviWrite::saveAudioFrame (uint32_t index,uint32_t len, uint8_t * data)
{
    // update our stats
    aviAudioTrack *trk=&(audioTracks[index]);
    trk->sizeInBytes+=len;
    trk->nbBlocks++;
    return indexMaker->addAudioFrame(index,len,0,data);
}
/**
    \fn setEnd
    \brief end movie writing, update headers & close file
*/
uint8_t aviWrite::setEnd (void)
{

  // First close the movie
  indexMaker->writeIndex();
  // and delete everything...
  LAll->End ();
  delete    LAll;
  LAll = NULL;
  _mainheader.dwTotalFrames = vframe;
  _videostream.dwLength = vframe;

// Update Header
// AUDIO SIZE ->TODO
  updateHeader (&_mainheader, &_videostream);
  printf("\n End of movie, \n video frames : %u\n",vframe);
    for(int i=0;i<nb_audio;i++)
    {
        printf("Track %d Size :%"LU" bytes, %"LU" blocks\n",i,audioTracks[i].sizeInBytes,audioTracks[i].nbBlocks);
    }

  // cleanup
  delete _file;
  _file=NULL;

  qfclose (_out);
  _out = NULL;
  return 1;

}
/**
        \fn setVideoStreamInfo

*/
bool aviWrite::setVideoStreamInfo (ADMFile * fo,
			 const AVIStreamHeader      &stream,
			 const ADM_BITMAPINFOHEADER &bih,
			 uint8_t * extra, uint32_t extraLen,
			 uint32_t maxxed)
{
  AviListAvi * alist;

  alist = new AviListAvi ("LIST", fo);
  // 12 LIST
  // 8 strf subchunk
  // 8 strl subchunk
  // 8 defaultoffset
  alist->Begin();
  alist->Write32("strl");
  // sub chunk 1
  alist->writeStrh(stream);
  alist->writeStrfBih(bih,extraLen,extra);

  // Place holder for odml super index
  uint64_t pos;
  alist->writeDummyChunk(AVI_INDEX_CHUNK_SIZE,&pos);
  printf("[ODML] videoTrack : using ODML placeholder of size %u bytes at pos 0x%"LLX"\n",AVI_INDEX_CHUNK_SIZE,pos);  
  openDmlHeaderPosition[0]=pos;
  alist->End ();
  delete alist;
  return 1;
}
/**
        \fn setVideoStreamInfo

*/
bool aviWrite::setAudioStreamInfo (ADMFile * fo,
			 const AVIStreamHeader      &stream,
			 const WAVHeader &wav,
			 int audioTrackNumber,
			 uint8_t * extra, uint32_t extraLen,
			 uint32_t maxxed)
{
  AviListAvi * alist;
  
  alist = new AviListAvi ("LIST", fo);  
  // 12 LIST
  // 8 strf subchunk
  // 8 strl subchunk
  // 8 defaultoffset
  alist->Begin();
  alist->Write32("strl");
  // sub chunk 1
  alist->writeStrh(stream);
  alist->writeStrfWav(wav,extraLen,extra);

  
  uint64_t pos;
  alist->writeDummyChunk(AVI_INDEX_CHUNK_SIZE,&pos);
  ADM_info("[ODML] Audio track %d, using ODML placeholder of size %u bytes, odmltrack=%d, pos=0x%"LLX"\n",
                            audioTrackNumber,AVI_INDEX_CHUNK_SIZE,1+audioTrackNumber,pos);  
  openDmlHeaderPosition[1+audioTrackNumber]=pos;
  alist->End ();
  delete alist;
  return 1;
}


// EOF
