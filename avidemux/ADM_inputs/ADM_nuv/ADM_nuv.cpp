/***************************************************************************
                          ADM_nuv.cpp  -  description
                             -------------------

                            Handle NUV (nuppel video) file format 

	This format is a bit tricky.

	Audio is simple : 44.1 khz, stereo, pcm
	Video could be :
		- Raw : Nothing to do
		- Jpeg: Okay, let's jpeg handle it
		- jpeg_lzo : Lossless compression after jpg
		- Raw_lzo : Lossless compression on YV412


	Everything is decompressed internally and seen as raw YV12

	Also a "L" compressed video frame means it is a drop.
	In that case, we just send the n-1 frame.

   I added also a new compression scheme X, which means encoded with Xvid
	I that case the video datas are sent uncompressed.

    begin                : Tue Jun 4 2002
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
//#define DEBUG
//#define VERBOSE_FRAME
//#define VERBOSE_SOUND
#define THRESHOLD 8192  //   /44.11*4 to gain the ms shift allowed
                                                 // 8192 ~ 45 ms
														// 4410 ~  25 ms

//#define DEBUG
//_____________________
#include "config.h"
#include <stdio.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef __FreeBSD__
	#include <sys/types.h>
#endif
#include <math.h>

#include "ADM_default.h"
#include "ADM_editor/ADM_Video.h"

#include "ADM_assert.h"

#include "fourcc.h"
#include "ADM_nuv.h"
#include "nuppel.h"
#include "minilzo.h"
/*
extern "C"
{
#include "ADM_nuv/RTjpeg.h"
#include "ADM_nuv/RTjpegN.h"
}*/

#include "RTjpegN.h"

#include "DIA_coreToolkit.h"
#include "DIA_fileSel.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_NUV
#include "ADM_osSupport/ADM_debug.h"

#include "prefs.h"
#include "ADM_userInterfaces/ADM_commonUI/DIA_working.h"

	#define DXFIELD(x) ((rtfileheader *)_nuv_header)->x
	#define DX(x) printf(" "#x" :%d\n",DXFIELD(x));
#include "ADM_assert.h"
typedef struct ChaineD
{
	struct ChaineD  *_next;
	rtframeheader 	*_frame;
	uint16_t				_kf;
	uint64_t				_pos;
}ChaineD;



/*
  	Return size of the given compressed frame
   	if Nuvrec, just plain YV12 size
    	If Xvid, real size


*/
uint8_t  nuvHeader::getFrameSize(uint32_t frame,uint32_t *size) 
{
*size=0;
	if(frame>=(uint32_t)_mainaviheader.dwTotalFrames)
			{
					printf("\n nuv::getsize out of bound!");
					return 0;
			}

		if(!_isXvid)
				{
						*size=(_video_bih.biWidth*_video_bih.biHeight*3)>>1;
						return 1;
				}


   *size=  _videoIndex[frame]._len;	
   return 1;
}

uint8_t  nuvHeader::getFrameNoAlloc(uint32_t framenum,ADMCompressedImage *img)
{
uint64_t 	off;
lzo_uint  	len, out,l;
uint8_t *planes[3];
        if(framenum>= (uint32_t)_videostream.dwLength) return 0;
	l=DXFIELD(width)*DXFIELD(height);

	planes[0]=_old;
	planes[1]=_old+l;
	planes[2]=_old+((l*5)>>2);

	img->flags=AVI_KEY_FRAME;

	#define SWAPUV {	memcpy(img->data,_old,l);	\
					memcpy(img->data+l,_old+l+(l>>2),l>>2); \
					memcpy(img->data+l+(l>>2),_old+l,l>>2);	}

	if((int32_t)framenum>=(int32_t)_mainaviheader.dwTotalFrames)
	{
		printf("\n nuv : out of bound frame !");
		return 0;
	}

	off= _video_bih.biWidth*_video_bih.biHeight;
	// Blackout frame
	if(!_isMyth)
	{
		memset(	_vbufjpg,0,off);
		memset(	_vbuflzo,0,_max);
		memset(_old,0,off);
		memset(_old,128,off>>1);

	}

	// rewind to previous correct frame (i.e. not dropped)
	if(!_isXvid)
		while((framenum)&& (_videoIndex[framenum]._compression=='L')) framenum--;

	// need lzo ?
	switch(_videoIndex[framenum]._compression)
		{
			case 'L':
						if(_isXvid)
							{
                          					img->dataLength=0;
								printf("\n Drop !\n");
        							return 1;
							}
						printf("--> Strange dropped frame at the beginning\n");
						return 1;
						break;
			case 'N':
						if(_isXvid)
							{
                          					img->dataLength=0;
								return 1;
							}
				//	printf("\n nuv black frame : Not HANDLED!!!!");
					uint32_t f;
						f=DXFIELD(width)*DXFIELD(height);
						// black out Y
						memset(img->data,0,f);
						// black out u & v
						memset(img->data+f,128,f>>1);
						img->dataLength=f+(f>>1);
						img->flags=AVI_KEY_FRAME;
						return 1;
						printf("\n Black Frame \n");

					break;

#define READNUV(wptr)				off=_videoIndex[framenum]._pos; \
				fseeko(_fd,off,SEEK_SET); \
				len=_videoIndex[framenum]._len; \
				if(!fread(wptr,len,1,_fd)) \
					{\
						printf("\n nuv : read error"); \
						return 0; \
					}
			case '0': // YV12 : just read it sam
					READNUV(img->data);

					// now swap u & v


					img->dataLength=l+(l>>1);

					memcpy(_vbufjpg,img->data+l,l>>2);
					memcpy(img->data+l,img->data+l+(l>>2),l>>2);
					memcpy(img->data+l+(l>>2),_vbufjpg,l>>2);
					img->flags=AVI_KEY_FRAME;
#ifdef VERBOSE_FRAME
						printf("\n YV12 Frame \n");
#endif

					return 1;
				break;

			case '1': // RTJpeg
				READNUV(_vbuflzo);


				_rtjpeg-> Decompress((int8_t *)_vbuflzo, planes);
				SWAPUV;

				img->dataLength=(l*3)>>1;

				img->flags=AVI_KEY_FRAME;
#ifdef VERBOSE_FRAME

						printf("\n RTJPEG Frame \n");
#endif
				return 1;
				break;

			case '3': // YV12 + LZO

				READNUV(_vbuflzo);
                               if(LZO_E_OK!= lzo1x_decompress( _vbuflzo,len, (uint8_t *)_old,&out, NULL))
				 {
					printf("\n nuv : LZO decompressing error !\n");
					return 0;
				}
				SWAPUV;

				img->dataLength=(l*3)>>1;

				img->flags=AVI_KEY_FRAME;
#ifdef VERBOSE_FRAME

					printf("\n RTJPEG  Frame \n");
#endif
				return 1;
				break;
			case '2': // YV12 + RTJPEG  + LZO

				READNUV(_vbuflzo);
                               if(LZO_E_OK!= lzo1x_decompress( _vbuflzo,len, (uint8_t *)_vbufjpg,&out, NULL))
					 {
                                    		printf("\n nuv : LZO decompressing error !\n"); return 0;
					 }
				//printf("\n Lzo  in : %u out: %u\n",len,out);
				// now jpeg-it
//			#warning fixme
				_rtjpeg-> Decompress((int8_t *)_vbufjpg, planes);

				// now swap u & v
				SWAPUV;

				img->dataLength=(l*3)>>1;
				//printf("\n Lzo  in : %u out: %u\n",len,*framelen);
				img->flags=AVI_KEY_FRAME;
#ifdef VERBOSE_FRAME

					printf("\n RTJPEG +LZO  Frame \n");
#endif
   				return 1;
				break;
			case 'X':
			case ';':
			case '4':
			case 'F':
					if(!_isXvid)
						{
							printf("\n Xvid detected WTF ???\n");
							exit(0);
                         			}
                         		READNUV(img->data);
                            		img->dataLength=  _videoIndex[framenum]._len;
#ifdef DEBUG
					printf("\n xvid : size : %" PRIu32 ,*framelen);
#endif
                                        if(_videoIndex[framenum]._kf)
                                                        img->flags=AVI_KEY_FRAME;
                                                else
                                                        img->flags=0;

#ifdef DEBUG
                             		printf("xvid flags : %" PRIu32 "\n",*flags);
#endif
					return 1;
					break;
			default:
					printf("\n unknown compression :%c\n",
							_videoIndex[framenum]._compression);
					ADM_assert(0);

		}
	ADM_assert(0);
	return (0);
}

