/**
      \file gui_misc
      \brief 2nd part of ADMImage, put here to avoid weird dependancies
*/
#include "ADM_default.h"
#include "ADM_image.h"
#include "ADM_bitmap.h"
#include "DIA_coreToolkit.h"
#include "ADM_colorspace.h"
#include "ADM_bitstream.h"
#include "ADM_codecs/ADM_codec.h"
//#include "ADM_codecs/ADM_ffmpeg.h"
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
    #if 0
 ffmpegEncoderFFMjpeg *codec=NULL;
  FILE *fd;
  uint8_t *buffer=NULL;
  uint32_t sz;


        sz = _width*_height*3;
        ADMBitstream bitstream(sz);
        buffer=new uint8_t[sz];
        bitstream.data=buffer;
        codec=new  ffmpegEncoderFFMjpeg(_width,_height,FF_MJPEG)  ;
        codec->init( 95,25000);
        if(!codec->encode(this,&bitstream))
        {
                GUI_Error_HIG(QT_TR_NOOP("Cannot encode the frame"), NULL);
                delete [] buffer;
                delete codec;
                return 0;
        }
        delete codec;
        fd=fopen(filename,"wb");
        if(!fd)
        {
                GUI_Error_HIG(QT_TR_NOOP("File error"),QT_TR_NOOP( "Cannot open \"%s\" for writing."), filename);
                delete [] buffer;
                return 0;
        }
        fwrite (buffer, bitstream.len, 1, fd);
        fclose(fd);
        delete [] buffer;
        return 1;
        #endif
#warning save as jpeg disabled
        return false;
}