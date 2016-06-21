/***************************************************************************
    copyright            : (C) 2007/2009 by mean
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
#include "fourcc.h"
#include <math.h>
#include "DIA_coreToolkit.h"
#include "ADM_indexFile.h"
#include "ADM_ts.h"
#include "ADM_videoInfoExtractor.h"

#define MY_CLASS tsHeader
#include "ADM_coreDemuxerMpegTemplate.cpp"



uint32_t ADM_UsecFromFps1000(uint32_t fps1000);

/**
      \fn open
      \brief open the flv file, gather infos and build index(es).
*/

uint8_t tsHeader::open(const char *name)
{
    char *idxName=(char *)malloc(strlen(name)+6);
    bool r=false;
    FP_TYPE appendType=FP_DONT_APPEND;
    uint32_t append;
    char *type;
    uint64_t startDts;
    uint32_t version=0;

    sprintf(idxName,"%s.idx2",name);
    indexFile index;
    if(!index.open(idxName))
    {
        printf("[tsDemux] Cannot open index file %s\n",idxName);
        free(idxName);
        return false;
    }
    if(!index.readSection("System"))
    {
        printf("[tsDemux] Cannot read system section\n");
        goto abt;
    }
    type=index.getAsString("Type");
    if(!type || type[0]!='T')
    {
        printf("[tsDemux] Incorrect or not found type\n");
        goto abt;
    }

    version=index.getAsUint32("Version");
    if(version!=ADM_INDEX_FILE_VERSION)
    {
        GUI_Error_HIG("Error","This file's index has been created with an older version of avidemux.\nPlease delete the idx2 file and reopen.");
        goto abt;
    }
    append=index.getAsUint32("Append");
    printf("[tsDemux] Append=%" PRIu32"\n",append);
    if(append) appendType=FP_APPEND;
    if(!parser.open(name,&appendType))
    {
        printf("[tsDemux] Cannot open root file (%s)\n",name);
        goto abt;
    }
    if(!readVideo(&index)) 
    {
        printf("[tsDemux] Cannot read Video section of %s\n",idxName);
        goto abt;
    }

    if(!readAudio(&index,name)) 
    {
        printf("[tsDemux] Cannot read Audio section of %s => No audio\n",idxName);
    }

    if(!readIndex(&index))
    {
        printf("[tsDemux] Cannot read index for file %s\n",idxName);
        goto abt;
    }
    if(!ListOfFrames.size())
    {
        ADM_info("[TSDemux] No video frames\n");
        goto abt;
    }
    updateIdr();
    updatePtsDts();
    _videostream.dwLength= _mainaviheader.dwTotalFrames=ListOfFrames.size();
    printf("[tsDemux] Found %d video frames\n",_videostream.dwLength);
    if(_videostream.dwLength)_isvideopresent=1;
//***********
    
    tsPacket=new tsPacketLinear(videoPid);
    if(tsPacket->open(name,appendType)==false) 
    {
        printf("tsDemux] Cannot tsPacket open the file\n");
        goto abt;
    }
    r=true;
    for(int i=0;i<listOfAudioTracks.size();i++)
    {
        ADM_tsTrackDescriptor *desc=listOfAudioTracks[i];
        ADM_audioStream *audioStream=ADM_audioCreateStream(&desc->header,desc->access);
        if(!audioStream)
        {
            
        }else       
        {
                desc->stream=audioStream;
                audioStream->setLanguage(desc->language);
        }
    }
abt:
    free(idxName);
    index.close();
    printf("[tsDemuxer] Loaded %d\n",r);
    return r;
}

/**
    \fn updateIdr
    \brief if IDR are present, handle them as intra and I as P frame
            if not, handle I as intra, there might be some badly decoded frames (missing ref)
*/
bool tsHeader::updateIdr()
{
    int nbIdr=0;
    int nbI=0,nbP=0,nbB=0;
    if(!ListOfFrames.size()) return false;
    for(int i=0;i<ListOfFrames.size();i++)
    {
        int type=ListOfFrames[i]->type;
        switch(type)
        {
            case 1: nbI++;break;
            case 2: nbP++;break;
            case 3: nbB++;break;
            case 4: nbIdr++;break;
            default:
                    ADM_assert(0);
                    break;
        }
    }
    printf("[TsDemuxer] Found %d I, %d B, %d P\n",nbI,nbB,nbP);
    printf("[TsH264] Found %d IDR\n",nbIdr);
    if(nbIdr>1) // Change IDR to I and I to P...
    { 
        printf("[TsH264] Remapping IDR to I and I TO P\n");
        for(int i=0;i<ListOfFrames.size();i++)
        {
            switch(ListOfFrames[i]->type)
            {
                case 4: ListOfFrames[i]->type=1;break;
                case 1: 
                        if(i)
                            ListOfFrames[i]->type=2;break;
                default: break;
            }
        }
    }else // I & IDR are all keyframes
    {
        for(int i=0;i<ListOfFrames.size();i++)
        {
            if(ListOfFrames[i]->type==4)
                ListOfFrames[i]->type=1;
        }
        
    }
    return true;
}

