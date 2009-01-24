/***************************************************************************
                          Stream handler for mpeg1/2 ES/TS/PS
                             -------------------
    begin                : Sat Oct 12 2002
    copyright            : (C) 2002 by mean
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
#include <math.h>
#include "ADM_default.h"
#include "avifmt.h"
#include "avifmt2.h"

#include "ADM_Video.h"
#include "ADM_audio/aviaudio.hxx"

#include "fourcc.h"
#include "DIA_coreToolkit.h"
#include "ADM_userInterfaces/ADM_commonUI/DIA_working.h"


#include "dmx_demuxerEs.h"
#include "dmx_demuxerPS.h"
#include "dmx_demuxerTS.h"
#include "dmx_demuxerMSDVR.h"

#include "dmx_video.h"
#include "dmx_audio.h"

#define W _mainaviheader.dwWidth
#define H _mainaviheader.dwHeight


dmxHeader::dmxHeader  (void)
{                
        _nbGop=0;
        _nbFrames=0;
                
        memset(&_mainaviheader,0,sizeof(_mainaviheader));
        memset(&_videostream,0,sizeof(_videostream));

        _extraData=NULL; 
        _extraDataLen=0;               
        _nbGop=0;

        _index=NULL;
        _audioAccess=NULL;
        _fieldEncoded=0;
        demuxer=NULL;
}
//
// Delete everything associated (cache...demuxer...)
//
dmxHeader::~dmxHeader  ()
{
       
        close();
        _audioAccess=NULL; // Will be destroyed by audio!
}
uint8_t            dmxHeader::getRaw(uint32_t framenum,uint8_t *ptr,uint32_t* framelen)
{
  uint8_t r;
        ADMCompressedImage img;
        img.data=ptr;
        r= getFrameNoAlloc(framenum,&img);
        *framelen=img.dataLength;
        return r;
}
uint8_t            dmxHeader::getRawStart(uint8_t *ptr,uint32_t *len)
{
                if(!_extraData)
                {
                        *len=0;
                        return 0;
                }
                memcpy(ptr,_extraData,_extraDataLen);
                *len=_extraDataLen;     
                return 1;

}
uint8_t  dmxHeader::setFlag(uint32_t frame,uint32_t flags)
{
        return 1; // not usefull for these
}
uint32_t  dmxHeader::getFlags(uint32_t frame,uint32_t *flags)
{
        *flags=0;
         if(frame>=_nbFrames ) return 0;
         switch(_index[frame].type)
         {
                case 'I': *flags=AVI_KEY_FRAME;break;
                case 'P': *flags=0;break;
                case 'B': *flags=AVI_B_FRAME;break;
                default: ADM_assert(0);

          }
        return 1; // not usefull for these
}


//------------------------
uint8_t                 dmxHeader::close(void)
{
      #define FREETAB(x) {if(x) {delete [] x;x=NULL;}}
          #define FREE(x) {if(x) {delete  x;x=NULL;}}
          
          FREE(demuxer);
          FREETAB(_extraData);
          FREETAB(_index);
          #undef FREE
          #undef FREETAB
    return 1;
}
//-----------------------------------
//-----------------------------------

WAVHeader *dmxHeader::getAudioInfo(void )
{
        //if(_audioStream) return _audioStream->getInfo();
                 return NULL;

}
//-----------------------------------
//-----------------------------------

uint8_t                 dmxHeader::getAudioStream(AVDMGenericAudioStream **audio)
{
#if 0
        if(_audioStream)
        {
                *audio=_audioStream;
                return 1;
        }
#endif
                *audio=NULL;
                return 0;


}
uint8_t  dmxHeader::getExtraHeaderData(uint32_t *len, uint8_t **data)
{
                if(!_extraData)
                {
                        *len=0;
                        *data=NULL;
                        return 0;
                }
                *data=_extraData;
                *len=_extraDataLen;     
                return 1;            
}
uint8_t dmxHeader::getFrameSize(uint32_t frame,uint32_t *size) {

        if(frame>=_nbFrames ) return 0;
        *size=_index[frame].size;
        return 1;

}
/*
        Open the index file and grab all informations


*/
#define MAX_LINE 4096
uint8_t                 dmxHeader::open(const char *name)
{
                FILE *file;
        
                uint32_t w=720,h=576,fps=0;
                uint8_t  type,progressif;
                char     realname[1024];
                uint32_t dummy;
                uint32_t vPid,vTsId;
                char     payload[MAX_LINE+1];
                
                char string[MAX_LINE+1]; //,str[1024];;
                uint8_t interlac=0;
                int multi;
                uint32_t oldIndex=0;
                        
                printf("\n  opening d2v file : %s\n",name);
                file=fopen(name,"rt");
                if(!file) 
                        {
                                printf("\n Error !!\n");
                                return 0;
                        }
                
                fgets(string,MAX_LINE,file);        // File header
                if(strncmp(string,"ADMY",4))
                {
                    if(strncmp(string,"ADMX",4)) // Older index file ?
                    {
                        fclose(file);
                        printf("This is not a mpeg index G2\n");
                         return 0;
                    }
                    oldIndex=1;
                }
        
                
                fgets(string,MAX_LINE,file);
                sscanf(string,"Type     : %c\n",&type); // ES for now
                

                fgets(string,MAX_LINE,file);
              //  sscanf(string,"File     : %s\n",realname);
char *start;
 
                 start=strstr(string,":");
                ADM_assert(start);
                strcpy(realname,start+2);
                int l=strlen(realname)-1;
                while(l&&(realname[l]==0x0a || realname[l]==0x0d))
                {
                        realname[l]=0;
                        l--;
                }
                

                
                fgets (string, MAX_LINE, file);
                sscanf(string,"Append   : %d\n",&multi);

                fgets(string,MAX_LINE,file);
                sscanf(string,"Image    : %c\n",&progressif); // Progressive
                if(progressif=='I') _fieldEncoded=1;
                fgets(string,MAX_LINE,file);
                sscanf(string,"Picture  : %u x %u %u fps\n",&w,&h,&fps); // width...

                payload[0]=0;
                if(!oldIndex)
                {
                    fgets(string,MAX_LINE,file);
                    sscanf (string, "Payload  : %s\n",payload);	// FIXME ! overflow possible
                    if(!strncmp(payload,"MPEG",4))
                    {
                          _payloadType=DMX_PAYLOAD_MPEG2;
                    }else
                    {
                      if(!strncmp(payload,"H264",4))
                      {
                            _payloadType=DMX_PAYLOAD_H264;
                      }else
                      {
                        if(!strncmp(payload,"MP_4",4))
                        {
                              _payloadType=DMX_PAYLOAD_MPEG4;
                        }else
                        {
                          ADM_assert(0); 
                        }
                        
                      }
                    }
                }else
                {
                  _payloadType=DMX_PAYLOAD_MPEG2;
                }
                
                
                fgets(string,MAX_LINE,file);
                sscanf(string,"Nb Gop   : %u \n",&_nbGop); // width...

                fgets(string,MAX_LINE,file);
                sscanf(string,"Nb Images: %u \n",&_nbFrames); // width...

                fgets(string,MAX_LINE,file);
                //fscanf(string,"Nb Audio : %u\n",0); 

                fgets(string,MAX_LINE,file);
                //fprintf(out,"Main aud : %u\n",preferedAudio); 

                fgets(string,MAX_LINE,file);
                sscanf(string,"Streams  : V%X:%X \n",&vTsId,&vPid); 

                printf("For file :%s\n",realname);                
                printf("Pic      :%dx%d, %d fps\n",w,h,fps);
                printf("#Gop     :%lu\n",_nbGop);
                printf("#Img     :%lu\n",_nbFrames);

                
                switch(type)
                {
                        case 'M':
                                {
                                  MPEG_TRACK track;
                                  track.pid=vTsId;
                                  track.pes=vPid;
                          //        demuxer=new dmx_demuxerMSDVR(1,&track,0);
                                  break;
                                }
                        case 'T' :
                                {
                                        MPEG_TRACK track;
                                        track.pid=vTsId;
                                        track.pes=vPid;
                                        demuxer=new dmx_demuxerTS(1,&track,0,DMX_MPG_TS);
                                        break;

                                }
                        case 'S' :
                              {
                                      MPEG_TRACK track;
                                      track.pid=vTsId;
                                      track.pes=vPid;
                                      demuxer=new dmx_demuxerTS(1,&track,0,DMX_MPG_TS2);
                                      break;

                              }
                        case 'P':
                                {
                                        MPEG_TRACK track;
                                        track.pid=0;
                                        track.pes=vPid;
                                        demuxer=new dmx_demuxerPS(1,&track,multi);
                                        break;
                                }
                        case 'E':
                                demuxer=new dmx_demuxerES();
                                break;
                        default:
                                ADM_assert(0);
                }


                if(!demuxer->open(realname))
                {
                                printf("\n cannot open mpeg >%s<\n",realname);
                                delete demuxer;
                                demuxer=NULL;
                                fclose(file);
                                return 0;
                }
                
                _index=new dmxIndex[_nbFrames+1];
                if(!_index)
                        {
                          GUI_Error_HIG(QT_TR_NOOP("Out of memory"), NULL);
                                        ADM_assert(0);
                        }
                memset(_index,0,_nbFrames*sizeof(dmxIndex));
                
                // -------------- Read the file (video)---------------------
                uint32_t read=0;
                uint32_t currentImage=0;
                uint32_t gop,imageStart,imageNb;
                uint64_t abs,rel;
                uint8_t  imgtype;
                uint32_t imgsize;
                uint64_t imgrel,imgabs;
                char *str,*needle;
                
                DIA_working *work=new DIA_working(QT_TR_NOOP("Opening MPEG"));
                while(read<_nbGop)
                {
                        if(!fgets(string,MAX_LINE,file)) break;
                        if(string[0]!='V') continue;
                        //printf("%s\n",string);
                        // # NGop NImg nbImg Pos rel type:size type:size
                        sscanf(string,"V %u %u %u ",&gop,&imageStart,&imageNb);
                                ADM_assert(read==gop);
                                if(currentImage!=imageStart)
				{
					printf("At gop :%u read:%u, expected image %u, got %u,imagenb:%u\n",gop,read,currentImage,imageStart,imageNb);
					printf("String :%s\n",string);
					ADM_assert(0);
				}
                        
                                
                                // now split the image
                                needle=strstr(string,":");
                                ADM_assert(needle);
                                needle--;
                                // 
                                
                                for(uint32_t i=currentImage;i<currentImage+imageNb;i++)
                                {
                                        str=strstr(needle,":"); 
                                        if(!str)
                                        {
                                                printf("****** Error reading index, index damaged ?****\n");
                                                printf("Gop: %d/%d\n",read,_nbGop);
                                                printf("Img: %d/%d/%d\n",i,i-currentImage,imageNb);
                                                printf("Str:%s\n",string);
                                                printf("****** Error reading index, index damaged ?****\n");
                                                ADM_assert(0);
                                        }
                                        str--;
#ifdef __WIN32                                        
                                        sscanf(str,"%c:%I64x,%I64x,%x",&imgtype,&imgabs,&imgrel,&imgsize);
#else                                      
                                        sscanf(str,"%c:%llx,%llx,%x",&imgtype,&imgabs,&imgrel,&imgsize);
#endif                                          
                                        if(i>=_nbFrames)         
                                        {
                                                printf("Max frame exceeded :%d/%d\n",i,_nbFrames);
                                                ADM_assert(i<_nbFrames);
                                        }
                                        
                                        _index[i].type=imgtype;
                                        _index[i].size=imgsize;
                                       
                                       
                                        _index[i].absolute=imgabs;
                                        _index[i].relative=imgrel;
                                        

                                        str[1]=' '; // remove :
                                }
                                currentImage+=imageNb;
                                read++;
	                        work->update(  read,_nbGop)   ;
                }
                delete work;
                fclose(file);
                if(!_nbFrames)
                {
                  printf("No image!\n");
                  return 0; 
                }
                        // Drop the last P/B frames as we won't be able to decode them
                        // (last frame must be an I frame for decodeGop to work)
                        uint32_t dropped=0;
                        for(uint32_t y=_nbFrames-1;y>0;y--)
                        {
                                        if(_index[y].type!='B') break;
                                        _nbFrames--;
                                        dropped++;
                        }
                        printf("Dropping %d last B/P frames\n",dropped);                        
                        printf(" Creating start sequence (%llu)..\n",_index[0].absolute);
                        
                        //
                        
                        uint32_t scancode=0;
                        uint32_t count=0,found=0;
                        uint32_t firstPic=_index[0].size;
                        uint8_t *tmp=new uint8_t[firstPic];
                        

                        
                        demuxer->setPos(_index[0].absolute,
                                        _index[0].relative);
                        
                        
                        demuxer->read(tmp,firstPic);
                        _extraDataLen=0;
                        _extraData=NULL;
                        // lookup up gop start
                        while(count<firstPic)
                        {
                                scancode<<=8;
                                scancode+=tmp[count];
                                count++;
                                if(scancode==0x000001b8 || scancode==0x00000100)
                                {
                                        found=1;
                                        break;
                                }                                                       
                        }
                        if(found && count>4)
                        {
                                
                                _extraDataLen=count-4;
                                _extraData=new uint8_t[_extraDataLen];
                                memcpy(_extraData,tmp,_extraDataLen);
                                mixDump(tmp,50);
                                printf("\n");
                                printf("Image :%d, seqLen : %u seq %x %x %x %x\n",
                                        firstPic,
                                        _extraDataLen, _extraData[0],
                                                        _extraData[1],
                                                        _extraData[2],
                                                        _extraData[3]);                                          
                        }
                        else
                        {
                                printf("Mmm cound not find a gop start.....\n");
                        }
                        delete [] tmp;                                                
                   
                         demuxer->setPos(_index[0].absolute,
                                        _index[0].relative);

                                                 
                        if(_fieldEncoded) 
                        {
                                printf("This is field encoded...\n");
                                mergeFields();
                        }
                        _isaudiopresent=0; 
                        _isvideopresent=1; 
                        _videostream.dwScale=1000;
                        _videostream.dwRate=fps;
    
                        _mainaviheader.dwMicroSecPerFrame=(uint32_t)floor(50);;     
                        _videostream.fccType=fourCC::get((uint8_t *)"vids");
                        _video_bih.biBitCount=24;
      
                        switch(_payloadType)
                        {
                          case DMX_PAYLOAD_MPEG2:_videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"MPEG");;break;
                          case DMX_PAYLOAD_MPEG4:_videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"DX50");;break;
                          case DMX_PAYLOAD_H264:_videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"H264");;break;
                          default: ADM_assert(0);
                        }
                        
      
                        _videostream.dwInitialFrames= 0;
                        _videostream.dwStart= 0;
                        _video_bih.biWidth=_mainaviheader.dwWidth=w ;
                        _video_bih.biHeight=_mainaviheader.dwHeight=h;
     
                        _lastFrame=0xffffffff;

                       _videostream.dwLength= _mainaviheader.dwTotalFrames=_nbFrames;     
                       // Dump();                      
                        
                        // switch DTS->PTS
                        if(!renumber())
                        {
                          GUI_Error_HIG(QT_TR_NOOP("MPEG renumbering error"), NULL);
                                return 0;
                        }
                        //Dump();
                        if(type!='E')
                        {
                                // We have potentially some audio
                                // Try to get it
#if 0
                                dmxAudioStream *tmp;
                                tmp=new dmxAudioStream;
                                if(!tmp->open(name)) delete tmp;
                                else
                                {
                                        _audioStream=tmp;
                                }
#endif
                        }
                        
     printf("Mpeg index file successfully read\n");         
     return 1; 
}
/***/
void          dmxHeader:: Dump (void )
{
         
        for(uint32_t i=0;i<50;i++)
        {
                printf("%d : %c S:%x A:%x R:%x\n",i,_index[i].type,_index[i].size,_index[i].absolute,_index[i].relative);

        }

}
//
//      Merge the 2 fields to create frames
//
uint8_t  dmxHeader::mergeFields(void)
{
dmxIndex       *tmp=new  dmxIndex[_nbFrames/2+1];;
uint32_t        newNb=_nbFrames>>1;
        for(uint32_t c=0;c<newNb;c++)
        {
                // copy 1st field
                memcpy(&tmp[c],&_index[c*2],sizeof(dmxIndex));
                tmp[c].size+=_index[c*2+1].size;
        }
        delete [] _index;
        _index=tmp;
        _nbFrames=newNb;
        return 1;

}

