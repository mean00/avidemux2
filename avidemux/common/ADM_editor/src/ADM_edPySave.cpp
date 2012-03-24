/***************************************************************************
                          ADM_edLoadSave.cpp  -  description
                             -------------------

	Save / load workbench

    begin                : Thu Feb 28 2002
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
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "fourcc.h"
#include "ADM_quota.h"
#include "ADM_edit.hxx"

#include "audioEncoderApi.h"
#include "audiofilter_normalize_param.h"
#include "DIA_coreToolkit.h"
#include "ADM_videoEncoderApi.h"
#include "ADM_paramList.h"
#include "prefs.h"
#include "avi_vars.h"
#include "ADM_muxerProto.h"
#include "ADM_audioFilterInterface.h"
#include "GUI_ui.h"
#include "ADM_videoFilters.h"
#include "ADM_videoFilterApi.h"
#include "errno.h"
/**
    \fn dumpConf
    \brief dump configuration as name=value pairs
*/
static void dumpConf(FILE *fd,CONFcouple *c)
{
 if(!c) return;
    int count=0;    
    uint32_t n=c->getSize();
    for(int j=0;j<n;j++)
    {
        char *name,*value;
        c->getInternalName(j,&name,&value);
        qfprintf(fd,",\"%s=%s\"",name,value);
        // tinyPy does not like line > 1024 chars
        if(count>=20)
        {
            qfprintf(fd,"\\\n");
            count=0;
        }
        count++;
    }
    delete c;
    c=NULL;
}
/**
    \fn        saveAsPyScript
    \brief     Save the project as a python script
*/
bool ADM_Composer::saveAsPyScript (const char *name)
{
    const char *truefalse[]={"false","true"};
    ADM_info(" **Saving pyScript project %s**\n",name);
    char *    tmp;

    if (!_segments.getNbSegments())
        return 1;

    FILE *fd=NULL;

    if( !(fd = qfopen (name, "wt")) )
    {
        ADM_error("Cannot save python script file %s for writing (er:%d %s)\n",
                   name, errno, strerror(errno));
        return false;
    }

// Save source and segment
//______________________________________________
  qfprintf( fd, "#PY  <- Needed to identify");
  qfprintf (fd, "#\n");
  qfprintf (fd, "#--automatically built--\n");
  qfprintf (fd, "#--Project: %s\n\n",name);
  qfprintf (fd,"adm=Avidemux()\n");
  qfprintf (fd,"#** Video **\n");
  qfprintf (fd,"# %02ld videos source \n", _segments.getNbRefVideos());
 

  char *nm;
  

  for (uint32_t i = 0; i < _segments.getNbRefVideos(); i++)
    {
        nm=ADM_cleanupPath(_segments.getRefVideo(i)->_aviheader->getMyName() );
        if(!i)
        {
                qfprintf (fd, "adm.loadVideo(\"%s\")\n", nm);
        }
        else
        {
                qfprintf (fd, "adm.appendVideo(\"%s\")\n", nm);
        }
        ADM_dealloc(nm);
    }

  qfprintf (fd,"#%02ld segments\n", _segments.getNbSegments());
  qfprintf (fd,"adm.clearSegments()\n");
  
 

    for (uint32_t i = 0; i < _segments.getNbSegments(); i++)
    {
        _SEGMENT *seg=_segments.getSegment(i);
        qfprintf (fd, "adm.addSegment(%"LU",",seg->_reference);
        qfprintf (fd, "%"LLU",",seg->_refStartTimeUs);
        qfprintf (fd, "%"LLU")\n",seg->_durationUs);
    }

// Markers
//

        qfprintf(fd,"adm.markerA=%"LLU"\n",getMarkerAPts());
        qfprintf(fd,"adm.markerB=%"LLU"\n",getMarkerBPts());

// postproc
//___________________________

uint32_t pptype, ppstrength;
bool ppswap;
        video_body->getPostProc( &pptype, &ppstrength, &ppswap);
        qfprintf(fd,"\n#** Postproc **\n");
        qfprintf(fd,"adm.setPostProc(%d,%d,%d)\n",pptype,ppstrength,ppswap);

// Video codec
//___________________________
        
		qfprintf(fd, "\n#** Video Codec conf **\n");
        CONFcouple *couples=NULL;
    
        qfprintf(fd, "adm.videoCodec(\"%s\"", videoEncoder6_GetCurrentEncoderName());
        videoEncoder6_GetConfiguration(&couples);
        dumpConf(fd,couples);
        qfprintf(fd,")\n");

// Video filters....
//______________________________________________
    qfprintf(fd,"\n#** Filters **\n");
    int nbFilter=ADM_vf_getSize();
    for(int i=0;i<nbFilter;i++)
    {
        // Grab its name...
        uint32_t tag=ADM_vf_getTag(i);
        qfprintf(fd, "adm.addVideoFilter(\"%s\"", ADM_vf_getInternalNameFromTag(tag));
        // Now get the filter settings (if any)
        CONFcouple *c=NULL;
        ADM_vf_getConfigurationFromIndex(i,&c);
        dumpConf(fd,c);
        qfprintf(fd, ")\n");
    }


// Audio Source
//______________________________________________

// Audio
//______________________________________________
    qfprintf(fd,"\n#** Audio **\n");
    qfprintf(fd,"adm.audioClearTracks()\n");
   // Codec
    for(int i=0;i<video_body->getNumberOfActiveAudioTracks();i++)
    {
         qfprintf(fd,"\n#** Track %d **\n",i);        
         EditableAudioTrack *track=video_body->getEditableAudioTrackAt(i);
         ADM_assert(track);
         qfprintf(fd,"\n# This is wrong\n");        
         qfprintf(fd,"adm.audioAddTrack(%d)\n",i);
         qfprintf(fd,"adm.audioCodec(%d,\"%s\"",i,audioCodecGetName(i)); 
         dumpConf(fd,track->encoderConf);
         qfprintf(fd,");\n");

         //qfprintf(fd,"app.audio.addTrack(%d);\n", source); 

    // Filters
    // Mixer
         CHANNEL_CONF channel=track->audioEncodingConfig.audioFilterGetMixer();
         if(channel!=CHANNEL_INVALID)
         {
                qfprintf(fd,"adm.audioSetMixer(%d,\"%s\");\n",i,track->audioEncodingConfig.audioMixerAsString()); // setCurrentMixerFromString
         }
    // Resample
        uint32_t x=track->audioEncodingConfig.audioFilterGetResample();
        if(x) qfprintf(fd,"adm.audioSetResample(%d,%u)\n",i,track->audioEncodingConfig.audioFilterGetResample());
   

   // Change fps ?
        switch(track->audioEncodingConfig.audioFilterGetFrameRate())
        {
                case FILMCONV_NONE:      ;
                                         qfprintf(fd,"adm.audioSetPal2Film(%d,0)\n",i);
                                         qfprintf(fd,"adm.audioSetFilm2Pal(%d,0)\n",i);
                                         break;
                case FILMCONV_PAL2FILM:  qfprintf(fd,"adm.audioSetPal2Film(%d,1)\n",i);break;
                case FILMCONV_FILM2PAL:  qfprintf(fd,"adm.audioSetFilm2Pal(%d,1)\n",i);break;
                default:ADM_assert(0);break;
        }
   // --------- Normalize ----------------
        ADM_GAINMode mode;
        uint32_t gain;
        track->audioEncodingConfig.audioFilterGetNormalize(&mode,&gain);
        if(mode && gain)
        {
            qfprintf(fd,"adm.audioSetNormalize(%d,%d,%d)\n",i,(int)mode,(int)gain); 
        }else
            qfprintf(fd,"adm.audioSetNormalize(%d,%d,%d)\n",i,0,0); 
       
    }
  // -------- Muxer -----------------------
        qfprintf(fd,"\n#** Muxer **\n");
        CONFcouple *containerConf=NULL;
        uint32_t index=UI_GetCurrentFormat();
        const char *containerName=ADM_mx_getName(index);
        ADM_mx_getExtraConf( index,&containerConf);
        
        qfprintf(fd,"adm.setContainer(\"%s\"",containerName); 
        dumpConf(fd,containerConf);
        qfprintf(fd,")\n");
  // -------- /Muxer -----------------------
        qfprintf(fd,"\n#End of script\n");
        // All done
        qfclose (fd);
  
  return 1;


}

//EOF
