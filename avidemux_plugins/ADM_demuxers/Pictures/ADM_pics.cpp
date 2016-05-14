/***************************************************************************
                          ADM_pics.cpp  -  description
                             -------------------

                             Open a bunch of bmps and read them as a movie
                             Useful for people doing raytracing or doing img/img
                             modifications


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

#include "ADM_default.h"

#include <string.h>
#include <math.h>

#include "ADM_Video.h"
#include "fourcc.h"
#include "ADM_pics.h"
//#include "ADM_toolkit/bitmap.h"

#define aprintf(...) {}
static uint16_t s16;
static uint32_t s32;
#define MAX_ACCEPTED_OPEN_FILE 99999

#define US_PER_PIC (40*1000)

picHeader::picHeader(void)
{
	_nb_file = 0;
	_imgSize = NULL;
	_fileMask = NULL;
}
/**
    \fn getTime
*/

uint64_t                   picHeader::getTime(uint32_t frameNum)
{
    float f=    US_PER_PIC;
    f*=frameNum;
    return (uint64_t)f;

}
/**
    \fn getVideoDuration
*/

uint64_t                   picHeader::getVideoDuration(void)
{
    float f= US_PER_PIC;
    f*=_videostream.dwLength;
    return (uint64_t)f;


}


/**
    \fn getFrameSize
*/
uint8_t                 picHeader::getFrameSize(uint32_t frame,uint32_t *size)
{
    if (frame >= (uint32_t)_videostream.dwLength)
		return 0;
    *size= _imgSize[frame];
    return 1;
}
/**
    \fn getFrame
*/
uint8_t picHeader::getFrame(uint32_t framenum, ADMCompressedImage *img)
{
    if (framenum >= (uint32_t)_videostream.dwLength)
            return 0;

    img->flags = AVI_KEY_FRAME;

    FILE* fd = openFrameFile(framenum);
    if(!fd)
        return false;
    fread(img->data, _imgSize[framenum] - _offset, 1, fd);
    img->dataLength = _imgSize[framenum] - _offset;

    uint64_t timeP=US_PER_PIC;
    timeP*=framenum;
    img->demuxerDts=timeP;
    img->demuxerPts=timeP;
    fclose(fd);

    return 1;
}
//****************************************************************
uint8_t picHeader::close(void)
{
	_nb_file = 0;

	if (_fileMask)
	{
		ADM_dealloc(_fileMask);
		_fileMask = NULL;
	}

	if (_imgSize)
	{
		delete [] _imgSize;
		_imgSize = NULL;
	}

	return 0;
}



//****************************************************************
/*
	Open a bunch of images


*/


uint32_t picHeader::read32(FILE * fd)
{
    uint32_t i;
    i = 0;
    i = (read8(fd) << 24) + (read8(fd) << 16) + (read8(fd) << 8) +
	(read8(fd));
    return i;

}

uint16_t picHeader::read16(FILE * fd)
{
    uint16_t i;

    i = 0;
    i = (read8(fd) << 8) + (read8(fd));
    return i;
}