//
//      Create GOP renumbering and PTS index entry
//
uint8_t  dmxHeader::renumber(void)
{


        dmxIndex       *tmp=new  dmxIndex[_nbFrames+2];;
        //__________________________________________
        // the direct index is in DTS time (i.e. decoder time)
        // we will now do the PTS index, so that frame numbering is done
        // according to the frame # as they are seen by editor / user                   
        // I1 P0 B0 B1 P1 B2 B3 I2 B7 B8
        // xx I1 B0 B1 P0 B2 B3 P1 B7 B8               
        //__________________________________________
        uint32_t forward=0;                     
        uint32_t curPTS=0;
        uint32_t dropped=0;
        uint32_t curSeek=1;
        printf("Reordering mpeg frames\n");
        for(uint32_t c=1;c<_nbFrames;c++)
        {
                switch(_index[c].type)
                {
                        case 'P':
                        case 'I':
                                        memcpy(&tmp[curPTS],&_index[forward],sizeof(dmxIndex));
                                        forward=c;
                                        curPTS++;
                                        dropped++;
                                        break;
                        case 'B' : // we copy as is
                                        if(dropped)
                                        {
                                                memcpy(&tmp[curPTS], &_index[c],sizeof(dmxIndex));
                                                curPTS++;
                                        }
                                        else
                                        { 
                                                printf("Frame dropped\n");
                                        }
                                        break;
                        default:
                                        printf("Frame : %u / %u , type %d\n",
                                                c,_nbFrames,_index[c].type);
                                
                                        ADM_assert(0);
                  }
           }
           // put back last frame we had in store
           memcpy(&tmp[curPTS], &_index[forward], sizeof(dmxIndex));
           
          // update
          _nbFrames=_videostream.dwLength= _mainaviheader.dwTotalFrames=curPTS+1;;
          delete [] _index;
          _index=tmp;             
          printf("Renumbered Gop  %d /%d\n",curSeek,_nbGop);
          return 1;
}

