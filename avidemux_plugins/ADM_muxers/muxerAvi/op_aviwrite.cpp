/** *************************************************************************
        \file op_aviwrite.cpp
        \brief low level avi muxer

		etc...


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

#include "ADM_default.h"
#include <math.h>
#include "ADM_muxer.h"

#include "avifmt.h"
#include "avifmt2.h"
#include "fourcc.h"

#include "avilist.h"
#include "op_aviwrite.hxx"

#include "ADM_quota.h"
#include "ADM_fileio.h"

#include "muxerAvi.h"

#if 1
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif


uint32_t ADM_UsecFromFps1000(uint32_t fps1000);
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
static void mx_bihFromVideo(ADM_BITMAPINFOHEADER *_bih,ADM_videoStream *video);
static void mx_mainHeaderFromVideoStream(MainAVIHeader  *header,ADM_videoStream *video);
static void mx_streamHeaderFromVideo(AVIStreamHeader *header,ADM_videoStream *video);

/**
    \fn aviWrite
*/
aviWrite::aviWrite( void )
{
	_out=NULL;
	LAll=NULL;
	LMovie=NULL;
	LMain=NULL;
    _file=NULL;
	odml_indexes=NULL;

    doODML=muxerConfig.odmlType;	// default; TODO: user should be able to choose NO for plain avi
    memset(&(audioTracks),0,sizeof(audioTracks));
}
/**
    \fn ~ aviWrite
*/

aviWrite::~aviWrite(){

	if (LAll)
		delete LAll;

	if (LMovie)
		delete LMovie;

	if (LMain)
		delete LMain;


	LAll = NULL;
	LMovie = NULL;
	LMain = NULL;

	odml_destroy_index();
}
/**
    \fn sync
*/
uint8_t aviWrite::sync( void )
{
	ADM_assert(_file);
	_file->flush();
	return 1;

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
// Update main header
#ifdef ADM_BIG_ENDIAN
	MainAVIHeader ma;
    AVIStreamHeader as;

	memcpy(&ma,mainheader,sizeof(MainAVIHeader));
	Endian_AviMainHeader(&ma);
  	_file->write ((uint8_t *)&ma, sizeof (ma));
    _file->seek(0x6c);

	memcpy(&as,videostream,sizeof(as));
	Endian_AviStreamHeader(&as);
  	_file->write ((uint8_t *)&as, sizeof (as));
#else
    _file->write ((uint8_t *)mainheader, sizeof (MainAVIHeader));
    _file->seek(0x6c);
    _file->write ((uint8_t *)videostream, sizeof (AVIStreamHeader));
#endif
  // update audio headers
    for(int i=0;i<nb_audio;i++)
    {
        _file->seek(audioTracks[i].audioHeaderOffset);
/*
        setStreamInfo (_file,
			(uint8_t *) header,
	 		(uint8_t *) &wav, sizeof (WAVHeader),
			odml_audio_super_idx_size,odml_stream_nbr,
			extra,extraLen, 0x1000);
*/
    }
  return 1;
}

/**
    \fn writeMainHeader
//   Beginning of the write process
//   We fill-in the headers
//	1- Create list and write main header
*/
uint8_t aviWrite::writeMainHeader( void )
{

  ADM_assert (_file);
  ADM_assert (LAll == NULL);
  _file->seek(0);


  LAll = new AviList ("RIFF", _file);
  LAll->Begin ("AVI ");
  // Header chunk
  LMain = new AviList ("LIST", _file);
  LMain->Begin ("hdrl");
  LMain->Write32 ("avih");
  LMain->Write32 (sizeof (MainAVIHeader));
#ifdef ADM_BIG_ENDIAN
	MainAVIHeader ma;
	memcpy(&ma,&_mainheader,sizeof(ma));
	Endian_AviMainHeader(&ma);
	LMain->Write((uint8_t *)&ma,sizeof(ma));
#else
  	LMain->Write ((uint8_t *) &_mainheader, sizeof (MainAVIHeader));
#endif
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
uint32_t odml_video_super_idx_size;
        ADM_assert (_file);
      _videostream.fccType = fourCC::get ((uint8_t *) "vids");
      _bih.biSize=sizeof(_bih)+extraLen;
        if(doODML!=NORMAL)
        {

            odml_video_super_idx_size=24+odml_default_nbrof_index*16;
        }else
        {
            odml_video_super_idx_size=24+odml_indexes[0].odml_nbrof_index*16;
        }
#ifdef ADM_BIG_ENDIAN
	// in case of Little endian, do the usual swap crap

	AVIStreamHeader as;
	ADM_BITMAPINFOHEADER b;
	memcpy(&as,&_videostream,sizeof(as));
	Endian_AviStreamHeader(&as);
	memcpy(&b,&_bih,sizeof(_bih));
	Endian_BitMapInfo( &b );
  	setStreamInfo (_file, (uint8_t *) &as,
		  (uint8_t *)&b,sizeof(ADM_BITMAPINFOHEADER),
            odml_video_super_idx_size,0,
            extra,extraLen,
            0x1000);
#else
  	setStreamInfo (_file, (uint8_t *) &_videostream,
		  (uint8_t *)&_bih,sizeof(ADM_BITMAPINFOHEADER),
            odml_video_super_idx_size,0,
            extra,extraLen,
            0x1000);

#endif
	return 1;
}

