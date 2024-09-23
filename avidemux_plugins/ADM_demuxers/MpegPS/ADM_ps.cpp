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
#include "DIA_coreToolkit.h"
#include "ADM_indexFile.h"
#include "ADM_ps.h"
#include "prefs.h"

#include <math.h>
#define MY_CLASS psHeader
#include "ADM_coreDemuxerMpegTemplate.cpp.h"

uint32_t ADM_UsecFromFps1000(uint32_t fps1000);
uint8_t psIndexer(const char *file);

/**
      \fn open
      \brief open the flv file, gather infos and build index(es).
*/

uint8_t psHeader::open(const char *name)
{
    char *idxName=(char *)malloc(strlen(name)+6);
    uint8_t r=1;
    uint32_t indexingPref = 2;
    if (!prefs->get(INDEXING_TS_PS_INDEXING, &indexingPref)) indexingPref = 2;    

    sprintf(idxName,"%s.idx2",name);
    ListOfIndexFiles.push_back(idxName);
    if(!ADM_fileExist(idxName) || (indexingPref==0))
        r=psIndexer(name);
    if(r!=ADM_OK)
    {
        if(r==ADM_IGN)
            ADM_warning("Indexing cancelled by the user, deleting the index file. Bye.\n");
        if(!r)
            ADM_error("Indexing of %s failed, aborting\n",name);
        if(ADM_fileExist(idxName) && !ADM_eraseFile(idxName))
            ADM_warning("Could not delete %s\n",idxName);
        free(idxName);
        return r;
    }

    char *type;
    uint64_t startDts;
    uint32_t version=0;
    bool reindex=false;
    int append=PS_DEFAULT_FRAGMENT_SIZE;
    indexFile index;
    r=0;

    if(!index.open(idxName))
    {
        printf("[psDemux] Cannot open index file %s\n",idxName);
        free(idxName);
        return false;
    }
    if(!index.readSection("System"))
    {
        printf("[psDemux] Cannot read system section\n");
        goto abt;
    }

    version=index.getAsUint32("Version");
    if(version!=ADM_INDEX_FILE_VERSION)
    {
        if(GUI_Question(QT_TRANSLATE_NOOP("psdemuxer","This file's index has been created with an older version of avidemux.\nThe file must be re-indexed. Proceed?")))
            reindex=true;
        goto abt;
    }
    type=index.getAsString("Type");
    if(!type || type[0]!='P')
    {
        printf("[psDemux] Incorrect or not found type\n");
        goto abt;
    }
    if(!index.getAsUint32("Append"))
        append=0;
    printf("[psDemux] Append=%" PRIu32"\n",append);
    if(!parser.open(name,&append))
    {
        printf("[psDemux] Cannot open root file %s\n",name);
        goto abt;
    }
    if(!readVideo(&index)) 
    {
        printf("[psDemux] Cannot read Video section of %s\n",idxName);
        goto abt;
    }
    if(!readAudio(&index,name)) 
    {
        printf("[psDemux] Cannot read Audio section of %s => No audio\n",idxName);
    }
    if(!readIndex(&index))
    {
        printf("[psDemux] Cannot read index for file %s\n",idxName);
        goto abt;
    }
    if(readScrReset(&index))
    {
        ADM_info("Adjusting timestamps\n");
        // Update PTS/DTS of video taking SCR Resets into account
        int nbPoints=listOfScrGap.size();
        int index=0;
        uint64_t pivot=listOfScrGap[0].position;
        uint64_t timeOffset=0;
        uint32_t nbImage=ListOfFrames.size();
        for(int i=0;i<nbImage;i++)
        {
            dmxFrame *frame=ListOfFrames[i];
            if(frame->startAt>pivot) // next gap
            {
                    timeOffset=listOfScrGap[index].timeOffset;
                    index++;
                    if(index>=nbPoints) pivot=0xfffffffffffffffLL;
                        else pivot=listOfScrGap[index].position;
            }
            if(frame->dts!=ADM_NO_PTS) frame->dts+=timeOffset;
            if(frame->pts!=ADM_NO_PTS) frame->pts+=timeOffset;
        }
        ADM_info("Adjusted %d scr reset out of %d\n",(int)index,(int)nbPoints);
        ADM_info("Updating audio with list of SCR\n");
        for(int i=0;i<listOfAudioTracks.size();i++)
                  listOfAudioTracks[i]->access->setScrGapList(&listOfScrGap) ;
    }
    updatePtsDts();
    {
    uint32_t fps=_videostream.dwRate;
    switch(fps)
    {
        case 23976:
            _videostream.dwScale=1001;
            _videostream.dwRate=24000;
            break;
        case 29970:
            _videostream.dwScale=1001;
            _videostream.dwRate=30000;
            break;
        case 24000:
        case 25000:
        case 30000:
        case 50000:
        case 60000:
            _videostream.dwScale=1000;
            _videostream.dwRate=fps;
            break;
        default:
            _videostream.dwScale=1;
            _videostream.dwRate=90000;
            _mainaviheader.dwMicroSecPerFrame=ADM_UsecFromFps1000(fps);
            break;
    }
    if(fieldEncoded)
    {
        printf("[psDemux] Doubling fps for field-encoded video");
        if(_videostream.dwRate<=45000)
            _videostream.dwRate*=2;
        else if(!(_videostream.dwScale%2))
            _videostream.dwScale/=2;
        if(_mainaviheader.dwMicroSecPerFrame)
            _mainaviheader.dwMicroSecPerFrame=ADM_UsecFromFps1000(fps*2);
        else
            printf(", new time base: %d / %d",_videostream.dwScale,_videostream.dwRate);
        printf("\n");
    }
    }
    _videostream.dwLength= _mainaviheader.dwTotalFrames=ListOfFrames.size();
    printf("[psDemux] Found %d video frames\n",_videostream.dwLength);
    if(_videostream.dwLength)_isvideopresent=1;
//***********
    
    psPacket=new psPacketLinear(0xE0);
    if(psPacket->open(name,append)==false)
    {
        printf("psDemux] Cannot psPacket open the file\n");
        goto abt;
    }
    r=1;
    for(int i=0;i<listOfAudioTracks.size();i++)
    {
        ADM_psTrackDescriptor *desc=listOfAudioTracks[i];
        ADM_audioStream *audioStream=ADM_audioCreateStream(&desc->header,desc->access);
        if(!audioStream)
        {
            
        }else       
        {
                desc->stream=audioStream;
        }
    }
abt:
    index.close();
    if(reindex)
    {
        uint8_t success=ADM_eraseFile(idxName);
        free(idxName);
        if(success)
            r=open(name);
        else
            ADM_error("Can't delete old index file.\n");
    }else
    {
        free(idxName);
        if(r)
            ADM_info("Loaded %s successfully\n",name);
        else
            ADM_warning("Loading %s failed\n",name);
    }
    return r;
}



