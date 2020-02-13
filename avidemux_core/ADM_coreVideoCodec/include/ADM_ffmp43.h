/***************************************************************************
  \name    ADM_ffmp43.h 
  \brief   Interface to libavcodec
 
    copyright            : (C) 2002/2016 by mean
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
#pragma once

#include "ADM_coreVideoCodec6_export.h"
#include "ADM_codec.h"
#include "ADM_paramList.h"
extern "C" 
{
#include "libavcodec/avcodec.h"
#include "libavutil/mem.h"
}
#include "ADM_hwAccel.h"

extern "C"
{
typedef void (AV_FATAL_HANDLER)(const char *why,int fileno,const char *filewhereitcrashed);
extern void av_setFatalHandler(AV_FATAL_HANDLER *func);
extern enum AVPixelFormat ADM_FFgetFormat(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt);
}
/**
 * 
 */
ADM_COREVIDEOCODEC6_EXPORT void ADM_lavInit(void);
ADM_COREVIDEOCODEC6_EXPORT void ADM_lavDestroy(void);

/**
    \class decoderFF
    \brief Base class for lavcodec based decoder
*/
class ADM_COREVIDEOCODEC6_EXPORT decoderFF:public decoders
{
friend class ADM_acceleratedDecoderFF;
public:
            typedef struct 
             {
                     bool swapUv;
             } decoderFF_param_t;    

protected:
    static const decoderFF_param_t defaultConfig;
    static const ADM_paramList decoderFF_param_template[];

