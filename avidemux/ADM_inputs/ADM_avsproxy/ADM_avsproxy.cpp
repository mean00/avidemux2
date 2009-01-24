/*

(c) Mean 2006
*/
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "ADM_assert.h"
#include "ADM_default.h"
#include "ADM_editor/ADM_Video.h"

#include "fourcc.h"
#include "ADM_avsproxy.h"
#include "ADM_avsproxy_internal.h"


#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_3GP
#include "ADM_osSupport/ADM_debug.h"




avsHeader::avsHeader()
{
    mySocket=0;
}
 avsHeader::~avsHeader(  )
{
    close();   
}


uint8_t avsHeader::open(const char *name)
{
    mySocket=0;
    if(!bindMe(9999))
    {
        printf("[avsProxy]Open failed\n");
        return 0;
    }
    // now time to grab some info
    avsInfo info;
    if(!askFor(AvsCmd_GetInfo,0,sizeof(info),(uint8_t*)&info))
    {
        printf("Get info failed\n");
        return 0;   
    }
    // Build header..
    _isaudiopresent = 0;	// Remove audio ATM
    _isvideopresent = 1;	// Remove audio ATM

#define CLR(x)              memset(& x,0,sizeof(  x));

    CLR(_videostream);
    CLR(_mainaviheader);

    _videostream.dwScale = 1000;
    _videostream.dwRate = info.fps1000;
    _mainaviheader.dwMicroSecPerFrame = 40000;;	// 25 fps hard coded
    _videostream.fccType = fourCC::get((uint8_t *) "YV12");

    _video_bih.biBitCount = 24;

    _videostream.dwLength = _mainaviheader.dwTotalFrames = info.nbFrames;
    _videostream.dwInitialFrames = 0;
    _videostream.dwStart = 0;
    //
    //_video_bih.biCompression= 24;
    //
    _video_bih.biWidth = _mainaviheader.dwWidth = info.width;
    _video_bih.biHeight = _mainaviheader.dwHeight = info.height;
    _video_bih.biCompression = _videostream.fccHandler =  fourCC::get((uint8_t *) "YV12");
    
    printf("Connection to avsproxy succeed\n");
    return 1;
}



uint8_t  avsHeader::getFrameNoAlloc(uint32_t framenum,ADMCompressedImage *img)
{
    uint32_t page=(_mainaviheader.dwWidth*_mainaviheader.dwHeight*3)>>1;
    
    if(framenum>=_mainaviheader.dwTotalFrames)
    {
        printf("Avisynth proxy out of bound %u / %u\n",framenum,_mainaviheader.dwTotalFrames);
        return 0;
    }
    if(!askFor(AvsCmd_GetFrame,framenum,page,img->data))
    {
        printf("Get frame failed for frame %u\n",framenum);
        return 0;   
    }
    img->dataLength=page;
    return 1;
}