/*
    __________________________________________________________
*/

void psHeader::Dump(void)
{
 
}
/**
    \fn close
    \brief cleanup
*/

uint8_t psHeader::close(void)
{
    // Destroy index
    int nb=ListOfFrames.size();
    for(int i=0;i<nb;i++)
    {
        if(ListOfFrames[i]) delete ListOfFrames[i];
        ListOfFrames[i]=0;
    }
    if(psPacket)
    {
        psPacket->close();
        delete psPacket;
        psPacket=NULL;
    }
    nb=listOfAudioTracks.size();
    for(int i=0;i<nb;i++)
    {
        delete listOfAudioTracks[i];
        listOfAudioTracks[i] = 0;
    }
    listOfAudioTracks.clear();
    nb = ListOfIndexFiles.size();
    for (int i=0; i<nb; i++)
    {
        mfcleanup(ListOfIndexFiles.back());
        ListOfIndexFiles.pop_back();
    }
    return 1;
}
/**
    \fn psHeader
    \brief constructor
*/

 psHeader::psHeader( void ) : vidHeader()
{ 
    fieldEncoded=false;
    lastFrame=0xffffffff;
    videoTrackSize=0;
    videoDuration = ADM_NO_PTS;
}
/**
    \fn psHeader
    \brief destructor
*/

 psHeader::~psHeader(  )
{
  close();
}

/**
        \fn getFrame
*/

uint8_t  psHeader::getFrame(uint32_t frame,ADMCompressedImage *img)
{
    if(frame>=ListOfFrames.size()) return 0;
    getFlags(frame,&(img->flags));
    dmxFrame *pk=ListOfFrames[frame];
    if(frame==(lastFrame+1) && pk->type!=1) // the next frame, not an intra
    {
        lastFrame++;
        bool r=psPacket->read(pk->len,img->data);
             img->dataLength=pk->len;
             img->demuxerFrameNo=frame;
             img->demuxerDts=pk->dts;
             img->demuxerPts=pk->pts;
             //printf("[>>>] %d:%02x %02x %02x %02x\n",frame,img->data[0],img->data[1],img->data[2],img->data[3]);
             return r;
    }
    if(pk->type==1) // an intra
    {
        if(!psPacket->seek(pk->startAt,pk->index)) return false;
         bool r=psPacket->read(pk->len,img->data);
             img->dataLength=pk->len;
             img->demuxerFrameNo=frame;
             img->demuxerDts=pk->dts;
             img->demuxerPts=pk->pts;
             //printf("[>>>] %d:%02x %02x %02x %02x\n",frame,img->data[0],img->data[1],img->data[2],img->data[3]);
             lastFrame=frame;
             return r;
    }

    // a random frame: need to rewind first, then seek forward
    uint32_t startPoint=frame;
    while(startPoint && ListOfFrames[startPoint]->type!=1)
        startPoint--;
    printf("[psDemux] Wanted frame %" PRIu32", going back to frame %" PRIu32", last frame was %" PRIu32",\n",frame,startPoint,lastFrame);
    pk=ListOfFrames[startPoint];
    if(!psPacket->seek(pk->startAt,pk->index))
    {
        printf("[psDemux] Failed to rewind to frame %" PRIu32"\n",startPoint);
        return false;
    }

    // now seek forward
    while(startPoint<frame)
    {
        pk=ListOfFrames[startPoint];
        if(!psPacket->read(pk->len,img->data))
        {
            printf("[psDemux] Read failed for frame %" PRIu32"\n",startPoint);
            lastFrame=0xffffffff;
            return false;
        }
        lastFrame=startPoint;
        startPoint++;
    }
    pk=ListOfFrames[frame];
    lastFrame++;

    bool r=psPacket->read(pk->len,img->data);

    img->dataLength=pk->len;
    img->demuxerFrameNo=frame;
    img->demuxerDts=pk->dts;
    img->demuxerPts=pk->pts;
    //printf("[>>>] %d:%02x %02x %02x %02x\n",frame,img->data[0],img->data[1],img->data[2],img->data[3]);
    return r;
}

/**
        \fn getExtraHeaderData
*/
uint8_t  psHeader::getExtraHeaderData(uint32_t *len, uint8_t **data)
{
                *len=0; //_tracks[0].extraDataLen;
                *data=NULL; //_tracks[0].extraData;
                return true;            
}
//EOF
