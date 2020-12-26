/***************************************************************************
    copyright            : (C) 2006 by mean
    email                : fixounet@free.fr
 * 
 * Some AAC extradata code borrowed from mplayer
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
#include <math.h>

#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"
#include "ADM_codecType.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_mkv.h"

#include "mkv_tags.h"
#include "ADM_aacinfo.h"
class entryDesc
{
  public:
          uint32_t     trackNo;
          uint32_t     trackType;
          uint32_t     extraDataLen;

          uint32_t fcc;
          uint32_t w,h,fps;
          uint32_t fq,chan,bpp;
          uint32_t defaultDuration;
          float    trackScale;
          uint8_t *extraData;
          std::string codecId;
          std::string language;
        
          void dump(void);
          uint32_t   headerRepeatSize;
          uint8_t    headerRepeat[MKV_MAX_REPEAT_HEADER_SIZE];
          entryDesc()
          {
              codecId=language=std::string("");
              trackNo=0;
              trackType=0;
              extraDataLen=0;
              fcc=w=h=fps=fq=chan=bpp=defaultDuration=0;
              trackScale=0;
              extraData=NULL;
              headerRepeatSize=0;
          }

};
/* Prototypes */
static uint8_t entryWalk(ADM_ebml_file *head,uint32_t headlen,entryDesc *entry);
uint32_t ADM_mkvCodecToFourcc(const char *codec);



#define AAC_SYNC_EXTENSION_TYPE 0x02b7
static int aac_get_sample_rate_index(uint32_t sample_rate)
{
    static const int srates[] = {
        92017, 75132, 55426, 46009, 37566, 27713,
        23004, 18783, 13856, 11502, 9391, 0
    };
    int i = 0;
    while (sample_rate < srates[i])
        i++;
    ADM_info("Found index of %d for aac fq of %d\n",i,sample_rate);
    return i;
}


/**
 * 
 * @param haystack
 * @param needle
 * @return 
 */
static bool hasNeedle(const char *haystack, const char *needle)
{
    if(NULL!=strstr( (char *)(haystack+12),needle)) return true;
    return false;
}
/**
 * \brief recreate codec extra data as soon as possible to detect / deal with sbr
 * strongly derived from mplayer code
 * @param codec
 * @param entry
 */
static void createAACExtraData(const char *codec,entryDesc *entry)
{
    int profile=3;
    int sampleRateIndex=aac_get_sample_rate_index(entry->fq);
    if(hasNeedle(codec,"MAIN")) profile=0;
    else if(hasNeedle(codec,"LC")) profile=1;
    else if(hasNeedle(codec,"SSR")) profile=2;
    uint8_t *e=entry->extraData;
    e[0]=((profile + 1) << 3) | ((sampleRateIndex & 0xE) >> 1);
    e[1]= ((sampleRateIndex & 0x1) << 7) | (entry->chan << 3);
    entry->extraDataLen=2;
    if(hasNeedle(codec,"SBR"))
    {
        entry->extraDataLen=5;
        entry->fq*=2;
        sampleRateIndex=aac_get_sample_rate_index(entry->fq);
        e[2] = AAC_SYNC_EXTENSION_TYPE >> 3;
        e[3] = ((AAC_SYNC_EXTENSION_TYPE & 0x07) << 5) | 5;
        e[4] = (1 << 7) | (sampleRateIndex << 3);
    }
    ADM_info("Created %d bytes\n",entry->extraDataLen);
    mixDump(entry->extraData,entry->extraDataLen);
}

/**
    \fn entryDesc::dump
    \brief Dump the track entry
*/
void entryDesc::dump(void)
{
      printf("*** TRACK SUMMARY **\n");
#define PRINT(x) printf(#x" :%u\n",x)
      PRINT(trackNo);
      switch(trackType)
      {
        case 1: // Video
          PRINT(trackType);
          printf("==>Video\n");
          PRINT(extraDataLen);
          PRINT(fcc);
          printf("%s\n",fourCC::tostring(fcc));
          PRINT(w);
          PRINT(h);
          PRINT(fps);
          break;
        case 2: // Video
          printf("==>Audio\n");
          PRINT(extraDataLen);
          PRINT(fcc);
          PRINT(fq);
          PRINT(chan);
          PRINT(bpp);
          break;
        default:
          printf("Unkown track type (%d)\n",trackType);
      }
}