/**
        \fn WriteAudioHeader
        \brief update *header with info taken from stream then write it to file
*/
uint8_t aviWrite::writeAudioHeader(ADM_audioStream *stream, AVIStreamHeader *header,uint8_t	odml_stream_nbr)
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
        if(doODML!=NORMAL)
        {
            odml_audio_super_idx_size=24+odml_default_nbrof_index*16;
        }else
        {
            odml_audio_super_idx_size=24+odml_indexes[odml_stream_nbr].odml_nbrof_index*16;
        }
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

            int SRI=4;	// Default 44.1 khz
            for(int i=0;i<16;i++) if(wav.frequency==aacBitrate[i]) SRI=i;
            aacHeader[0]=0x2;
            aacHeader[1]=0x0;
            aacHeader[2]=(2<<3)+(SRI>>1); // Profile LOW
            aacHeader[3]=((SRI&1)<<7)+((wav.channels)<<3);


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
#ifdef ADM_BIG_ENDIAN
	// in case of Little endian, do the usual swap crap

	AVIStreamHeader as;
	WAVHeader w;
	memcpy(&as,header,sizeof(as));
	Endian_AviStreamHeader(&as);
	memcpy(&w,&wav,sizeof(w));
	Endian_WavHeader( &w );
  	setStreamInfo (_file,
		(uint8_t *) &as,
		(uint8_t *)&w,sizeof(WAVHeader),
		odml_audio_super_idx_size,odml_stream_nbr,
		extra,extraLen,
		0x1000);
#else
	setStreamInfo (_file,
			(uint8_t *) header,
	 		(uint8_t *) &wav, sizeof (WAVHeader),
			odml_audio_super_idx_size,odml_stream_nbr,
			extra,extraLen, 0x1000);
