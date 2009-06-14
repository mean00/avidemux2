/***************************************************************************
                          gui_savenew.cpp  -  description
                             -------------------
    begin                : Fri May 3 2002
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
#include "ADM_userInterfaces/ADM_commonUI/GUI_ui.h"
#include "ADM_muxer.h"
#include "ADM_videoCopy.h"
#include "ADM_filterChain.h"

ADM_muxer               *ADM_MuxerSpawnFromIndex(int index);
extern ADM_audioStream  *createEncodingStream(uint64_t startTime,int32_t shift);
extern ADM_videoStream  *createVideoStream(ADM_videoFilterChain *chain,int index);
/**
    \fn A_Save
    \brief Instantiate & initiate streams to feed muxer
*/
int A_Save(const char *name)
{
    int ret=1;
    ADM_muxer *muxer=NULL;
    int index=UI_GetCurrentFormat();
    int process=UI_getCurrentVCodec();
    ADM_videoFilterChain *chain=NULL;
     uint64_t markerA,markerB;
        markerA=video_body->getMarkerAPts();
        markerB=video_body->getMarkerBPts();
    printf("[A_Save] Saving..\n");

    if(!(muxer=ADM_MuxerSpawnFromIndex(index)))
    {
        GUI_Error_HIG("Muxer","Cannot instantiante muxer");
        return 0;
    }

    // Audio Stream ?
    ADM_audioStream *audio=NULL;
    int nbAStream=1;
    if(!video_body->getAudioStream(&audio))
    {
        //GUI_Error_HIG("Audio","Cannot get audiostream");
        //return 0;
        audio=NULL;
        nbAStream=0; // FIXME
    }
    ADM_videoStream *video=NULL;
    // Video Stream 
    if(!process) // Copy
    {
        video=new ADM_videoStreamCopy(markerA,markerB);
    }else
    {
        // 1- create filter chain
       
        chain=createVideoFilterChain(markerA,markerB);
        if(!chain)
        {
                GUI_Error_HIG("Video","Cannot instantiante video Chain");
                delete muxer;
                return 0;
        }
        // 2- Create Encoder
        video=createVideoStream(chain,process);
        if(!video)
        {
                GUI_Error_HIG("Video","Cannot create encoder");
                destroyVideoFilterChain(chain);
                delete muxer;
                return 0;
        }
        
    }
    //
    ADM_audioStream *astreams[1];
    if (!audioProcessMode())
    {
        astreams[0]=audio;
    }else   
    {
        if(audio)
        {
            // Access..
            ADM_audioStream *access=createEncodingStream(markerA,0); // FIXME LEAK
            astreams[0]=access;
        }
    }
    if(!muxer->open(name,video,nbAStream,astreams))
    {
        GUI_Error_HIG("Muxer","Cannot open ");
        
    }else   
    {
        muxer->save();
        muxer->close();
    }
    //

    if(muxer) delete muxer;
    if (audioProcessMode() && astreams[0])
        delete astreams[0];
    delete video;
    return ret;
}
// Leftovers....
uint8_t  A_SaveAudioDualAudio(const char *inname)
{
}
//___________________________________
int A_SaveUnpackedVop(const char *name)
{
}
int A_SavePackedVop(const char *name)
{
}
//___________________________________
uint8_t  A_SaveAudioNVideo(const char *name)
{
}