/**
      \fn analyzeOneTrack
      \brief Grab info about the track (it is a recursive function !)

*/
uint8_t mkvHeader::analyzeOneTrack(void *head,uint32_t headlen)
{
    entryDesc entry;

    /* Set some defaults value */
    entry.chan=1;

    entryWalk(  (ADM_ebml_file *)head,headlen,&entry);
    entry.dump();

    //***************** First video track *****************
    if(entry.trackType==1 &&  !_isvideopresent)
    {
        _isvideopresent=1;
        if(entry.defaultDuration)
        {
            _tracks[0]._defaultFrameDuration=entry.defaultDuration;
            double inv=entry.defaultDuration; // in us
            inv=1/inv;
            inv*=1000.;
            inv*=1000.;
            inv*=1000.;
            _videostream.dwScale=1000;
            _videostream.dwRate=(uint32_t)inv;
        }else
        {
            printf("[MKV] No duration, assuming 25 fps\n");
            _videostream.dwScale=1000;
            _videostream.dwRate=25000;
            _tracks[0]._defaultFrameDuration=40000;
        }

        _mainaviheader.dwMicroSecPerFrame=_tracks[0]._defaultFrameDuration;
        _videostream.fccType=fourCC::get((uint8_t *)"vids");
        _video_bih.biBitCount=24;
        _videostream.dwInitialFrames= 0;
        _videostream.dwStart= 0;
        _video_bih.biWidth=_mainaviheader.dwWidth=entry.w;
        _video_bih.biHeight=_mainaviheader.dwHeight=entry.h;
        _videostream.fccHandler=_video_bih.biCompression=entry.fcc;

#define AV1_EXTRADATA_OFFSET 4

        // if it is vfw...
        if(fourCC::check(entry.fcc,(uint8_t *)"VFWX") && entry.extraData && entry.extraDataLen>=sizeof(ADM_BITMAPINFOHEADER))
        {
            ADM_info("VFW compatibility header, data=%d bytes\n",(int)entry.extraDataLen);
            memcpy(& _video_bih,entry.extraData,sizeof(ADM_BITMAPINFOHEADER));

            _videostream.fccHandler=_video_bih.biCompression;
            _mainaviheader.dwWidth= _video_bih.biWidth;
            _mainaviheader.dwHeight=_video_bih.biHeight;
            if(entry.extraDataLen>sizeof(ADM_BITMAPINFOHEADER))
            {
                int l=entry.extraDataLen-sizeof(ADM_BITMAPINFOHEADER);
                _tracks[0].extraData=new uint8_t[l];
                _tracks[0].extraDataLen=l;
                memcpy(_tracks[0].extraData,entry.extraData +sizeof(ADM_BITMAPINFOHEADER),l);
                ADM_info("VFW Header+%d bytes of extradata\n",l);   
                mixDump(_tracks[0].extraData,l);
            }
            delete [] entry.extraData;
            entry.extraData=NULL;
            entry.extraDataLen=0;
        }else if(fourCC::check(entry.fcc,(uint8_t *)"av01") && entry.extraData && entry.extraDataLen>AV1_EXTRADATA_OFFSET)
        {
            _tracks[0].extraDataLen=entry.extraDataLen-AV1_EXTRADATA_OFFSET;
            _tracks[0].extraData=new uint8_t[_tracks[0].extraDataLen];
            memcpy(_tracks[0].extraData, entry.extraData+AV1_EXTRADATA_OFFSET, _tracks[0].extraDataLen);
            delete [] entry.extraData;
            entry.extraData=NULL;
            entry.extraDataLen=0;
        }else
        {
            _tracks[0].extraData=entry.extraData;
            _tracks[0].extraDataLen=entry.extraDataLen;
        }
        if(isH264Compatible(entry.fcc) && _tracks[0].extraData
            && _tracks[0].extraDataLen > 8 // FIXME
            && (_tracks[0].extraData[5] & 0x1F) == 1) // 1x SPS
        {
            ADM_SPSInfo info;
            if(extractSPSInfo_mp4Header(_tracks[0].extraData, _tracks[0].extraDataLen, &info))
            {
                uint32_t sz=sizeof(ADM_SPSInfo);
                if(_tracks[0].infoCache)
                    delete [] _tracks[0].infoCache;
                _tracks[0].infoCache=new uint8_t[sz];
                memcpy(_tracks[0].infoCache, &info, sz);
                _tracks[0].infoCacheSize=sz;
                // now copy SPS to the raw parameter sets cache
                uint8_t *p=_tracks[0].extraData;
                sz=(*(p+6)<<8)+*(p+7);
                if(_tracks[0].extraDataLen > sz+8)
                {
                    if(_tracks[0].paramCache)
                        delete [] _tracks[0].paramCache;
                    _tracks[0].paramCache=new uint8_t[sz];
                    memcpy(_tracks[0].paramCache,p+8,sz);
                    _tracks[0].paramCacheSize=sz;
                }
            }
        }
        _tracks[0].streamIndex=entry.trackNo;

        uint32_t hdr=entry.headerRepeatSize;
        if(hdr)
        {
            _tracks[0].headerRepeatSize=entry.headerRepeatSize;
            memcpy(_tracks[0].headerRepeat,entry.headerRepeat,hdr);
            ADM_info("video has %d bytes of repeated headers\n",hdr);
        }
        return 1;
    }
    //***************** Audio tracks *****************
    if(entry.trackType==2 && _nbAudioTrack<ADM_MKV_MAX_TRACKS)
    {
        mkvTrak *t=&(_tracks[1+_nbAudioTrack]);
        t->language=entry.language;
        if(!entry.bpp) entry.bpp = 16;
        t->wavHeader.bitspersample = entry.bpp;
        t->wavHeader.blockalign = 1;
        t->wavHeader.byterate=0; // to be set later
        // MS/ACM : ACMX
        if(0x100001==entry.fcc)
        {
            int l=entry.extraDataLen;
            int wavSize=sizeof(WAVHeader);
            ADM_info("Found ACM compatibility header (%d / %d)\n",l,wavSize);
            if(l>=wavSize) // we need at least a wavheader
            {
                mixDump(entry.extraData,l);
                memcpy(&(t->wavHeader),entry.extraData,wavSize);
                ADM_info("Encoding : %d\n",t->wavHeader.encoding);
                wavSize+=2; // size of WAVEFORMATEX
                int x=l-wavSize;

                if(x>0) // If we have more than WAVEFORMATEX, it is extradata
                {
                    ADM_info("Found %d bytes of extradata\n",x);
                    t->extraData=new uint8_t[x];
                    t->extraDataLen=x;
                    memcpy(t->extraData,entry.extraData+wavSize,x);
                    if(t->wavHeader.encoding==0xfffe) // WAVE_FORMAT_EXTENSIBLE + extradata: might be AAC LATM
                        t->wavHeader.encoding=MKV_MUX_LATM;
                }
                if(t->wavHeader.encoding==MKV_MUX_LATM)
                {
                    t->wavHeader.byterate=0; // to be set later
                    t->wavHeader.blockalign=1; // restore default
                }
                delete [] entry.extraData;
                t->streamIndex=entry.trackNo;
                if(entry.defaultDuration)
                    t->_defaultFrameDuration=entry.defaultDuration;
                else
                    t->_defaultFrameDuration=0;
                // In ACM mode we should not have the stripped header stuff..
                _nbAudioTrack++;
                return 1;
            }
        }
        if(entry.fcc==WAV_AAC)
        {
            if(!entry.extraDataLen)
            {
                ADM_info("Recreating aac extradata..\n");
                entry.extraData = new uint8_t[5];
                createAACExtraData(entry.codecId.c_str(),&entry);
            }else
            {
                // check AAC infoata
                 AacAudioInfo info;
                if(ADM_getAacInfoFromConfig(entry.extraDataLen,entry.extraData,info))
                {
                    ADM_info("Decoding AAC extra data gives :\n");
                    ADM_info("Fq= %d\n",info.frequency);
                    ADM_info("channels= %d\n",info.channels);
                    ADM_info("SBR= %d\n",info.sbr);
                    entry.chan=info.channels;
                    entry.fq=info.frequency;
                }
            }
        }
        if(entry.fcc==WAV_PCM || entry.fcc==WAV_LPCM)
        {
            t->wavHeader.blockalign = entry.chan * t->wavHeader.bitspersample >> 3;
            t->wavHeader.byterate = t->wavHeader.blockalign * entry.fq;
        }
        t->wavHeader.encoding=entry.fcc;
        t->wavHeader.channels=entry.chan;
        t->wavHeader.frequency=entry.fq;
        t->streamIndex=entry.trackNo;
        if(entry.defaultDuration)
            t->_defaultFrameDuration=entry.defaultDuration;
        else
            t->_defaultFrameDuration=0;
        uint32_t hdr=entry.headerRepeatSize;
        if(hdr)
        {
            t->headerRepeatSize=entry.headerRepeatSize;
            memcpy(t->headerRepeat,entry.headerRepeat,hdr);
        }
        t->extraData=entry.extraData;
        t->extraDataLen=entry.extraDataLen;
        ADM_info("This track has %d bytes of extradata\n",t->extraDataLen);

        _nbAudioTrack++;
        return 1;
    }
    // Other tracks, ignored...
    if(entry.extraData)
    {
        ADM_info("Ignoring extradata\n");
        delete [] entry.extraData;
    }
    return 1;

}


