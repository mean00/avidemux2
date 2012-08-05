/***************************************************************************
    copyright            : (C) 2006 by mean
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
#include "ADM_cpp.h"
#include <math.h>

#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"

#include "ADM_mkv.h"

#include "mkv_tags.h"

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
        
          void dump(void);
          uint32_t   headerRepeatSize;
          uint8_t    headerRepeat[MKV_MAX_REPEAT_HEADER_SIZE];

};
/* Prototypes */
static uint8_t entryWalk(ADM_ebml_file *head,uint32_t headlen,entryDesc *entry);
uint32_t ADM_mkvCodecToFourcc(const char *codec);
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
      memset(&entry,0,sizeof(entry));
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
          _tracks[0]._defaultFrameDuration=25000;

        }

        _mainaviheader.dwMicroSecPerFrame=(uint32_t)floor(50);;
        _videostream.fccType=fourCC::get((uint8_t *)"vids");
        _video_bih.biBitCount=24;
        _videostream.dwInitialFrames= 0;
        _videostream.dwStart= 0;
        _video_bih.biWidth=_mainaviheader.dwWidth=entry.w;
        _video_bih.biHeight=_mainaviheader.dwHeight=entry.h;
        _videostream.fccHandler=_video_bih.biCompression=entry.fcc;

        // if it is vfw...
        if(fourCC::check(entry.fcc,(uint8_t *)"VFWX") && entry.extraData && entry.extraDataLen>=sizeof(ADM_BITMAPINFOHEADER))
        {
          ADM_info("VFW compatibility header, data=%d bytes\n",(int)entry.extraDataLen);
          memcpy(& _video_bih,entry.extraData,sizeof(ADM_BITMAPINFOHEADER));

          _videostream.fccHandler=_video_bih.biCompression;
          _mainaviheader.dwWidth=  _video_bih.biWidth;
          _mainaviheader.dwHeight= _video_bih.biHeight;
          if(entry.extraDataLen>sizeof(ADM_BITMAPINFOHEADER))
          {
                int l=entry.extraDataLen-sizeof(ADM_BITMAPINFOHEADER);
                _tracks[0].extraData=new uint8_t[l];
                _tracks[0].extraDataLen=l;
                memcpy(_tracks[0].extraData,entry.extraData +sizeof(ADM_BITMAPINFOHEADER),l);
                ADM_info("VFW Header+%d bytes of extradata\n",l);   
                mixDump(_tracks[0].extraData,l);
                printf("\n");
          }
          delete [] entry.extraData;
          entry.extraData=NULL;
          entry.extraDataLen=0;

        } 
        else
        {
            _tracks[0].extraData=entry.extraData;
            _tracks[0].extraDataLen=entry.extraDataLen;
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
         uint32_t  streamIndex;
         mkvTrak *t=&(_tracks[1+_nbAudioTrack]);

        // MS/ACM : ACMX
        if(0x100001==entry.fcc)
        {
            int l=entry.extraDataLen;
            int wavSize=sizeof(WAVHeader);
            ADM_info("Found ACM compatibility header (%d / %d)\n",l,wavSize);
            if(l>=wavSize) // we need at least a wavheader
            {
                mixDump(entry.extraData,l); printf("\n");
                memcpy(&(t->wavHeader),entry.extraData,wavSize);
                ADM_info("Encoding : %d\n",t->wavHeader.encoding);
                int x=l-wavSize;
                
                if(x>0) // If we have more than a wavheader, it is extradata
                {
                    t->extraData=new uint8_t[x];
                    t->extraDataLen=x;
                    memcpy(t->extraData,entry.extraData+wavSize,x);
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

         t->wavHeader.encoding=entry.fcc;
         t->wavHeader.channels=entry.chan;
         t->wavHeader.frequency=entry.fq;
         t->wavHeader.bitspersample=16;
         t->wavHeader.byterate=(128000)>>3; //FIXME
         t->streamIndex=entry.trackNo;
         t->extraData=entry.extraData;
         t->extraDataLen=entry.extraDataLen;
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

        _nbAudioTrack++;
        return 1;
      }
      // Other tracks, ignored...
      if(entry.extraData) delete [] entry.extraData;
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
        printf("[MKV] Tag 0x%"PRIx64" not found (len %"PRIu64")\n",id,len);
        father.skip(len);
        continue;
      }
      switch(id)
      {
        case  MKV_CONTENT_COMPRESSION_SETTINGS: 
#warning todo: check it is stripping
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
        case  MKV_VIDEO_WIDTH: entry->w=father.readUnsignedInt(len);break;
        case  MKV_VIDEO_HEIGHT: entry->h=father.readUnsignedInt(len);break;

        case  MKV_DISPLAY_HEIGHT: ADM_info("Display Height:%d\n",(int)father.readUnsignedInt(len));break;
        case  MKV_DISPLAY_WIDTH: ADM_info("Display Width:%d\n",(int)father.readUnsignedInt(len));break;

        case  MKV_AUDIO_CHANNELS: entry->chan=father.readUnsignedInt(len);break;
        case  MKV_TIMECODE_SCALE:
        case  MKV_TRACK_TIMECODESCALE:
                                {
                                    ADM_warning("[Mkv] TimeCodeScale=%"PRIu64"\n",father.readUnsignedInt(len));
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
        case  MKV_CONTENT_ONE_ENCODING:
        case  MKV_CONTENT_ENCODINGS:
        case  MKV_CONTENT_COMPRESSION:
                  entryWalk(&father,len,entry);
                  break;
        case MKV_CODEC_ID:
            {
            uint8_t *codec=new uint8_t[len+1];
                  father.readBin(codec,len);
                  codec[len]=0;
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
