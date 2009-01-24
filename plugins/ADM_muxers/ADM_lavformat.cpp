//
// C++ Implementation: ADM_lavformat
//
// Description:
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
#include <math.h>
#include <string.h>

extern "C"
{
	#include "ADM_libraries/ADM_ffmpeg/ADM_lavformat/avformat.h"
};

#undef malloc
#undef realloc
#undef free

#include "ADM_default.h"
#include "DIA_coreToolkit.h"

#include "ADM_audiofilter/audioprocess.hxx"
#include "ADM_audio/ADM_a52info.h"

#include "ADM_codecs/ADM_codec.h"
#include "ADM_editor/ADM_Video.h"

#include "ADM_lavformat.h"
#include "fourcc.h"

#include "prefs.h"
#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_LAVFORMAT
#include "ADM_osSupport/ADM_debug.h"

#include "ADM_cpuCap.h"
static    AVOutputFormat *fmt;
static    AVFormatContext *oc;
static    AVStream *audio_st, *video_st;
static    double audio_pts, video_pts;

static uint32_t one;
uint64_t  _curDTS;

static uint8_t ADM_4cc_to_lavcodec(const char *fcc, CodecID *outlavid);
static uint8_t ADM_WaveTag_to_lavcodec(uint32_t tag, CodecID *outlavid);

extern uint8_t isVP6Compatible (uint32_t fourcc);

// convert in in us to out in 90Khz tick
int64_t adm_90k( double in)
{
int64_t out;
	// in=in/1000*1000 us->s
	// in/=(1/90000) nb 90 khz tick
	// in*=90*000/000*000
	// in =  in*=90/000
	//in=in*90./1000.;
	out= (int64_t)in;
	return out;
}
//___________________________________________________________________________
lavMuxer::lavMuxer( void )
{
	fmt=NULL;
	oc=NULL;
	audio_st=NULL;
	video_st=NULL;
	_fps1000=0;
	_audioByterate=0;
	_lastAudioDts=0;
	_frameNo=0;
	_running=0;
	_curDTS=0;
        _audioFq=0;
}
//___________________________________________________________________________
lavMuxer::~lavMuxer()
{
	close();
}
//___________________________________________________________________________
uint8_t lavMuxer::open(const char *filename, uint32_t inbitrate,ADM_MUXER_TYPE type, aviInfo *info, WAVHeader *audioheader)
{
        return open(filename,inbitrate,type,info,0,NULL,audioheader,0,NULL);
}