uint8_t picHeader::read8(FILE * fd)
{
    uint8_t i;
    ADM_assert(fd);
    i = 0;
    if (!fread(&i, 1, 1, fd)) {
	printf("\n Problem reading the file !\n");
    }
    return i;
}
//****************************************************************
uint8_t picHeader::open(const char *inname)
{
    uint32_t nnum;
    uint32_t *fcc;
    uint8_t fcc_tab[4];
    FILE *fd;
    char *end;
    uint32_t w = 0, h = 0, bpp = 0;

    // 1- identity the file type
    //
    fcc = (uint32_t *) fcc_tab;
    fd = ADM_fopen(inname, "rb");
    if (!fd) {
	printf("\n Cannot open that file!\n");
	return 0;
    }
    fread(fcc_tab, 4, 1, fd);
    fclose(fd);
    if (fourCC::check(*fcc, (uint8_t *) "RIFF")) {
	_type = PIC_BMP;
	printf("\n It looks like BMP (RIFF)...\n");
    } else {
	if (fcc_tab[0] == 'B' && fcc_tab[1] == 'M') {
	    _type = PIC_BMP2;
	    printf("\n It looks like BMP (BM)...\n");
	} else if (fcc_tab[0] == 0xff && fcc_tab[1] == 0xd8) {
	    _type = PIC_JPEG;
	    printf("\n It looks like Jpg...\n");
	} else {
	    if (fcc_tab[1] == 'P' && fcc_tab[2] == 'N'
		&& fcc_tab[3] == 'G')
		{
    		printf("\n It looks like PNG...\n");
		    _type = PIC_PNG;
	    } else {
		printf("\n Cannot identify file (%x %x)\n", *fcc,
		       *fcc & 0xffff);
		return 0;
	    }
	}
    }

    // Then spit the name in name and extension
    char *name;
    char *extension;
    ADM_PathSplit(inname, &name, &extension);


    nnum = 1;

    end = name + strlen(name) - 1;
    while ((*end >= '0') && (*end <= '9')) {
	end--;
	nnum++;
    };
char realname[250];
char realstring[250];

    if (nnum == 1) {
	printf("\n only one file!");
        _nb_file=1;
		 _fileMask = ADM_strdup(inname);
    }
    else
    {
    nnum--;
    end++;
    _first = atoi(end);
	printf("\n First: %" PRIu32", Digit count: %" PRIu32"\n", _first, nnum);
    *(end) = 0;
	printf(" Path: %s\n", name);

	sprintf(realstring, "%s%%0%" PRIu32"d.%s", name, nnum, extension);
	_fileMask = ADM_strdup(realstring);
	printf(" File Mask: %s\n\n", _fileMask);

    _nb_file = 0;

	for (uint32_t i = 0; i < MAX_ACCEPTED_OPEN_FILE; i++)
	{
		sprintf(realname, realstring, i + _first);
		printf(" %" PRIu32" : %s\n", i, realname);

		fd = ADM_fopen(realname, "rb");

		if (fd == NULL)
			break;

		fclose(fd);
		_nb_file++;
	}
	}
    printf("\n found %" PRIu32" images\n", _nb_file);

    _imgSize = new uint32_t[_nb_file];
    //_________________________________
    // now open them and assign imgSize
    //__________________________________
	for (uint32_t i = 0; i < _nb_file; i++)
	{
		fd = openFrameFile(i);
		ADM_assert(fd != NULL);

		fseek(fd, 0, SEEK_END);
		_imgSize[i] = ftell(fd);

		fclose(fd);
	}

	fd = openFrameFile(0);

	delete [] name;
	delete [] extension;

    //
    //      Image is bmp type
    //________________________
    switch (_type) {
    case PIC_BMP:
	{
	    ADM_BITMAPINFOHEADER bmph;

		fread(&s16, 2, 1, fd);
	    if (s16 != 0x4D42) {
		printf("\n incorrect bmp sig.\n");
		fclose(fd);
		return 0;
	    }
		fread(&s32, 4, 1, fd);
		fread(&s32, 4, 1, fd);
		fread(&s32, 4, 1, fd);
		fread(&bmph, sizeof(bmph), 1, fd);
	    if (bmph.biCompression != 0) {
		printf("\ncannot handle compressed bmp\n");
		fclose(fd);
		return 0;
	    }
	    _offset = bmph.biSize + 14;
	    w = bmph.biWidth;
	    h = bmph.biHeight;
		bpp = bmph.biBitCount;
	}
	break;


	//Retrieve width & height
	//_______________________
    case PIC_JPEG:
	{
	    uint16_t tag = 0, count = 0, off;

	    _offset = 0;
	    fseek(fd, 0, SEEK_SET);
	    read16(fd);	// skip jpeg ffd8
	    while (count < 15 && tag != 0xFFC0) {

		tag = read16(fd);
		if ((tag >> 8) != 0xff) {
		    printf("invalid jpeg tag found (%x)\n", tag);
		}
		if (tag == 0xFFC0) {
		    read16(fd);	// size
		    read8(fd);	// precision
		    h = read16(fd);
		    w = read16(fd);
                    if(w&1) w++;
                    if(h&1) h++;
		} else {

		    off = read16(fd);
		    if (off < 2) {
			printf("Offset too short!\n");
			fclose(fd);
			return 0;
		    }
		    aprintf("Found tag : %x , jumping %d bytes\n", tag,
			    off);
		    fseek(fd, off - 2, SEEK_CUR);
		}
		count++;
	    }
	    if (tag != 0xffc0) {
		printf("Cannot fint start of frame\n");
		fclose(fd);
		return 0;
	    }
	    printf("\n %" PRIu32" x %" PRIu32"..\n", w, h);
	}
	break;

    case PIC_BMP2:
	{
	    ADM_BITMAPINFOHEADER bmph;

	    fseek(fd, 10, SEEK_SET);

#define MK32() (fcc_tab[0]+(fcc_tab[1]<<8)+(fcc_tab[2]<<16)+ \
						(fcc_tab[3]<<24))

	    fread(fcc_tab, 4, 1, fd);
	    _offset = MK32();
	    // size, width height follow as int32
	    fread(&bmph, sizeof(bmph), 1, fd);
#ifdef ADM_BIG_ENDIAN
	    Endian_BitMapInfo(&bmph);
#endif
	    if (bmph.biCompression != 0) {
		printf("\ncannot handle compressed bmp\n");
		fclose(fd);
		return 0;
	    }
	    w = bmph.biWidth;
	    h = bmph.biHeight;
		bpp = bmph.biBitCount;
	    printf("W: %d H: %d offset: %d\n", w, h, _offset);
	}

	break;

	case PIC_PNG:
	    {
    	     _offset = 0;
			 fseek(fd, 0, SEEK_SET);
			 read32(fd);
			 read32(fd);
			 read32(fd);
			 read32(fd);
			 w=read32(fd);
			 h=read32(fd);
    	     // It is big endian
    	     printf("Png seems to be %d x %d \n",w,h);
	    }
	    break;
    default:
	ADM_assert(0);
    }

	fclose(fd);

