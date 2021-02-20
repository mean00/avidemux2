/***************************************************************************
    \file  ADM_ffVp9
    \brief Decoders using lavcodec
    \author mean & all (c) 2017
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stddef.h>

#include "ADM_default.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_hwAccel.h"
#include "ADM_codecFFVP9.h"

#define xlog(...) {}

/**
 * \fn ctor
 */
decoderFFVP9::decoderFFVP9 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
    : decoderFFSimple(w,h,fcc,extraDataLen,extraData,bpp,true)
{
    _parserContext=NULL;
    decoderMultiThread();

    if (_context && _usingMT && (codec->capabilities & AV_CODEC_CAP_SLICE_THREADS))
    {
        _context->thread_count = _threads;
        _context->thread_type = FF_THREAD_SLICE;    // this is important! the default FF_THREAD_FRAME wont work with Avidemux
    }

    if(!finish())
        return;

    _parserContext=av_parser_init(AV_CODEC_ID_VP9);
    if(!_parserContext)
        _initCompleted=false;
}
/**
 * \fn dtor
 */
decoderFFVP9::~decoderFFVP9 ()
{
    if(_parserContext)
    {   
        av_parser_close(_parserContext);
        _parserContext=NULL;
    }
}
/**
 * \fn uncompress
 * \brief not sure this is correct, we run the parser and feed everything to libavcodec
 */
bool    decoderFFVP9::uncompress (ADMCompressedImage * in, ADMImage * out)
{
    uint8_t *parsedPointer=NULL;
    int      parsedLen=0;
    bool result=false;
    // run parser on it..
    int offset=0;
    xlog("Parse %d\n",in->dataLength);
    while(offset<in->dataLength)
    {
        int bufSize=in->dataLength-offset;
        if(_drain) bufSize=0;
        int r=av_parser_parse2( _parserContext, _context,
                                        &parsedPointer, &parsedLen, 
                                        in->data+offset, bufSize,
                                        in->demuxerPts, in->demuxerDts, -1);
        if(r<=0 || !parsedPointer)
            break;
        offset+=r;
        xlog("Got pic\n");
        ADMCompressedImage dummy;
        dummy=*in;
        dummy.data=parsedPointer;
        dummy.dataLength=parsedLen;
        result=decoderFFSimple::uncompress(&dummy,out);
    }
    xlog("/Parse\n");
    return result;
}
// eof