//-------------------------
// We do no try to optimize
// sequential access,
// it is up to the underlying layer to do so (demuxer)
// Here we look for simplicity
//__________________________
uint8_t   dmxHeader::getFrameNoAlloc(uint32_t framenum,ADMCompressedImage *img)
{
uint32_t f;
dmxIndex *idx;

        if(framenum>=_nbFrames) return 0;
        
        img->flags=getFlags(framenum,&f);

        idx=&(_index[framenum]);
        img->dataLength=idx->size;
        
        if(!demuxer->setPos(idx->absolute,idx->relative)) 
                {
                        printf("[DMX] set pos failed\n");
                        return 0;
                }
 
        if(!demuxer->read(img->data,img->dataLength))
        {
                printf("[DMX] Read failed\n");
                return 0;
        }
        return 1;
}
/*********************************/
 uint8_t  dmxHeader::changeAudioStream(uint32_t newstream)
{
//  ADM_assert(_audioStream);
//  return _audioStream->changeAudioTrack(newstream);
    return false;
}
uint32_t  dmxHeader::getCurrentAudioStreamNumber(void)
{
//  if(!_audioStream) return 0;
 // return _audioStream->currentTrack;
    return 0;
}
uint8_t  dmxHeader::getAudioStreamsInfo(uint32_t *nbStreams, audioInfo **infos)
{
#if 0
    if(!_audioStream)
    {
        *nbStreams=0;
        *infos=NULL;
        return 1;
    }
    return _audioStream->getAudioStreamsInfo(nbStreams,infos);
#endif
    return 0;
}
/**
    \fn getAudioStream
*/
uint8_t   dmxHeader::getAudioStream(ADM_audioStream  **audio)
{
    *audio=NULL;
    return 1;

}
// EOF