#endif

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
        curindex = 0;
        vframe = 0;
        nb_audio=0;

        memset (&_mainheader, 0, sizeof (MainAVIHeader));
        mx_mainHeaderFromVideoStream(&_mainheader,video);
        _mainheader.dwStreams = 1+nbAudioStreams;
        nb_audio=nbAudioStreams;
        _mainheader.dwTotalFrames = 0;

        memset (&_videostream, 0, sizeof (AVIStreamHeader));
        mx_streamHeaderFromVideo(&_videostream,video);
        _videostream.dwLength = 0;


        mx_bihFromVideo(&_bih,video);


        // test for free data structures
        if(odml_indexes!=NULL){
            aprintf("\n ODML writer error: data structures not empty for init!");
            return 0;
        }


	if(doODML!=NO){
		// get number of streams
		odml_nbrof_streams=_mainheader.dwStreams;
		aprintf("\nnumber of streams: %d\n",odml_nbrof_streams);
		// get number of frames per index

		odml_index_size=(long)ceil(1000000.0/(double)_mainheader.dwMicroSecPerFrame*600.0);	// one index per 10 Minutes; decrease if 4GB are not enough for this amount of time
                aprintf("\n old number of frames per index: %d\n",odml_index_size);
                double fps=video->getAvgFps1000();
                        fps/=1000;
                        aprintf("Fps1000:%f\n",fps);
                        fps=600*fps; // 10 mn worth;
                        odml_index_size=(int)floor(fps);

		aprintf("\nnumber of frames per index: %d\n",odml_index_size);
		// get number or indexes per stream
		odml_default_nbrof_index=2048;
		aprintf("\nnumber of indexes per stream: %d\n",odml_default_nbrof_index);
		// init some other values
		odml_header_fpos=0;
		odml_riff_fpos[0]=0;odml_riff_fpos[1]=0;odml_riff_fpos[2]=0;odml_riff_fpos[3]=0;
		odml_riff_count=0;
		odml_frames_inAVI=0;
		// create odml index data structure
		odml_indexes=(odml_super_index_t*) ADM_alloc (sizeof(odml_super_index_t) * odml_nbrof_streams); // super index list
		memset(odml_indexes,0,sizeof(odml_super_index_t) * odml_nbrof_streams);
		for(int a=0;a<odml_nbrof_streams;++a)
                {	// for each stream -> one super index
                        odml_indexes[a].odml_nbrof_index=odml_default_nbrof_index;
			odml_indexes[a].odml_index= (odml_index_t*) ADM_alloc (sizeof(odml_index_t) * odml_default_nbrof_index); // index list
			memset(odml_indexes[a].odml_index,0,sizeof(odml_index_t) * odml_default_nbrof_index);
			for(int b=0;b<odml_default_nbrof_index;++b)
                        {	// for each index
				odml_indexes[a].odml_index[b].index=(odml_index_data_t*) ADM_alloc (sizeof(odml_index_data_t) * odml_index_size);	// index data
				memset(odml_indexes[a].odml_index[b].index,0,sizeof(odml_index_data_t) * odml_index_size);
				odml_indexes[a].odml_index[b].nEntriesInUse=0;	// (redundant)
			}
			// init data
			odml_indexes[a].index_count=0;
		}
	}
            else
        {
            odml_default_nbrof_index=16;
        }

  //___________________
  // Prepare header
  //___________________

	writeMainHeader( );

    uint32_t videoextraLen;
    uint8_t  *videoextra;
    video->getExtraData(&videoextraLen, &videoextra);
	writeVideoHeader(videoextra,videoextraLen );
    for(int i=0;i<nb_audio;i++)
    {
        audioTracks[i].audioHeaderOffset=LMain->Tell();
        writeAudioHeader(audiostream[i],&(audioTracks[i].header),i+1);
    }
	odml_write_dummy_chunk(LMain, &odml_header_fpos, 16);

	LMain->End();
	delete LMain;
	LMain=NULL;
  //___________________________________
  // Write the beginning of the movie part
  //___________________________________
  ADM_assert (!LMovie);

  LMovie = new AviList ("LIST", _file);
  LMovie->Begin ("movi");
  curindex = 0;
  // the *2 is for audio and video
  // the *3 if for security sake


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
	// write initial index chunks
	if(vframe==2 && doODML!=NO)
    {	// apparently some players require a video frame at first in the movi list, so we put the initial index dummys behind it (bye bye index before data)
		odml_write_dummy_chunk(LMovie, &(odml_indexes[0].odml_index[0].fpos), 24+8*odml_index_size);
        for(int i=1;i<odml_nbrof_streams;i++)
			odml_write_dummy_chunk(LMovie, &(odml_indexes[i].odml_index[0].fpos), 24+8*odml_index_size);
	}
	// test for new riff
	odml_riff_break(len+8); // data size + fcc + size info (padding is handled in odml_riff_break)
	// index frame
	if(!odml_index_frame(0, len,flags&AVI_KEY_FRAME))
    {
		aprintf("\ncan not index video frame %u\n",vframe);
	}
  return saveFrame (len, flags, data, (uint8_t *) "00dc");

}
/**
       \fn saveAudioFrame
*/
uint8_t aviWrite::saveAudioFrame (uint32_t index,uint32_t len, uint8_t * data)
{
    aviAudioTrack *trk=&(audioTracks[index]);
    trk->sizeInBytes+=len;
    trk->nbBlocks++;

	odml_riff_break(len+8); // data size + fcc + size info (padding is handled in odml_riff_break)
	if(!odml_index_frame(1+index, len,false)){
		aprintf("\ncan not index audio frame stream:%u block %u size :%u\n",index,trk->nbBlocks-1,len);
	}
    char tag[5]="01wb";
    tag[1]='1'+index;
    return saveFrame (len, (uint32_t) 0, data, (uint8_t *)tag);
}
/**
    \fn saveFrame
*/
uint8_t aviWrite::saveFrame (uint32_t len, uint32_t flags,    uint8_t * data, uint8_t * fcc)
{
  uint32_t offset;
    // offset of this chunk compared to the beginning
    //  do not write idx1 in case of ODML
    //offset = LMovie->Tell () - 8 - LMovie->TellBegin ();
    if(doODML!=NORMAL)
    {
        offset = LMovie->Tell () - 8 - LMovie->TellBegin ();
    }

  LMovie->WriteChunk (fcc, len, data);
  // Now store the index part


    if(doODML!=NORMAL)
    {
      IdxEntry entry;

      entry.fcc = fourCC::get (fcc);
      entry.len = len;
      entry.flags = flags;
      entry.offset = offset;
      myindex.push_back(entry);
      curindex++;
    }

  return 1;
}

