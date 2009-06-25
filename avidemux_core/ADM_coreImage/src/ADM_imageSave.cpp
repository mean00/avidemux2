/**
      \file A
      \brief 2nd part of ADMImage, put here to avoid weird dependancies
*/
#include "ADM_default.h"
#include "ADM_image.h"
#include "ADM_bitmap.h"
#include "DIA_coreToolkit.h"
#include "ADM_colorspace.h"
extern "C" 
{
#include "ADM_lavcodec.h"
}

/**
    \fn saveAsBmp
    \brief save current image into filename, into bmp format
*/
uint8_t  ADMImage::saveAsBmp(const char *filename)
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

  ADMImage image(_width,_height);


  printf ("\n %u x %u=%u\n", bmph.biWidth, bmph.biHeight, sz);

  uint8_t *out;

        out=(uint8_t *)ADM_alloc(sz);
        if(!out)
        {
            GUI_Error_HIG(QT_TR_NOOP("Memory error"), NULL);
//            ADM_dealloc(out);
            return 0;
        }

        if(!COL_yv12rgbBMP(bmph.biWidth, bmph.biHeight,data, out))
        {
              GUI_Error_HIG(QT_TR_NOOP("Error converting to BMP"), NULL);
              ADM_dealloc(out);
              return 0;
        }
        fd = fopen (filename, "wb");
        if (!fd)
        {
                GUI_Error_HIG (QT_TR_NOOP("Something bad happened"), NULL);
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
    \fn saveAsJpg
    \brief save current image into filename, into jpg format
*/
uint8_t  ADMImage::saveAsJpg(const char *filename)
{

AVCodecContext   *context=NULL;   
AVFrame          frame;     
bool             result=false;
AVCodec          *codec=NULL;
uint8_t          buffer[_width*_height*4];
int              sz=0,r=0;

    context=avcodec_alloc_context();
    if(!context) 
    {
        printf("[saveAsJpg] Cannot allocate context\n");
        return false;
    }
    codec=avcodec_find_encoder(CODEC_ID_MJPEG);
    if(!codec)
    {
        printf("[saveAsJpg] Cannot allocate codec\n");
        goto jpgCleanup;
    }
    context->pix_fmt =PIX_FMT_YUV420P;
    context->strict_std_compliance = -1;
    r=avcodec_open(context, codec); 
    if(r<0)
    {
        printf("[saveAsJpg] Cannot mix codec and context\n");
        ADM_dealloc (context);
        return false;
    }
    // Setup our image & stuff....
        

        frame.linesize[0] = _width; 
        frame.linesize[1] = _width>>1; 
        frame.linesize[2] = _width>>1; 
        
        frame.data[0] = GetWritePtr(PLANAR_Y);
        frame.data[2] = GetWritePtr(PLANAR_U);
        frame.data[1] = GetWritePtr(PLANAR_V);
    // Grab a temp buffer
    
    // Encode!
      context->flags |= CODEC_FLAG_QSCALE;
      frame.quality = (int) floor (FF_QP2LAMBDA * 2+ 0.5);
        
        if ((sz = avcodec_encode_video (context, buffer, _width*_height*4, &frame)) < 0)
        {
            printf("[jpeg] Error %d encoding video\n",sz);
            goto  jpgCleanup;
        }
        // Ok now write our file...
        {
            FILE *f=fopen(filename,"wb");
            if(f)
            {
                fwrite(buffer,sz,1,f);
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
        ADM_dealloc (context);
    }
    context=NULL;
    return result;
}