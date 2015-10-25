/***************************************************************************
           \file               gui_savenew.cpp
           \brief Save movie files

    copyright            : (C) 2002/2009 by mean
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

#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_threads.h"

#include "avi_vars.h"
#include "prototype.h"
#include "DIA_coreToolkit.h"
#include "DIA_enter.h"
#include "ADM_coreAudio.h"
#include "ADM_encoderConf.h"

#include "ADM_encoderConf.h"


#include "DIA_fileSel.h"
#include "ADM_commonUI/GUI_ui.h"
#include "ADM_muxer.h"
#include "ADM_muxerGate/include/ADM_videoCopy.h"
#include "ADM_filterChain.h"
#include "ADM_muxerGate/include/ADM_videoProcess.h"
#include "ADM_bitstream.h"
#include "ADM_filterChain.h"
#include "ADM_videoEncoderApi.h"
#include "ADM_vidMisc.h"
#include "ADM_slave.h"

#define ADM_MAX_AUDIO_STREAM 10
/*

*/
//ADM_muxer               *ADM_MuxerSpawnFromIndex(int index);
#include "ADM_muxerProto.h"
extern ADM_audioStream  *audioCreateEncodingStream(EditableAudioTrack *ed,bool globalHeader,uint64_t startTime);
extern ADM_audioStream  *audioCreateCopyStream(uint64_t startTime,int32_t shift,ADM_audioStream *input, bool needPerfectAudio);

/**
    \class admSaver
    \brief Wrapper for saving

*/
class admSaver
{
protected:
        uint64_t             startAudioTime; // Actual start time (for both audio & video) can differ from markerA
        std::string          fileName;
        std::string          logFileName;
        ADM_muxer            *muxer;
        ADM_videoFilterChain *chain;
        ADM_audioStream      *audio;
        ADM_videoStream      *video;
        uint64_t             markerA,markerB;
        int                  muxerIndex;
        int                  videoEncoderIndex;
        ADM_coreVideoEncoder *handleFirstPass(ADM_coreVideoEncoder *pass1);
        ADM_videoStream      *setupVideo(void);
        bool                  setupAudio();
        ADM_audioStream      *audioAccess[ADM_MAX_AUDIO_STREAM]; // audio tracks to feed to the muxer
        int                   nbAudioTracks;
        
public:
                              admSaver(const char *out);
                              ~admSaver();
        bool                  save(void);

};


/**
    \fn A_Save
    \brief Instantiate & initiate streams to feed muxer
*/
int A_Save(const char *name)
{
    admSaver *save=new admSaver(name);
    bool r=save->save();
    delete save;
    ADM_slaveSendResult(r);
    return (int)r;
}

/**
    \fn admSaver
*/
 admSaver::admSaver(const char *out)
{
        nbAudioTracks=video_body->getNumberOfActiveAudioTracks();
        if(nbAudioTracks>=ADM_MAX_AUDIO_STREAM) 
        {
            ADM_warning("Too much audio tracks, limiting to %d\n",ADM_MAX_AUDIO_STREAM);
            nbAudioTracks=ADM_MAX_AUDIO_STREAM;
        }
        fileName=std::string(out);
        logFileName=fileName;
        logFileName+=std::string(".stats");
        muxer=NULL;
        chain=NULL;
        audio=NULL;
        video=NULL;
        for(int i=0;i<ADM_MAX_AUDIO_STREAM;i++)
            audioAccess[i]=NULL;
        markerA=video_body->getMarkerAPts();
        markerB=video_body->getMarkerBPts();
        startAudioTime=markerA; // Actual start time (for both audio & video ), 
        muxerIndex=UI_GetCurrentFormat();
        videoEncoderIndex=UI_getCurrentVCodec();
        ADM_info("[Save] Encoder index=%d\n",videoEncoderIndex);
}
/**
    \fn ~admSaver
*/
admSaver::~admSaver()
{
 if(muxer)
        delete muxer;
 muxer=NULL;
 logFileName="";
 for(int i=0;i<nbAudioTracks;i++)
    if ( audioAccess[i])
    {
        delete audioAccess[i];
        audioAccess[i]=NULL;
    }
 if(video)
        delete video;
 video=NULL;
  if(chain)
  {
        destroyVideoFilterChain(chain);
   }
   chain=NULL;

    // encoder must not be destroyed, it will be destroyed with video
}