/**
    \fn setEnd
    \brief end movie writing, update headers & close file
*/
uint8_t aviWrite::setEnd (void)
{

  // First close the movie
  LMovie->End ();
  delete LMovie;
  LMovie = NULL;

    if(doODML!=NORMAL)
    {  // Regular index
            printf ("\n writing %u index parts", curindex);
            printf ("\n received %u video parts", vframe);

            // Updating compared to what has been really written
            //

            // Write index
            LAll->Write32 ("idx1");
            LAll->Write32 (curindex * 16);

            for (uint32_t i = 0; i < curindex; i++)
                {
                LAll->Write32 (myindex[i].fcc);
                LAll->Write32 (myindex[i].flags);
                LAll->Write32 (myindex[i].offset);	// abs position
                LAll->Write32 (myindex[i].len);
                }
            //  GMV: do not write idx1 in case of ODML
    }
  // Close movie
#ifndef MOVINDEX
  LAll->End ();
  delete    LAll;
  LAll = NULL;
#endif


 printf ("\n Updating headers...\n");

//  ODML header and index
	if(doODML==NORMAL)
        {
		odml_write_sindex(0, "00dc");	// video super index
		if(odml_nbrof_streams>1)odml_write_sindex(1,"01wb");	// audio super index
		if(odml_nbrof_streams>2)odml_write_sindex(2,"02wb");	// audio super index (dual)
		// odml header
		_file->seek(odml_header_fpos);
		AviList* LHeader =  new AviList("LIST", _file);
		LHeader->Begin("odml");
		LHeader->Write32("dmlh");
		LHeader->Write32((uint32_t)4);	// chunk size
		LHeader->Write32(vframe);	// total number of frames
		LHeader->End();
		delete LHeader;
		// indexes
		if(!odml_write_index(0, "00dc", "ix00")){	// video indexes
			aprintf("error writing video indexes");
		}
        char tag1[5];strcpy(tag1,"00dc");
        char tag2[5];strcpy(tag2,"ix01");
        for(int i=0;i<odml_nbrof_streams;i++)
        {
            switch(i)
            {
                case 0:break;
                case 1:strcpy(tag1,"01wb");break;
                default:tag1[1]='1'+i;break;
            }

            tag2[3]='1'+i;
			if(!odml_write_index(1, tag1, tag2))
            {	// audio indexes
				aprintf("error writing audio indexes");
			}
        }
	}
	odml_destroy_index();
#ifdef MOVINDEX
  LAll->End ();
  delete    LAll;
  LAll = NULL;
#endif

// GMV: set number or frames in first riff
  //_mainheader.dwTotalFrames = vframe;
	if(doODML==NORMAL)
		_mainheader.dwTotalFrames=odml_frames_inAVI;
	else
  _mainheader.dwTotalFrames = vframe;


  _videostream.dwLength = vframe;


// Update Header
  updateHeader (&_mainheader, &_videostream);


	printf("\n End of movie, \n video frames : %u\n",vframe);
    for(int i=0;i<nb_audio;i++)
    {
        printf("Track %d Size :%"LU" bytes, %"LU" blocks\n",i,audioTracks[i].sizeInBytes,audioTracks[i].nbBlocks);
    }

  // need to update headers now
  // AUDIO SIZE ->TODO
  delete _file;
  _file=NULL;

  qfclose (_out);
  _out = NULL;
  return 1;

}
/**
        \fn setStreamInfo

*/