uint8_t lavMuxer::open(const char *filename,uint32_t inbitrate, ADM_MUXER_TYPE type, aviInfo *info,
              uint32_t videoExtraDataSize, uint8_t *videoExtraData, WAVHeader *audioheader,
              uint32_t audioextraSize,uint8_t *audioextraData)
{
 AVCodecContext *c;
 	_type=type;
	_fps1000=info->fps1000;
	switch(_type)
	{
	case MUXER_TS:
		fmt=guess_format("mpegts", NULL, NULL);
		break;
	case MUXER_DVD:
		fmt = guess_format("dvd", NULL, NULL);
		break;
	case MUXER_VCD:
		fmt = guess_format("vcd", NULL, NULL);
		break;
	case MUXER_SVCD:
		fmt = guess_format("svcd", NULL, NULL);
		break;
	case MUXER_MP4:
		fmt = guess_format("mp4", NULL, NULL);
		break;
	case MUXER_PSP:
		fmt = guess_format("psp", NULL, NULL);
		break;
	case MUXER_FLV:
		fmt = guess_format("flv", NULL, NULL);
		break;
	case MUXER_MATROSKA:
		fmt = guess_format("matroska", NULL, NULL);
		break;

	default:
		fmt=NULL;
	}
	if (!fmt)
	{
        	printf("Lav:Cannot guess format\n");
                ADM_assert(0);
		return 0;
	}
	oc = av_alloc_format_context();
	if (!oc)
	{
       		printf("Lav:Cannot allocate context\n");
		return 0;
	}
	oc->oformat = fmt;
	snprintf(oc->filename,1000,"file://%s",filename);
	// Video
	//________

	video_st = av_new_stream(oc, 0);
	if (!video_st)
	{
		printf("Lav: new stream failed\n");
		return 0;
	}

	c = video_st->codec;
	switch(_type)
	{
				case MUXER_FLV:
					 c->codec=new AVCodec;
					 memset(c->codec,0,sizeof(AVCodec));
					 if(fourCC::check(info->fcc,(uint8_t *)"FLV1"))
					 {
						 c->codec_id=CODEC_ID_FLV1;
					 	 c->codec->name=ADM_strdup("FLV1");
					 }else
					 {
						 if(isVP6Compatible(info->fcc))
						 			{
							 		 c->codec_id=CODEC_ID_VP6F;
					 				 c->codec->name=ADM_strdup("VP6F");
						 			}
						 else
							 ADM_assert(0);

					 }

					 break;
                case MUXER_MATROSKA:
                        strcpy(oc->title,"Avidemux");
                        strcpy(oc->author,"Avidemux");
                        c->sample_aspect_ratio.num=1;
                        c->sample_aspect_ratio.den=1;
                        if(isMpeg4Compatible(info->fcc))
                        {
                                c->codec_id = CODEC_ID_MPEG4;
                                c->has_b_frames=1; // in doubt...
                        }else
                        {
                                if(isH264Compatible(info->fcc))
                                {
                                        c->has_b_frames=1; // in doubt...
                                        c->codec_id = CODEC_ID_H264;
                                        c->codec=new AVCodec;
                                        memset(c->codec,0,sizeof(AVCodec));
                                        c->codec->name=ADM_strdup("H264");
                                }
                                else
                                {
                                   if(!ADM_4cc_to_lavcodec((const char *)&(info->fcc),&(c->codec_id)))
                                   {
                                      printf("[lavFormat] Cannot map  this\n");
                                      return 0;
                                   }

                                }
                        }
                        if(videoExtraDataSize)
                        {
                                c->extradata=videoExtraData;
                                c->extradata_size= videoExtraDataSize;
                        }
                        break;
                case MUXER_MP4:
                case MUXER_PSP:
                {
                        // probably a memeleak here
                        char *foo=ADM_strdup(filename);
                        
                        strcpy(oc->title,ADM_GetFileName(foo));
                        strcpy(oc->author,"Avidemux");
                        c->sample_aspect_ratio.num=1;
                        c->sample_aspect_ratio.den=1;
                        if(isMpeg4Compatible(info->fcc))
                        {
                                c->codec_id = CODEC_ID_MPEG4;
                                c->has_b_frames=1; // in doubt...
                        }else
                        {
                                if(isH264Compatible(info->fcc))
                                {
                                        c->has_b_frames=1; // in doubt...
                                        c->codec_id = CODEC_ID_H264;
                                        c->codec=new AVCodec;
                                        memset(c->codec,0,sizeof(AVCodec));
                                        c->codec->name=ADM_strdup("H264");
                                }
                                else
                                {
                                        if(isDVCompatible(info->fcc))
                                        {
                                          c->codec_id = CODEC_ID_DVVIDEO;
                                        }else
                                        {
                                          if(fourCC::check(info->fcc,(uint8_t *)"H263"))
                                          {
                                                    c->codec_id=CODEC_ID_H263;
                                            }else{
                                                    c->codec_id = CODEC_ID_MPEG4; // Default value
                                                    printf("Ooops, cant mux that...\n");
                                                    printf("Ooops, cant mux that...\n");
                                                    printf("Ooops, cant mux that...\n");
                                                }
                                        }
                                }
                        }
                        if(videoExtraDataSize)
                        {
                                c->extradata=videoExtraData;
                                c->extradata_size= videoExtraDataSize;
                        }
                        if(MUXER_PSP==_type)
                        {
                            c->rc_buffer_size=0; //8*1024*224;
                            c->rc_max_rate=0; //768*1000;
                            c->rc_min_rate=0;
                            c->bit_rate=768*1000;
                        }
                        else
                        {
                            c->rc_buffer_size=8*1024*224;
                            c->rc_max_rate=9500*1000;
                            c->rc_min_rate=0;
                            if(!inbitrate)
                                    c->bit_rate=9000*1000;
                            else
                                    c->bit_rate=inbitrate;
                        }
                }
                        break;
                case MUXER_TS:
                        c->codec_id = CODEC_ID_MPEG2VIDEO;
                        c->rc_buffer_size=8*1024*224;
                        c->rc_max_rate=9500*1000;
                        c->rc_min_rate=0;
                        if(!inbitrate)
                                c->bit_rate=9000*1000;
                        else
                                c->bit_rate=inbitrate;

                        break;
		case MUXER_DVD:
			c->codec_id = CODEC_ID_MPEG2VIDEO;
			c->rc_buffer_size=8*1024*224;
			c->rc_max_rate=9500*1000;
			c->rc_min_rate=0;
			if(!inbitrate)
				c->bit_rate=9000*1000;
			else
				c->bit_rate=inbitrate;

			break;
		case MUXER_VCD:
			c->codec_id = CODEC_ID_MPEG1VIDEO;

			c->rc_buffer_size=8*1024*40;
			c->rc_max_rate=1152*1000;
			c->rc_min_rate=1152*1000;

			c->bit_rate=1152*1000;


			break;
		case MUXER_SVCD:
			c->codec_id = CODEC_ID_MPEG2VIDEO;

			c->rc_buffer_size=8*1024*112;
			c->rc_max_rate=2500*1000;
			c->rc_min_rate=0*1000;
			if(!inbitrate)
				c->bit_rate=2040*1000;
			else
				c->bit_rate=inbitrate;

			break;
		default:
			ADM_assert(0);
	}

	c->codec_type = CODEC_TYPE_VIDEO;
	c->flags=CODEC_FLAG_QSCALE;
	c->width = info->width;
	c->height = info->height;

       AVRational fps25=(AVRational){1001,25025};
       AVRational fps24=(AVRational){1001,24000};
       AVRational fps30= (AVRational){1001,30000};
       AVRational fpsfree= (AVRational){1000,_fps1000};


    	switch(_fps1000)
	{
		case 25000:
                {
			 c->time_base= fps25;
			 break;
                }
		case 23976:
                        if(_type==MUXER_MP4 || _type==MUXER_PSP || _type==MUXER_FLV || _type==MUXER_MATROSKA)
                        {
                                 c->time_base= fps24; //(AVRational){1001,24000};
                                break;
                        }
		case  29970:
			 c->time_base=fps30;
			break;
		default:
                      {
                            if(_type==MUXER_MP4 || _type==MUXER_PSP || _type==MUXER_FLV || _type==MUXER_MATROSKA)
                            {
                                    c->time_base=fpsfree;// (AVRational){1000,_fps1000};
                                    break;
                            }
                            else
                            {
                                GUI_Error_HIG(QT_TR_NOOP("Incompatible frame rate"), NULL);
                                return 0;
                            }
                            }
                        break;
	}


	c->gop_size=15;
	c->max_b_frames=2;
	c->has_b_frames=1;


	// Audio
	//________
        if(audioheader)
        {
          audio_st = av_new_stream(oc, 1);
          if (!audio_st)
          {
                  printf("Lav: new stream failed\n");
                  return 0;
          }


          c = audio_st->codec;
          c->frame_size=1024; //For AAC mainly, sample per frame
          printf("[LavFormat] Bitrate %u\n",(audioheader->byterate*8)/1000);
          _audioFq=c->sample_rate = audioheader->frequency;
#if 0
           if(_type== MUXER_PSP && audioheader->encoding==WAV_AAC)
            {
                    _audioFq=c->sample_rate = audioheader->frequency/2;                 //_audioFq*=2; // SBR
             }
#endif

          switch(audioheader->encoding)
          {
                  case WAV_AC3: c->codec_id = CODEC_ID_AC3;c->frame_size=6*256;break;
                  case WAV_MP2: c->codec_id = CODEC_ID_MP2;break;
                  case WAV_MP3:
  #warning FIXME : Probe deeper
                              c->frame_size=1152;
                              c->codec_id = CODEC_ID_MP3;
                              break;
                  case WAV_PCM:
                                  // One chunk is 10 ms (1/100 of fq)
                                  c->frame_size=4;
                                  c->codec_id = CODEC_ID_PCM_S16LE;break;
                  case WAV_AAC:
                                  c->extradata=audioextraData;
                                  c->extradata_size= audioextraSize;
                                  c->codec_id = CODEC_ID_AAC;
                                  break;
                  default:
                          if(_type==MUXER_MATROSKA)
                          {
                           if(ADM_WaveTag_to_lavcodec(audioheader->encoding, &(c->codec_id)))
                           {
                             if(audioextraData)
                             {
                                  c->extradata=audioextraData;
                                  c->extradata_size= audioextraSize;
                             }
                             // Put a dummy time increment
                              c->time_base= fps25;
                             break;
                           }
                          }

                          printf("Cant mux that ! audio\n");
                          c->codec_id = CODEC_ID_MP2;
                          return 0;
                          break;
          }
          c->codec_type = CODEC_TYPE_AUDIO;

          c->bit_rate = audioheader->byterate*8;
          c->rc_buffer_size=(c->bit_rate/(2*8)); // 500 ms worth

          c->channels = audioheader->channels;
          _audioByterate=audioheader->byterate;

        }
        // /audio


//----------------------
	switch(_type)
	{
				case MUXER_FLV:
                case MUXER_PSP:
                case MUXER_MP4:
                case MUXER_MATROSKA:
                        oc->mux_rate=10080*1000; // Needed ?
                        break;

                case MUXER_TS:
                        oc->mux_rate=10080*1000;
                        break;
		case MUXER_DVD:
			oc->packet_size=2048;
			oc->mux_rate=10080*1000;
			break;
		case MUXER_VCD:
			oc->packet_size=2324;
			oc->mux_rate=2352 * 75 * 8;

			break;
		case MUXER_SVCD:

			oc->packet_size=2324;
			oc->mux_rate=2*2352 * 75 * 8; // ?

			break;
		default:
			ADM_assert(0);
	}
	oc->preload=AV_TIME_BASE/10; // 100 ms preloading
	oc->max_delay=200*1000; // 500 ms

	if (av_set_parameters(oc, NULL) < 0)
	{
		printf("Lav: set param failed \n");
		return 0;
	}
	 if (url_fopen(&(oc->pb), filename, URL_WRONLY) < 0)
	 {
	 	printf("Lav: Failed to open file :%s\n",filename);
		return 0;
        }

	ADM_assert(av_write_header(oc)>=0);
	dump_format(oc, 0, filename, 1);


	printf("lavformat mpeg muxer initialized\n");

	_running=1;

	one=(1000*1000*1000)/_fps1000;
	_curDTS=one;

	return 1;
}
//___________________________________________________________________________
uint8_t lavMuxer::writeAudioPacket(uint32_t len, uint8_t *buf,uint32_t sample)
{

        int ret;
        AVPacket pkt;
        double f;
        int64_t timeInUs;
        static uint64_t sz = 0;
            //printf("Audio paclet : size %u, sample %u\n",len,sample);

           if(!audio_st) return 0;
           if(!len) return 1;
            av_init_packet(&pkt);
            timeInUs=(int64_t)sample2time_us(sample);
            aprintf("Sample: %u, time: %"LLU", size: %"LLU", this round: %u\n",sample, timeInUs, sz, len);
            sz+=len;
            /* Rescale to ?? */
            if(_type==MUXER_FLV || _type==MUXER_MATROSKA) /* The FLV muxer expects packets dated in ms, there is something i did not get... WTF */
            {
            			f=timeInUs/1000; // ms
            			f=floor(f+0.4);
            }
            else
            {
            	f=timeInUs;
            	f/=1000000.; // In sec
            	f*=_audioFq; // In samples
            	f=floor(f+0.4);
            }
            pkt.dts=pkt.pts=f;
            aprintf("Adm audio dts: %"LLU"\n",pkt.dts);
            //printf("F:%f Q:%u D=%u\n",f,pkt.pts,timeInUs-_lastAudioDts);

            pkt.flags |= PKT_FLAG_KEY;
            pkt.data= buf;
            pkt.size= len;
            pkt.stream_index=1;
            //pkt.duration=pkt.dts-_lastAudioDts; // Duration
            aprintf("A: sample: %d, frame_pts: %"LLU", fq: %d\n", sample, pkt.dts, audio_st->codec->sample_rate);

            ret = av_write_frame(oc, &pkt);
            _lastAudioDts=timeInUs;
            if(ret)
            {
                        printf("[LavFormat]Error writing audio packet\n");
                        printf("[LavFormat]pts %llu dts %llu\n",pkt.pts,pkt.dts);
                        return 0;
            }
            return 1;
}
//________________________________________________________________________
uint64_t  lavMuxer::sample2time_us( uint32_t sample )
{
double f;

                f=sample;
                f*=1000.*1000.;
                f/=_audioFq;              // Sample / Frequency = time in seconds *10E6 to get in in us

                return (uint64_t)floor(f);

}
//___________________________________________________________________________
uint8_t lavMuxer::needAudio( void )
{

        if(!audio_st) return 0;

	double f;
	uint64_t dts=_lastAudioDts;  // Last audio dts


		aprintf("Need audio  ?: %llu / %llu : %llu\n ",dts,_curDTS,_curDTS+one);
		if((dts+5000>=_curDTS) && (dts<=_curDTS+one)) return 1;
		if(dts<=_curDTS)
		{
			printf("LavMuxer:Audio DTS is too low %llu / %llu!\n",dts,_curDTS);
			return 1;
		}
		return 0;
}
//___________________________________________________________________________
uint8_t lavMuxer::writeVideoPacket(ADMBitstream *bitstream)
{
int ret;

double p,d;
  	AVPacket pkt;
            av_init_packet(&pkt);

        p=bitstream->ptsFrame+1;      // Pts           // Time p/fps1000=out/den  out=p*den*1000/fps1000
        p=(p*1000*1000*1000);
        p=p/_fps1000;                  // in us

        d=bitstream->dtsFrame;		// dts
	d=(d*1000*1000*1000);
	d=d/_fps1000;


	_curDTS=(int64_t)floor(d);
    aprintf("Adm video unscaled dts=:%u\n",(uint32_t)d);
        // Rescale
#define RESCALE(x) x=x*1000.;\
                   x=x/_fps1000;

        p=bitstream->ptsFrame+1;
        RESCALE(p);
// MP4/ TS
        d=bitstream->dtsFrame;  // p & d are now in seconds
        RESCALE(d);
        switch(_type)  // video_st->codec->time_base.den
        {
        case MUXER_FLV :
        case MUXER_MATROSKA:
                    {
                        p=p*1000;
                        d=d*1000; // in milliseconds
                        break;
                    }
        
        default:
                    p=p*video_st->codec->time_base.den;
                    d=d*video_st->codec->time_base.den;
                    break;
        }
    	pkt.dts=(int64_t)floor(d);
    	pkt.pts=(int64_t)floor(p);

       // printf("Lavformat : Pts :%u dts:%u",displayframe,frameno);
	aprintf("Lavformat : Pts :%llu dts:%llu",pkt.pts,pkt.dts);
	pkt.stream_index=0;

        pkt.data= bitstream->data;
        pkt.size= bitstream->len;
	// Look if it is a gop start or seq start
        if(_type==MUXER_MP4 || _type==MUXER_PSP || _type==MUXER_FLV || _type==MUXER_MATROSKA)
        {
            if(bitstream->flags & AVI_KEY_FRAME)
                        pkt.flags |= PKT_FLAG_KEY;
        }else
            if(!bitstream->data[0] &&  !bitstream->data[1] && bitstream->data[2]==1)
	{
            if(bitstream->data[3]==0xb3 || bitstream->data[3]==0xb8 ) // Seq start or gop start
		pkt.flags |= PKT_FLAG_KEY;
		//printf("Intra\n");
	}
    //printf("Adm video dts=:%u\n",pkt.dts);
	ret =av_write_frame(oc, &pkt);
	if(ret)
	{
		printf("[LavFormat]Error writing video packet\n");
		return 0;
	}
        aprintf("V: frame %lu pts%d\n",bitstream->dtsFrame,pkt.pts);

	return 1;
}
//___________________________________________________________________________
uint8_t lavMuxer::forceRestamp(void)
{
	return 1;
}
//___________________________________________________________________________
uint8_t lavMuxer::close( void )
{
	if(_running)
	{
		_running=0;
		// Flush
		// Cause deadlock :
		av_write_trailer(oc);
		url_fclose((oc->pb));

	}
	if(audio_st)
	{
		 av_free(audio_st);
	}
	if(video_st)
	{
		 av_free(video_st);
	}
	video_st=NULL;
	audio_st=NULL;
	if(oc)
		av_free(oc);
	oc=NULL;
	return 1;
}
//___________________________________________________________________________
uint8_t lavMuxer::audioEmpty( void)
{
	return 0;
}
extern "C"
{
     extern  int        mpegps_init(void );
     extern  int        movenc_init(void );
     extern  int        flvenc_init(void );
     extern  int        matroskaenc_init(void );
};
extern URLProtocol file_protocol ;
uint8_t lavformat_init(void)
{
                movenc_init();
                flvenc_init();
                matroskaenc_init();
                register_protocol(&file_protocol);
}
extern "C"
{
/**
    \fn ADM_useAlternateTagging
    \brief returns 1 if using haali compatible tagging, 0 if normal. For mp3 in mp4ff.
*/
int ADM_useAlternateTagging(void)
{
  uint32_t v=1;
  prefs->get(FEATURE_ALTERNATE_MP3_TAG,&v);
  return v;
}
}

