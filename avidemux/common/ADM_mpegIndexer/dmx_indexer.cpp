/***************************************************************************
                        2nd gen indexer


    copyright            : (C) 2005 by mean
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
#include "config.h"

#include "ADM_default.h"
#include "prefs.h"
#include "ADM_assert.h"


#include "DIA_coreToolkit.h"
#include "DIA_fileSel.h"
#include "ADM_quota.h"
#include "ADM_commonUI/DIA_idx_pg.h"
#include "avidemutils.h"


#include "ADM_debugID.h"
#define MODULE_NAME MODULE_MPEG
#include "ADM_debug.h"
#include "dmx_demuxerEs.h"
#include "dmx_demuxerPS.h"
#include "dmx_demuxerTS.h"
#include "dmx_demuxerMSDVR.h"
#include "dmx_identify.h"

#define MIN_DELTA_PTS 150 // autofix in ms

#include "dmx_indexer_internal.h"




static uint32_t computeTimeDifference(TimeStamp *f,TimeStamp *l);

uint8_t dmx_indexer(const char *mpeg,char *file);

/**
        \fn    dmx_indexer
        \brief Create index file
        @param mpeg Name of the file to index
        @param file Name of the index file to create
        @param preferedAudio Default audio track
        @param nbTracks # of tracks, including video
        @param tracks track descriptor
        @return 1 on success, 0 on failure

    The incoming file can be mpeg PS/TS/ES or ASF.
    The payload can be mostly mpeg2, with some work done to support later mpeg4/H264 in TS mostly

*/
uint8_t dmx_indexer(const char *mpeg,const char *file,uint32_t preferedAudio,uint8_t autosync,uint32_t nbTracks,MPEG_TRACK *tracks)
{
        DIA_progressIndexing *work;
        dmx_demuxer *demuxer;

        char *realname=ADM_PathCanonize(mpeg);
        FILE *out;
        DMX_TYPE mpegType;
        uint8_t  mpegTypeChar;
        uint32_t multi=0;

        dmx_payloadType payloadType=DMX_PAYLOAD_MPEG2;



        mpegType=dmxIdentify(realname);

        if(mpegType==DMX_MPG_UNKNOWN)
        {
                delete [] realname;
                return 0;
        }



        switch(mpegType)
        {
               case DMX_MPG_MSDVR:
                                {
                                  dmx_demuxerMSDVR *dmx;
                                  dmx=new dmx_demuxerMSDVR(nbTracks,tracks,0);
                                  demuxer=dmx;
                                  mpegTypeChar='M';
                                  break;
                                }
                case DMX_MPG_TS:
                case DMX_MPG_TS2:
                                {
                                dmx_demuxerTS *dmx;
                                dmx=new dmx_demuxerTS(nbTracks,tracks,0,mpegType);
                                demuxer=dmx;
                                switch(mpegType)
                                {
                                  case DMX_MPG_TS :mpegTypeChar='T';break;
                                  case DMX_MPG_TS2 :mpegTypeChar='S';break;
                                  default: ADM_assert(0);
                                }
                                switch(tracks[0].streamType)
                                  {
                                    case ADM_STREAM_H264:       payloadType=DMX_PAYLOAD_H264;break;
                                    case ADM_STREAM_MPEG4:      payloadType=DMX_PAYLOAD_MPEG4;break;
                                    case ADM_STREAM_MPEG_VIDEO: payloadType=DMX_PAYLOAD_MPEG2;break;
                                  default: ADM_assert(0);

                                  }
                                break;
                                }
                case DMX_MPG_ES:
                                demuxer=new dmx_demuxerES;
                                mpegTypeChar='E';
                                break;
                case DMX_MPG_H264_ES:
                                payloadType=DMX_PAYLOAD_H264;
                                demuxer=new dmx_demuxerES;
                                mpegTypeChar='E';
                                break;
                case DMX_MPG_PS:
                		{
                		dmx_demuxerPS *dmx;
                                fileParser    *fp;
                                FP_TYPE       type=FP_PROBE;
                                        fp=new fileParser;
                                        fp->open(realname,&type);
                                        delete fp;
                                        if(type==FP_APPEND)
                                        {
                                          if(GUI_Question(QT_TR_NOOP("There is several mpeg file, append them ?")))
                                                        multi=1;
                                        }
                                        dmx=new dmx_demuxerPS(nbTracks,tracks,multi);
                                        demuxer=dmx;
                                        mpegTypeChar='P';
                            }
                                break;
                default : ADM_assert(0);

        }

        demuxer->open(realname);

        out=qfopen(file,"wt");
        if(!out)
        {
                        printf("\n Error : cannot open index !");
                        delete demuxer;
                        delete [] realname;
                        return 0;
        }
        qfprintf(out,"ADMY0003\n");
        qfprintf(out,"Type     : %c\n",mpegTypeChar); // ES for now
        qfprintf(out,"File     : %s\n",realname);
        qfprintf(out,"Append   : %d\n",multi);
        qfprintf(out,"Image    : %c\n",'P'); // Progressive
        qfprintf(out,"Picture  : %04lu x %04lu %05lu fps\n",0,0,0); // width...
        qfprintf(out,"Payload  : %c%c%c%c\n",'M','P','E','G'); // width...
        qfprintf(out,"Nb Gop   : %08lu \n",0); // width...
        qfprintf(out,"Nb Images: %010lu \n",0); // width...
        qfprintf(out,"Nb Audio : %02lu\n",nbTracks-1);
        qfprintf(out,"Main aud : %02lu\n",preferedAudio);
        qfprintf(out,"Streams  : ");
        for(int s=0;s<nbTracks;s++)
        {
                if(!s){
			qfprintf(out,"V%04x:%04x ",tracks[0].pid,tracks[0].pes);
		}else{
			qfprintf(out,"A%04x:%04x ",tracks[s].pid,tracks[s].pes);
		}
        }
        qfprintf(out,"\n");
        qfprintf(out,"# NGop NImg nbImg type:abs:rel:size ...\n");


        uint8_t  grabbing=0,seq_found=0;

        uint32_t total_frame=0,val;

		uint32_t originalPriority = getpriority(PRIO_PROCESS, 0);
		uint32_t priorityLevel;

		prefs->get(PRIORITY_INDEXING,&priorityLevel);
		setpriority(PRIO_PROCESS, 0, ADM_getNiceValue(priorityLevel));

        work=new DIA_progressIndexing(mpeg);

        printf("*********Indexing started (%d audio tracks)***********\n",nbTracks);
        dmx_runData run;

        memset(&run,0,sizeof(dmx_runData));

        run.totalFileSize=demuxer->getSize();
        run.demuxer=demuxer;
        run.work=work;
        run.nbTrack=nbTracks;
        run.fd=out;

        dmx_videoIndexer *idxer=NULL;

        switch(payloadType)
        {
          case DMX_PAYLOAD_MPEG2:
                  {
                            idxer=new dmx_videoIndexerMpeg2(&run);
                            break;
                  }
          case DMX_PAYLOAD_MPEG4:ADM_assert(0);
          case DMX_PAYLOAD_H264:
                            idxer=new dmx_videoIndexerH264(&run);
                            break;
          default: ADM_assert(0);
        }

        idxer->run();
        idxer->cleanup();

        delete idxer;
        idxer=NULL;


        printf("*********Indexing Ended (%d audio tracks)***********\n",nbTracks);

         switch(run.imageAR)
         {
           case 1: 	qfprintf(out,"# Video Aspect Ratio : %s\n", "1:1" );break;
           case 2: 	qfprintf(out,"# Video Aspect Ratio : %s\n", "4:3" );break;
           case 3: 	qfprintf(out,"# Video Aspect Ratio : %s\n", "16:9" );break;
           default:
              printf("imageAR=%u\n",run.imageAR);
              GUI_Error_HIG(QT_TR_NOOP("Can't determine aspect ratio"),NULL);
	}

        /* Now update......... */
          fseeko(out,0,SEEK_SET);
        // Update if needed
        uint32_t compfps,delta=computeTimeDifference(&(run.firstStamp),&(run.lastStamp));

        delta=delta/1000; // in second
        if(delta)
        {
                 compfps= (1000*run.nbImage)/delta;    // 3 Million images should be enough, no overflow
        }
        else
        {
                compfps=run.imageFPS;
        }

        // Detect film (i.e. NTSC with computed fps close to 24)
        if(run.imageFPS==29970 || run.imageFPS==30000)
        {
                if(compfps>23800 && compfps < 24200) run.imageFPS=23976;
        }
        // Detect interlaced vs progressive
        // If field encoded, the average fps is about twice as theoritical fps
        char type='P';
        float err;

        err=run.imageFPS*2;
        err-=compfps;
        err*=100;
        err/=run.imageFPS*2;
        if(err<0) err=-err;
        printf("%"LU" :%"LU" / %"LU" , %f\n",run.imageFPS,run.imageFPS*2,compfps,err);

        if(err<10)
        {
                type='I';
                printf("Seems to be field encoded\n");
        }
        else
        {
                printf("Seems to be frame encoded\n");
        }

        // Now dump the delta PTS
        // *****************Update header*************
        qfprintf(out,"ADMY0003\n");
        qfprintf(out,"Type     : %c\n",mpegTypeChar); // ES for now
        qfprintf(out,"File     : %s\n",realname);
        qfprintf(out,"Append   : %d\n",multi);
        qfprintf(out,"Image    : %c\n",type); // Progressive
        qfprintf(out,"Picture  : %04lu x %04lu %05lu fps\n",run.imageW,run.imageH,run.imageFPS); // width...
        switch(payloadType)
        {
          case DMX_PAYLOAD_MPEG2:
                            qfprintf(out,"Payload  : MPEG\n"); // MPEG,MP_4,H264
                            break;
          case DMX_PAYLOAD_MPEG4:
                            qfprintf(out,"Payload  : MP_4\n"); // MPEG,MP_4,H264
                            break;
          case DMX_PAYLOAD_H264:
                            qfprintf(out,"Payload  : H264\n"); // MPEG,MP_4,H264
                            break;
          default: ADM_assert(0);
        }
        qfprintf(out,"Nb Gop   : %08lu \n",run.nbGop); // width...
        qfprintf(out,"Nb Images: %010lu \n",run.nbImage); // width...

        qfclose(out);
        delete work;

        printf("*********Indexing stopped***********\n");
        printf("Found       :%"LU" gop\n",run.nbGop);
        printf("Found       :%"LU" image\n",run.nbImage);
        printf("Average fps :%"LU" /1000 fps\n",compfps);

          delete demuxer;
          delete [] realname;

		  setpriority(PRIO_PROCESS, 0, originalPriority);

          return 1;
}
uint32_t computeTimeDifference(TimeStamp *f,TimeStamp *l)
{
        uint32_t h,m,s,ms,r=0,result=0;
#define DOIT(x,next) if(l->x < f->x) \
                        { \
                                r=(next-(f->x-l->x));\
                                result-=1; \
                        }\
                         else r=(l->x-f->x);\
                        result=result*next+r;

#define PRINT(z,x) printf(#z"%02d:%02d:%02d\n",x->hh,x->mm,x->ss,x->ff);
        DOIT(hh,0);
        DOIT(mm,60);
        DOIT(ss,60);
        DOIT(ff,1000);
#if 0
        PRINT(first,f);
        PRINT(last,l);

#endif
        printf("Time difference:%lu s\n",result/1000);
        return result;

}

//