uint8_t aviWrite::setStreamInfo (ADMFile * fo,
			 uint8_t * stream,
			 uint8_t * info, uint32_t infolen,
			 uint32_t odml_headerlen,
			 uint8_t odml_stream_nbr,
			 uint8_t * extra, uint32_t extraLen,
			 uint32_t maxxed)
{


  AviList * alist;
  uint8_t * junk;
  int32_t junklen;

  alist = new AviList ("LIST", fo);


  // 12 LIST
  // 8 strf subchunk
  // 8 strl subchunk
  // 8 defaultoffset
  alist->Begin ("strl");

  // sub chunk 1
  alist->WriteChunk ((uint8_t *) "strh", sizeof (AVIStreamHeader),
		     (uint8_t *) stream);

  uint8_t *buf=new uint8_t[infolen+extraLen];

	memcpy(buf,info,infolen);
	if(extraLen)
		memcpy(infolen+buf,extra,extraLen);

  alist->WriteChunk ((uint8_t *) "strf", infolen+extraLen, buf);

    // compute junkLen, it might also hold the oldml superindex
    uint32_t consumed=odml_headerlen*4;
  junklen = (consumed+maxxed-1)/maxxed;
  junklen=junklen*maxxed;

  printf("[ODML] using ODML placeholder of size %u bytes\n",junklen);
  junk = (uint8_t *) ADM_alloc (junklen);
  ADM_assert (junk);
  memset (junk,0, junklen);
  //
  // Fill junk with out info string
  uint32_t len=strlen("Avidemux");

  if(junklen>len)
  	memcpy(junk,"Avidemux",len);

  if(doODML!=NO)
  {
    odml_indexes[odml_stream_nbr].fpos=_file->tell();
    odml_indexes[odml_stream_nbr].pad=junklen;
  }
  alist->WriteChunk ((uint8_t *) "JUNK", junklen, junk);
  ADM_dealloc (junk);

  // MOD Feb 2005 by GMV: ODML header
// MOVEINDEX
#ifndef MOVINDEX
  //odml_write_dummy_chunk(alist, &odml_indexes[odml_stream_nbr].fpos, odml_headerlen);
#endif
  // END MOD Feb 2005 by GMV

  alist->End ();
  delete alist;
  delete[] buf;
  return 1;


}
/**
        \fn getPos
        \brief return position in file after the index (not written yet)
*/
uint32_t	aviWrite::getPos( void )
{
uint32_t pos;
	 // we take size of file + index
	 // with 32 bytes per index entry
	 //
	 ADM_assert(_file);
	 pos=_file->tell();
	 return pos+curindex*4*4;
}

/**
        \fn odml_destroy_index
*/
void aviWrite::odml_destroy_index(void){
	// destroy odml index data structure
	if(doODML!=NO){
		if(odml_indexes){
			for(int a=0;a<odml_nbrof_streams;++a){
				if(odml_indexes[a].odml_index){
					for(int b=0;b<odml_indexes[a].odml_nbrof_index;++b)
                                        {
						if(odml_indexes[a].odml_index[b].index)
							ADM_dealloc (odml_indexes[a].odml_index[b].index);
					}
					ADM_dealloc (odml_indexes[a].odml_index);
				}
			}
			ADM_dealloc (odml_indexes);
		}
		odml_indexes=NULL;
	}
}
/**
        \fn odml_write_dummy_chunk
*/
void aviWrite::odml_write_dummy_chunk(AviList* alist, uint64_t* fpos, uint32_t size){
	if(doODML!=NO){
		// save file position
		*fpos=alist->Tell();
		aprintf("\nwrite dummy chunk at file position %lu with data size %u\n",*fpos, size);
		// generate dummy data
		uint8_t* dummy=(uint8_t*)ADM_alloc (size);
		memset(dummy,0,size);
		// write dummy chunk
		alist->WriteChunk ((uint8_t *) "JUNK", size, dummy);
		// clean up
		ADM_dealloc (dummy);
	}
}
/**
        \fn reallocIndeces
*/
void aviWrite::reallocIndeces( odml_super_index_t *idx)
{
    uint32_t nw,old;
    odml_index_t   *newindex;
    odml_index_t *oldindex;
            old=idx->odml_nbrof_index;
            nw=old*2;
            printf("Increasing # of indeces from %d to %d\n",old,nw);
            oldindex=idx->odml_index;
            newindex=(odml_index_t *)ADM_alloc(sizeof(odml_index_t)*nw);
            memset(newindex,0,sizeof(odml_index_t)*nw);
            memcpy(newindex,oldindex,old*sizeof(odml_index_t));
            idx->odml_index=newindex;
            ADM_dealloc(oldindex);
            idx->odml_nbrof_index=nw;
            // Now fill in the new
            uint32_t lineSize=sizeof(odml_index_data_t) * odml_index_size;
             for(int b=old;b<nw;++b)
                        {	// for each index, alloc
                            newindex[b].index=(odml_index_data_t*) ADM_alloc (lineSize);
                            memset(newindex[b].index,0,lineSize);
                            newindex[b].nEntriesInUse=0;	// (redundant)
                        }

}


