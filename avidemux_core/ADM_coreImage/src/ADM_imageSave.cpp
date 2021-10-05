/**
      \file A
      \brief 2nd part of ADMImage, put here to avoid weird dependancies
*/

#include "ADM_default.h"
#include "ADM_image.h"
#include "ADM_bitmap.h"
#include "DIA_coreToolkit.h"
#include "ADM_colorspace.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/mem.h"
}

#if 0
/**
    \fn SwapMe
*/
static inline void SwapMe(uint8_t *tgt,uint8_t *src,int nb);
void SwapMe(uint8_t *tgt,uint8_t *src,int nb)
{
    uint8_t r,g,b;

   while(nb--)
   {
       r=*src++;
       g=*src++;
       b=*src++;
       *tgt++=r;
       *tgt++=g;
       *tgt++=b;
       
   }
   return;
    
}
#endif

/**
    \fn saveAsBmp
    \brief save current image into filename, into bmp format
*/
bool  ADMImage::saveAsBmpInternal(const char *filename)
{
  ADM_BITMAPFILEHEADER bmfh;
  ADM_BITMAPINFOHEADER bmph;
  FILE *fd;
  uint16_t s16;
  uint32_t s32;
  uint32_t y;

  const uint32_t stride = ADM_IMAGE_ALIGN(_width * 3);
  const uint32_t sz = stride * _height;

#define ALIGN_DWORD(x) ((x+3)&(~3))

  uint32_t packed = 0;
  for(y=0; y < _height; y++)
        packed = ALIGN_DWORD(packed + _width * 3);

  bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
  bmfh.bfOffBits = sizeof (bmfh) + sizeof (bmph);
//_________________________________________
  bmph.biSize = sizeof (bmph);
  bmph.biWidth = _width;
  bmph.biHeight = _height;
  bmph.biPlanes = 1;
  bmph.biBitCount = 24;
  bmph.biCompression = 0;	// COMPRESSION NONE
  bmph.biSizeImage = packed;
  bmph.biXPelsPerMeter = 0;
  bmph.biYPelsPerMeter = 0;
  bmph.biClrUsed = 0;
  bmph.biClrImportant = 0;
/*
	bmph.resolutionUnits=0;
	bmph.origin=0;
	bmph.colorEncoding=0;
*/

    uint8_t *tmp=(uint8_t *)ADM_alloc(sz);
    uint8_t *out=(uint8_t *)ADM_alloc(packed);
    if(!tmp || !out)
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Cannot allocate enough memory"), NULL);
        ADM_dealloc(tmp);
        ADM_dealloc(out);
        return 0;
    }

    ADMColorScalerSimple converter(bmph.biWidth, bmph.biHeight, ADM_PIXFRMT_YV12, ADM_PIXFRMT_BGR24);
    converter.convertImage(this,tmp);

    // Pack data and swap lines
    uint8_t *up=out;
    uint8_t *down=tmp+(_height-1)*stride;
    packed = 0;
    for(y=0; y < _height; y++)
    {
        uint32_t end=packed+_width*3;
        packed = ALIGN_DWORD(end);
        uint32_t padding=packed-end;
        memcpy(up,down,_width*3);
        down-=stride;
        up+=_width*3;
        if(padding)
        {
            memset(up,0,padding);
            up+=padding;
        }
    }
    ADM_dealloc(tmp);
    tmp=NULL;

    fd = ADM_fopen (filename, "wb");
    if (!fd)
    {
        GUI_Error_HIG (QT_TRANSLATE_NOOP("adm","Cannot create output file"), NULL);
        ADM_dealloc(out);
        return 0;
    }

	// Bitmpap file header, not using tructure due to gcc padding it
#ifdef ADM_BIG_ENDIAN
	s16 = 0x424D;
#else
  	s16 = 0x4D42;
#endif
  	s32 = 14 + sizeof (bmph) + packed;
#ifdef ADM_BIG_ENDIAN
	#define SWAP32(x) x=R32(x)
#else
	#define SWAP32(x) ;
#endif
        SWAP32(s32);
        fwrite (&s16, 2, 1, fd);
        fwrite (&s32, 4, 1, fd);
        s32 = 0;
        fwrite (&s32, 4, 1, fd);
        s32 = 14 + sizeof (bmph);
        SWAP32(s32);
        fwrite (&s32, 4, 1, fd);