//_______________________________________
//              Now build header info
//_______________________________________
    _isaudiopresent = 0;	// Remove audio ATM
    _isvideopresent = 1;	// Remove audio ATM

#define CLR(x)              memset(& x,0,sizeof(  x));

    CLR(_videostream);
    CLR(_mainaviheader);

    _videostream.dwScale = 1;
    _videostream.dwRate = 25;
    _mainaviheader.dwMicroSecPerFrame = US_PER_PIC;;	// 25 fps hard coded
    _videostream.fccType = fourCC::get((uint8_t *) "vids");

	if (bpp)
		_video_bih.biBitCount = bpp;
	else
		_video_bih.biBitCount = 24;

    _videostream.dwLength = _mainaviheader.dwTotalFrames = _nb_file;
    _videostream.dwInitialFrames = 0;
    _videostream.dwStart = 0;
    //
    //_video_bih.biCompression= 24;
    //
    _video_bih.biWidth = _mainaviheader.dwWidth = w;
    _video_bih.biHeight = _mainaviheader.dwHeight = h;
    //_video_bih.biPlanes= 24;
    switch(_type)
    {
        case PIC_JPEG:
	        _video_bih.biCompression = _videostream.fccHandler =
	                fourCC::get((uint8_t *) "MJPG");
	        break;
	    case PIC_BMP:
	    case PIC_BMP2:
	        _video_bih.biCompression = _videostream.fccHandler = fourCC::get((uint8_t *) "DIB ");
	        break;
	    case PIC_PNG:
	        _video_bih.biCompression = _videostream.fccHandler =
	                fourCC::get((uint8_t *) "PNG ");
	        break;
        default:
            ADM_assert(0);
    }
    printf("Offset : %" PRIu32"\n", _offset);
    return 1;
}
//****************************************************************
uint8_t picHeader::setFlag(uint32_t frame, uint32_t flags)
{
    UNUSED_ARG(frame);
    UNUSED_ARG(flags);
    return 0;
}
//****************************************************************
uint32_t picHeader::getFlags(uint32_t frame, uint32_t * flags)
{
    UNUSED_ARG(frame);
    *flags = AVI_KEY_FRAME;
    return 1;
}

FILE* picHeader::openFrameFile(uint32_t frameNum)
{
    char filename[250];
    sprintf(filename, _fileMask, frameNum + _first);
    return ADM_fopen(filename, "rb");
}
bool       picHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
 uint64_t timeP=US_PER_PIC;
    timeP*=frame;
    *pts=timeP;
    *dts=timeP;
    return true;
}
bool       picHeader::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
    return false;
}
// EOF
