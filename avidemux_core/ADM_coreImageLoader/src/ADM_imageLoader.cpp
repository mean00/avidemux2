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

#include "ADM_default.h"
#include "ADM_image.h"
#include "ADM_bitmap.h"
#include "DIA_coreToolkit.h"
#include "ADM_imageLoader.h"
#include "ADM_colorspace.h"
#include "ADM_codec.h"
#include "fourcc.h"

/**
 */
typedef enum 
{
        ADM_PICTURE_UNKNOWN=0,
        ADM_PICTURE_JPG=1,
        ADM_PICTURE_PNG=2,
        ADM_PICTURE_BMP=3,
        ADM_PICTURE_BMP2=4
        
} ADM_PICTURE_TYPE;

static ADM_PICTURE_TYPE ADM_identifyImageFile(const char *filename,uint32_t *w,uint32_t *h);


//**********************************
static ADMImage *createImageFromFile_jpeg(const char *filename);
static ADMImage *createImageFromFile_Bmp2(const char *filename);
static ADMImage *createImageFromFile_png(const char *filename);
//***********************************
static uint8_t read8(FILE *fd)
{
	return fgetc(fd);
}
static uint32_t read16(FILE *fd)
{
	uint32_t a,b;

	a=fgetc(fd);
	b=fgetc(fd);
	return (a<<8)+b;

}
static uint32_t read32(FILE *fd)
{
	uint32_t a,b,c,d;

	a=fgetc(fd);
	b=fgetc(fd);
	c=fgetc(fd);
	d=fgetc(fd);
	return (a<<24)+(b<<16)+(c<<8)+d;

}
static int getFileSize(FILE *fd)
{
        fseek(fd, 0, SEEK_END);
        int imageSize = ftell(fd);
        fseek(fd, 0, SEEK_SET);
        return imageSize;
}

/**
		\fn 	createImageFromFile
		\brief 	Create and returns an ADMImage from a file, only YV12 jpg supported ATM

*/
ADMImage *createImageFromFile(const char *filename)
{
    uint32_t w,h;
    switch(ADM_identifyImageFile(filename,&w,&h))
    {
        case  ADM_PICTURE_UNKNOWN:
                ADM_warning("[imageLoader] Trouble identifying /loading %s\n",filename);
                return NULL;
        case ADM_PICTURE_JPG:
                return createImageFromFile_jpeg(filename);
                break;
        case ADM_PICTURE_PNG:
                return createImageFromFile_png(filename);
                break;
        case ADM_PICTURE_BMP2:
                return createImageFromFile_Bmp2(filename);
                break;
        default:
                ADM_assert(0);
                break;

    }
    ADM_assert(0);
    return NULL;
}
/**
 * 
 * @param source
 * @param w
 * @param h
 * @return 
 */
static ADMImage *convertImageColorSpace( ADMImage *source, int w, int h)
{
   
    	ADMImageDefault *image=new ADMImageDefault(w,h);        
        ADM_colorspace sourceFormat=source->_colorspace;   
        bool swap=false;
        
        if(ADM_COLOR_RGB32A==sourceFormat)
        {
            image->addAlphaChannel();
            // Extract alpha channel
#define ALPHA_OFFSET 3            
            uint8_t *alpha=source->GetReadPtr(PLANAR_Y)+ALPHA_OFFSET;
            uint8_t *alphaDest=image->GetWritePtr(PLANAR_ALPHA);
            int   sourceStride=source->GetPitch(PLANAR_Y);
            int   destStride=image->GetPitch(PLANAR_ALPHA);

            for(int y=0;y<h;y++)
            {
                uint8_t *inAlpha=alpha;
                alpha+=sourceStride;
                uint8_t *outAlpha=alphaDest;
                alphaDest+=destStride;
                for(int x=0;x<w;x++)
                {
                    *outAlpha=*inAlpha;
                    outAlpha++;
                    inAlpha+=4;
                }
            }
            swap=true;
        }
        ADMColorScalerSimple converter(w,h,sourceFormat,ADM_COLOR_YV12);     
        converter.convertImage(source,image); 
        
        if(swap)
        {
            uint8_t **s=image->_planes,*v;
            v=s[1];
            s[1]=s[2];
            s[2]=v;
        }
        return image;
}

/**
 * 
 * @param fd
 * @param width
 * @param height
 * @return 
 */