uint8_t    nuvHeader::close( void )
{
#warning Memory leak
	//if(_rtjpeg)		delete _rtjpeg;
	_rtjpeg=NULL;
	if(_fd)
 		{
               		fclose(_fd);
             	}
         _fd=NULL;
	#define RMIT(x) if(x) { delete [] x;x=NULL;}
		RMIT(_vbufjpg);
		RMIT(_videoIndex);
		RMIT(_audioIndex);
		RMIT(_rIndex);
		RMIT(_tableIndex);
		RMIT(_vbuflzo);
		RMIT(_old);


	// audio track will be destroyed by editor.. no need to bother

 	return 1;
}
void nuvHeader::Dump( void )
{

	printf("\n NuppelVideo Header Dump :\n");
	printf("************************ :\n");
	printf(" Info    : %s\n",DXFIELD(finfo));
	printf(" version : %s\n",DXFIELD(version));
	printf(" fps     : %f\n",DXFIELD(fps));

	DX(width);
	DX(height);
	DX(videoblocks);
	DX(audioblocks);
	DX(keyframedist);
	aprintf("Myth  : %d \n",_isMyth);
	aprintf("Xvid  : %d \n",_isXvid);
	aprintf("PCM  : %d \n",_isPCM);

}
WAVHeader 		*nuvHeader::getAudioInfo(void )
{
	if(!_audioTrack) return NULL;
	return _audioTrack->getInfo();

};
uint8_t			nuvHeader::getAudioStream(AVDMGenericAudioStream **audio)
{

	*audio=_audioTrack;
	return 1;

};
uint8_t  nuvHeader::setFlag(uint32_t frame,uint32_t flags)
{
	if(flags & AVI_KEY_FRAME)
		   _videoIndex[ frame ]._kf=1      ;
	return 1;
};
uint32_t nuvHeader::getFlags(uint32_t frame,uint32_t *flags)
{
	if(frame==0)
	{
		*flags=AVI_KEY_FRAME;
		return AVI_KEY_FRAME;

	}
	if((uint32_t)frame>=(uint32_t)_mainaviheader.dwTotalFrames)
		{
				printf("\n out of bounds!\n");
				return 0;
		}
	if(!_isXvid)
	{
		*flags=AVI_KEY_FRAME;
		return AVI_KEY_FRAME;
	}
	 if(( _videoIndex[ frame ]._kf)&&(frame<  (uint32_t)_mainaviheader.dwTotalFrames))
		{
       	*flags=AVI_KEY_FRAME;
#ifdef DEBUG
		printf("\n frame %" PRIu32 " is a keyframe\n",frame);
#endif
		return  1;
		}
#ifdef DEBUG
		printf("\n frame %" PRIu32 " is *NOT* a keyframe\n",frame);
#endif

	*flags=0;
	return 1;
};