/**
    \fn entryWalk
    \brief walk a trackEntry atom and grabs all infos. Store them in entry
*/
uint8_t entryWalk(ADM_ebml_file *head,uint32_t headlen,entryDesc *entry)
{
  ADM_ebml_file father( head,headlen);
   uint64_t id,len;
  ADM_MKV_TYPE type;
  const char *ss;

  while(!father.finished())
  {
      father.readElemId(&id,&len);
      if(!ADM_searchMkvTag( (MKV_ELEM_ID)id,&ss,&type))
      {
        printf("[MKV/entryWalk] Tag 0x%" PRIx64" not found (len %" PRIu64")\n",id,len);
        father.skip(len);
        continue;
      }
      switch(id)
      {
        case  MKV_CONTENT_COMPRESSION_SETTINGS: 
//#warning todo: check it is stripping
                    if(len<=MKV_MAX_REPEAT_HEADER_SIZE)
                    {
                        father.readBin(entry->headerRepeat,len);
                        entry->headerRepeatSize=len;
                    };
                    break;
        case  MKV_TRACK_NUMBER: entry->trackNo=father.readUnsignedInt(len);break;
        case  MKV_TRACK_TYPE: entry->trackType=father.readUnsignedInt(len);break;

        case  MKV_AUDIO_FREQUENCY: entry->fq=(uint32_t)floor(father.readFloat(len));break;
        //case MKV_AUDIO_OUT_FREQUENCY:entry->fq=(uint32_t)floor(father.readFloat(len));break;
        case  MKV_AUDIO_BITS_PER_SAMPLE: entry->bpp=father.readUnsignedInt(len);break;
        case  MKV_VIDEO_WIDTH: entry->w=father.readUnsignedInt(len);break;
        case  MKV_VIDEO_HEIGHT: entry->h=father.readUnsignedInt(len);break;

        case  MKV_VIDEO_COLOUR_MATRIX_COEFF: ADM_info("MatrixCoefficients: %u\n",father.readUnsignedInt(len));break;
        case  MKV_VIDEO_COLOUR_RANGE: ADM_info("Range: %u\n",father.readUnsignedInt(len));break;
        case  MKV_VIDEO_COLOUR_TRANSFER: ADM_info("TransferCharacteristics: %u\n",father.readUnsignedInt(len));break;
        case  MKV_VIDEO_COLOUR_PRIMARIES: ADM_info("Primaries: %u\n",father.readUnsignedInt(len));break;

        case  MKV_DISPLAY_HEIGHT: ADM_info("Display Height:%d\n",(int)father.readUnsignedInt(len));break;
        case  MKV_DISPLAY_WIDTH: ADM_info("Display Width:%d\n",(int)father.readUnsignedInt(len));break;

        case  MKV_AUDIO_CHANNELS: entry->chan=father.readUnsignedInt(len);break;
        case  MKV_TIMECODE_SCALE:
        case  MKV_TRACK_TIMECODESCALE:
                                {
                                    ADM_warning("[Mkv] TimeCodeScale=%" PRIu64"\n",father.readUnsignedInt(len));
                                };break; //FIXME

        case  MKV_FRAME_DEFAULT_DURATION: entry->defaultDuration=father.readUnsignedInt(len)/1000; break; // In us
        case  MKV_CODEC_EXTRADATA:
        {
              uint8_t *data=new uint8_t[len];
                    father.readBin(data,len);
                    entry->extraData=data;
                    entry->extraDataLen=len;
                    break;
        }
        case  MKV_AUDIO_SETTINGS:
        case  MKV_VIDEO_SETTINGS:
        case  MKV_VIDEO_COLOUR:
        case  MKV_CONTENT_ONE_ENCODING:
        case  MKV_CONTENT_ENCODINGS:
        case  MKV_CONTENT_COMPRESSION:
                  entryWalk(&father,len,entry);
                  break;
        case MKV_LANGUAGE:
                {
                 char s[100];
                 s[99]=0;
                 father.readString(s,len);
                 if(!strlen(s))
                     strcpy(s,"eng"); // english is default
                 if(!strcmp(s,"unknown")) // we were using "unknown", which is not a valid ISO 639 code
                 {
                     memset(s,'\0',100);
                     std::string und=ADM_UNKNOWN_LANGUAGE;
                     strcpy(s,und.c_str());
                     ADM_info("Found 'unknown' as language code, replacing it with '%s'\n",und.c_str());
                 }else
                 {
                     ADM_info("Found language  = %s\n",s);
                 }
                 entry->language=std::string(s);
                }
            break;
        case MKV_CODEC_ID:
            {
            uint8_t *codec=new uint8_t[len+1];
                  father.readBin(codec,len);
                  codec[len]=0;
                  std::string codecAsString=std::string((char *)codec);
                  entry->codecId=codecAsString;
                  entry->fcc=ADM_mkvCodecToFourcc((char *)codec);
                  
                  delete [] codec;

            }
                  break;
        default: printf("[MKV]not handled %s\n",ss);
                  father.skip(len);
        }

      }
  return 1;
}//EOF
