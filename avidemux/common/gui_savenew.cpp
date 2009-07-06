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

#include "ADM_default.h"
#include "ADM_threads.h"

#include "avi_vars.h"
#include "prototype.h"
#include "DIA_coreToolkit.h"
#include "DIA_enter.h"
#include "ADM_coreAudio.h"
#include "audioprocess.hxx"

#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"
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

ADM_muxer               *ADM_MuxerSpawnFromIndex(int index);
extern ADM_audioStream  *createEncodingStream(uint64_t startTime,int32_t shift);
extern ADM_videoStream  *createVideoStream(ADM_coreVideoEncoder *encoder);

/**
    \class admSaver
    \brief Wrapper for saving

*/
class admSaver
{
protected:
        const char           *fileName;
        char                 *logFileName;
        ADM_muxer            *muxer;
        ADM_videoFilterChain *chain;
        ADM_audioStream      *audio;
        ADM_videoStream      *video;
        ADM_audioStream      *astreams[1];
        uint64_t             markerA,markerB;
        int                  muxerIndex;
        int                  videoEncoderIndex;
        ADM_coreVideoEncoder *handleFirstPass(ADM_coreVideoEncoder *pass1);
        
public:
                admSaver(const char *out);
                ~admSaver();
        bool    save(void);
        
};
/**
    \fn admSaver
*/
 admSaver::admSaver(const char *out)
{
        fileName=out;
        logFileName=new char[strlen(out)+10];
        strcpy(logFileName,out);
        strcat(logFileName,".stats");
        muxer=NULL;
        chain=NULL;
        audio=NULL;
        video=NULL;
        astreams[0]=NULL;
        markerA=video_body->getMarkerAPts();
        markerB=video_body->getMarkerBPts();
        muxerIndex=UI_GetCurrentFormat();
        videoEncoderIndex=UI_getCurrentVCodec();
        printf("[Save] Encoder index=%d\n",index);
}
/**
    \fn ~admSaver
*/
admSaver::~admSaver()
{
 if(muxer) 
        delete muxer;
 muxer=NULL;
 if(logFileName) 
        delete [] logFileName;
 logFileName=NULL;
 if (audioProcessMode() && astreams[0])
        delete astreams[0];
 astreams[0]=NULL;
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
    \fn A_Save
    \brief Instantiate & initiate streams to feed muxer
*/
int A_Save(const char *name)
{
    admSaver *save=new admSaver(name);
    bool r=save->save();
    delete save;
    return (int)r;
}

/**
    \fn handleFirstPass
*/
ADM_coreVideoEncoder *admSaver::handleFirstPass(ADM_coreVideoEncoder *pass1)
{
#define BUFFER_SIZE 1920*1800*3

                printf("[Save] Performing Pass one,using %s as log file\n",logFileName);
                pass1->setPassAndLogFile(1,logFileName);
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
                
                while(pass1->encode(&bitstream))
                {

                }
                delete [] buffer;
                printf("[Save] Pass 1 done, restarting for pass 2\n");
                // Destroy filter chain & create the new encoder
                destroyVideoFilterChain(chain);
                chain=NULL;
                chain=createVideoFilterChain(markerA,markerB);
                delete pass1;
                if(!chain)
                {
                    printf("[Save] Cannot recreate video filter chain\n");
                    return NULL;
                }
                int sz=chain->size();
                ADM_assert(sz);
                ADM_coreVideoFilter  *last;
                last=(*chain)[sz-1]; // Grab last filter
                ADM_coreVideoEncoder *pass2=createVideoEncoderFromIndex(last,videoEncoderIndex);
                if(!pass2) 
                {
                    printf("[Save] Cannot create encoder for pass 2\n");
                    return NULL;
                }
                pass2->setPassAndLogFile(2,logFileName);
    return pass2;
}
/**
    \fn save
*/
bool admSaver::save(void)
{
    
    int ret=1;
    
    
     
     
    printf("[A_Save] Saving..\n");

    
    if(!(muxer=ADM_MuxerSpawnFromIndex(muxerIndex)))
    {
        GUI_Error_HIG("Muxer","Cannot instantiante muxer");
        return 0;
    }

    // Audio Stream ?
    ADM_audioStream *audio=NULL;
    int nbAStream=1;
    if(!video_body->getAudioStream(&audio))
    {
        audio=NULL;
        nbAStream=0; // FIXME
    }else
    {
        audio->goToTime(markerA); // Rewind audio
    }
    ADM_videoStream *video=NULL;
    // Video Stream 
    if(!videoEncoderIndex) // Copy
    {
        video=new ADM_videoStreamCopy(markerA,markerB);
    }else
    {
        // 1- create filter chain
        //******************************
       
        chain=createVideoFilterChain(markerA,markerB);
        if(!chain)
        {
                GUI_Error_HIG("Video","Cannot instantiante video Chain");
                return false;
        }
        // 2- Create Encoder
        //********************
        int sz=chain->size();
        ADM_assert(sz);
        ADM_coreVideoFilter  *last;
        last=(*chain)[sz-1]; // Grab last filter
        ADM_coreVideoEncoder *encoder=createVideoEncoderFromIndex(last,videoEncoderIndex);
        if(!encoder)
        {
           GUI_Error_HIG("Video","Cannot create encoder");
           return false;
        }
        // 3 dual Pass ?
        //*****************
        if(encoder->isDualPass())
        {
            encoder=handleFirstPass(encoder); // Do pass 1 and switch to pass 2
            if(!encoder)
            {
                printf("[Save] cannot create encoder for pass 2\n");
                return false;
            }
        }
        if(encoder->setup()==false)
        {
            GUI_Error_HIG("Video","Cannot setup codec. Bitrate too low ?");
            delete encoder;
            encoder=NULL;
            return false;

        }
        video= new ADM_videoStreamProcess(encoder);
        if(!video)
        {
                GUI_Error_HIG("Video","Cannot create encoder");
                delete encoder;
                return 0;
        }
        
    }
    //
    ADM_audioStream *astreams[1];
    if (!audioProcessMode())
    {
        astreams[0]=audio; //copy
    }else    
    {
        if(audio)   // Process
        {
            // Access..
            ADM_audioStream *access=createEncodingStream(markerA,0); // FIXME LEAK
            astreams[0]=access;
        }
    }
    if(!muxer->open(fileName,video,nbAStream,astreams))
    {
        GUI_Error_HIG("Muxer","Cannot open ");
        
    }else   
    {

        muxer->save();
        muxer->close();
    }
    return true;
}
//EOF
