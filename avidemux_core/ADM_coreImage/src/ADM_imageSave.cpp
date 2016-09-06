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
}

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

/**
    \fn saveAsBmp
    \brief save current image into filename, into bmp format
*/
bool  ADMImage::saveAsBmpInternal(const char *filename)
{
  ADM_BITMAPFILEHEADER bmfh;
  ADM_BITMAPINFOHEADER bmph;
  FILE *fd;
  uint32_t sz;
  uint16_t s16;
  uint32_t s32;

  sz = _width* _height * 3;

  bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
  bmfh.bfOffBits = sizeof (bmfh) + sizeof (bmph);
//_________________________________________
  bmph.biSize = sizeof (bmph);
  bmph.biWidth = _width;
  bmph.biHeight = _height;
  bmph.biPlanes = 1;
  bmph.biBitCount = 24;
  bmph.biCompression = 0;	// COMPRESSION NONE
  bmph.biSizeImage = sz;
  bmph.biXPelsPerMeter = 0;
  bmph.biYPelsPerMeter = 0;
  bmph.biClrUsed = 0;
  bmph.biClrImportant = 0;
/*
	bmph.resolutionUnits=0;
	bmph.origin=0;
	bmph.colorEncoding=0;
*/

  uint8_t *out;

        out=(uint8_t *)ADM_alloc(sz);
        if(!out)
        {
            GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Memory error"), NULL);
//            ADM_dealloc(out);
            return 0;
        }
        ADMColorScalerSimple converter(bmph.biWidth, bmph.biHeight, ADM_COLOR_YV12,ADM_COLOR_RGB24);
        converter.convertImage(this,out);
        uint32_t ww=bmph.biWidth;
        uint32_t hh=bmph.biHeight;
        uint8_t *swap = new uint8_t[ww*3];
        uint8_t *up=out;
        uint8_t *down=out+(hh-1)*ww*3;
        
        for(int y=0;y<hh>>1;y++)
        {
            SwapMe(swap,up,ww); 
            SwapMe(up,down,ww);
            memcpy( down,swap,ww*3);
            down-=3*ww;
            up+=3*ww;
        }

		delete [] swap;

        fd = ADM_fopen (filename, "wb");
        if (!fd)
        {
                GUI_Error_HIG (QT_TRANSLATE_NOOP("adm","Something bad happened"), NULL);
                ADM_dealloc(out);
                return 0;
        }

	// Bitmpap file header, not using tructure due to gcc padding it
#ifdef ADM_BIG_ENDIAN
	s16 = 0x424D;
#else
  	s16 = 0x4D42;
#endif
  	s32 = 14 + sizeof (bmph) + sz;
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
        fwrite (out, sz, 1, fd);

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
    \fn saveAsJpg
    \brief save current image into filename, into jpg format
*/
bool  ADMImage::saveAsJpgInternal(const char *filename)
{

AVCodecContext   *context=NULL;   
AVFrame          *frame=NULL;
bool             result=false;
AVCodec          *codec=NULL;
int              sz=0,r=0;
ADM_byteBuffer   byteBuffer;

    frame=av_frame_alloc();
    if(!frame)
    {
        printf("[saveAsJpg] Cannot allocate frame\n");
        goto  jpgCleanup;
    }

    codec=avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if(!codec)
    {
        printf("[saveAsJpg] Cannot allocate codec\n");
        goto  jpgCleanup;
    }

    context=avcodec_alloc_context3(codec);
    if(!context) 
    {
        printf("[saveAsJpg] Cannot allocate context\n");
        goto  jpgCleanup;
    }

    context->pix_fmt =AV_PIX_FMT_YUV420P;
    context->strict_std_compliance = -1;
    context->time_base.den=1;
    context->time_base.num=1;
    context->width=_width;
    context->height=_height;
    context->flags |= CODEC_FLAG_QSCALE;
    r=avcodec_open2(context, codec, NULL); 
    if(r<0)
    {
        printf("[saveAsJpg] Cannot mix codec and context\n");
        ADM_dealloc (context);
        return false;
    }
    // Setup our image & stuff....
        

        frame->linesize[0] = GetPitch(PLANAR_Y); 
        frame->linesize[1] = GetPitch(PLANAR_U); 
        frame->linesize[2] = GetPitch(PLANAR_V); 
        
        frame->data[0] = GetWritePtr(PLANAR_Y);
        frame->data[2] = GetWritePtr(PLANAR_U);
        frame->data[1] = GetWritePtr(PLANAR_V);
    // Grab a temp buffer
    
    // Encode!
     
      frame->quality = (int) floor (FF_QP2LAMBDA * 2+ 0.5);

      byteBuffer.setSize(_width*_height*4);
	  

      AVPacket pkt;
      av_init_packet(&pkt);
      int gotSomething;
      pkt.size=_width*_height*4;
      pkt.data=byteBuffer.at(0);
      r=avcodec_encode_video2(context,&pkt,frame,&gotSomething);
      if(r || !gotSomething)
      {
            ADM_error("[jpeg] Error %d encoding video\n",r);
            goto  jpgCleanup;
      }
      
        // Ok now write our file...
        {
            FILE *f=ADM_fopen(filename,"wb");
            if(f)
            {
                fwrite(byteBuffer.at(0),pkt.size,1,f);
                fclose(f);
                result=true;
            }else
            {
                printf("[saveAsJpeg] Cannot open %s for writing!\n",filename);

            }
       }

// Cleanup
jpgCleanup:
    if(context)
    {
        avcodec_close (context);
        av_free (context);
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
    \fn saveAsBmp
    \brief save current image into filename, into bmp format
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