/**
    \fn handleFirstPass
*/
ADM_coreVideoEncoder *admSaver::handleFirstPass(ADM_coreVideoEncoder *pass1)
{
#define BUFFER_SIZE 1920*1800*3
ADM_coreVideoFilter  *last;
bool skip=false;
    int sze=chain->size();
    ADM_assert(sze);
    last=(*chain)[sze-1]; // Grab last filter
    uint64_t videoDuration=last->getInfo()->totalDuration;

    if(videoDuration<5000) videoDuration=5000;
    printf("[Save] Performing Pass one,using %s as log file\n",logFileName.c_str());
    
    if(ADM_fileExist(logFileName.c_str()))
    {
        if(GUI_Question(QT_TRANSLATE_NOOP("adm","Reuse previous first pass data ?\nWarning, the settings must be close.")))
        {
            skip=true;
        }
    }
    
    if(!skip)
    {
        pass1->setPassAndLogFile(1,logFileName.c_str());
        if(false==pass1->setup())
        {
            printf("[Save] setup failed for pass1 encoder\n");
            delete pass1;
            return NULL;
        }
        ADMBitstream bitstream;

        uint8_t *buffer=new uint8_t[BUFFER_SIZE];
        bitstream.data=buffer;
        bitstream.bufferSize=BUFFER_SIZE;
        DIA_workingBase  *encoding=createWorking("Pass1");
        int nbFrames=0;
        while(pass1->encode(&bitstream))
        {
            if(bitstream.pts!=ADM_NO_PTS)
            {
                float f=100;
                f/=videoDuration;
                f*=bitstream.pts;
                uint32_t percent=(uint32_t)f;
                encoding->update(percent);
            }
            nbFrames++;
        }
        delete encoding;
        delete [] buffer;
        delete pass1;
        encoding=NULL;
        buffer=NULL;
        pass1=NULL;

        printf("[Save] Pass 1 done, encoded %d frames, restarting for pass 2\n",nbFrames);
        // Destroy filter chain & create the new encoder
        destroyVideoFilterChain(chain);
        chain=NULL;
    }
    chain=createVideoFilterChain(markerA,markerB);

    if(!chain)
    {
        printf("[Save] Cannot recreate video filter chain\n");
        return NULL;
    }
    int sz=chain->size();
    ADM_assert(sz);
    last=(*chain)[sz-1]; // Grab last filter
    ADM_coreVideoEncoder *pass2=createVideoEncoderFromIndex(last,videoEncoderIndex,muxer->useGlobalHeader()); 
    if(!pass2)
    {
        printf("[Save] Cannot create encoder for pass 2\n");
        return NULL;
    }
    pass2->setPassAndLogFile(2,logFileName.c_str());
    return pass2;
}
/**
    \fn setupVideo
    \brief prepare video (copy or process)
*/
ADM_videoStream *admSaver::setupVideo(void)
{
    ADM_videoStream *video=NULL;
    // Video Stream
    if(!videoEncoderIndex) // Copy
    {
        aviInfo info;
        video_body->getVideoInfo(&info);

        uint8_t *extra;
        uint32_t extraLen;
        video_body->getExtraHeaderData(&extraLen,&extra);
//#warning do something better
        ADM_videoStreamCopy *copy=NULL;
        if(isH264Compatible(info.fcc))
        {
            if(muxer->preferH264AnnexB())
            {
                ADM_info("The video stream is H264\n");
                ADM_info("The muxer prefers AnnexB H264 bitstream\n");
                if(extraLen)
                {
                    // Need to do mp4 to annexB    
                    ADM_info("Probably AnnexB bitstream\n");
                    copy=new ADM_videoStreamCopyToAnnexB(markerA,markerB);
                }
            }else
            {
                if(!extraLen)
                {
                    ADM_info("Probably AnnexB bitstream\n");
                    copy=new ADM_videoStreamCopyFromAnnexB(markerA,markerB);
                }
            }
         }
        
        if(!copy)
        {
            copy=new ADM_videoStreamCopy(markerA,markerB);
        }
        video=copy;
        // In that case, get the real time and update audio with it...
        // Because we might have go back in time to catch the first intra
        startAudioTime=copy->getStartTime();
      
        
    }else
    {
        // 1- create filter chain
        //******************************

        chain=createVideoFilterChain(markerA,markerB);
        if(!chain)
        {
                GUI_Error_HIG("Video","Cannot instantiante video Chain");
                return NULL;
        }
        // 2- Create Encoder
        //********************
        int sz=chain->size();
        ADM_assert(sz);
        ADM_coreVideoFilter  *last;
        last=(*chain)[sz-1]; // Grab last filter
        ADM_coreVideoEncoder *encoder=createVideoEncoderFromIndex(last,videoEncoderIndex,muxer->useGlobalHeader()); // FIXME GLOBAL HEADERS
        if(!encoder)
        {
           GUI_Error_HIG("Video","Cannot create encoder");
           return NULL;
        }
        // 3 dual Pass ?
        //*****************
        if(encoder->isDualPass())
        {
            encoder=handleFirstPass(encoder); // Do pass 1 and switch to pass 2
            if(!encoder)
            {
                printf("[Save] cannot create encoder for pass 2\n");
                return NULL;
            }
        }
        if(encoder->setup()==false)
        {
            GUI_Error_HIG("Video","Cannot setup codec. Bitrate too low ?");
            delete encoder;
            encoder=NULL;
            return NULL;
        }
        video= new ADM_videoStreamProcess(encoder);
        if(!video)
        {
                GUI_Error_HIG("Video","Cannot create encoder");
                delete encoder;
                return NULL;
        }
    }  
    return video;
}
/**
    \fn    setupAudio
    \brief create the audio streams we will use (copy/process)
*/
bool admSaver::setupAudio()
{
    bool r=true;
    ADM_info("Setting up %d audio track(s)\n",nbAudioTracks);
    for(int i=0;i<nbAudioTracks;i++)
    {
            EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(i);
            ADM_audioStream *access=NULL;
            if(ed->encoderIndex) // encode
            {
                // Access..
                ADM_info("[audioTrack %d] Creating audio encoding stream, starttime %s(encoding with encoder=%d)\n",i,ADM_us2plain(startAudioTime),ed->encoderIndex);
                access=audioCreateEncodingStream(ed,muxer->useGlobalHeader(),startAudioTime); // FIXME LEAK FIXME 
            }else // copy mode...
            {
                ADM_info("[audioTrack %d] Creating audio encoding stream, starttime %s(copy)\n",i,ADM_us2plain(startAudioTime));
                int32_t shift=0;
                if(ed->audioEncodingConfig.shiftEnabled)
                {
                    shift=ed->audioEncodingConfig.shiftInMs;
                    ADM_info("Using shift of %d ms\n",(int)shift);
                }
                access=audioCreateCopyStream(startAudioTime,shift,ed->edTrack,!muxer->canDealWithTimeStamps());
            }
            if(!access)
            {
                    GUI_Error_HIG("Audio","Cannot setup audio encoder, make sure your stream is compatible with audio encoder (number of channels, bitrate, format)");
                    return false;
            }
            audioAccess[i]=access;

    }
    return r;
}
/**
    \fn save
    \brief Prepare the audio and video tracks and feed them to the muxer
*/