static bool readJpegInfo(FILE *fd, int &width, int &height)
{
        uint16_t tag = 0, count = 0, off;
        int w,h;
        fseek(fd, 0, SEEK_SET);
        read16(fd);	// skip jpeg ffd8
        while (count < 15 && tag != 0xFFC0)
        {
            tag = read16(fd);
            if ((tag >> 8) != 0xff)
            {
                ADM_warning("[imageLoader]invalid jpeg tag found (%x)\n", tag);
            }
            if (tag == 0xFFC0)
            {
                read16(fd);	// size
                read8(fd);	// precision
                h = read16(fd);
                w = read16(fd);
                if(w&1) w++;
                if(h&1) h++;
            }
            else
            {
                off = read16(fd);
                if (off < 2)
                {
                    ADM_warning("[imageLoader]Offset too short!\n");
                    return false;
                }
                fseek(fd, off - 2, SEEK_CUR);
            }
            count++;
        }
        if (tag != 0xffc0)
        {
            ADM_warning("[imageLoader]Cannot fint start of frame\n");
            return false;
        }
        width=w;
        height=h;
        return true;
}

/**
 * 	\fn createImageFromFile_jpeg
 *  \brief Create image from jpeg file
 */
ADMImage *createImageFromFile_jpeg(const char *filename)
{

FILE *fd;
uint32_t _imgSize;
int  w = 0, h = 0;

        fd = ADM_fopen(filename, "rb");
        if(!fd)
        {
            ADM_warning("Cannot open jpeg file\n");
            return NULL;
        }
        _imgSize = getFileSize(fd);

        if(!readJpegInfo(fd,w,h))
        {
            ADM_warning("Cannot get info from jpeg\n");
            fclose(fd);
            fd=NULL;
            return NULL;
        }
        
        ADM_info("[imageLoader] %d x %d.., total Size : %u \n", w, h,_imgSize);

        // Load the binary coded image
        ADM_byteBuffer buffer(_imgSize);
        fseek(fd, 0, SEEK_SET);
        fread(buffer.at(0),_imgSize,1,fd);
        fclose(fd);
        //

        ADMImageRef tmpImage(w,h); // It is a reference image
        // Now unpack it ...
        decoders *dec=ADM_coreCodecGetDecoder (fourCC::get((uint8_t *)"MJPG"),   w,   h, 0 , NULL,0);
        if(!dec)
        {
            ADM_warning("Cannot find decoder for mpjeg");
            return NULL;
        }
        ADMCompressedImage bin;
        bin.data=buffer.at(0);
        bin.dataLength=_imgSize; // This is more than actually, but who cares...

        dec->uncompress (&bin, &tmpImage);
        ADMImage *image=convertImageColorSpace(&tmpImage,w,h);        
        delete dec;
        dec=NULL;
        return image;
}

/**
 * 	\fn createImageFromFile_png
 *  \brief Create image from PNG
 */
ADMImage *createImageFromFile_png(const char *filename)
{
    
    uint32_t offset,size;
    FILE *fd=NULL;
    uint32_t w,h;

       fd = ADM_fopen(filename, "rb");
       if(!fd)
       {
           ADM_warning("Cannot open png file\n");
           return NULL;
       }
       size=getFileSize(fd);
       read32(fd);
       read32(fd);
       read32(fd);
       read32(fd);
       w=read32(fd);
       h=read32(fd);
       fseek(fd,0,SEEK_SET);
       ADM_byteBuffer buffer(size);

       fread(buffer.at(0),size,1,fd);
       fclose(fd);
       ADMImageRef tmpImage(w,h);
    	// Decode PNG
        decoders *dec=ADM_coreCodecGetDecoder (fourCC::get((uint8_t *)"PNG "),   w,   h, 0 , NULL,0);
    	if(!dec)
        {
            ADM_warning("Cannot get PNG decoder");
            return NULL;
        }
    	ADMCompressedImage bin;
    	bin.data=buffer.at(0);
    	bin.dataLength=size; // This is more than actually, but who cares...

    	bool success=dec->uncompress (&bin, &tmpImage);
   
        
        if(!success)
        {
            ADM_warning("PNG Decompressing failed\n");
            delete dec;
            dec=NULL;
            return NULL;
        }
        ADMImage *image=convertImageColorSpace(&tmpImage,w,h);
        if(tmpImage._alpha)
        {
            ADM_info("We do have alpha channel\n");
        }
        delete dec;
        dec=NULL;
    	return image;
}
/**
 * 	\fn createImageFromFile_bmp2
 *  \brief Create image from Bmp2 (BM6)
 */
