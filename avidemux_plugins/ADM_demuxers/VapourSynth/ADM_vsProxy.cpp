/***************************************************************************
    \file ADM_vs.cpp
    \author (C) 2015 by mean    email                : fixounet@free.fr
    \brief VapourSynth demuxer

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
#include "fourcc.h"
#include "DIA_coreToolkit.h"
#include "ADM_videoInfoExtractor.h"


#include "ADM_vsProxy.h"
#include <math.h>

extern uint8_t ADM_InitMemcpy(void);

static const VSAPI *vsapi = NULL;
#if 0
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif
/**
 */
int main(int ac, char **av)
{
    if(ac!=2)
    {
        printf("vsProxy scriptFile\n");
        exit(-1); 
    }
    ADM_InitMemcpy();
    
#ifdef _WIN32
    WSADATA wsaData;
    int iResult;
            ADM_info("Initializing WinSock\n");
            fflush(stdout);
            iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
            if (iResult != NO_ERROR)
            {
                    printf("Error at WSAStartup()\n");
                    fflush(stdout);
                    exit(-1);
            }	
            ADM_info("WinSock ok\n");

#endif    
    
    
    vapourSynthProxy proxy;
    bool r=proxy.run(av[1]);
    if(r)
    {
        printf("Success\n");
        exit(0);
    }else
    {
        printf("Failure\n");
        exit(-1);
    }
    return 0;
}

/**
 */
vapourSynthProxy::vapourSynthProxy()
{
    _script=NULL;
    _buffer=NULL;
}
/**
 */
vapourSynthProxy::~vapourSynthProxy()
{
    if(_buffer)
    {
        delete [] _buffer;
        _buffer=NULL;
    }
}
/**
 * 
 */
void vapourSynthProxy::abort(void)
{
        if(_script)
        {
            vsscript_freeScript(_script);
            _script=NULL;
        }
        vsscript_finalize();
}
/**
 * 
 * @param vi
 * @return 
 */
bool vapourSynthProxy::fillInfo( const VSVideoInfo *vi)
{
    ADM_info("Format    : %s\n",vi->format->name);
    ADM_info("FrameRate : %d / %d\n",vi->fpsNum,vi->fpsDen);
    ADM_info("Width     : %d\n",vi->width);
    ADM_info("Height    : %d\n",vi->height);
    ADM_info("Frames    : %d\n",vi->numFrames);
    ADM_info("Flags     : 0x%x\n",vi->flags);
    
    _info.width=vi->width;
    _info.height=vi->height;
    double fps=(double)vi->fpsNum/(double)vi->fpsDen;
    _info.fps1000=fps*1000;
    _info.version=AVSHEADER_API_VERSION;
    _info.nbFrames=vi->numFrames;
    _info.frequency=0;
    _info.channels=0;
    if(strcmp(vi->format->name,"YUV420P8"))
    {
        printf("Only YUV420P8 supported!\n");
        return false;
    }
    return true;
}
/**
 */