/*

	Open the nuv file.
	Scan it to rebuild a pseudo index
	allocate lzo & jpg buffer statically



*/
uint8_t    nuvHeader::open(const char *name)
{
uint32_t a=0,v=0,t=0;
uint32_t max=0;
uint8_t sync_met=0;
uint32_t byte_per_frame;
double double_per_frame;
uint8_t cont;
uint32_t rcount=0;

		_fd=fopen(name,"rb");
		if(!_fd) return 0;

		uint32_t o;
		fread(&o,4,1,_fd);
		fseek(_fd,0,SEEK_SET);
		if(fourCC::check(o,(uint8_t *)"ADNV"))
		{
			fclose(_fd);
			_fd=NULL;
			return loadIndex(name);
		}


		fseeko(_fd,0,SEEK_END);
		_filesize=ftello(_fd);
		fseeko(_fd,0,SEEK_SET);

		printf("\n  Filesize : 	%" PRIu64 ,_filesize);

			// init lzo stuff
          if ( lzo_init() != LZO_E_OK )
		    {
					printf("\n nuv: error initializing lzo !\n");
					return 0;
			}

		// now parse header
		rtfileheader *head;
		head=new rtfileheader;
		ADM_assert(head);
		_nuv_header=(void *)head;

		ADM_assert(fread(head,sizeof(rtfileheader),1,_fd));
		if(fourCC::check((uint8_t *)head->finfo,(uint8_t *)"Myth"))
		{
			_isMyth=1;
                        printf("IsMyth : yes\n");

		}
		Dump();
		printf("\n Sizeof frame header : %" PRIu64, sizeof(rtframeheader));
		//
		//
		//

		printf("\n Building NuppelVideo file index :\n");
		rtframeheader frame;

		ChaineD ahead,vhead;
		ChaineD *aqueue,*vqueue,*rqueue;
		ChaineD *n;
		ChaineD rhead;

		uint8_t next_is_kf=0;

		aqueue=&ahead;
		vqueue=&vhead;
		rqueue=&rhead;

		aqueue->_next=vqueue->_next=NULL;

		frame.packetlength=0;

		uint32_t current_audio=0;

		// compute audio duration
		double_per_frame=DXFIELD(fps);
		ADM_assert(double_per_frame);

		// fps -> 1/x = duration of a frame in ms.
		double_per_frame=1.0f/double_per_frame;
		double_per_frame=1000.0f*double_per_frame;

		printf("\n Duration of a frame : %f ms\n",double_per_frame);

		// we go now duration of a frame in millisecond
		// *4 (stero/16bits) *44.1 to have byte per frame
		double_per_frame=double_per_frame*4.0f*(_audio_frequency/1000.);;

		byte_per_frame=(uint32_t )floor(double_per_frame+0.490f);

		printf(" double : %0f\n byte per frame :%" PRIu32 "\n", double_per_frame,byte_per_frame);



		uint64_t next;
		int32_t overshot=0;

		cont=1;
		next=ftello(_fd);
		frame.packetlength=0;
		if(_isMyth)
			_rtjpeg=new RTjpeg();
		else
			_rtjpeg=new baseRTold();

		int w=DXFIELD(width);
		int h=DXFIELD(height);
		int fmt=0;

		_rtjpeg->SetFormat(&fmt);
		_rtjpeg->SetSize(&w,&h);

    		DIA_working *work=new DIA_working (QT_TR_NOOP("Opening Nuppel video"));
		while(  (next<_filesize) && cont )
		{
			if(work->update(  (uint32_t)( next>>8),(uint32_t)(_filesize>>8))) /* 2 Gb * 256 should be enough ... */
			{
                              if(GUI_Question(QT_TR_NOOP("Sure you want to abort ?")))
				{
						// purge aqueue,vqueue & rqueue

								#define PURGE \
								while(aqueue) \
								{\
									vqueue=aqueue; \
									aqueue=aqueue->_next; \
									ADM_dealloc(vqueue); \
								}
								aqueue=ahead._next;
								PURGE;
								aqueue=vhead._next;
								PURGE;
								aqueue=rhead._next;
								PURGE;
								fclose(_fd);
								_fd=NULL;
								return 0;
				}
				else
				{
					delete work;
					work=new DIA_working(QT_TR_NOOP("Opening Nuppel video"));
				}


			}

			if(frame.packetlength) fseeko(_fd,frame.packetlength,SEEK_CUR);

//			printf("\n Position : %x\n",ftell(_fd));
			ADM_assert(fread(&frame,sizeof(rtframeheader),1,_fd));

#ifdef DEBUG
			printf("\n Type  : %c",frame.frametype);
			printf("\n Compr : %c",frame.comptype);
			printf("\n Keyf  : %d",frame.keyframe);
			printf("\n Len   : %d",frame.packetlength);
#endif
/**/

		//	printf("Type: %c Compression : %c len:%u\n",frame.frametype,frame.comptype,frame.packetlength);
			if(!sync_met)
			{
				if(frame.frametype=='R')
				{
					sync_met=1;
					printf("start sync found\n");
				}
				else printf("drop\n");

			}
			switch(frame.frametype)
			{
				case 'R':

						/* Store R tags in rqueue for indexing/ chop by external tools */
						rcount++;
						n=(ChaineD *)ADM_alloc(sizeof(ChaineD));
						ADM_assert(n);
						n->_next=NULL;
						n->_frame=(rtframeheader *)ADM_alloc(sizeof(rtframeheader));
						n->_frame->comptype='R';
						//memcpy(n->_frame,&frame,sizeof(frame));
						n->_pos=ftello(_fd);
						rqueue->_next=n;
						rqueue=n;
						/* /store*/

						double estim;
						uint32_t iestim;

						estim=double_per_frame;
						estim*=v;
						iestim=(uint32_t)floor(estim+0.49);
						iestim-=iestim&3;
#ifdef VERBOSE_SOUND
						printf("\n Audio in stock : %" PRIu32 "\n",current_audio);
						printf(  " Audio computed : %" PRIu32 "\n",iestim);
						printf(  " delta          : %d\n",abs(iestim-current_audio));
#endif
						frame.packetlength=0;
						next_is_kf=1;
		
       				
						if(current_audio>iestim)
							{
#ifdef VERBOSE_SOUND
								printf("\n **** WARNING **** WARNING "
								": Too much audio !!!\n");
#endif


							}
						overshot= current_audio-   iestim; // >0 means too much audio <0 means no enough
						
#ifdef VERBOSE_SOUND
          					printf("\n Frame %" PRIu32 ", overshot %" PRIu32,v,overshot);
#endif
						if(  (overshot < -THRESHOLD)&& _isPCM &&_audioResync)
						{
						// we insert a dummy packet in audio chain to compensate
												
						n=(ChaineD *)ADM_alloc(sizeof(ChaineD));
						ADM_assert(n);
						n->_next=NULL;
						n->_frame=(rtframeheader *)ADM_alloc(sizeof(rtframeheader));
						n->_frame->packetlength=THRESHOLD;
#ifdef VERBOSE_SOUND
          					printf("\n Added %" PRIu32 " bytes",THRESHOLD);
#endif

						n->_frame->comptype='R';
						//memcpy(n->_frame,&frame,sizeof(frame));
						n->_pos=ftello(_fd);
						aqueue->_next=n;
						aqueue=n;
						a++;
						current_audio+=THRESHOLD;
						overshot+=THRESHOLD;

                     }
						break;

						break;
				case 'D':
								// jpeg headers, link them
								if(frame.comptype=='R')
								{
										
										uint64_t old;
										uint8_t *buffer;

										old=ftello(_fd);
										buffer=new uint8_t[frame.packetlength];
										ADM_assert(buffer);

										_lzo_pos=old;
										_lzo_size=frame.packetlength;

                                   						fread(buffer, frame.packetlength,1,_fd);
										printf("\n Initializing jpeg table with %u bytes\n",frame.packetlength);
										_rtjpeg->InitLong((char *)buffer, DXFIELD(width), DXFIELD(height) );
										 _jpegHeaderFound=1;
										fseeko(_fd,old,SEEK_SET);
										delete [] buffer;
								}
						break;
				case 'N'	: // FFV1 audio config
						uint64_t old3;
						uint32_t ext;
						old3=ftello(_fd);
						fread(&_audio_frequency,4,1,_fd);
						fread(&ext,4,1,_fd);
						if(ext)
						{
							printf("\n *****FFV1 Audio extension present but ignored !\n");
						}
						printf("\n FFV1 audio frequency : %" PRIu32 "\n:",_audio_frequency);
						fseeko(_fd,old3,SEEK_SET);
						// Now we recompute the audio # of bytes to keep sync_met
						// compute audio duration
						double_per_frame=DXFIELD(fps);
						// fps -> 1/x = duration of a frame in ms.
						double_per_frame=1.0f/double_per_frame;
						double_per_frame=1000.0f*double_per_frame;
						printf("\n Duration of a frame : %f ms\n",double_per_frame);
						double_per_frame=double_per_frame*4.0f*(_audio_frequency/1000.);;
						byte_per_frame=(uint32_t )floor(double_per_frame+0.490f);
						printf(" double : %0f\n byte per frame :%" PRIu32 "\n", double_per_frame,byte_per_frame);
						break;
				case 'M'	: // FFV1 video config
						uint64_t old2;
						old2=ftello(_fd);

						fread(&_ffv1_fourcc,4,1,_fd);
						fread(&_ffv1_extraLen,4,1,_fd);
						printf("\n FFV1 detected:");
						fourCC::print(_ffv1_fourcc);
						fourCC::print(_ffv1_extraLen);
						printf("\n");
						if(_ffv1_extraLen!=(uint32_t)(frame.packetlength-8))
						{
							printf("extra ;  %" PRIu32 " , packet %" PRIu32 "\n",_ffv1_extraLen,
											frame.packetlength);
							ADM_assert(0);

						}
						// some codecs need extra data to be initialized properly
						if(_ffv1_extraLen)
						{
							_ffv1_extraData=new uint8_t[_ffv1_extraLen];
							ADM_assert(_ffv1_extraData);
							fread(_ffv1_extraData,_ffv1_extraLen,1,_fd);

						}
						_isFFV1=1;
						_isXvid=1;
						fseeko(_fd,old2,SEEK_SET);
						break;
				case 'S':
				case 'T':
						break;
				case 'X':
						// myth stuff
						printf(" Myth info header\n");
						_mythData=new mythHeader;
						if(frame.packetlength!=sizeof(mythHeader))
							{
                                                            GUI_Error_HIG(QT_TR_NOOP("Size mismatch"), QT_TR_NOOP("Expect a crash."));

							}
						uint64_t old;
						old=ftello(_fd);
						fread(_mythData, frame.packetlength,1,_fd);
						_dump();
						fseeko(_fd,old,SEEK_SET);
						if(fourCC::check(_mythData->video_fourcc,(uint8_t *)"DIVX"))
						{
							printf("\n looks like mpeg4 video to me\n");
						 	_isXvid=1;
						 }
						 if(!fourCC::check(_mythData->audio_fourcc,(uint8_t *)"RAWA"))
						 {
						 	_isPCM=0;
						 }
						 // update for sync
						 _audio_frequency=_mythData->audio_sample_rate;
						break;
				// Video Chunk !!
				case 'V':
						if(!sync_met) break;
						n=(ChaineD *)ADM_alloc(sizeof(ChaineD));
						ADM_assert(n);
						n->_next=NULL;
						n->_frame=(rtframeheader *)ADM_alloc(sizeof(rtframeheader));
						memcpy(n->_frame,&frame,sizeof(frame));
						n->_pos=ftello(_fd);
						 n->_kf=0;
						if(next_is_kf)
							{
		                          next_is_kf=0;
									 n->_kf=1;
#ifdef DEBUG
									printf("\n**********************************KF*******************\n");
#endif
							}
						vqueue->_next=n;
						vqueue=n;
						v++;
						break;
				case 'A': 	
						uint32_t alen;
						if(!sync_met) break;

						// if it !PCM we take it as is
						if(!_isPCM)
						{
							n=(ChaineD *)ADM_alloc(sizeof(ChaineD));
							ADM_assert(n);
							n->_next=NULL;
							n->_frame=(rtframeheader *)ADM_alloc(sizeof(rtframeheader));
							memcpy(n->_frame,&frame,sizeof(frame));
							alen= n->_frame->packetlength;
							n->_pos=ftello(_fd);
							aqueue->_next=n;
							aqueue=n;
							a++;
							current_audio+=alen; //frame.packetlength;
							break;
						
						}
						if(overshot>THRESHOLD)
							{
#ifdef VERBOSE_SOUND
          					printf("\n Skipped  %" PRIu32 " bytes",frame.packetlength);
#endif

									overshot-=frame.packetlength;
									break;
							}
						n=(ChaineD *)ADM_alloc(sizeof(ChaineD));
						ADM_assert(n);
						n->_next=NULL;
						n->_frame=(rtframeheader *)ADM_alloc(sizeof(rtframeheader));


						memcpy(n->_frame,&frame,sizeof(frame));


						alen= n->_frame->packetlength;

						n->_pos=ftello(_fd);
						aqueue->_next=n;
						aqueue=n;
						a++;
						current_audio+=alen; //frame.packetlength;
						break;
				default:
//					ADM_assert(0);
					printf("\n Type unknown : %c %d\n",frame.frametype,frame.frametype);
					cont=0;
					break;


			}

               next=frame.packetlength;
				next+=ftello(_fd);
		}

						double estim;
						uint32_t iestim;

						estim=double_per_frame;
						estim*=v;
						iestim=(uint32_t)floor(estim+0.49);
						iestim-=iestim&3;
#ifdef VERBOSE_SOUND
						printf("\n FINAL Audio in stock : %" PRIu32 "\n",current_audio);
						printf(  " Audio computed : %" PRIu32 "\n",iestim);
						printf(  " delta          : %d\n",abs(iestim-current_audio));
#endif
						if(current_audio<=iestim)
							{
							// we insert a dummy packet in audio chain to compensate

						n=(ChaineD *)ADM_alloc(sizeof(ChaineD));
						ADM_assert(n);
						n->_next=NULL;
						n->_frame=(rtframeheader *)ADM_alloc(sizeof(rtframeheader));
						n->_frame->packetlength=iestim-current_audio;
						n->_frame->comptype='R';
						//memcpy(n->_frame,&frame,sizeof(frame));
						n->_pos=ftello(_fd);
						aqueue->_next=n;
						aqueue=n;
						a++;
						current_audio=iestim;
    					}

        printf("\n Computed audio : %" PRIu32 " \n",current_audio);
		printf("\n Scanning completed.\n");
		printf(" video : %" PRIu32 "",v);
		printf(" audio : %" PRIu32 "",a);
		printf(" jpegT : %" PRIu32 "",t);

		printf("\n Collapsing index....\n");

		// allocate linear index
		_videoIndex=new nuvIndex[v];
		_audioIndex=new nuvIndex[a];
		_tableIndex=new nuvIndex[t];
		_rIndex=new nuvIndex[rcount];
		_rcount=rcount;

		ADM_assert(_videoIndex);
		ADM_assert(_audioIndex);
		ADM_assert(_tableIndex);
		
		ChaineD *p;
		uint32_t kf=0;

		// first scan RT table
      		n=vhead._next;
		for(uint32_t i=0;i<v-t;i++)
		{
			ADM_assert(n);
			_videoIndex[i]._pos=n->_pos;	
			_videoIndex[i]._len=n->_frame->packetlength;
			if( _videoIndex[i]._len>max) max=_videoIndex[i]._len;
			_videoIndex[i]._compression=n->_frame->comptype;


			if(   _videoIndex[i]._compression=='X') _isXvid=1;

			if(  n->_kf)
              {
				_videoIndex[i]._kf=1;
				kf++;
				}
				else	     _videoIndex[i]._kf=0;


			p=n;
			n=n->_next;
			ADM_dealloc(p->_frame);
			ADM_dealloc(p);
		}

		// Scan rframe chain
		printf("\n Found %d sync point \n",_rcount);
       		n=rhead._next;
		uint32_t i;
		for(  i=0;i<rcount;i++)
		{
			ADM_assert(n);
			_rIndex[i]._pos=n->_pos;
			_rIndex[i]._len=n->_frame->packetlength;
			_rIndex[i]._compression=n->_frame->comptype;
			p=n;
			n=n->_next;
			ADM_dealloc(p->_frame);
			ADM_dealloc(p);
		}

			// Scan audio chain

       		n=ahead._next;
		for(  i=0;i<a;i++)
		{
			ADM_assert(n);
			_audioIndex[i]._pos=n->_pos;
			_audioIndex[i]._len=n->_frame->packetlength;
			_audioIndex[i]._compression=n->_frame->comptype;
			p=n;
			n=n->_next;
			ADM_dealloc(p->_frame);
			ADM_dealloc(p);
		}
		printf("\n Index collapsed, found %" PRIu32 " keyframes\n",kf);
		//
		//	Build generic header so that client can use it...
		//
		//
		_max=max;
		_vbuflzo=new uint8_t[max];
		_vbufjpg=new uint8_t[DXFIELD(width)*DXFIELD(height)];


		if(a==0)
 			_isaudiopresent=0;
		else
			_isaudiopresent=1;
      		_isvideopresent=1;

#define CLR(x)              memset(& x,0,sizeof(  x));

               CLR( _videostream);
               CLR(  _mainaviheader);

    		_videostream.dwScale=1000;
              	_videostream.dwRate=(uint32_t)floor(DXFIELD(fps)*1000);

		double msec;

		msec=DXFIELD(fps);
		msec=1.0f/msec;
		msec=msec*1000000.0f;

              _mainaviheader.dwMicroSecPerFrame=(uint32_t)floor(msec+0.49f);;     // 25 fps hard coded
              _videostream.fccType=fourCC::get((uint8_t *)"vids");
              memset( &_video_bih,0,sizeof(_video_bih));
               _video_bih.biBitCount=24;
		if(!_isXvid)
			{
              			_videostream.fccHandler=fourCC::get((uint8_t *)"YV12");
              			_video_bih.biCompression=0;
            		}
		else
			{
				if(_isFFV1)
					_videostream.fccHandler=_ffv1_fourcc;
				else
	         	     		_videostream.fccHandler=fourCC::get((uint8_t *)"XVID"); // pseudo four CC to for xvid interlaced
	              		_video_bih.biCompression=_videostream.fccHandler;
			}

              _videostream.dwLength= _mainaviheader.dwTotalFrames=(v-t); // ??
               _videostream.dwInitialFrames= 0;
               _videostream.dwStart= 0;
               _video_bih.biWidth=_mainaviheader.dwWidth=DXFIELD(width) ;
               _video_bih.biHeight=_mainaviheader.dwHeight=DXFIELD(height);
		if(a)
			{
				_audioTrack=new nuvAudio(_audioIndex,a,_fd,_audio_frequency,_mythData);
			}
#ifdef DEBUG

	for(uint32_t i=0;i<(uint32_t)(v-t);i++)
		{
			printf("frame %" PRIu32 "  : kf : %d	\n",	i,_videoIndex[i]._kf);
		}

#endif
	       _old=new uint8_t [ DXFIELD(height)*DXFIELD(width)*3]; // too much
	       delete work;
#if 1
		{ unsigned int autoidx = 0;
		  char *mname=NULL;
			prefs->get(FEATURE_TRYAUTOIDX,&autoidx);
			if( autoidx ){
				mname = (char*)ADM_alloc(strlen(name)+strlen(".idx")+1);
				ADM_assert(mname);
				sprintf(mname,"%s.idx",name);
				if( saveIndex( mname,name) == 1 ){
					ADM_dealloc(mname);
					return 1;
				}
				ADM_dealloc(mname);
			}
                        if(GUI_Question(QT_TR_NOOP("Do you want to save an index ?"))){
				GUI_FileSelWrite("Nuv index to save..",&mname);
				if(mname){
					saveIndex( mname,name);
					ADM_dealloc(mname);
				}
			}
		}
#endif
	       return 1;

}
// Constructor , does nothing except insure null pointers
nuvHeader::nuvHeader(void )
{
uint32_t sync;

_audioResync=1;
_videoIndex=NULL;
_audioIndex=NULL;
_tableIndex=NULL;
_rIndex=NULL;

_fd=NULL;
_vbuflzo=NULL;
_vbufjpg=NULL;
_audioTrack=NULL;
_isXvid=0;
_isFFV1=0;
_isMyth=0;
_max=0;
_mythData=NULL;
_isPCM=1;
_jpegHeaderFound=0;
_rtjpeg=NULL;
_old=NULL;
_lzo_pos=_lzo_size=0;
_rcount=0;
_ffv1_fourcc=0;
_ffv1_extraLen=0;
_ffv1_extraData=NULL;
_audio_frequency=44100;
if(prefs->get(FEATURE_DISABLE_NUV_RESYNC,&sync))
{
	if(sync) 
	{
		printf("******** AUDIO RESYNC DISABLED *************\n");
		_audioResync=0;
	}
}

}
nuvHeader::~nuvHeader( )
{
	close();
}