bool admSaver::save(void)
{


    int ret=false;
    
    
    ADM_info("Audio starting time %s\n",ADM_us2plain(startAudioTime));
    ADM_info("[A_Save] Saving..\n");
    
    
    EditableAudioTrack *ed=NULL;
    
    if(!videoEncoderIndex) 
    {
        if(false==video_body-> checkCutsAreOnIntra())
        {
            if(!GUI_Question("The video is in copy mode but the cut points are not on keyframes.\n"
                            "The video will be saved but there will corruption at cut point(s).\n"
                             "Do you want to continue anyway ?"))
            {
                return false;
            }
        }
    }

    if(!(muxer=ADM_MuxerSpawnFromIndex(muxerIndex)))
    {
        GUI_Error_HIG("Muxer","Cannot instantiante muxer");
        return 0;
    }
     
    ADM_videoStream *video=setupVideo();
    if(!video)
    {
        return false;
    }
    // adjust audio starting time
     for(int i=0;i<nbAudioTracks;i++)
        {
            ADM_audioStream  *stream=video_body->getAudioStreamAt(i);
            stream->goToTime(startAudioTime);
        }
    if(false==setupAudio())
    {
        if(video) delete video;
        if(muxer) delete muxer;
        muxer=NULL;
        return false;
    }
   
   
    if(!muxer->open(fileName.c_str(),video,nbAudioTracks,audioAccess))
    {
        GUI_Error_HIG("Muxer","Cannot open ");
    }else
    {
        ret=muxer->save();
        muxer->close();
    }
abort123:
    if(video)
        delete video;
    video=NULL;
    for(int i=0;i<nbAudioTracks;i++)
    {
        delete audioAccess[i];
        audioAccess[i]=NULL;
    }
    return ret;
}
//EOF