#ifdef ADM_BIG_ENDIAN
	Endian_BitMapInfo(&bmph);
#endif
        fwrite (&bmph, sizeof (bmph), 1, fd);
        fwrite (out, packed, 1, fd);

        fclose(fd);
        ADM_dealloc(out);
        return 1;
}
/**
    \fn saveAsBmp
    \brief save current image into filename, into bmp format
*/
bool  ADMImage::saveAsBmp(const char *filename)
{
    if(refType==ADM_HW_NONE)
        return saveAsBmpInternal(filename);
    
    ADMImageDefault clone(_width, _height);
    
    clone.duplicateFull(this);
    clone.hwDownloadFromRef();
    return clone.saveAsBmpInternal(filename);
    
}

/**
    \fn saveAsJpgInternal
    \brief save current image into filename, into jpg format
*/
bool  ADMImage::saveAsJpgInternal(const char *filename)
{
    AVCodecContext *context=NULL;
    AVFrame *frame=NULL;
    bool result=false;
    AVCodec *codec=NULL;
    int r=0;
    FILE *f=NULL;

    frame=av_frame_alloc();
    if(!frame)
    {
        ADM_error("Cannot allocate frame\n");
        return false;
    }

    if(false==expandColorRange()) return false;

    codec=avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if(!codec)
    {
        ADM_error("Cannot allocate encoder\n");
        goto jpgCleanup;
    }

    context=avcodec_alloc_context3(codec);
    if(!context) 
    {
        ADM_error("Cannot allocate context\n");
        goto jpgCleanup;
    }

    context->pix_fmt =AV_PIX_FMT_YUV420P;
    context->color_range = AVCOL_RANGE_JPEG;
    context->strict_std_compliance = -1;
    context->time_base.den=1;
    context->time_base.num=1;
    context->width=_width;
    context->height=_height;
    context->flags |= AV_CODEC_FLAG_QSCALE;

    r=avcodec_open2(context, codec, NULL);

    if(r<0)
    {
        char msg[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(msg, AV_ERROR_MAX_STRING_SIZE, r);
        ADM_error("Cannot combine codec and context, error %d (%s)\n",r,msg);
        goto jpgCleanup;
    }
    // Setup our image & stuff....
    frame->width=_width;
    frame->height=_height;
    frame->format=AV_PIX_FMT_YUV420P;
    frame->color_range = AVCOL_RANGE_JPEG;

    frame->linesize[0] = GetPitch(PLANAR_Y);
    frame->linesize[1] = GetPitch(PLANAR_U);
    frame->linesize[2] = GetPitch(PLANAR_V);

    frame->data[0] = GetReadPtr(PLANAR_Y);
    frame->data[1] = GetReadPtr(PLANAR_U);
    frame->data[2] = GetReadPtr(PLANAR_V);

    frame->quality = (int) floor (FF_QP2LAMBDA * 2+ 0.5);

    // Encode!
    r = avcodec_send_frame(context,frame);

    if(r<0)
    {
        char msg[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(msg, AV_ERROR_MAX_STRING_SIZE, r);
        ADM_error("Error %d (%s) sending data to encoder.\n",r,msg);
        goto jpgCleanup;
    }

    AVPacket pkt;
    av_init_packet(&pkt);

    r = avcodec_receive_packet(context,&pkt);

    if(r<0)
    {
        char msg[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(msg, AV_ERROR_MAX_STRING_SIZE, r);
        ADM_error("Error %d (%s) encoding to JPEG.\n",r,msg);
        av_packet_unref(&pkt);
        goto jpgCleanup;
    }
    // Ok now write our file...
    f=ADM_fopen(filename,"wb");
    if(!f)
    {
        ADM_error("Cannot open %s for writing!\n",filename);
        av_packet_unref(&pkt);
        goto jpgCleanup;
    }
    fwrite(pkt.data,pkt.size,1,f);
    fclose(f);
    f=NULL;
    av_packet_unref(&pkt);
    result=true;

// Cleanup
jpgCleanup:
    if(context)
    {
        avcodec_free_context(&context);
        context=NULL;
    }

    if(frame)
    {
        av_frame_free(&frame);
        frame=NULL;
    }

    return result;
}

/**
    \fn saveAsPngInternal
    \brief save current image into filename in PNG format
*/
bool ADMImage::saveAsPngInternal(const char *filename)
{
    AVCodecContext *context=NULL;
    AVFrame *frame=NULL;
    AVCodec *codec=NULL;
    FILE *f=NULL;
    bool result=false;
    const uint32_t sz=ADM_IMAGE_ALIGN(_width*3)*_height;
    int r=0;
    uint8_t *out=NULL;
    ADMColorScalerSimple converter(_width, _height, ADM_PIXFRMT_YV12, ADM_PIXFRMT_RGB24);

    frame=av_frame_alloc();
    if(!frame)
    {
        ADM_error("Cannot allocate frame\n");
        return false;
    }

    codec=avcodec_find_encoder(AV_CODEC_ID_PNG);
    if(!codec)
    {
        ADM_error("Cannot allocate encoder\n");
        goto __cleanup;
    }

    context=avcodec_alloc_context3(codec);
    if(!context)
    {
        ADM_error("Cannot allocate context\n");
        goto __cleanup;
    }

    context->pix_fmt=AV_PIX_FMT_RGB24;
    context->color_range = (_range == ADM_COL_RANGE_MPEG)? AVCOL_RANGE_MPEG : AVCOL_RANGE_JPEG;
    context->strict_std_compliance = -1;
    context->time_base.den=1;
    context->time_base.num=1;
    context->width=_width;
    context->height=_height;

    r=avcodec_open2(context, codec, NULL);

    if(r<0)
    {
        char msg[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(msg, AV_ERROR_MAX_STRING_SIZE, r);
        ADM_error("Cannot combine codec and context, error %d (%s)\n",r,msg);
        goto __cleanup;
    }

    out=(uint8_t *)ADM_alloc(sz);
    if(!out)
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Memory error"), NULL);
        goto __cleanup;
    }

    // convert colorspace
    converter.convertImage(this,out);

    // setup AVFrame
    frame->width = _width;
    frame->height = _height;
    frame->format = AV_PIX_FMT_RGB24;

    frame->linesize[0] = ADM_IMAGE_ALIGN(_width*3);
    frame->linesize[1] = 0;
    frame->linesize[2] = 0;

    frame->data[0] = out;
    frame->data[1] = NULL;
    frame->data[2] = NULL;

    // Encode
    r = avcodec_send_frame(context,frame);

    ADM_dealloc(out);

    if(r<0)
    {
        char msg[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(msg, AV_ERROR_MAX_STRING_SIZE, r);
        ADM_error("Error %d (%s) sending data to encoder.\n",r,msg);
        goto __cleanup;
    }

    AVPacket pkt;
    av_init_packet(&pkt);

    r = avcodec_receive_packet(context,&pkt);

    if(r<0)
    {
        char msg[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(msg, AV_ERROR_MAX_STRING_SIZE, r);
        ADM_error("Error %d (%s) encoding to PNG.\n",r,msg);
        av_packet_unref(&pkt);
        goto __cleanup;
    }

    // Ok now write our file...
    f=ADM_fopen(filename,"wb");
    if(!f)
    {
        ADM_error("Cannot open %s for writing!\n",filename);
        av_packet_unref(&pkt);
        goto __cleanup;
    }
    fwrite(pkt.data,pkt.size,1,f);
    fclose(f);
    f=NULL;
    av_packet_unref(&pkt);
    result=true;

__cleanup:
    // Cleanup
    if(context)
    {
        avcodec_free_context(&context);
        context=NULL;
    }

    if(frame)
    {
        av_frame_free(&frame);
        frame=NULL;
    }

    return result;
}

/**
    \fn saveAsJpg
    \brief save current image into filename, into JPEG format
*/
bool  ADMImage::saveAsJpg(const char *filename)
{
    if(refType==ADM_HW_NONE)
        return saveAsJpgInternal(filename);
    
    ADMImageDefault clone(_width, _height);
    
    clone.duplicateFull(this);
    clone.hwDownloadFromRef();
    return clone.saveAsJpgInternal(filename);
    
}

/**
    \fn saveAsPng
    \brief save current image into filename in PNG format
*/
bool ADMImage::saveAsPng(const char *filename)
{
    if(refType==ADM_HW_NONE)
        return saveAsPngInternal(filename);

    ADMImageDefault clone(_width, _height);

    clone.duplicateFull(this);
    clone.hwDownloadFromRef();
    return clone.saveAsPngInternal(filename);

}