/*
    __________________________________________________________
*/

void tsHeader::Dump(void)
{
 
}
/**
    \fn close
    \brief cleanup
*/

uint8_t tsHeader::close(void)
{
    ADM_info("Destroying TS demuxer\n");
    // Destroy index
    int n=ListOfFrames.size();
    for(int i=0;i<n;i++)
        delete ListOfFrames[i];
    ListOfFrames.clear();

    // Destroy audio tracks
    n=listOfAudioTracks.size();
    for(int i=0;i<n;i++)
    {
        ADM_tsTrackDescriptor *desc=listOfAudioTracks[i];
        delete desc;
        listOfAudioTracks[i]=NULL;
    } // Container will be destroyed by vector destructor
    listOfAudioTracks.clear();
    if(tsPacket)
    {
        tsPacket->close();
        delete tsPacket;
        tsPacket=NULL;
    }
    return 1;
}
/**
    \fn tsHeader
    \brief constructor
*/

 tsHeader::tsHeader( void ) : vidHeader()
{ 
    interlaced=false;
    lastFrame=0xffffffff;
    videoPid=0;
    videoNeedEscaping=false;
    
}
/**
    \fn tsHeader
    \brief destructor
*/

 tsHeader::~tsHeader(  )
{
  close();
}


/**
        \fn getFrame
*/

#define UNESCAPE() if(r==true && 0 && videoNeedEscaping)  \
                    {\
                        uint32_t l=img->dataLength;\
                        uint32_t l2=ADM_unescapeH264(l,img->data, img->data);\
                        if(l!=l2)\
                            memset(img->data+l2,0,l-l2);\
                    }

uint8_t  tsHeader::getFrame(uint32_t frame,ADMCompressedImage *img)
{
    if(frame>=ListOfFrames.size()) return 0;
    dmxFrame *pk=ListOfFrames[frame];
    // next frame
    if(frame==(lastFrame+1) && pk->type!=1)
    {
        lastFrame++;
        bool r=tsPacket->read(pk->len,img->data);
             img->dataLength=pk->len;
             img->demuxerFrameNo=frame;
             img->demuxerDts=pk->dts;
             img->demuxerPts=pk->pts;
             //printf("[>>>] %d:%02x %02x %02x %02x\n",frame,img->data[0],img->data[1],img->data[2],img->data[3]);
             getFlags(frame,&(img->flags));
             UNESCAPE();
             return r;
    }
    // Intra ?
    if(pk->type==1 || pk->type==4)
    {
        if(!tsPacket->seek(pk->startAt,pk->index)) return false;
         bool r=tsPacket->read(pk->len,img->data);
             img->dataLength=pk->len;
             img->demuxerFrameNo=frame;
             img->demuxerDts=pk->dts;
             img->demuxerPts=pk->pts;
             getFlags(frame,&(img->flags));
             //printf("[>>>] %d:%02x %02x %02x %02x\n",frame,img->data[0],img->data[1],img->data[2],img->data[3]);
             lastFrame=frame;
             UNESCAPE();
             return r;

    }
    
    // Random frame
    // Need to rewind, then forward
    int startPoint=frame;
    while(startPoint && !ListOfFrames[startPoint]->startAt) startPoint--;
    printf("[tsDemux] Wanted frame %" PRIu32", going back to frame %" PRIu32", last frame was %" PRIu32",\n",frame,startPoint,lastFrame);
    pk=ListOfFrames[startPoint];
    if(!tsPacket->seek(pk->startAt,pk->index)) 
    {
            printf("[tsDemux] Failed to rewind to frame %" PRIu32"\n",startPoint);
            return false;
    }
    // Now forward
    while(startPoint<frame)
    {
        pk=ListOfFrames[startPoint];
        if(!tsPacket->read(pk->len,img->data))
        {
            printf("[tsDemux] Read fail for frame %" PRIu32"\n",startPoint);
            lastFrame=0xffffffff;
            return false;
        }
        startPoint++;
        lastFrame=startPoint;
    }
    pk=ListOfFrames[frame];
    lastFrame++;
    bool r=tsPacket->read(pk->len,img->data);
         img->dataLength=pk->len;
         img->demuxerFrameNo=frame;
         img->demuxerDts=pk->dts;
         img->demuxerPts=pk->pts;
         //printf("[>>>] %d:%02x %02x %02x %02x\n",frame,img->data[0],img->data[1],img->data[2],img->data[3]);
         getFlags(frame,&(img->flags));
         UNESCAPE();
         return r;
}


//EOF