bool vapourSynthProxy::run(const char *name)
{

    ADM_info("Opening %s as VapourSynth file\n",name);
    
   
    
    if (!vsscript_init()) 
    {
          ADM_warning("Cannot initialize vsapi script_init. Check PYTHONPATH\n");
          return false;
    }
    vsapi = vsscript_getVSApi();
    if(!vsapi)
    {
        ADM_warning("Cannot get vsAPI entry point\n");
        vsscript_finalize();
        return 0;
    }
        
    ADM_info("VapourSynth init ok, opening file..\n");
    if (vsscript_evaluateFile(&_script, name, 0)) 
    {
        ADM_warning("Evaluate script failed <%s>\n", vsscript_getError(_script));
        abort();
        return 0;
    }
    _node = vsscript_getOutput(_script, 0);
    if (!_node) 
    {
       ADM_warning("vsscript_getOutputNode failed\n");
       abort();
       return 0;
    }
    const VSVideoInfo *vi = vsapi->getVideoInfo(_node);
    if(!vi)
    {
          ADM_warning("Cannot get information on node\n");
          abort();
          return 0;
    }
   if (!isConstantFormat(vi) || !vi->numFrames) 
   {
         ADM_warning("Varying format => unsupported\n");
          vsapi->freeNode(_node);
          abort();
          return false;
    }
      
    if(!fillInfo(vi))
    {
        printf("Unsupported settings in script files\n");
        abort();
        return 0;
    }

    
    _buffer=new uint8_t[vi->width*vi->height*4];
    
  
    //--
    
    
    avsSocket sket;
    uint32_t port=9999;
    if(!sket.createBindAndAccept(&port))
    {
        ADM_error("Cannot bind socket\n");
        abort();
        return false;
    }
    ADM_info("Listening on port %d\n",(int)port);
    avsSocket *slave=sket.waitForConnect(20*1000);
    if(!slave)
    {
        ADM_warning("No connection , timeout\n");
        return false;
    }
    
    bool success=manageSlave(slave,vi);
    delete slave;
    vsapi->freeNode(_node);
    vsscript_freeScript(_script);
    vsscript_finalize();
    _node=NULL;
    _script=NULL;
    return success;
    
}
/**
 * 
 * @param vi
 * @param frame
 * @return 
 */

bool vapourSynthProxy::packFrame( const VSVideoInfo *vi,const VSFrameRef *frame)
{
    const int mapp[3]={0,2,1};
    uint8_t *target=_buffer;
    for (int plane = 0; plane < vi->format->numPlanes; plane++) 
    {
        int p=mapp[plane];
        int stride = vsapi->getStride(frame, p);
        const uint8_t *readPtr = vsapi->getReadPtr(frame, p);
        int rowSize = _info.width;
        int height  = _info.height;

        if(p)
        {
            rowSize>>=1;height>>=1;
        }
        for (int y = 0; y < height; y++) 
        {

            memcpy(target,readPtr,rowSize);
            target  += rowSize; // useless memcpy...
            readPtr += stride;
        }
    }
    return true;
}

/**
 * 
 * @param slave
 * @return 
 */
bool vapourSynthProxy::manageSlave(avsSocket *slave,const VSVideoInfo *vi)
{
    uint32_t cmd,frame,len;
    uint8_t payload[1000]; // Never used normally...
    while(1)
    {        
	if(!slave->receive(&cmd,&frame,&len,payload))
        {
                printf("Error in receive\n");
                fflush(stdout);
                return false;

        }
        switch(cmd)
        {
            case AvsCmd_GetInfo:
                printf("Received get info...\n");
                fflush(stdout);
                if(len!=8)
                {
                    // Version
                    printf("This version of avsproxy is not compatible with the avidemux version you are using\n");
                    fflush(stdout);
                    return false;
                }
                uint32_t api,ver;
                api=*(uint32_t *)payload;
                ver=*(uint32_t *)(payload+4);
                printf("Connection from avidemux, api=%d version=%d\n",api,ver);
                if(api!=AVSHEADER_API_VERSION)
                {
                                printf("This version of avsproxy has api version %d, avidemux has version %d, exiting\n",AVSHEADER_API_VERSION,api);
                                fflush(stdout);
                                return false;
                }
                slave->sendData(AvsCmd_SendInfo,0,sizeof(_info),(uint8_t *)&_info);
                break;
        case AvsCmd_GetFrame:
            char errMsg[1024];
            int error = 0;

            const VSFrameRef *vsframe = vsapi->getFrame(frame, _node, errMsg, sizeof(errMsg));
            if (!vsframe) 
            { 
                ADM_error("Error getting frame %d\n",frame);
                return false;
            }
            packFrame(vi,vsframe);
            vsapi->freeFrame(vsframe);
            if(!slave->sendData(AvsCmd_SendFrame,frame,(_info.width*_info.height*3)>>1,(uint8_t *)_buffer))
            {
                ADM_error("Error sending data for frame %d\n",frame);
                return false;
            }
            break;
        }

    }
    return false;
}
//EOF