/**
        \fn odml_index_frame
*/
bool aviWrite::odml_index_frame(int stream_nbr, uint32_t data_size, bool keyFrame){
	if(doODML!=NO){
//		ADM_assert(!stream_nbr<odml_nbrof_streams);
		odml_super_index_t* sidx=odml_indexes+stream_nbr;	// access to super index
		if(sidx->odml_index[sidx->index_count].nEntriesInUse==odml_index_size)
                {	// new index needed?
			if(sidx->index_count>=sidx->odml_nbrof_index-1)	// can index counter be increased?
                                reallocIndeces(sidx);
                        ++(sidx->index_count);	// increment index counter

			// handle possible riff break
			odml_riff_break(data_size+8); // data size + fcc + size info (padding is handled in odml_riff_break)
			// write placeholder
			odml_write_dummy_chunk(LMovie, &(sidx->odml_index[sidx->index_count].fpos), 24+8*odml_index_size);
			sidx->odml_index[sidx->index_count].nEntriesInUse=0;
		}
		odml_index_t* idx=sidx->odml_index+(sidx->index_count);		// access to index
		odml_index_data_t* idxd=idx->index+(idx->nEntriesInUse);	// access to unused index data

		uint64_t pos=LMovie->Tell()+8;	// preview position of data
		idxd->fpos=pos;	// store file position of data

		if(keyFrame)
			idxd->size=data_size; //store data size
		else	// if no key frame
			idxd->size=data_size|0x80000000; //store data size with bit 31 set

		++(idx->nEntriesInUse);	// advance to next free index data entry
	}
	return true;
}
/**
        \fn odml_write_sindex
*/

void aviWrite::odml_write_sindex(int stream_nbr, const char* stream_fcc)
{

	// Warning: This changes the file position
	if(doODML==NORMAL)
                {
                    uint32_t startAt=odml_indexes[stream_nbr].fpos;
                    uint32_t pad=odml_indexes[stream_nbr].pad;
                    uint32_t endAt=startAt+pad+8;
#ifndef MOVINDEX
		_file->seek(startAt);
#endif
		aprintf("\nwriting super index at file pos %lu, total available size %u\n",odml_indexes[stream_nbr].fpos,pad);
		AviList* LIndex =  new AviList("JUNK", _file);	// abused writing aid (don't call Begin or End; the fcc is unused until 'Begin')
                uint32_t nbEntries=odml_indexes[stream_nbr].index_count+1;
		LIndex->Write32("indx");			// 4cc
		LIndex->Write32(24+nbEntries*16);	// size
		LIndex->Write16(4);				// wLongsPerEntry
		LIndex->Write8(0);				// bIndexSubType
		LIndex->Write8(0);				// bIndexType (AVI_INDEX_OF_INDEXES)
		LIndex->Write32(nbEntries);// nEntriesInUse
		LIndex->Write32(stream_fcc);			// dwChunkId;
		LIndex->Write32((uint32_t)0);
                LIndex->Write32((uint32_t)0);
                LIndex->Write32((uint32_t)0);// reserved
		for(uint32_t a=0;a<nbEntries;++a)
                {	// for each chunk index
                        uint64_t pos;
                        pos=odml_indexes[stream_nbr].odml_index[a].fpos;
                        LIndex->Write64(pos);	//absolute file position
                        LIndex->Write32(32 + 8 * odml_index_size);	// complete index chunk size
                        LIndex->Write32(odml_indexes[stream_nbr].odml_index[a].nEntriesInUse);	// duration
                        aprintf("\nstream %u, index %u Position: %lu  EntriesInUse:%u\n",stream_nbr, a ,pos,
                        odml_indexes[stream_nbr].odml_index[a].nEntriesInUse);
		}
                uint32_t at=LIndex->Tell();

                int32_t junkLen=endAt-at-8;
                ADM_assert(junkLen>=9);
                printf("Padding ODML index with junk of size %d, total padding %u\n",junkLen, odml_indexes[stream_nbr].pad);
		delete LIndex;
// Now create out junk chunk if needed, to padd the odml
                AviList *Junk=new AviList("JUNK",_file);
                uint8_t *zz=new uint8_t[junkLen];
                if(junkLen>9) strcpy((char *)zz,"Avidemux");
                Junk->WriteChunk ((uint8_t *)"JUNK", junkLen, zz);
                delete [] zz;
                ADM_assert(endAt==Junk->Tell());
                delete Junk;


	}
}
/**
        \fn odml_write_index
*/