void nuvHeader::_dump( void )
{
	if(!_mythData) return ;
#ifdef DEBUG
	#define VBS(x) printf(#x" : %s\n",_mythData->x);
	#define VBI(x) printf(#x" : %d\n",_mythData->x);
	uint32_t acc,vcc;
	VBI(version);
	vcc=(uint32_t )_mythData->video_fourcc;
	acc=(uint32_t )_mythData->audio_fourcc;
	
	

	printf(" Video : %x",vcc);fourCC::print(_mythData->video_fourcc);printf("\n");
	printf(" Audio : %x ",acc);fourCC::print(_mythData->audio_fourcc);printf("\n");
	
	

	VBI(audio_sample_rate);
	VBI(audio_bits_per_sample);
	VBI(audio_channels);
	VBI(audio_compression_ratio);
	VBI(audio_quality);
	
	VBI(rtjpeg_quality);
	VBI(rtjpeg_luma_filter);
	VBI(rtjpeg_chroma_filter);

	VBI(lavc_bitrate);
	VBI(lavc_qmin);
	VBI(lavc_qmax);



#endif


}
/**
		Save a quick index for easy access
*/
uint8_t nuvHeader::saveIndex( const char *name,const char *org)
{
	FILE *fd;
	uint32_t j=0;

	fd=fopen(name,"wb");
	if(!fd)
	{
		printf("\n Error writing file\n");
		return 0;
	}
	fprintf(fd,"ADNV\n"); // mark it as a avidemux index
	fprintf(fd,"File: %s\n",org ); // mark it as a avidemux index
	fprintf(fd,"wh: %" PRIu32 " %" PRIu32 "\n",DXFIELD(width), DXFIELD(height) ); // mark it as a avidemux index
	fprintf(fd,"fps: %" PRIu32 "\n",_videostream.dwRate);
	fprintf(fd,"Lzo Pos:%" PRIu64 "\n",_lzo_pos);
	fprintf(fd,"Lzo Size:%" PRIu64 "\n",_lzo_size);
	fprintf(fd,"\n");
	fprintf(fd,"Myth:%d\n",_isMyth);
	fprintf(fd,"Xvid:%d\n",_isXvid);
	fprintf(fd,"FFV1:%d\n",_isFFV1);
	fprintf(fd,"ff4c:%x\n",_ffv1_fourcc);
	fprintf(fd,"extr:%x\n",_ffv1_extraLen);
	if(_ffv1_extraLen)
	{
		for(uint32_t i=0;i<_ffv1_extraLen;i++)
			fprintf(fd,"%02x ",_ffv1_extraData[i]);
		fprintf(fd,"\n");
	}
	fprintf(fd,"PCM:%d\n",_isPCM);
	fprintf(fd,"Fq:%d\n",_audio_frequency);
	fprintf(fd,"%" PRIu32 " video frames\n",_mainaviheader.dwTotalFrames);
	for( int32_t j=0;j<_mainaviheader.dwTotalFrames;j++)
	{
		fprintf(fd,"Comp:%c  Pos :%" PRIu64 " Size:%" PRIu32 " Kf:%" PRIu32 "\n",
				_videoIndex[j]._compression,
				_videoIndex[j]._pos,
				_videoIndex[j]._len,
				_videoIndex[j]._kf);

	}
	if(_audioTrack)
	{
		uint32_t nbc=0;
		_audioTrack->getNbChunk(&nbc);
		fprintf(fd,"Audio : %" PRIu32 " chunks\n",nbc);
		for(  j=0;j<nbc;j++)
		{
			fprintf(fd,"Comp:%c  Pos :%" PRIu64 " Size:%" PRIu32 "\n",
				_audioIndex[j]._compression,
				_audioIndex[j]._pos,
				_audioIndex[j]._len
				);

		}
	}
	// Save sync points

		fprintf(fd,"RSync : %" PRIu32 " chunks\n",_rcount);
		for(  j=0;j<_rcount;j++)
		{
			fprintf(fd,"Comp:%c  Pos :%" PRIu64 " \n",
				_rIndex[j]._compression,
				_rIndex[j]._pos
				);

		}
	fclose(fd);


	return 1;
}
/**
		Load  as quick index for easy access
*/
extern uint8_t mk_hex(uint8_t a,uint8_t b);