typedef struct
{
  const char *name;
  int    id;
}lavFCC_t;

lavFCC_t lavFCC[]=
{
  {"DIV3",CODEC_ID_MSMPEG4V3},
  {"FLV1",CODEC_ID_FLV1},
  {"MPEG",CODEC_ID_MPEG2VIDEO},
  {"DUMMY",0Xffff}
};
/**
    \fn ADM_4cc_to_lavcodec
    \brief Convert avi fourcc to lavcodec id (video)

*/
uint8_t ADM_4cc_to_lavcodec(const char *fcc, CodecID *outlavid)
{
    uint32_t nb=sizeof( lavFCC)/sizeof(lavFCC_t);
    nb--;
    for(int i=0;i<nb;i++)
    {
      if(!strncmp(lavFCC[i].name,fcc,4))
      {
        *outlavid=(CodecID)lavFCC[i].id;
        return 1;
      }

    }
    return 0;

}
//*****************
typedef struct
{
  uint32_t tag;
  int    id;
}lavWaveTag_t;

lavWaveTag_t lavWaveTag[]=
{
  {WAV_AC3,CODEC_ID_AC3},
  {WAV_OGG,CODEC_ID_VORBIS},
  {WAV_AAC,CODEC_ID_AAC},
  {0xdead,0Xffff} // DUMMY
};
/**
    \fn ADM_WaveTag_to_lavcodec
    \brief Convert ADM audio codec id to lavcodec id (video)

*/
uint8_t ADM_WaveTag_to_lavcodec(uint32_t tag, CodecID *outlavid)
{
    uint32_t nb=sizeof( lavWaveTag)/sizeof(lavWaveTag_t);
    nb--;
    for(int i=0;i<nb;i++)
    {
      if(lavWaveTag[i].tag==tag)
      {
        *outlavid=(CodecID)lavWaveTag[i].id;
        return 1;
      }

    }
    return 0;

}

//___________________________________________________________________________
//EOF


