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
/**
 * 
 */
picHeader::picHeader(void)
{
	_nbFiles = 0;
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
    fread(img->data, _imgSize[framenum] , 1, fd);
    img->dataLength = _imgSize[framenum];

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
	_nbFiles = 0;
        _imgSize.clear();
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
/**
 * 
 * @param inname
 * @param nbOfDigits
 * @param filePrefix
 */
static void splitImageSequence(std::string inname,int &nbOfDigits, int &first,std::string &filePrefix)
{   
    std::string workName=inname;   
    int num=0;
    first=0;
    while( workName.size() )
    {
        const char c=workName[workName.size()-1];
        if((c<'0') || (c>'9'))
                break;       
        num++;
        first=first*10+(c-'0');
        workName.resize(workName.size()-1);
    }
    nbOfDigits=num;
    filePrefix=workName;
}
/**
 * \fn open
 * @param inname
 * @return 
 */
uint8_t picHeader::open(const char *inname)
{
    uint32_t nnum;
    FILE *fd;    
    uint32_t bpp = 0;
    
    // 1- identity the image type    
    ADM_PICTURE_TYPE imageType=ADM_identifyImageFile(inname,&_w,&_h);
    if(imageType==ADM_PICTURE_UNKNOWN)
    {
        ADM_warning("\n Cannot open that file!\n");
	return 0;
    }
    _type=imageType;
    
    // Then spit the name in name and extension
    int nbOfDigits;
    std::string name,extension;
    std::string prefix;
    ADM_PathSplit(std::string(inname),name,extension);
    splitImageSequence(name,nbOfDigits,_first,prefix);
    
    if(!nbOfDigits) // no digit at all
    {
        _nbFiles=1;
    }
    else
    {
        char realstring[1024];
        sprintf(realstring, "%s%%0%" PRIu32"d.%s", prefix.c_str(), nbOfDigits, extension.c_str());
        _filePrefix=std::string(realstring);
        _nbFiles = 0;
        for (uint32_t i = 0; i < MAX_ACCEPTED_OPEN_FILE; i++)
        {
                sprintf(realstring, _filePrefix.c_str(), i + _first);
                printf(" %" PRIu32" : %s\n", i, realstring);

                fd = ADM_fopen(realstring, "rb");
                if (fd == NULL)
                        break;
                fclose(fd);
                _nbFiles++;
        }
    }
    printf("\n found %" PRIu32" images\n", _nbFiles);
    //_________________________________
    // now open them and assign imgSize
    //__________________________________
    for (uint32_t i = 0; i < _nbFiles; i++)
    {
            fd = openFrameFile(i);
            ADM_assert(fd);
            fseek(fd, 0, SEEK_END);
            _imgSize.push_back(ftell(fd));
            fclose(fd);
    }
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

    _videostream.dwLength = _mainaviheader.dwTotalFrames = _nbFiles;
    _videostream.dwInitialFrames = 0;
    _videostream.dwStart = 0;
    //
    //_video_bih.biCompression= 24;
    //
    _video_bih.biWidth = _mainaviheader.dwWidth = _w;
    _video_bih.biHeight = _mainaviheader.dwHeight = _h;
    //_video_bih.biPlanes= 24;
    switch(_type)
    {
#define SET_FCC(x) _video_bih.biCompression = _videostream.fccHandler =  fourCC::get((uint8_t *) x);break;
	        break;
            case ADM_PICTURE_JPG : SET_FCC("MJPG")
	    case ADM_PICTURE_BMP : 
	    case ADM_PICTURE_BMP2: SET_FCC("DIB ")
	    case ADM_PICTURE_PNG : SET_FCC("PNG ")
            default:
                ADM_assert(0);
    }
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
    sprintf(filename, _filePrefix.c_str(), frameNum + _first);
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
