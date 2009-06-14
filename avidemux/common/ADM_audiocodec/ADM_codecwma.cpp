/***************************************************************************
                          ADM_codecwma.cpp  -  description
                             -------------------
        We do also AMR here

    begin                : Tue Nov 12 2002
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
#include "ADM_default.h"
#include <math.h>
#include "ADM_lavcodec.h"
#include "fourcc.h"
#include "DIA_coreToolkit.h"

#include "ADM_coreAudio.h"
#include "ADM_audiocodec/ADM_audiocodec.h"

#define _context ((AVCodecContext *)_contextVoid)

uint8_t scratchPad[SCRATCH_PAD_SIZE];

   uint8_t ADM_AudiocodecWMA::beginDecompress( void )
   {
            _tail=_head=0;
            return 1;
   };
   uint8_t ADM_AudiocodecWMA::endDecompress( void )
   {
          _tail=_head=0;
          return 1;
   };

 ADM_AudiocodecWMA::ADM_AudiocodecWMA(uint32_t fourcc,WAVHeader *info,uint32_t l,uint8_t *d)
       :  ADM_Audiocodec(fourcc)
 {
    _tail=_head=0;

    _contextVoid=(void *)avcodec_alloc_context();
    ADM_assert(_contextVoid);
    // Fills in some values...
    _context->sample_rate = info->frequency;
    _context->channels = info->channels;
    _blockalign=_context->block_align = info->blockalign;
    _context->bit_rate = info->byterate*8;
    switch(fourcc)
    {
      case WAV_WMA:
        _context->codec_id = CODEC_ID_WMAV2;
        break;
      case WAV_QDM2:
        _context->codec_id = CODEC_ID_QDM2;
        break;
      case WAV_AMV_ADPCM:
        _context->codec_id = CODEC_ID_ADPCM_IMA_AMV;
        _blockalign=1;
        break;
      case WAV_NELLYMOSER:
        _context->codec_id = CODEC_ID_NELLYMOSER;
        _blockalign=1;
        break;

      default:
             ADM_assert(0);
    }
    _context->extradata=(uint8_t *)d;
    _context->extradata_size=(int)l;
    printf(" Using %"LU" bytes of extra header data\n",l);
    mixDump((uint8_t *)_context->extradata,_context->extradata_size);

   AVCodec *codec=avcodec_find_decoder(_context->codec_id);
   if(!codec) {GUI_Error_HIG(QT_TR_NOOP("Internal error"), QT_TR_NOOP("Cannot open WMA2 codec."));ADM_assert(0);}
    if (avcodec_open(_context, codec) < 0)
    {
        printf("\n Lavc audio decoder init failed !\n");
        ADM_assert(0);
    }
    if(!_blockalign)
    {
      if(_context->block_align) _blockalign=_context->block_align;
      else
      {
        printf("FFWMA : no blockalign taking 378\n");
        _blockalign=378;
      }
    }
    printf("FFwma init successful (blockalign %d)\n",info->blockalign);
}
 ADM_AudiocodecWMA::~ADM_AudiocodecWMA()
 {
        avcodec_close(_context);
        ADM_dealloc(_context);
        _contextVoid=NULL;
}
/*-------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------*/

uint8_t ADM_AudiocodecWMA::run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut)
{
int out=0;
int max=0,pout=0;
int16_t *run16;
int nbChunk;

        *nbOut=0;
        // Shrink
        if(_head && (_tail+nbIn)*3>ADMWA_BUF*2)
        {
            memmove(_buffer,_buffer+_head,_tail-_head);
            _tail-=_head;
            _head=0;
        }
        //
        ADM_assert(nbIn+_tail<ADMWA_BUF);
        memcpy(_buffer+_tail,inptr,nbIn);
        _tail+=nbIn;
        while(_tail-_head>=_blockalign)
        {
          nbChunk=(_tail-_head)/_blockalign;
          pout=SCRATCH_PAD_SIZE;
          out=avcodec_decode_audio2(_context,(int16_t *)scratchPad,
                                   &pout,_buffer+_head,nbChunk*_blockalign);

          if(out<0)
          {
            printf( " *** WMA decoding error (%u)***\n",_blockalign);
            _head+=1; // Try skipping some bytes
            continue;
          }
          if(pout>=SCRATCH_PAD_SIZE)
          {
            printf("Produced : %u, buffer %u,in%u\n",pout,SCRATCH_PAD_SIZE,_tail-_head);
            ADM_assert(0);
          }
          if(_context->codec_id == CODEC_ID_NELLYMOSER)
          { // Hack, it returns inconsistent size
            out=nbChunk*_blockalign;
          }
          _head+=out; // consumed bytes
          pout>>=1;
          *nbOut+=pout;
          run16=(int16_t *)scratchPad;
          for(int i=0;i<pout;i++)
          {
            *outptr++=((float)run16[i])/32767.;
          }
        }



        return 1;
}

//---
