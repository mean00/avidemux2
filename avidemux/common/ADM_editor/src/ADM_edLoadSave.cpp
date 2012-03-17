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
using std::string;
#include "ADM_default.h"
#include "fourcc.h"
#include "ADM_quota.h"
#include "ADM_edit.hxx"

#include "audioEncoderApi.h"
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
        
    uint32_t n=c->getSize();
    for(int j=0;j<n;j++)
    {
        char *name,*value;
        c->getInternalName(j,&name,&value);
        qfprintf(fd,",\"%s=%s\"",name,value);
    }
    delete c;
    c=NULL;
}
/**
    \fn        saveAsScript
    \brief     Save the project as a script
*/
uint8_t ADM_Composer::saveAsScript (const char *name, const char *outputname)
{
    const char *truefalse[]={"false","true"};
    ADM_info(" **Saving script project %s**\n",name);

    if (!_segments.getNbSegments())
        return 1;

    FILE *fd=NULL;

    if( !(fd = qfopen (name, "wt")) )
    {
    fprintf(stderr,"\ncan't open script file \"%s\" for writing: %u (%s)\n",
                   name, errno, strerror(errno));
    return 0;
    }

// Save source and segment
//______________________________________________
  qfprintf( fd,"//AD  <- Needed to identify");
  qfprintf (fd, "//\n");
  qfprintf (fd, "//--automatically built--\n");
  qfprintf (fd, "//--Project: %s\n\n",name);

  qfprintf (fd,"\n//** Video **\n");
  qfprintf (fd,"// %02ld videos source \n", _segments.getNbRefVideos());

  char *nm;
  

  for (uint32_t i = 0; i < _segments.getNbRefVideos(); i++)
    {
        nm=ADM_cleanupPath(_segments.getRefVideo(i)->_aviheader->getMyName() );
        if(!i)
        {
                qfprintf (fd, "adm.loadVideo(\"%s\");\n", nm);
        }
        else
        {
                qfprintf (fd, "adm.appendVideo(\"%s\");\n", nm);
        }
        ADM_dealloc(nm);
    }

  qfprintf (fd,"//%02ld segments\n", _segments.getNbSegments());
  qfprintf (fd,"adm.clearSegments();\n");
  
 

    for (uint32_t i = 0; i < _segments.getNbSegments(); i++)
    {
        _SEGMENT *seg=_segments.getSegment(i);
        qfprintf (fd, "adm.addSegment(%"LU",%"LLU",%"LLU");\n",seg->_reference,seg->_refStartTimeUs,seg->_durationUs);
    }

// Markers
//

        qfprintf(fd,"adm.markerA=%"LLU";\n",getMarkerAPts());
        qfprintf(fd,"adm.markerB=%"LLU";\n",getMarkerBPts());

// postproc
//___________________________

uint32_t pptype, ppstrength;
bool ppswap;
        video_body->getPostProc( &pptype, &ppstrength, &ppswap);
        qfprintf(fd,"\n//** Postproc **\n");
        qfprintf(fd,"adm.setPostProc(%d,%d,%d);\n",pptype,ppstrength,ppswap);

// fps
#if 0
	if( avifileinfo )
    {
	  aviInfo info;
		video_body->getVideoInfo(&info);
		qfprintf(fd,"\napp.video.setFps1000(%u);\n",info.fps1000);
	}
#endif

// Video codec
//___________________________
        
		qfprintf(fd, "\n//** Video Codec conf **\n");
        CONFcouple *couples=NULL;
    
        qfprintf(fd, "adm.videoCodec(\"%s\"", videoEncoder6_GetCurrentEncoderName());
        videoEncoder6_GetConfiguration(&couples);
        dumpConf(fd,couples);
        qfprintf(fd,");\n");

// Video filters....
//______________________________________________
    qfprintf(fd,"\n//** Filters **\n");
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
        qfprintf(fd, ");\n");
    }


// Audio Source
//______________________________________________

// Audio
//______________________________________________

   uint32_t delay,bitrate;
   
   qfprintf(fd,"\n//** Audio **\n");
   qfprintf(fd,"adm.audioReset();\n");
#if 0
   // External audio ?
        char *audioName;
        AudioSource  source;

        source=getCurrentAudioSource(&audioName);
        if(!audioName) audioName="";

        if(source!=AudioAvi)
        {
                char *nm=ADM_cleanupPath(audioName);
                qfprintf(fd,"app.audio.load(\"%s\",\"%s\");\n", audioSourceFromEnum(source),nm); 
                ADM_dealloc(nm);
        }
        else 
        { // Maybe not the 1st track
          int source;
               source=video_body->getCurrentAudioStreamNumber(0);
               if(source)
                        qfprintf(fd,"app.audio.setTrack(%d);\n", source); 
                        
        }
#endif
        { // Maybe not the 1st track
          int source;
               source=video_body->getCurrentAudioStreamNumber(0);
               ADM_info("Audio source %d\n",source);
               if(source)
                        qfprintf(fd,"adm.setAudioTrack(%d);\n", source); 
                        
        }

   couples=NULL;
   getAudioExtraConf(0,&bitrate,&couples);
    qfprintf(fd,"adm.audioCodec(\"%s\",%d",audioCodecGetName(0),bitrate); 
    dumpConf(fd,couples);
    qfprintf(fd,");\n");

  
  // -------- Muxer -----------------------
        qfprintf(fd,"\n//** Muxer **\n");
        CONFcouple *containerConf=NULL;
        uint32_t index=UI_GetCurrentFormat();
        const char *containerName=ADM_mx_getName(index);
        ADM_mx_getExtraConf( index,&containerConf);
        
        qfprintf(fd,"adm.setContainer(\"%s\"",containerName); 
        dumpConf(fd,containerConf);
        qfprintf(fd,");\n");
  // -------- /Muxer -----------------------
  if(outputname)
  {
        char *o=ADM_cleanupPath(outputname);
        qfprintf(fd,"setSuccess(adm.save(\"%s\"));\n",o);
        ADM_dealloc(o);
  }
  else
  {
        qfprintf(fd,"setSuccess(%d);\n",1);
  }

  qfprintf(fd,"//adm.Exit();\n");
  qfprintf(fd,"\n//End of script\n");
  // All done
  qfclose (fd);
  
  return 1;


}

//EOF
