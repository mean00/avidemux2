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
#include "ADM_default.h"
#include "fourcc.h"
#include "ADM_quota.h"
#include "ADM_editor/ADM_edit.hxx"

#include "audioEncoderApi.h"
#include "DIA_coreToolkit.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"
#include "ADM_paramList.h"
#include "prefs.h"
#include "avi_vars.h"

// Ugly but sooo usefull
extern uint32_t frameStart,frameEnd;
static uint32_t edFrameStart,edFrameEnd;
const char *getCurrentContainerAsString(void);


/*______________________________________________
        Save the project as a script
______________________________________________*/
uint8_t ADM_Composer::saveAsScript (const char *name, const char *outputname)
{
const char *truefalse[]={"false","true"};
printf("\n **Saving script project **\n");
  char *    tmp;

  if (!_segments.getNbSegments())
    return 1;

  FILE *    fd;

  if( !(fd = qfopen (name, "wt")) ){
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

  qfprintf (fd, "var app = new Avidemux();\n");
  qfprintf (fd,"\n//** Video **\n");
  qfprintf (fd,"// %02ld videos source \n", _segments.getNbRefVideos());
  char *nm;
  uint32_t vop=!!(video_body->getSpecificMpeg4Info()&ADM_VOP_ON);

  for (uint32_t i = 0; i < _segments.getNbRefVideos(); i++)
    {
        nm=ADM_cleanupPath(_segments.getRefVideo(i)->_aviheader->getMyName() );
        if(vop)
        {
          qfprintf(fd,"app.forceUnpack();\n");
        }
        if(!i)
        {
                qfprintf (fd, "app.load(\"%s\");\n", nm);
        }
        else
        {
            qfprintf (fd, "app.append(\"%s\");\n", nm);
        }
        ADM_dealloc(nm);
    }

  qfprintf (fd,"//%02ld segments\n", _segments.getNbSegments());
  qfprintf (fd,"app.clearSegments();\n");
  
 

    for (uint32_t i = 0; i < _segments.getNbSegments(); i++)
    {
        _SEGMENT *seg=_segments.getSegment(i);
        qfprintf (fd, "app.addSegment(%"LU",%"LLU",%"LLU");\n",seg->_reference,seg->_refStartTimeUs,seg->_durationUs);
    }

// Markers
//
        qfprintf(fd,"app.markerA=%"LLU";\n",getMarkerAPts());
        qfprintf(fd,"app.markerB=%"LLU";\n",getMarkerBPts());
        
// postproc
//___________________________

        uint32_t pptype, ppstrength,ppswap;
                video_body->getPostProc( &pptype, &ppstrength, &ppswap);
                qfprintf(fd,"\n//** Postproc **\n");
                qfprintf(fd,"app.video.setPostProc(%d,%d,%d);\n",pptype,ppstrength,ppswap);

// fps
#if 0
	if( avifileinfo )
    {
	  aviInfo info;
		video_body->getVideoInfo(&info);
		qfprintf(fd,"\napp.video.setFps1000(%u);\n",info.fps1000);
	}
#endif
// Filter
//___________________________
        qfprintf(fd,"\n//** Filters **\n");
//        filterSaveScriptJS(fd);

// Video codec
//___________________________
#if 0
		uint8_t *extraData;
		uint32_t extraDataSize;

		qfprintf(fd, "\n//** Video Codec conf **\n");
		videoCodecGetConf(&extraDataSize, &extraData);

		if (videoCodecGetType() == CodecExternal)
			qfprintf(fd, "app.video.codecPlugin(\"%s\", \"%s\", \"%s\", \"%s\");\n", videoCodecPluginGetGuid(), videoCodecGetName(), videoCodecGetMode(), extraData);
		else
		{
			qfprintf(fd, "app.video.codec(\"%s\", \"%s\", \"", videoCodecGetName(), videoCodecGetMode());

			// Now deal with extra data
			qfprintf(fd, "%d ", extraDataSize);

			if (extraDataSize)
				for(int i = 0; i < extraDataSize; i++)
					qfprintf(fd, "%02x ", extraData[i]);

			qfprintf(fd, "\");\n");
		}
#endif        
// Audio Source
//______________________________________________

// Audio
//______________________________________________

   uint32_t delay,bitrate;
   
   qfprintf(fd,"\n//** Audio **\n");
   qfprintf(fd,"app.audio.reset();\n");
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
   CONFcouple *couples=NULL;
   getAudioExtraConf(&bitrate,&couples);
   if(couples==NULL)
    qfprintf(fd,"app.audio.codec(\"%s\",%d);\n",audioCodecGetName(),bitrate); 
   else
    {
        qfprintf(fd,"app.audio.codec(\"%s\",%d", audioCodecGetName(),bitrate); 
        uint32_t n=couples->getNumber();
        
        for(int i=0;i<n;i++)
        {
            char *name,*value;
            couples->getEntry(i,&name,&value);
            qfprintf(fd,",\"%s=%s\"",name,value);
        }
        qfprintf(fd,");\n");
        delete couples;
        couples=NULL;
    }
   
#if 0   
   //qfprintf(fd,"app.audio.process=%s;\n",truefalse[audioProcessMode()]);
   qfprintf(fd,"app.audio.normalizeMode=%d;\n",audioGetNormalizeMode());
   qfprintf(fd,"app.audio.normalizeValue=%d;\n",audioGetNormalizeValue());
   qfprintf(fd,"app.audio.delay=%d;\n",audioGetDelay());
   qfprintf(fd,"app.audio.mixer(\"%s\");\n",getCurrentMixerString());

    // VBR ?
    if(currentaudiostream)
    {
        uint32_t encoding=currentaudiostream->getInfo()->encoding;
        if((encoding==WAV_MP3 || encoding==WAV_MP2))
        {
            qfprintf(fd,"app.audio.scanVBR();\n");
        }
    }


   // Change fps ?
        switch(audioGetFpsConv())
        {
                case FILMCONV_NONE:      ;break;
                case FILMCONV_PAL2FILM:  qfprintf(fd,"app.audio.pal2film=true;\n");break;
                case FILMCONV_FILM2PAL:  qfprintf(fd,"app.audio.film2pal=true;\n");break;
                default:ADM_assert(0);
        }
   // Resampling
        switch(audioGetResampling())
        {
                case RESAMPLING_NONE:         ;break;
                case RESAMPLING_CUSTOM:        qfprintf(fd,"app.audio.resample=%u;\n",audioGetResample());break;
                default:ADM_assert(0);
        }
        if (audioGetDrc()) qfprintf(fd,"app.audio.drc=true;\n");
        
#endif        
  // Mixer

  // container
        
  qfprintf(fd,"app.setContainer(\"%s\");\n",getCurrentContainerAsString());
  if(outputname)
  {
        char *o=ADM_cleanupPath(outputname);
        qfprintf(fd,"setSuccess(app.save(\"%s\"));\n",o);
        ADM_dealloc(o);
  }
  else
  {
        qfprintf(fd,"setSuccess(%d);\n",1);
  }
  qfprintf(fd,"//app.Exit();\n");
  qfprintf(fd,"\n//End of script\n");
  // All done
  qfclose (fd);
  
  return 1;


}

//EOF