uint8_t nuvHeader::loadIndex( const char *name)
{
char str[1024];
char filename[1024];
char filename2[1024];
uint32_t nb=0,w,h;
FILE *fd;


     if ( lzo_init() != LZO_E_OK )
		    {
					printf("\n nuv: error initializing lzo !\n");
					return 0;
			}
	fd=fopen(name,"rt");
	if(!fd)
	{
		printf("\n could not open %s file..\n",name);
	 	return 0;
	 }

	fscanf(fd,"%s\n",str);
	if(strcmp(str,"ADNV"))
	{
		printf("wrong filetype!\n");
		fclose(fd);
		return 0;
	}
	fscanf(fd,"File: %s\n",filename ); // mark it as a avidemux index

	_fd=fopen(filename,"rb");
	if(!_fd)
	{
		char *p;
		printf("\n could not open %s file..\n",filename);
		/* check for index file in dirname(name) */
		strncpy(filename2,name,sizeof(filename2));
		filename2[sizeof(filename2)-1] = '\0';
		if( (p = rindex(filename2,'/')) ){
			*(++p) = '\0';
		}else{
			filename2[0] = '\0';
		}
		if( (p = rindex(filename,'/')) ){
			p++;
		}else{
			p = filename;
		}
		strncat(filename2,p,sizeof(filename2)-strlen(filename2));
		filename2[sizeof(filename2)-1] = '\0';
		if( !strncmp(filename,filename2,sizeof(filename2)) ){
			/* we don't have a second filename */
			fclose(fd);
			return 0;
		}
		_fd=fopen(filename2,"rb");
		if(!_fd){
			printf(" could not open %s file..\n", filename2);
			fclose(fd);
	 		return 0;
		}
		printf(" using %s instead..\n", filename2);
	 }

	uint32_t fps;

	printf("\n ******** Nuppel index detected **********\n");

	fscanf(fd,"wh: %" SCNu32 " %" SCNu32 "\n",&w,&h ); // mark it as a avidemux index
	fscanf(fd,"fps: %" SCNu32 "\n",&fps );
	fscanf(fd,"Lzo Pos:%" SCNu64 "\n",&_lzo_pos);
	fscanf(fd,"Lzo Size:%" SCNu64 "\n\n",&_lzo_size);
	
	fscanf(fd,"Myth:%"SCNu8"\n",&_isMyth);
	fscanf(fd,"Xvid:%"SCNu8"\n",&_isXvid);
	fscanf(fd,"FFV1:%"SCNu8"\n",&_isFFV1);
	fscanf(fd,"ff4c:%x\n",&_ffv1_fourcc);	
	
	fscanf(fd,"extr:%x\n",&_ffv1_extraLen);
	if(_ffv1_extraLen)
	{
		char *start;
		char *str=new char[_ffv1_extraLen*3+4];
		
		fgets(str,_ffv1_extraLen*3+3,fd);
		
		_ffv1_extraData=new uint8_t[_ffv1_extraLen];
		
		start=str;
		for(uint32_t i=0;i<_ffv1_extraLen;i++)
		{
			_ffv1_extraData[i]=mk_hex(*start,*(start+1));
			start+=3;
		}
		delete(str);
		
	}
	
	fscanf(fd,"PCM:%" SCNu8 "\n",&_isPCM);
	fscanf(fd,"Fq:%d\n",&_audio_frequency);
	fscanf(fd,"%" SCNu32 " video frames\n",&nb);

	aprintf("fps			: %" PRIu32 "\n",fps );
	aprintf("Lzo Pos			:%" PRIu64 " Size:%" PRIu64 "\n",_lzo_pos,_lzo_size);
	aprintf("Myth			:%d\n",_isMyth);
	aprintf("Xvid			:%d\n",_isXvid);
	aprintf("FFV1			:%d (",_isFFV1);
	fourCC::print(_ffv1_fourcc); aprintf(")\n");
	aprintf("PCM			:%d\n",_isPCM);
	aprintf("%" PRIu32 " video frames\n",nb);


	rtfileheader *head=new rtfileheader;
	memset(head,0,sizeof(*head));
	_nuv_header=(void *)head;
	head->width=w;
	head->height=h;
	head->fps=fps/1000.;

	memset(&_videostream,0,sizeof(_videostream));
 	memset( &_video_bih,0,sizeof(_video_bih));

	_videostream.dwScale=1000;
	_videostream.dwRate=fps;
	_videostream.dwLength=_mainaviheader.dwTotalFrames=nb;
         _video_bih.biBitCount=24;
	 _video_bih.biWidth=_mainaviheader.dwWidth=DXFIELD(width) ;
         _video_bih.biHeight=_mainaviheader.dwHeight=DXFIELD(height);

	_videoIndex=new nuvIndex[nb+1];
	memset(_videoIndex,0,sizeof(nuvIndex)*nb);
	aprintf("found %" PRIu32 " video frames\n",nb);
	// loop to read video
	for(uint32_t j=0;j<nb;j++)
	{
		fgets(str,1000,fd);
		//printf("%s",str);
		char compress = 0;
		sscanf(str,"Comp:%c Pos :%" SCNu64 " Size:%" SCNu32 " Kf:%" SCNu32 "\n",
				//&(_videoIndex[j]._compression),
				&compress,
				&(_videoIndex[j]._pos),
				&(_videoIndex[j]._len),
				&(_videoIndex[j]._kf));
		_videoIndex[j]._compression = compress;
/*
		printf("Comp:%c  Pos :%" PRIu64 " Size:%" PRIu32 " Kf:%u\n",
				(_videoIndex[j]._compression),
				(_videoIndex[j]._pos),
				(_videoIndex[j]._len),
				(_videoIndex[j]._kf));
*/
	}

		aprintf("Initializing RTjpeg with %" PRIu32 " x %" PRIu32 " image\n",w,h);
		int ww=w,hh=h;
		int fmt=0;


		if(_isMyth)
		{
			_rtjpeg=new RTjpeg();
			aprintf("Rtjpeg new type..\n");
		}
		else
		{
			_rtjpeg=new baseRTold();
		}
		_rtjpeg->SetFormat(&fmt);
		_rtjpeg->SetSize(&ww,&hh);


		// if it is not xvid and not myth initialize RTjpeg
		if(!_isXvid && !_isMyth)
		{
			aprintf("Rtjpeg old type..\n");
			ADM_assert(_lzo_size);
			//uint64_t pos;

			fseeko(_fd,_lzo_pos,SEEK_SET);
			fread(str,_lzo_size,1,_fd);
			_rtjpeg->InitLong((char *)str, DXFIELD(width), DXFIELD(height) );
			printf("Jpeg : %" PRIu64 ", %" PRIu64 "\n",_lzo_pos,_lzo_size);
		}


		if(_isXvid)
		{
			if(_isFFV1)
			{				 
				 _video_bih.biCompression=_videostream.fccHandler=_ffv1_fourcc;
			}
			else
			{
       			        _video_bih.biCompression=_videostream.fccHandler=fourCC::get((uint8_t *)"XVID");
				 // pseudo four CC to for xvid interlaced
			}
		}
		else
		{
 				_videostream.fccHandler=fourCC::get((uint8_t *)"YV12");
              	 		_video_bih.biCompression=0;
		}


		_vbuflzo=new uint8_t[DXFIELD(width)*DXFIELD(height)];
		_vbufjpg=new uint8_t[DXFIELD(width)*DXFIELD(height)];
		_old=new uint8_t [ DXFIELD(height)*DXFIELD(width)*3]; // too much
		memset(_old,0,DXFIELD(height)*DXFIELD(width)*3);
	 // time to do the audio
	uint32_t nbc=0;
		aprintf("\n Reading audio...\n");
		fgets(str,1000,fd);
		printf("%s\n",str);
		sscanf(str,"Audio : %" SCNu32 " chunks\n",&nbc);
		aprintf("%" PRIu32 " audio chunk\n",nbc);

			_audioIndex=new nuvIndex[nbc+1];
			memset(_audioIndex,0,sizeof(nuvIndex)*nbc);

		for(uint32_t j=0;j<nbc;j++)
		{
			char compress = 0;
			fscanf(fd,"Comp:%c  Pos :%" SCNu64 " Size:%" SCNu32 "\n",
				&compress,
				&_audioIndex[j]._pos,
				&_audioIndex[j]._len
				);
				_audioIndex[j]._compression = compress;
		}
                // Build a fake audio header...
                mythHeader hdr;
                memset(&hdr,0,sizeof(hdr));
                if(_isPCM) hdr.audio_fourcc=fourCC::get((uint8_t *)"RAWA");
                  else     hdr.audio_fourcc=WAV_MP3;
                hdr.audio_bits_per_sample=16;;
                hdr.audio_sample_rate=_audio_frequency; 
                hdr.audio_channels=2;;
                //
		_audioTrack=new nuvAudio(_audioIndex,nbc,_fd,_audio_frequency,&hdr);
		_isaudiopresent=1;
		_isvideopresent=1;
		_max=DXFIELD(width)*DXFIELD(height);

		fseeko(_fd,0,SEEK_END);
		_filesize=ftello(_fd);
		fseeko(_fd,0,SEEK_SET);

	fclose(fd);
	Dump();
	return 1;
}