bool aviWrite::odml_write_index(int stream_nbr, const char* stream_fcc, const char* index_fcc){	// write index
	// Warning: This changes the file position
	if(doODML==NORMAL){
		aprintf ("\n writing %d interleaved ODML indexes for %u frames in stream %s", odml_indexes[stream_nbr].index_count+1, vframe, stream_fcc);
		AviList* LIndex =  new AviList("JUNK", _file);	// abused writing aid (don't call Begin or End; the fcc is unused until 'Begin')
		for(int a=0;a<=odml_indexes[stream_nbr].index_count;++a){	// for each index
			odml_index_t* idx=odml_indexes[stream_nbr].odml_index+a;		// access to index
			_file->seek(idx->fpos);					// shift file pointer
			LIndex->Write32(index_fcc);			// 4cc
			LIndex->Write32(24+odml_index_size*8);		// data size
			LIndex->Write16(2);				// wLongsPerEntry
			LIndex->Write8(0);				// bIndexSubType
			LIndex->Write8(1);				// bIndexType (AVI_INDEX_OF_CHUNKS)
			LIndex->Write32(idx->nEntriesInUse);		// nEntriesInUse
			LIndex->Write32(stream_fcc);			// dwChunkId;
			uint64_t base_off=idx->index[0].fpos-8;		// lets take the position of the first frame in the index as base
			uint64_t rel_pos;
			LIndex->Write64(base_off);			// qwBaseOffset
			LIndex->Write32((uint32_t)0);			// reserved
			for(int b=0;b<idx->nEntriesInUse;++b){		// for each frame in the current index
				odml_index_data_t* idxd=idx->index+b;	// access to index data
				rel_pos=idxd->fpos-base_off;	// get relative file position
				if(rel_pos>(uint64_t)4*1024*1024*1024){	// index chunks have a maximum offset of 4GB
					printf("\nData rate too high for index size. Decrease index duration.\n"); // decrease the multiplicator in saveBegin that calculates odml_index_size
					printf("base:%lu abs:%lu rel:%lu stream:%d index:%d entry:%d",base_off,idxd->fpos,rel_pos,stream_nbr,a,b);
					delete LIndex;
					return false;
				}
				LIndex->Write32(rel_pos);	// relative file position
				LIndex->Write32(idxd->size);		// data size
			}
		}
		delete LIndex;
	}
	return true;
}
/**
        \fn odml_riff_break
*/
void aviWrite::odml_riff_break(uint32_t len){	// advance to the next riff if required
	if(doODML!=NO){
		// get padded size
		uint64_t len2=len;
		if(len & 1)++len2;
		// preview file position
		len2+=LMovie->Tell();
		// will we get over the next GB border?
		if( len2>((uint64_t)1024*1024*1024*(odml_riff_count+1)) ){
			if(doODML==HIDDEN){
				aprintf("\nstarting new (hidden) RIFF at %lu\n",LMovie->Tell());
				if(odml_riff_count<4)	// we have only 4 buffers but this has to be enough
					odml_write_dummy_chunk(LMovie, odml_riff_fpos+odml_riff_count, 16);	// write dummy
				if(odml_riff_count==0) odml_frames_inAVI=vframe-1;	// rescue number of frames in first AVI (-1 since there may be no audio for the last video frame)
			}else{	// restart riff and movie
				aprintf("\nstarting new RIFF at %lu\n",LMovie->Tell());
				// restart lists
				LMovie->End();
				LAll->End();
				LAll->Begin ("AVIX");
				LMovie->Begin ("movi");
			}
			++odml_riff_count;
		}
		// ODML required for movie?
		if(doODML==HIDDEN){
			if( ((uint64_t)getPos()+len+17) >= ((uint64_t)4*1024*1024*1024) ){	//if (written data + new chunk + index (old type) for new chunk + possible padding) does not fit into 4GB
				printf("\nswitching to ODML mode at %lu\n",LMovie->Tell());
				uint64_t last_pos=LMovie->Tell();	// rescue current file position
				// close First RIFF
				for(int a=0;a<4;++a){	// for each hidden riff
					if(odml_riff_fpos[a]!=0){
						_file->seek(odml_riff_fpos[a]);	// set file pointer to start of next riff
						LMovie->End();
						LAll->End();
						LAll->Begin("AVIX");
						LMovie->Begin("movi");
					}
				}
				// goto end of file
				_file->seek(last_pos);
				// following riffs can start directly
				doODML=NORMAL;	// write RIFF breaks directly
			}
		}
	}
}
/**
    \fn mx_bihFromVideo
    \brief build a bih from video
*/
void mx_bihFromVideo(ADM_BITMAPINFOHEADER *bih,ADM_videoStream *video)
{
        //
         bih->biSize=sizeof(ADM_BITMAPINFOHEADER); //uint32_t 	biSize;
         bih->biWidth=video->getWidth(); //uint32_t  	biWidth;
         bih->biHeight=video->getHeight(); //uint32_t  	biHeight;
         bih->biPlanes=1; //    uint16_t 	biPlanes;
         bih->biBitCount=24; //
         bih->biCompression=video->getFCC(); //    uint32_t 	biCompression;
         bih->biSizeImage=(bih->biWidth*bih->biHeight*3)>>1;//    uint32_t 	biSizeImage;
         bih->biXPelsPerMeter=0;
         bih->biYPelsPerMeter=0;
         bih->biClrUsed=0;
         bih->biClrImportant=0;
        // Recompute image size
        uint32_t is;
            is=bih->biWidth*bih->biHeight;
            is*=(bih->biBitCount+7)/8;
            bih->biSizeImage=is;
}
/**
        \fn mx_mainHeaderFromVideoStream
        \brief Write MainAVIHeader from video
*/
void mx_mainHeaderFromVideoStream(MainAVIHeader  *header,ADM_videoStream *video)
{
    header->dwMicroSecPerFrame= ADM_UsecFromFps1000(video->getAvgFps1000()); //int32_t	dwMicroSecPerFrame;	// frame display rate (or 0L)
    header->dwMaxBytesPerSec=8*1000*1000; //int32_t	dwMaxBytesPerSec;	// max. transfer rate
    header->dwPaddingGranularity=0; //int32_t	dwPaddingGranularity;	// pad to multiples of this
					// size; normally 2K.
    header->dwFlags=0; // FIXME HAS INDEX //int32_t	dwFlags;		// the ever-present flags
    //header->dwTotalFrames=0; //int32_t	dwTotalFrames;		// # frames in file
    header->dwInitialFrames=0; //int32_t	dwInitialFrames;
   // Must be set by caller  header->dwStreams=int32_t	dwStreams;
    header->dwSuggestedBufferSize=64*1024;// int32_t	dwSuggestedBufferSize;

    header->dwWidth=video->getWidth();//int32_t	dwWidth;
    header->dwHeight=video->getHeight();//int32_t	dwHeight;
}
/**
    \fn mx_streamHeaderFromVideo
    \fill in AVIStreamHeader from video. Only for video stream header of course.

*/
static void mx_streamHeaderFromVideo(AVIStreamHeader *header,ADM_videoStream *video)
{
	header->fccType=fourCC::get((uint8_t *)"vids");  //uint32_t	fccType;
	header->fccType=video->getFCC(); //uint32_t	fccHandler;
	header->dwFlags=0; //int32_t	dwFlags;	/* Contains AVITF_* flags */
	header->wPriority=0; //int16_t	wPriority;	/* dwPriority - splited for audio */
	header->wLanguage=0; //int16_t	wLanguage;
	header->dwInitialFrames=0;//int32_t	dwInitialFrames;
	header->dwScale=1000;//  int32_t	dwScale;
	header->dwRate=video->getAvgFps1000();// int32_t	dwRate;		/* dwRate / dwScale == samples/second */
	header->dwStart=0;// int32_t	dwStart;
	header->dwLength=0; // int32_t	dwLength;	/* In units above... */
	header->dwSuggestedBufferSize=1000000;// int32_t	dwSuggestedBufferSize;
	header->dwQuality=0;// int32_t	dwQuality;
	header->dwSampleSize=0;// int32_t	dwSampleSize;
/*
	struct {
		int16_t left;
		int16_t top;
		int16_t right;
		int16_t bottom;
	} rcFrame;
*/
}

// EOF