ADMImage *createImageFromFile_Bmp2(const char *filename)
{

    ADM_BITMAPINFOHEADER bmph;
    uint8_t fcc_tab[4];
    uint32_t offset;
    FILE *fd=NULL;
    uint32_t w,h;

	fd = ADM_fopen(filename, "rb");
        if(!fd)
        {
            ADM_warning("Cannot open BMP picture\n");
            return NULL;
        }
 	fseek(fd, 10, SEEK_SET);

 #define MK32() (fcc_tab[0]+(fcc_tab[1]<<8)+(fcc_tab[2]<<16)+ \
 						(fcc_tab[3]<<24))

        fread(fcc_tab, 4, 1, fd);
        offset = MK32();
        // size, width height follow as int32
        fread(&bmph, sizeof(bmph), 1, fd);
 #ifdef ADM_BIG_ENDIAN
 	    Endian_BitMapInfo(&bmph);
 #endif
        if (bmph.biCompression != 0)
        {
            ADM_warning("[imageLoader] BMP2:Cannot handle compressed bmp\n");
            fclose(fd);
            return NULL;
        }
        w = bmph.biWidth;
        h = bmph.biHeight;
        ADM_info("[imageLoader] BMP2 W: %" PRIu32" H: %" PRIu32" offset : %" PRIu32"\n", w, h, offset);
// Load the binary coded image
 	fseek(fd,offset,SEEK_SET);
        
        ADM_byteBuffer buffer(w*h*4);
        
        fread(buffer.at(0),w*h*4,1,fd);
        fclose(fd);

  // Colorconversion

    	
        ADMImageRefWrittable ref(w,h);
        
        ref._planes[0]=buffer.at(0)+(h-1)*w*3;
        ref._planeStride[0]=-w*3;
        ref._colorspace=ADM_COLOR_RGB24;

    	return convertImageColorSpace(&ref,w,h);
}
/**
 * 		\fn ADM_identidyImageFile
 * 		\brief Identidy image type, returns type and width/height
 */
#define MAX_JPEG_TAG 20
ADM_PICTURE_TYPE ADM_identifyImageFile(const char *filename,uint32_t *w,uint32_t *h)
{
    uint8_t fcc_tab[4];
    FILE *fd;
    uint32_t off,tag=0,count,size;

    // 1- identity the file type
    //
    
    fd = ADM_fopen(filename, "rb");
    if (!fd)
    {
        ADM_info("[imageIdentify] Cannot open that file!\n");
        return ADM_PICTURE_UNKNOWN;
    }
    fread(fcc_tab, 4, 1, fd);

    // 2- JPEG ?
    if (fcc_tab[0] == 0xff && fcc_tab[1] == 0xd8)
    {
        int width,height;
         if(readJpegInfo(fd,width,height))
         {
             ADM_info("Identified as jpeg (%d x %d)\n",width,height);
             *w=width;
             *h=height;
             fclose(fd);
             return ADM_PICTURE_JPG;
         }else
         {
             fclose(fd);
             return ADM_PICTURE_UNKNOWN;
         }     
    }
    // PNG ?
    if (fcc_tab[1] == 'P' && fcc_tab[2] == 'N' && fcc_tab[3] == 'G')
    {
        fseek(fd, 0, SEEK_SET);
        read32(fd);
        read32(fd);
        read32(fd);
        read32(fd);
        *w=read32(fd);
        *h=read32(fd);
        fclose(fd);
        return ADM_PICTURE_PNG;
    }
    // BMP2?
    if (fcc_tab[0] == 'B' && fcc_tab[1] == 'M')
    {
        ADM_BITMAPINFOHEADER bmph;

            fseek(fd, 10, SEEK_SET);
            fread(fcc_tab, 4, 1, fd);
            // size, width height follow as int32
            fread(&bmph, sizeof(bmph), 1, fd);
     #ifdef ADM_BIG_ENDIAN
            Endian_BitMapInfo(&bmph);
     #endif
            if (bmph.biCompression != 0)
            {
                ADM_warning("[imageIdentify] BMP2:Cannot handle compressed bmp\n");
                fclose(fd);
                return ADM_PICTURE_UNKNOWN;
            }
            *w = bmph.biWidth;
            *h = bmph.biHeight;
            fclose(fd);
            return ADM_PICTURE_BMP2;
    }
    // Unknown filetype...
    fclose(fd);
    return ADM_PICTURE_UNKNOWN;
}
//EOF

