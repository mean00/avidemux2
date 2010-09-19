/***************************************************************************

    copyright            : (C) 2007 by mean
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


#include <math.h>

#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"
#include "ADM_mp4.h"
#include "DIA_coreToolkit.h"
//#include "ADM_codecs/ADM_codec.h"

#include "ADM_mp4Tree.h"

#define adm_printf(...) {}
#define aprintf(...) {}

#define QT_TR_NOOP(x) x
#define TRACK_OTHER 0
#define TRACK_AUDIO 1
#define TRACK_VIDEO 2

uint32_t ADM_UsecFromFps1000(uint32_t fps1000);
// 14496-1 / 8.2.1
typedef enum
{
	Tag_InitialObjDesc	=0x02,
	Tag_ES_Desc		=0x03,
	Tag_DecConfigDesc 	=0x04,
	Tag_DecSpecificInfo 	=0x05
}MP4_Tag;

//extern char* ms2timedisplay(uint32_t ms);

/**
      \fn    LookupMainAtoms
      \brief Search main atoms to ease job for other part
*/
uint8_t     MP4Header::lookupMainAtoms(void *ztom)
{

  adm_atom *tom=(adm_atom *)ztom;
  adm_atom *moov;
  ADMAtoms id;
  uint32_t container;
  printf("Analyzing file and atoms\n");
  if(!ADM_mp4SimpleSearchAtom(tom, ADM_MP4_MOOV,&moov))
  {
       adm_printf(ADM_PRINT_ERROR,"Cannot locate moov atom\n");
       return 0;
  }
  ADM_assert(moov);
  while(!moov->isDone())
  {
    adm_atom son(moov);
    if(!ADM_mp4SearchAtomName(son.getFCC(), &id,&container))
    {
      adm_printf(ADM_PRINT_DEBUG,"Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
    }
    else
    {
      switch( id)
      {
        case ADM_MP4_MVHD: parseMvhd(&son);break;
        case ADM_MP4_TRACK:
            if(!parseTrack(&son))
            {
                printf("Parse Track failed\n");
            } ;
            break;
        default :
                adm_printf(ADM_PRINT_DEBUG,"atom %s not handled\n",fourCC::tostringBE(son.getFCC()));
                break;
      }


    }
    son.skipAtom();
  }
  delete moov;
  printf("Done finding main atoms\n");
  return 1;
}
/**
      \fn parseMvhd
      \brief Parse mvhd header
*/
void MP4Header::parseMvhd(void *ztom)
{
	adm_atom *tom = (adm_atom*)ztom;
	int version = tom->read();

	tom->skipBytes(3);	// flags

	if (version == 1)
		tom->skipBytes(16);
	else
		tom->skipBytes(8);

	int scale = tom->read32();
	uint64_t duration = (version == 1) ? tom->read64() : tom->read32();

	_videoScale = scale;

	printf("Warning: scale is not in ms %"LU"!\n", _videoScale);

	if (_videoScale)
	{
		duration = 1000 * duration; // In ms
		duration /= _videoScale;
	}
	else
		_videoScale = 1000;

	//printf("Movie duration: %s\n", ms2timedisplay(duration));

	_movieDuration = duration;
}

/**
      \fn parseMvhd
      \brief Parse mvhd header
*/
uint8_t MP4Header::parseTrack(void *ztom)
{
  adm_atom *tom=(adm_atom *)ztom;
  ADMAtoms id;
  uint32_t container;
  uint32_t w,h;
  uint32_t trackType=TRACK_OTHER;

  printf("Parsing Track\n");
   while(!tom->isDone())
  {
     adm_atom son(tom);
     if(!ADM_mp4SearchAtomName(son.getFCC(), &id,&container))
     {
       adm_printf(ADM_PRINT_DEBUG,"Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
       son.skipAtom();
       continue;
     }
     adm_printf(ADM_PRINT_DEBUG,"\tProcessing atom %s \n",fourCC::tostringBE(son.getFCC()));
     switch(id)
     {
       case ADM_MP4_TKHD:
              {
				  int version = son.read();

				  son.skipBytes(3);

				  if (version == 1)
					  tom->skipBytes(16);
				  else
					  tom->skipBytes(8);

				  adm_printf(ADM_PRINT_DEBUG,"Track Id: %"LU"\n", son.read32());
				  son.skipBytes(4);

				  uint64_t duration = (version == 1) ? son.read64() : son.read32();

				  adm_printf(ADM_PRINT_DEBUG, "Duration: %"LU" (ms)\n", (duration * 1000) / _videoScale);
				  son.skipBytes(8);
				  son.skipBytes(8);
				  son.skipBytes(36);

				  w = son.read32() >> 16;
				  h = son.read32() >> 16;

				  adm_printf(ADM_PRINT_DEBUG,"tkhd: %ld %ld\n", w, h);
				  break;
              }
        case ADM_MP4_MDIA:
        {
            parseMdia(&son,&trackType,w,h);
            break;
        }
        case ADM_MP4_EDTS:
        {
            ADM_info("EDTS atom found\n");
            parseEdts(&son);
            break;
        }
       default:
          ADM_info("Unprocessed atom :%s\n",fourCC::tostringBE(son.getFCC()));
     }
     son.skipAtom();
  }
  return 1;
}
/**
      \fn parseMdia
      \brief Parse mdia header
*/
uint8_t MP4Header::parseMdia(void *ztom,uint32_t *trackType,uint32_t w, uint32_t h)
{
  adm_atom *tom=(adm_atom *)ztom;
  ADMAtoms id;
  uint32_t container;
  uint32_t trackScale=_videoScale;
  uint64_t trackDuration;
  *trackType=TRACK_OTHER;
  uint8_t r=0;
  printf("<<Parsing Mdia>>\n");
  while(!tom->isDone())
  {
     adm_atom son(tom);
     if(!ADM_mp4SearchAtomName(son.getFCC(), &id,&container))
     {
       adm_printf(ADM_PRINT_DEBUG,"[MDIA]Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
       son.skipAtom();
       continue;
     }
     switch(id)
     {
       case ADM_MP4_MDHD:
       {
		   int version = son.read();

		   son.skipBytes(3); // flags

		   if (version == 1)
			   son.skipBytes(16);
		   else
			   son.skipBytes(8);

		   trackScale = son.read32();

		   adm_printf(ADM_PRINT_DEBUG, "MDHD, Trackscale in mdhd: %u\n", trackScale);

		   if (!trackScale)
			   trackScale = 600; // default

		   uint64_t duration = (version == 1) ? son.read64() : son.read32();

		   adm_printf(ADM_PRINT_DEBUG, "MDHD, duration in mdhd: %u (unscaled)\n", duration);
		   duration = (duration * 1000.) / trackScale;
		   adm_printf(ADM_PRINT_DEBUG, "MDHD, duration in mdhd: %u (scaled ms)\n", duration);
		   trackDuration = duration;
//		   printf("MDHD, Track duration: %s, trackScale: %u\n", ms2timedisplay((1000 * duration) / trackScale), trackScale);

		   break;
       }
       case ADM_MP4_HDLR:
       {
            uint32_t type;

                son.read32();
                son.read32();
                type=son.read32();
                printf("[HDLR]\n");
                switch(type)
                {
                case MKFCCR('v','i','d','e')://'vide':
                        *trackType=TRACK_VIDEO;
                        printf("hdlr video found \n ");
                        _movieDuration=trackDuration;
                        _videoScale=trackScale;
                        break;
                case MKFCCR('s','o','u','n'): //'soun':
                        *trackType=TRACK_AUDIO;
                        printf("hdlr audio found \n ");
                        break;
                case MKFCCR('u','r','l',' ')://'url ':
                    {
                        int s;
                        son.read32();
                        son.read32();
                        son.read32();
                        s=son.read();
                        char *str=new char[s+1];
                        son.readPayload((uint8_t *)str,s);
                        str[s]=0;
                        printf("Url : <%s>\n",str);
                        delete [] str;
                      }
                      break;

                }
                break;
       }
       case ADM_MP4_MINF:
       {
            // We are only interested in stbl

            while(!son.isDone())
            {
              adm_atom grandson(&son);
              if(!ADM_mp4SearchAtomName(grandson.getFCC(), &id,&container))
              {
                adm_printf(ADM_PRINT_DEBUG,"[MINF]Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
                grandson.skipAtom();
                continue;
              }
              if(id==ADM_MP4_STBL)
              {
                   if(! parseStbl(&grandson,*trackType, w, h,trackScale))
                   {
                      printf("STBL failed\n");
                      return 0;
                   }
                   r=1;
              }
              grandson.skipAtom();
        }
       }
       break;
        default:
            adm_printf(ADM_PRINT_DEBUG,"** atom  NOT HANDLED [%s] \n",fourCC::tostringBE(son.getFCC()));
     }

     son.skipAtom();
  }
  return r;
}

/**
        \fn parseEdts
        \brief parse sample table. this is the most important function.
*/
uint8_t       MP4Header::parseEdts(void *ztom)
{
  adm_atom *tom=(adm_atom *)ztom;
  ADMAtoms id;
  uint32_t container;

  ADM_info("Parsing Edts>>\n");
  while(!tom->isDone())
  {
     adm_atom son(tom);
     if(!ADM_mp4SearchAtomName(son.getFCC(), &id,&container))
     {
       adm_printf(ADM_PRINT_DEBUG,"[EDTS]Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
       son.skipAtom();
       continue;
     }
    switch(id)
    {
       case ADM_MP4_ELST:
       {
              ADM_info("ELST atom found\n");
              son.skipBytes(4);
              uint32_t nb=son.read32();
              ADM_info("Found %"LU" entries in list:\n",nb);
              for(int i=0;i<nb;i++)
                {
                    uint32_t editDuration=son.read32();
                    uint32_t mediaTime=son.read32();
                    uint32_t playbackSpeed=son.read32();
                    ADM_info("Duration : %"LU", mediaTime:%"LU" speed=%"LU"\n",editDuration,mediaTime,playbackSpeed);
                }
              son.skipAtom();
              break;
        
       }
       break;
        default:
            adm_printf(ADM_PRINT_DEBUG,"** atom  NOT HANDLED [%s] \n",fourCC::tostringBE(son.getFCC()));
     }
   }
   
   tom->skipAtom();
   return true;
}
/**
        \fn parseStbl
        \brief parse sample table. this is the most important function.
*/
uint8_t       MP4Header::parseStbl(void *ztom,uint32_t trackType,uint32_t w,uint32_t h,uint32_t trackScale)
{
  adm_atom *tom=(adm_atom *)ztom;
  ADMAtoms id;
  uint32_t container;
  MPsampleinfo  info;


  memset(&info,0,sizeof(info));


  printf("<<Parsing Stbl>>\n");
  while(!tom->isDone())
  {
     adm_atom son(tom);
     if(!ADM_mp4SearchAtomName(son.getFCC(), &id,&container))
     {
       adm_printf(ADM_PRINT_DEBUG,"[STBL]Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
       son.skipAtom();
       continue;
     }
     switch(id)
     {
       case ADM_MP4_STSS:  // Sync sample atom (i.e. keyframes)
       {
          son.read32();
          info.nbSync=son.read32();
          printf("Stss:%u\n",info.nbSync);
          if(info.nbSync)
          {
                  info.Sync=new uint32_t[info.nbSync];
                  for(int i=0;i<info.nbSync;i++)
                  {
                          info.Sync[i]=son.read32();
                  }
          }
          break;

       }
       case ADM_MP4_STTS:
            {
                printf("stts:%"LU"\n",son.read32()); // version & flags
                info.nbStts=son.read32();
                printf("Time stts atom found (%"LU")\n",info.nbStts);
                printf("Using myscale %"LU"\n",trackScale);
                info.SttsN=new uint32_t[info.nbStts];
                info.SttsC=new uint32_t[info.nbStts];
                //double dur;
                for(int i=0;i<info.nbStts;i++)
                {

                        info.SttsN[i]=son.read32();
                        info.SttsC[i]=son.read32();
                        adm_printf(ADM_PRINT_VERY_VERBOSE,"stts: count:%u size:%u (unscaled)\n",info.SttsN[i],info.SttsC[i]);
                        //dur*=1000.*1000.;; // us
                        //dur/=myScale;
                }
            }
            break;
       case ADM_MP4_STSC:
            {
                son.read32();
                info.nbSc=son.read32();
                info.Sc=new uint32_t[info.nbSc];
                info.Sn=new uint32_t[info.nbSc];
                for(int j=0;j<info.nbSc;j++)
                {

                        info.Sc[j]=son.read32();
                        info.Sn[j]=son.read32();
                        son.read32();
                        adm_printf(ADM_PRINT_VERY_VERBOSE,"\t sc  %d : sc start:%u sc count: %u\n",j,info.Sc[j],info.Sn[j]);
                }

            }
            break;
       case ADM_MP4_STSZ:
          {
            uint32_t n;
              son.read32();
              n=son.read32();
              info.nbSz=son.read32();
              info.SzIndentical=0;
              printf("%"LU" frames /%"LU" nbsz..\n",n,info.nbSz);
              if(n)
                      {
                            adm_printf(ADM_PRINT_VERY_VERBOSE,"\t\t%"LU" frames of the same size %"LU" , n=%"LU"\n",
                                info.nbSz,info.SzIndentical,n);
                            info.SzIndentical=n;
                            info.Sz=NULL;
                      }
              else
              {
                      info.Sz=new uint32_t[info.nbSz];
                      for(int j=0;j<info.nbSz;j++)
                      {
                                      info.Sz[j]=son.read32();
                      }
              }
          }
          break;
           case ADM_MP4_CTTS: // Composition time to sample
            {
                uint32_t n,i,j,k,v;

                  printf("ctts:%"LU"\n",son.read32()); // version & flags
                  n=son.read32();
                  if(n==1) // all the same , ignore
                  {
                    break;
                  }
                uint32_t *values=new uint32_t [n];
                uint32_t *count=new uint32_t [n];
                for(i=0;i<n;i++)
                {
                    count[i]=son.read32();
                    values[i]=son.read32();
                }
                uint32_t sum=0;
                for(i=0;i<n;i++)
                {
                    sum+=count[i];
                }
                info.Ctts=new uint32_t[sum+1]; // keep a safe margin

                for(i=0;i<n;i++)
                {
                    if(i<20)
                    {
                        adm_printf(ADM_PRINT_VERY_VERBOSE,"Ctts: nb: %u (%x) val:%u (%x)\n",count[i],count[i],values[i],values[i]);
                    }
                    for(k=0;k<count[i];k++)
                    {
                        info.Ctts[info.nbCtts++]=values[i];
                    }
                }
                delete [] values;
                delete [] count;
                if(!info.nbCtts)
                {
                    delete [] info.Ctts;
                    info.Ctts=NULL;
                    printf("Destroying Ctts, seems invalid\n");
                }
                ADM_assert(info.nbCtts<sum+1);
                printf("Found %u elements\n",info.nbCtts);
            }
            break;
       case ADM_MP4_STCO:
       {
		   son.skipBytes(4);

		   info.nbCo = son.read32();
		   printf("\t\tnbCo: %u\n", info.nbCo);

		   info.Co = new uint64_t[info.nbCo];

		   for(int j = 0; j < info.nbCo; j++)
		   {
			   info.Co[j] = son.read32();
			   adm_printf(ADM_PRINT_VERY_VERBOSE, "Chunk offset: %u / %u : %"LLU"\n", j, info.nbCo - 1, info.Co[j]);
		   }
       }
       break;
       case ADM_MP4_STCO64:
       {
		   son.skipBytes(4);

		   info.nbCo = son.read32();
		   printf("\t\tnbCo: %u\n", info.nbCo);

		   info.Co = new uint64_t[info.nbCo];

		   for(int j = 0; j< info.nbCo; j++)
		   {
			   info.Co[j] = son.read64();
			   adm_printf(ADM_PRINT_VERY_VERBOSE, "Chunk offset: %u / %u : %"LLU"\n", j, info.nbCo - 1, info.Co[j]);
		   }
       }
       break;
       case ADM_MP4_STSD:
       {
                son.read32(); // flags & version
                int nbEntries=son.read32();
                int left;
                adm_printf(ADM_PRINT_DEBUG,"[STSD]Found %d entries\n",nbEntries);
                for(int i=0;i<nbEntries;i++)
                {
                   int entrySize=son.read32();
                   int entryName=son.read32();
                   left=entrySize-8;
                   if(i || (trackType==TRACK_VIDEO && _videoFound) || (trackType==TRACK_OTHER))
                   {
                    son.skipBytes(left);
                    printf("[STSD] ignoring %s, size %u\n",fourCC::tostringBE(entryName),entrySize);
                    if(trackType==TRACK_OTHER) printf("[STSD] because track=other\n");
                    continue;
                   }
                   switch(trackType)
                   {
                     case TRACK_VIDEO:
                     {
                          uint32_t lw=0,lh=0;
                                printf("[STSD] VIDEO %s, size %u\n",fourCC::tostringBE(entryName),entrySize);
                                son.skipBytes(8);  // reserved etc..
                                left-=8;
                                son.read32(); // version/revision
                                left-=4;
                                printf("[STSD] vendor %s\n",fourCC::tostringBE(son.read32()));
                                left-=4;

                                son.skipBytes(8); // spatial qual etc..
                                left-=8;

                                printf("[STSD] width :%u\n",lw=son.read16());
                                printf("[STSD] height :%u\n",lh=son.read16());
                                left-=4;

                                son.skipBytes(8); // Resolution
                                left-=8;

                                printf("[STSD] datasize :%u\n",son.read32());
                                left-=4;

                                printf("[STSD] FrameCount :%u\n",son.read16());
                                left-=4;

                                // Codec name
                                uint32_t u32=son.read();
                                if(u32>31) u32=31;
                                printf("Codec string :%d <",u32);
                                for(int i=0;i<u32;i++) printf("%c",son.read());
                                printf(">\n");
                                son.skipBytes(32-1-u32);
                                left-=32;
                                //
                                if(left>=4)
                                {
                                    son.read32();
                                    left-=4; //Depth & color Id
                                }else left=0;
                                //
                                printf("LEFT:%d\n",left);

                                if(left>8)
                                {
//                                  decodeVideoAtom(&son);
                                }
                                //
#define commonPart(x)             _videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)#x);

                                 lw=(lw+7)&(~7);
                                 lh=(lh+7)&(~7);
                                 _video_bih.biWidth=_mainaviheader.dwWidth=lw ;
                                  _video_bih.biHeight=_mainaviheader.dwHeight=lh;
                                  _video_bih.biCompression=_videostream.fccHandler;

                                //
                                switch(entryName)
                                {
                                  case MKFCCR('m','j','p','b'):  //mjpegb
                                  {
                                        commonPart(MJPB);
                                        left=0;
                                  }
                                  case MKFCCR('S','V','Q','1'):  //mjpegb
                                 {
                                       commonPart(SVQ1);
                                       left=0;
                                 }
                                  break;

                                  case MKFCCR('m','j','p','a'):  //mjpegb
                                 {
                                       commonPart(MJPG);
                                       left=0;
                                 }
                                  break;
                                case MKFCCR('s','2','6','3'):  //s263 d263
                                  {
                                        commonPart(H263);
                                         adm_atom d263(&son);
                                         printf("Reading s253, got %s\n",fourCC::tostringBE(d263.getFCC()));
                                          left=0;
                                  }
                                  break;
                                   case MKFCCR('m','p','4','v'):  //mp4v
                                  {
                                        commonPart(DIVX);
                                         adm_atom esds(&son);
                                         printf("Reading esds, got %s\n",fourCC::tostringBE(esds.getFCC()));
                                         if(esds.getFCC()==MKFCCR('e','s','d','s'))
                                              decodeEsds(&esds,TRACK_VIDEO);
                                          left=0;
                                  }
                                        break;
                                  case MKFCCR('S','V','Q','3'):
                                  {//'SVQ3':
                                    // For SVQ3, the codec needs it to begin by SVQ3
                                    // We go back by 4 bytes to get the 4CC
                                            printf("SVQ3 atom found\n");
                                            VDEO.extraDataSize=left+4;
                                            VDEO.extraData=new uint8_t[ VDEO.extraDataSize ];
                                            if(!son.readPayload(VDEO.extraData+4,VDEO.extraDataSize-4 ))
                                            {
                                              GUI_Error_HIG(QT_TR_NOOP("Problem reading SVQ3 headers"), NULL);
                                            }
                                            VDEO.extraData[0]='S';
                                            VDEO.extraData[1]='V';
                                            VDEO.extraData[2]='Q';
                                            VDEO.extraData[3]='3';
                                            printf("SVQ3 Header size : %"LU"",_videoExtraLen);
                                            commonPart(SVQ3);
                                            left=0;
                                  }
                                            break;
                                  case MKFCCR('d','v','c',' ') : //'dvc ':
                                  case MKFCCR('d','v','c','p'): //'dvcp':
                                          commonPart(DVDS);
                                          break;
                                  case MKFCCR('c','v','i','d'): //'cvid'
                                          commonPart(cvid);
                                          break;
                                  case MKFCCR('h','2','6','3'): //'dv':
                                          commonPart(H263);
                                          break;
                                  case MKFCCR('M','J','P','G'): //'jpeg':
                                  case MKFCCR('j','p','e','g'): //'jpeg':
                                  case MKFCCR('A','V','D','J'): //'jpeg':
                                          commonPart(MJPG);
                                          break;
                                  case MKFCCR('a','v','c','1'): // avc1
                                          {
                                          commonPart(H264);
                                          // There is a avcC atom just after
                                          // configuration data for h264
nextAtom:
                                          adm_atom avcc(&son);
                                          printf("Reading avcC, got %s\n",fourCC::tostringBE(avcc.getFCC()));
                                          switch(avcc.getFCC())
                                          {
                                              case MKFCCR('a','v','c','C'): break;
                                              default:
                                              case MKFCCR('c','o','l','r'):  // Color atom
                                              case MKFCCR('p','a','s','p'):
                                              case MKFCCR('c','l','a','p'):
                                                  avcc.skipAtom();
                                                  goto nextAtom;
                                                  break;
                                          }
                                          int len,offset;
                                          VDEO.extraDataSize=avcc.getRemainingSize();
                                          VDEO.extraData=new uint8_t [VDEO.extraDataSize];
                                          avcc.readPayload(VDEO.extraData,VDEO.extraDataSize);
                                          printf("avcC size:%d\n",VDEO.extraDataSize);
                                    // Dump some info
                                        #define MKD8(x) VDEO.extraData[x]
                                        #define MKD16(x) ((MKD8(x)<<8)+MKD8(x+1))
                                        #define MKD32(x) ((MKD16(x)<<16)+MKD16(x+2))

                                            printf("avcC Revision             :%x\n", MKD8(0));
                                            printf("avcC AVCProfileIndication :%x\n", MKD8(1));
                                            printf("avcC profile_compatibility:%x\n", MKD8(2));
                                            printf("avcC AVCLevelIndication   :%x\n", MKD8(3));

                                            printf("avcC lengthSizeMinusOne   :%x\n", MKD8(4));
                                            printf("avcC NumSeq               :%x\n", MKD8(5));
                                            len=MKD16(6);
                                            printf("avcC sequenceParSetLen    :%x ",len );
                                            offset=8;
                                            mixDump(VDEO.extraData+offset,len);

                                            offset=8+len;
                                            printf("\navcC numOfPictureParSets  :%x\n", MKD8(offset++));
                                            len=MKD16(offset);
                                            offset++;
                                            printf("avcC Pic len              :%x\n",len);
                                            mixDump(VDEO.extraData+offset,len);
                                            left=0;
                                            }
                                            break;
                                  default:
                                            if(left>10)
                                            {
                                                adm_atom avcc(&son);
                                                printf("Reading , got %s\n",fourCC::tostringBE(avcc.getFCC()));
                                                left=0;

                                            }
                                            break;
                                } // Entry name
                     }
                     break;
                     case TRACK_AUDIO:
                     {
                        uint32_t channels,bpp,encoding,fq,packSize;

                                // Put some defaults
                                ADIO.encoding=1234;
                                ADIO.frequency=44100;
                                ADIO.byterate=128000>>3;
                                ADIO.channels=2;
                                ADIO.bitspersample=16;

                                printf("[STSD] AUDIO <%s>, 0x%08x, size %u\n",fourCC::tostringBE(entryName),entryName,entrySize);
                                son.skipBytes(8);  // reserved etc..
                                left-=8;

                                int atomVersion=son.read16();  // version
                                left-=2;
                                printf("[STSD]Revision       :%d\n",atomVersion);
                                son.skipBytes(2);  // revision
                                left-=2;

                                printf("[STSD]Vendor         : %s\n",fourCC::tostringBE(son.read32()));
                                left-=4;

                                ADIO.channels=channels=son.read16(); // Channel
                                left-=2;
                                printf("[STSD]Channels       :%d\n",ADIO.channels);
                                ADIO.bitspersample=bpp=son.read16(); // version/revision
                                left-=2;
                                printf("[STSD]Bit per sample :%d\n",bpp);

                                encoding=son.read16(); // version/revision
                                left-=2;
                                printf("[STSD]Encoding       :%d\n",encoding);

                                packSize=son.read16(); // Packet Size
                                left-=2;
                                printf("[STSD]Packet size    :%d\n",encoding);


                                fq=ADIO.frequency=son.read16();
                                printf("[STSD]Fq:%u\n",fq);
                                if(ADIO.frequency<6000) ADIO.frequency=48000;
                                printf("[STSD]Fq       :%d\n",ADIO.frequency); // Bps
                                        son.skipBytes(2); // Fixed point
                                left-=4;
                                if(atomVersion)
                                {
                                    
                                    info.samplePerPacket=son.read32();
                                    info.bytePerPacket=son.read32();
                                    info.bytePerFrame=son.read32();
                                        #define ADM_NOT_NULL(x)          if(!info.x) info.x=1;
                                    printf("[STSD] Sample per packet %u\n",info.samplePerPacket);
                                    printf("[STSD] Bytes per packet  %u\n",info.bytePerPacket);
                                    printf("[STSD] Bytes per frame   %u\n",info.bytePerFrame);
                                    printf("[STSD] Bytes per sample   %u\n",son.read32());
                                    ADM_NOT_NULL(samplePerPacket);  
                                    ADM_NOT_NULL(bytePerPacket);
                                    ADM_NOT_NULL(bytePerFrame);
                                    
                                    left-=16;
                                }else
                                {
                                  info.samplePerPacket=1;
                                  info.bytePerPacket=1;
                                  info.bytePerFrame=1;
                                }
                                switch(atomVersion)
                                {
                                  case 0:break;
                                  case 1: break;
                                  case 2:
                                          ADIO.frequency=44100; // FIXME
                                          ADIO.channels=son.read32();
                                          printf("Channels            :%d\n",ADIO.channels); // Channels
                                          printf("Tak(7F000)          :%x\n",son.read32()); // Channels
                                          printf("Bits  per channel   :%d\n",son.read32());  // Vendor
                                          printf("Format specific     :%x\n",son.read32());  // Vendor
                                          printf("Byte per audio packe:%x\n",son.read32());  // Vendor
                                          printf("LPCM                :%x\n",son.read32());  // Vendor
                                          left-=(5*4+4+16);
                                          break;
                                }
                                printf("[STSD] chan:%u bpp:%u encoding:%u fq:%u (left %u)\n",channels,bpp,encoding,fq,left);
#define audioCodec(x) ADIO.encoding=WAV_##x;
                                switch(entryName)
                                {

                                    case MKFCCR('t','w','o','s'):
                                            audioCodec(LPCM);
                                            ADIO.byterate=ADIO.frequency*ADIO.bitspersample*ADIO.channels/8;
                                            break;

                                    case MKFCCR('u','l','a','w'):
                                            audioCodec(ULAW);
                                            ADIO.byterate=ADIO.frequency;
                                            break;
                                    case MKFCCR('s','o','w','t'):
                                            audioCodec(PCM);
                                            ADIO.byterate=ADIO.frequency*ADIO.bitspersample*ADIO.channels/8;
                                            break;
                                    case MKFCCR('.','m','p','3'): //.mp3
                                            audioCodec(MP3);
                                            ADIO.byterate=128000>>3;
                                            break;
                                    case MKFCCR('r','a','w',' '):
                                            audioCodec(8BITS_UNSIGNED);
                                            ADIO.byterate=ADIO.frequency*ADIO.channels;
                                            break;
                                    case MKFCCR('s','a','m','r'):
                                    {
                                            audioCodec(AMRNB);
                                            ADIO.frequency=8000;
                                            ADIO.channels=1;
                                            ADIO.bitspersample=16;
                                            ADIO.byterate=12000/8;
                                            if(left>10)
                                            {
                                               adm_atom amr(&son);
                                              printf("Reading wave, got %s\n",fourCC::tostringBE(amr.getFCC()));
                                              left=0;
                                            }
                                    }
                                            break;
                                    case MKFCCR('Q','D','M','2'):
                                        {
                                            int64_t sz;
                                              audioCodec(QDM2);
                                              sz=son.getRemainingSize();
                                              _tracks[1+nbAudioTrack].extraDataSize=sz;
                                              _tracks[1+nbAudioTrack].extraData=new uint8_t[sz];
                                              son.readPayload(_tracks[1+nbAudioTrack].extraData,sz);
                                              left=0;
                                        }
                                        break;
                                    
                                
                                    case MKFCCR('m','s',0,0x55): // why 55 ???
                                    case MKFCCR('m','s',0,0x11): // why 11 ???
                                    case MKFCCR('m','p','4','a'):
                                    {
                                              if(entryName==MKFCCR('m','s',0,0x11))
                                                        audioCodec(MSADPCM)
                                                else
                                                        audioCodec(AAC);
                                            if(left>10)
                                            {
                                            
                                            
                                              while(!son.isDone())
                                              {
                                                adm_atom wave(&son);
                                                printf("> got %s atom\n",fourCC::tostringBE(wave.getFCC()));
                                                switch(wave.getFCC())
                                                {
                                                case MKFCCR('c','h','a','n'):
                                                           printf("Found channel layout atom, skipping\n");
                                                           break;
                                                case MKFCCR('w','a','v','e'):
                                                  {
                                                     // mp4a
                                                     //   wave
                                                     //     frma
                                                     //     mp4a
                                                     //     esds
                                                     while(!wave.isDone())
                                                     {
                                                         adm_atom item(&wave);
                                                         printf("parsing wave, got %s,0x%x\n",fourCC::tostringBE(item.getFCC()),
                                                                      item.getFCC());
                                                         
                                                         switch(item.getFCC())
                                                         {
                                                           case MKFCCR('f','r','m','a'):
                                                              {
                                                              uint32_t codecid=item.read32();
                                                              printf("frma Codec Id :%s\n",fourCC::tostringBE(codecid));
                                                              }
                                                              break;
                                                           case MKFCCR('m','s',0,0x55):
                                                           case MKFCCR('m','s',0,0x11):
                                                            {
                                                              // We have a waveformat here
                                                              printf("[STSD]Found MS audio header:\n");
                                                              ADIO.encoding=ADM_swap16(item.read16());
                                                              ADIO.channels=ADM_swap16(item.read16());
                                                              ADIO.frequency=ADM_swap32(item.read32());
                                                              ADIO.byterate=ADM_swap32(item.read32());
                                                              ADIO.blockalign=ADM_swap16(item.read16());
                                                              ADIO.bitspersample=ADM_swap16(item.read16());
                                                              printWavHeader(&(ADIO));

                                                            }
                                                           break;
                                                            case MKFCCR('m','p','4','a'):
                                                              break;
                                                            case MKFCCR('e','s','d','s'):
                                                              {
                                                                   decodeEsds(&item,TRACK_AUDIO);
                                                                   break;
                                                              }
                                                              break;
                                                           default:
                                                             break;
                                                         }

                                                         item.skipAtom();

                                                     }  // Wave iddone
                                                     left=0;
                                                     
                                                  }  // if ==wave
                                                  break;
                                              case MKFCCR('e','s','d','s'):
                                                          {
                                                               decodeEsds(&wave,TRACK_AUDIO);
                                                               break;
                                                          }
                                              default:
                                                  printf("UNHANDLED ATOM : %s\n",fourCC::tostringBE(wave.getFCC()));
                                                  break;
                                              }
                                              wave.skipAtom();
                                             } // while
                                            } // if left > 10
                                            left=0;
                                    }
                                            break; // mp4a

                                }
                     }
                          break;
                     default:
                          ADM_assert(0);
                   }
                   son.skipBytes(left);
                }
       }
              break;
       default:
          printf("[STBL]Skipping atom %s\n",fourCC::tostringBE(son.getFCC()));
     }
     son.skipAtom();
  }
  uint8_t r=0;
  uint32_t nbo=0;
  switch(trackType)
  {
    case TRACK_VIDEO:
        {
          if(_tracks[0].index)
          {
              printf("Already got a video track\n");
              return 1;
          }
          r=indexify(&(_tracks[0]),trackScale,&info,0,&nbo);

          _videostream.dwLength= _mainaviheader.dwTotalFrames=_tracks[0].nbIndex;
          // update fps
          float f=_videostream.dwLength;
          if(_movieDuration) f=1000000.*f/_movieDuration;
              else  f=25000;
          _videostream.dwRate=(uint32_t)floor(f+0.49);
           _mainaviheader.dwMicroSecPerFrame=ADM_UsecFromFps1000(_videostream.dwRate);
          // if we have a sync atom ???
          if(info.nbSync)
          {
            // Mark keyframes
            for(int i=0;i<info.nbSync;i++)
            {
              uint32_t sync=info.Sync[i];
              if(sync) sync--;
              _tracks[0].index[sync].intra=AVI_KEY_FRAME;
            }
          }
          else
          { // All frames are kf
            for(int i=0;i<_tracks[0].nbIndex;i++)
            {
              _tracks[0].index[i].intra=AVI_KEY_FRAME;
            }
          }
          // Now do the CTTS thing
          if(info.Ctts)
          {
            updateCtts(&info);
          }else 
          {
                // No ctts, dts=pts
                for(int i=0;i<_videostream.dwLength;i++)
                {
                     _tracks[0].index[i].pts= _tracks[0].index[i].dts;
                }
          }
          VDEO.index[0].intra=AVI_KEY_FRAME;
        }
          break;
    case TRACK_AUDIO:
          printf("Cur audio track :%u\n",nbAudioTrack);
          if(info.SzIndentical ==1 && (ADIO.encoding==WAV_LPCM || ADIO.encoding==WAV_PCM ))
            {
              printf("Overriding size %"LU" -> %"LU"\n", info.SzIndentical,info.SzIndentical*2*ADIO.channels);
              info.SzIndentical=info.SzIndentical*2*ADIO.channels;
            }
            r=indexify(&(_tracks[1+nbAudioTrack]),trackScale,&info,1,&nbo);
            printf("Indexed audio, nb blocks:%u\n",nbo);
            if(r)
            {
                nbo=_tracks[1+nbAudioTrack].nbIndex;
                if(nbo)
                    _tracks[1+nbAudioTrack].nbIndex=nbo;
                else
                    _tracks[1+nbAudioTrack].nbIndex=info.nbSz;
                printf("Indexed audio, nb blocks:%u (final)\n",_tracks[1+nbAudioTrack].nbIndex);
                _tracks[1+nbAudioTrack].scale=trackScale;
                nbAudioTrack++;
            }

            break;
    case TRACK_OTHER:
        r=1;
        break;
  }
  return r;
}
/**
      \fn decodeEsds
      \brief Decode esds atom
*/
uint8_t MP4Header::decodeEsds(void *ztom,uint32_t trackType)
{
adm_atom *tom=(adm_atom *)ztom;
int tag,l;
            // in case of mpeg4 we only take
            // the mpeg4 vol header
            printf("[MP4]Esds atom found\n");

            tom->skipBytes(4);
            tag=0xff;
            while(tag!=Tag_DecSpecificInfo && !tom->isDone())
            {
                    tag=tom->read();
                    l=readPackedLen(tom);
                    printf("\t Tag : %u Len : %u\n",tag,l);
                    switch(tag)
                    {
                            case Tag_ES_Desc:
                                    printf("\t ES_Desc\n");
                                    tom->skipBytes(3);
                                    break;
                            case Tag_DecConfigDesc:
                            {
                                    uint8_t objectTypeIndication=tom->read();
                                    printf("\tDecConfigDesc : Tag %u\n",objectTypeIndication);
                                    if(trackType==TRACK_AUDIO && ADIO.encoding==WAV_AAC)
                                    {
                                      switch(objectTypeIndication)
                                      {
                                          case 0x69:
                                          case 0x6b:
                                          case 0x6d:
											  ADIO.encoding=WAV_MP3;
											  break;
                                          case 226:ADIO.encoding=WAV_AC3;break;
                                          break;

                                      }
                                    }
                                    tom->skipBytes(1+3+4+4);
                                    break;
                            }
                            case Tag_DecSpecificInfo:
                                    printf("\t DecSpecicInfo\n");
                                    switch(trackType)
                                    {
                                        case TRACK_VIDEO: // Video
                                            if(!VDEO.extraDataSize)
                                            {
                                                    VDEO.extraDataSize=l;
                                                    VDEO.extraData=new uint8_t[l];
                                                    fread(VDEO.extraData,VDEO.extraDataSize,1,_fd);
                                            }
                                            break;
                                        case TRACK_AUDIO:
                                            printf("Esds for audio\n");
                                            _tracks[1+nbAudioTrack].extraDataSize=l;
                                            _tracks[1+nbAudioTrack].extraData=new uint8_t[l];
                                            fread(_tracks[1+nbAudioTrack].extraData,
                                            _tracks[1+nbAudioTrack].extraDataSize,1,_fd);
                                            printf("\t %d bytes of extraData\n",(int)l);
                                            break;
                                        default: printf("Unknown track type for esds %d\n",trackType);
                                    }
                            }
            }

    tom->skipAtom();
    return 1;
}
/**
    \fn updateCtts
    \brief compute deltaPtsDts from ctts, needed to create valid mp4 when copying
*/
uint8_t MP4Header::updateCtts(MPsampleinfo *info )
{
    uint32_t scope=info->nbCtts;
    float f;
    if(scope>_videostream.dwLength) scope=_videostream.dwLength;
    printf("[MP4]**************** Updating CTTS **********************\n");
    for(int i=0;i<scope;i++)
    {
        f=(int32_t)info->Ctts[i];
        f/=_videoScale;
        f*=1000000; // us
        f+=_tracks[0].index[i].dts;
        _tracks[0].index[i].pts=(uint64_t)f;
    }

  return 1;
}
//***********************************
MPsampleinfo::MPsampleinfo(void)
{
  memset(this,0,sizeof( MPsampleinfo));
}
MPsampleinfo::~MPsampleinfo()
{
#define MPCLEAR(x) {if(x) delete [] x;x=NULL;}
      MPCLEAR (Co);
      MPCLEAR (Sz);
      MPCLEAR (Sc);
      MPCLEAR (Sn);
      MPCLEAR (SttsN);
      MPCLEAR (SttsC);
      MPCLEAR (Sync);
      MPCLEAR (Ctts);
}

// EOF