           uint8_t      _allowNull;
           bool         hurryUp;
           bool         _initCompleted;
           bool         _drain;
           bool         _done;
           bool         _keepFeeding;
           bool         _endOfStream;
           bool         _setBpp;
           bool         _setFcc;
           int          codecId;
           uint8_t      _refCopy;
           uint32_t     _bpp;
           AVCodecContext *_context;
           uint8_t      *_extraDataCopy;
           int          _extraDataLen;
           uint32_t     _fcc;
           AVFrame      *_frame;
           uint32_t     _gmc;
           uint32_t     _usingMT;
           uint32_t     _threads;
           ADM_acceleratedDecoderFF *hwDecoder;
           decoderFF_param_t decoderFF_params;

protected:
           uint32_t     frameType (void);
           uint8_t      clonePic (AVFrame * src, ADMImage * out);
           void         decoderMultiThread ();
           uint32_t     admFrameTypeFromLav (AVFrame *pic);

public:
                        decoderFF (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
        virtual         ~ decoderFF ();
        virtual bool initialized(void);
            bool        setHwDecoder(ADM_acceleratedDecoderFF *h)
                        {
                            if(h)
                            {
                                hwDecoder=h;
                                return true;
                            }
                            return false;
                        }
            ADM_acceleratedDecoderFF    *getHwDecoder() {return hwDecoder;}
        virtual bool    dontcopy (void)
        {    
          if(hwDecoder)
            return hwDecoder->dontcopy();
          return true;
        }
        virtual bool uncompress (ADMCompressedImage * in, ADMImage * out);
        virtual bool getConfiguration(CONFcouple **conf);
        virtual bool resetConfiguration();
        virtual bool setConfiguration(CONFcouple * conf);
        virtual bool setParam (void);
        virtual bool bFramePossible (void)
        {
          return false;
        }
        virtual bool decodeHeaderOnly (void);
        virtual bool decodeFull (void);
      //  virtual uint32_t getSpecificMpeg4Info (void);
        virtual uint8_t getPARWidth (void);
        virtual uint8_t getPARHeight (void);
        virtual bool decodeErrorHandler(int code);
        virtual bool keepFeeding(void) { return _keepFeeding; }
        virtual bool endOfStreamReached(void) { return _endOfStream; }
        virtual void setEndOfStream(bool reached) { _endOfStream=reached; }
        virtual bool getDrainingState(void) { return _drain; }
        virtual void setDrainingState(bool OnOff) { _drain=OnOff; }
        virtual bool getDrainingInitiated(void) { return _done; }
        virtual void setDrainingInitiated(bool initiated) { _done=initiated; }
        virtual bool flush(void);
        virtual const char *getDecoderName(void) 
                          {
                              if(hwDecoder)
                                  return hwDecoder->getName();
                              return "Lavcodec";
                          }
        // for hw accel
        AVFrame *getFramePointer() {return _frame;}
};

#define FF_SIMPLE_DECLARE(x,y) \
        class x:public decoderFF \
        {\
            protected: \
            public: \
                    x (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp); \
                    y \
        };

FF_SIMPLE_DECLARE(decoderFFDiv3,)
FF_SIMPLE_DECLARE(decoderFFDV,)
FF_SIMPLE_DECLARE(decoderFFhuff,)
FF_SIMPLE_DECLARE(decoderFF_ffhuff,)
FF_SIMPLE_DECLARE(decoderFFficv, bool uncompress(ADMCompressedImage *in, ADMImage *out);)
FF_SIMPLE_DECLARE(decoderFFPng,)
FF_SIMPLE_DECLARE(decoderFFMpeg4, bool uncompress(ADMCompressedImage *in, ADMImage *out);
                                // mpeg4 can have B-frame
                                virtual bool bFramePossible (void)   {  return 1;   })
FF_SIMPLE_DECLARE(decoderFFMpeg12,
                                // mpeg1/2 can have B-frame
                                virtual bool bFramePossible (void)   {  return 1;
                                  })
FF_SIMPLE_DECLARE(decoderFFH264,
                                 virtual bool bFramePossible (void)   {    return true;    }
                                bool   uncompress (ADMCompressedImage * in, ADMImage * out);
                                )

FF_SIMPLE_DECLARE(decoderFFH265,
                                 virtual bool bFramePossible (void)   {   return true;    }
                                )


#define WRAP_Open_Template(funcz,argz,display,codecid,extra) \
{\
AVCodec *codec=funcz(argz);\
  if(!codec) {GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Codec"),QT_TRANSLATE_NOOP("adm","Internal error finding codec" display));return;} \
  if(!_frame) { ADM_error("Could not allocate AVFrame.\n"); return; } \
  codecId=codecid; \
  _context = avcodec_alloc_context3 (codec);\
  if(!_context) { ADM_error("Could not allocate AVCodecContext.\n"); return; } \
  _context->max_b_frames = 0;\
  _context->width = _w;\
  _context->height = _h;\
  _context->pix_fmt = AV_PIX_FMT_YUV420P;\
  _context->workaround_bugs=1*FF_BUG_AUTODETECT +0*FF_BUG_NO_PADDING; \
  _context->error_concealment=3; \
  _context->opaque=this; \
  _context->get_format=ADM_FFgetFormat; \
  if (_setBpp) {\
    _context->bits_per_coded_sample = _bpp;\
  }\
  if (_setFcc) {\
    _context->codec_tag=_fcc; \
  }\
  if (_extraDataCopy) {\
    _context->extradata = _extraDataCopy;\
    _context->extradata_size = _extraDataLen;\
  }\
  if (_usingMT) {\
    _context->thread_count = _threads;\
  }\
  extra \
  \
  if (avcodec_open2(_context, codec, NULL) < 0)  \
                      { \
                                        ADM_info("[lavc] Decoder init: " display" video decoder failed!\n"); \
                                        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Codec"),QT_TRANSLATE_NOOP("adm","Internal error opening " display)); \
                                        return; \
                                } \
                                else \
                                { \
                                        ADM_info("[lavc] Decoder init: " display" video decoder initialized! (%s)\n",codec->long_name); \
                                } \
  _initCompleted=true; \
}

#define WRAP_Open(x)            {WRAP_Open_Template(avcodec_find_decoder,x,#x,x,;);}
#define WRAP_OpenByName(x,y)    {WRAP_Open_Template(avcodec_find_decoder_by_name,#x,#x,y,;);}


// EOF

