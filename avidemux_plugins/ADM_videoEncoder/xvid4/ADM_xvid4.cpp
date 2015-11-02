/***************************************************************************
                          \fn ADM_Xvid4
                          \brief Front end for xvid4 Mpeg4 asp encoder
                             -------------------
    
    copyright            : (C) 2002/2009 by mean/gruntster
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
#include "ADM_xvid4.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"
#include "ADM_coreUtils.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

#define MMSET(x) memset(&(x),0,sizeof(x))

xvid4_encoder xvid4Settings = XVID_DEFAULT_CONF;

typedef enum
{
        ME_NONE = 0,
        ME_LOW = XVID_ME_HALFPELREFINE16,
        ME_MEDIUM = XVID_ME_HALFPELREFINE16 | XVID_ME_ADVANCEDDIAMOND16,
        ME_HIGH = XVID_ME_HALFPELREFINE16 | XVID_ME_EXTSEARCH16 | XVID_ME_HALFPELREFINE8 | XVID_ME_USESQUARES16
} MotionEstimationMode;

uint32_t motionMode[4]=
{
    ME_NONE,ME_LOW,ME_MEDIUM,ME_HIGH
};

typedef enum
{
        RD_NONE = -1,
        RD_DCT_ME = 0,
        RD_HPEL_QPEL_16 = RD_DCT_ME | XVID_ME_HALFPELREFINE16_RD | XVID_ME_QUARTERPELREFINE16_RD,
        RD_HPEL_QPEL_8 = RD_HPEL_QPEL_16 | XVID_ME_HALFPELREFINE8_RD | XVID_ME_QUARTERPELREFINE8_RD | XVID_ME_CHECKPREDICTION_RD,
        RD_SQUARE = RD_HPEL_QPEL_8 | XVID_ME_EXTSEARCH_RD
} RateDistortionMode;

uint32_t rdMode[5]=
{
    RD_NONE,
    RD_DCT_ME,
    RD_HPEL_QPEL_16,
    RD_HPEL_QPEL_8,
    RD_SQUARE
};

static uint32_t outFrameStatic=0;


/**
        \fn xvid4Encoder
*/
xvid4Encoder::xvid4Encoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoder(src)
{
    printf("[xvid4] Creating.\n");
    this->globalHeader=globalHeader;
    handle=NULL;
    MMSET(xvid_enc_frame);
    frameNum=0;
    backRef=fwdRef=refIndex=0;
    pass=0;
    memset(&pass1,0,sizeof(pass1));
    memset(&pass2,0,sizeof(pass2));
        
}
/**
    \fn  setPassAndLogFile
*/
bool        xvid4Encoder::setPassAndLogFile(int pass,const char *name)
{
        logFile=std::string(name);
        this->pass=pass;
        ADM_info("Checking pass %d, using stat file =%s\n",pass,logFile.c_str());
        return true;
}

const  char        *xvid4Encoder::getFourcc(void)
{
    if(xvid4Settings.useXvidFCC)
        return "XVID";
    return "DIVX";
}
/**
    \fn query
    \brief query xvid about version and flags
*/
bool xvid4Encoder::query(void)
{

 xvid_gbl_init_t   xvid_gbl_init2;
 xvid_gbl_info_t   xvid_gbl_info;
 
  MMSET (xvid_gbl_init2);
  MMSET (xvid_gbl_info);

  printf ("[xvid] Initializing global Xvid 4\n");
  xvid_gbl_init2.version = XVID_VERSION;
  xvid_global (NULL, XVID_GBL_INIT, &xvid_gbl_init2, NULL);
  xvid_gbl_info.version = XVID_VERSION;
  xvid_global (NULL, XVID_GBL_INFO, &xvid_gbl_info, NULL);

  if (xvid_gbl_info.build)
      printf ("[xvid] Build: %s\n", xvid_gbl_info.build);

  printf ("[xvid] SIMD supported: (%x)\n", xvid_gbl_info.cpu_flags);
#define CPUF(x) if(xvid_gbl_info.cpu_flags  & XVID_CPU_##x) printf("\t\t"#x" ON\n"); else  printf("\t\t"#x" Off\n");
#if defined( ADM_CPU_X86)  
  CPUF (MMX);
  CPUF (MMXEXT);
  CPUF (SSE);
  CPUF (SSE2);
  CPUF (3DNOW);
  CPUF (3DNOWEXT);
#endif
  return true;
}
/**
    \fn setupPass
*/
bool xvid4Encoder::setupPass(void)
{
    uint32_t avgKbits=0;
    switch(this->pass)
    {
        case 1:
                plugins[0].func = xvid_plugin_2pass1;
                plugins[0].param = &pass1;
                memset(&pass1,0,sizeof(pass1));
                pass1.version=XVID_VERSION;
                pass1.filename=ADM_strdup(logFile.c_str()); // LEAK!
                break;
        case 2:
                {
                plugins[0].func = xvid_plugin_2pass2;
                plugins[0].param = &pass2;
                memset(&pass2,0,sizeof(pass2));
                pass2.version=XVID_VERSION;
                pass2.filename=ADM_strdup(logFile.c_str()); // LEAK!
                uint64_t duration=source->getInfo()->totalDuration;
                switch(xvid4Settings.params.mode)
                {
                  case COMPRESS_2PASS:
                            if(false==ADM_computeAverageBitrateFromDuration(duration,
                                        xvid4Settings.params.finalsize,&avgKbits))
                            {
                                ADM_error("Cannot compute average size\n");
                                return false;
                            }
                            break;
                  case COMPRESS_2PASS_BITRATE:
                            avgKbits=xvid4Settings.params.avg_bitrate;
                            break;
                  default:
                            ADM_assert(0);
                            break;
                }
                ADM_info("Using average bitrate of %d kb/s\n",avgKbits);
                pass2.bitrate=avgKbits*1000;  // kb -> bit/s
                }
                break;
        default:
                ADM_assert(0);
                break;
                
    }
    return true;
}
/**
    \fn setup
*/
bool xvid4Encoder::setup(void)
{
  ADM_info("Xvid4, setting up");
  query();
  xvid_enc_create_t xvid_enc_create;
  // Here we go...
  MMSET (xvid_enc_create);
  MMSET(single);
  xvid_enc_create.version = XVID_VERSION;
  xvid_enc_create.width = getWidth();
  xvid_enc_create.height =getHeight();
  xvid_enc_create.profile=xvid4Settings.profile;
 // Some sane defaults...
  xvid_enc_create.bquant_ratio = 150;
  xvid_enc_create.bquant_offset = 100;
  xvid_enc_create.global |= XVID_GLOBAL_CLOSED_GOP ;

  int thread;
    switch(xvid4Settings.nbThreads)
    {
        default:
        case 0:case 1: thread=1;break;
        case 3: case 4:
        case 2: thread=xvid4Settings.nbThreads;break;
        
        case 99: thread=ADM_cpu_num_processors();break;
    }
    ADM_info("[Xvid] Using %d threads\n",(int)thread);
    xvid_enc_create.num_threads=thread;
    single.version = XVID_VERSION;

    switch(xvid4Settings.params.mode)
    {
      case COMPRESS_2PASS:
      case COMPRESS_2PASS_BITRATE:
           if(false==setupPass())
            {
                ADM_warning("[xvid4] Multipass setup failed\n");
                return false;
            }
            break;
      case COMPRESS_SAME:
      case COMPRESS_CQ:
      case COMPRESS_CBR:
                  
                  plugins[0].func = xvid_plugin_single;
                  plugins[0].param = &single;
                  switch(xvid4Settings.params.mode)
                  {
                    case COMPRESS_CBR:
                            single.bitrate = xvid4Settings.params.bitrate*1000; // b/s
                            ADM_info("[xvid4] Bitrate = %d kb/s\n",int(single.bitrate /1000));
/*
                            single. reaction_delay_factor; 
                            single. averaging_period;      
                            single. buffer;                
*/
                            break;
                    case COMPRESS_CQ:
                            
                            break;
                    case COMPRESS_SAME:

                            break;
                   }
            break;
     default:
            return false;
    }
   
  
  plugins[1].func = xvid4Encoder::hook;
  plugins[1].param = NULL;
  xvid_enc_create.plugins = plugins;
  xvid_enc_create.num_plugins = 2;

  xvid_enc_create.max_bframes = xvid4Settings.maxBFrame;
  xvid_enc_create.max_key_interval = xvid4Settings.maxKeyFrameInterval;
    // dummy
    for(int i=0;i<3;i++)
    {
        xvid_enc_create.min_quant[i]=xvid4Settings.qMin;
        xvid_enc_create.max_quant[i]=xvid4Settings.qMax;
    }
    
    //Framerate
    int n,d;    
    uint64_t f=source->getInfo()->frameIncrement;
    usSecondsToFrac(f,&n,&d);
    xvid_enc_create.fincr = n;
    xvid_enc_create.fbase = d;
    int xerr = xvid_encore (NULL, XVID_ENC_CREATE, &xvid_enc_create, NULL);
    if (xerr < 0)
    {
      ADM_error ("[xvid] init error: %d\n", xerr);
      return false;
    }

    handle = xvid_enc_create.handle;
    image=new ADMImageDefault(getWidth(),getHeight());
    uint64_t inc=source->getInfo()->frameIncrement;
    if(inc<30000) // Less than 30 ms , fps > 30 fps it is probably field
     {
            inc*=2;
            ADM_warning("It is probably field encoded, doubling increment\n");
     }
    if(xvid4Settings.maxBFrame)
        encoderDelay=inc*2;
    else
        encoderDelay=0;
    ADM_info("Xvid4, setup ok\n");
    return true;
}


/** 
    \fn ~ADM_ffMpeg4Encoder
*/
xvid4Encoder::~xvid4Encoder()
{
    ADM_info("[xvid4] Destroying.\n");
    if(handle)
    {
        xvid_encore(handle, XVID_ENC_DESTROY, NULL, NULL);
        handle=NULL;
    }
}

/**
    \fn encode
*/
bool         xvid4Encoder::encode (ADMBitstream * out)
{
    // 1 fetch a frame...
    uint32_t nb;
    // update
again:    
    if(source->getNextFrame(&nb,image)==false)
    {
        ADM_warning("[xvid4] Cannot get next image\n");
        return false;
    }
    // Store Pts/DTS
    ADM_timeMapping map; // Store real PTS <->lav value mapping
    map.realTS=image->Pts+getEncoderDelay();
    aprintf("Pushing fn=%d Time=%"PRIu64"\n",frameNum,map.realTS);
   
    map.internalTS=frameNum++;
    mapper.push_back(map);
    queueOfDts.push_back(image->Pts);

    // 2-preamble
    if(false==preAmble(image))
    {
        ADM_warning("[Xvid4] preAmble failed\n");
        return false;
    }
    xvid_enc_frame.bitstream = out->data;
    int size = xvid_encore(handle, XVID_ENC_ENCODE, &xvid_enc_frame, &xvid_enc_stats);
    if (size < 0)
    {
        ADM_error("[Xvid] Error performing encode %d\n", size);
        return false;
    }
    if(!size)
    {
        ADM_info("Dummy null frame\n");
        goto again;
    }
    // 3-encode
    if(false==postAmble(out,&xvid_enc_stats,size))
    {
        ADM_warning("[Xvid4] postAmble failed\n");
        return false;     
    }
    return true;
}

/**
    \fn isDualPass

*/
bool         xvid4Encoder::isDualPass(void) 
{
    if(xvid4Settings.params.mode==COMPRESS_2PASS || xvid4Settings.params.mode==COMPRESS_2PASS_BITRATE ) return true;
    return false;

}

/**
        \fn preAmble
        \fn prepare a frame to be encoded
*/
bool  xvid4Encoder::preAmble (ADMImage * in)
{
  MMSET (xvid_enc_stats);

  xvid_enc_frame.version = XVID_VERSION;
  xvid_enc_stats.version = XVID_VERSION;

  /* Bind output buffer */

  xvid_enc_frame.length = 0;
  if (xvid4Settings.cqmMode==1)
    xvid_enc_frame.vol_flags |= XVID_VOL_MPEGQUANT;

   switch(xvid4Settings.params.mode)
    {
      case COMPRESS_SAME:
      case COMPRESS_CQ:
            xvid_enc_frame.quant = xvid4Settings.params.qz;
            break;
      default:break;
    }
#define SVOP(x,y) if(xvid4Settings.x) xvid_enc_frame.vop_flags|=XVID_VOP_##y

  xvid_enc_frame.motion = motionMode[xvid4Settings.motionEstimation];
  xvid_enc_frame.vop_flags|=XVID_VOP_INTER4V;
  xvid_enc_frame.vop_flags |= XVID_VOP_HALFPEL;
  xvid_enc_frame.par=xvid4Settings.arMode;
  
  SVOP (trellis, TRELLISQUANT);
  SVOP (hqAcPred, HQACPRED);
  SVOP (rdOnBFrame, RD_BVOP);
  SVOP (optimizeChrome, CHROMAOPT);
  if(xvid4Settings.rdMode!=0)
    xvid_enc_frame.vop_flags|=XVID_VOP_MODEDECISION_RD;
  // ME 
  //if (_param.chroma_me)
  //  {
    //  xvid_enc_frame.motion |= XVID_ME_CHROMA_BVOP;
      //xvid_enc_frame.motion |= XVID_ME_CHROMA_PVOP;
    //}
 

    xvid_enc_frame.motion|=rdMode[xvid4Settings.rdMode];
 
#if 0

  if (_param.turbo)
    {
      xvid_enc_frame.motion |= XVID_ME_FASTREFINE16;
      xvid_enc_frame.motion |= XVID_ME_FASTREFINE8;
      xvid_enc_frame.motion |= XVID_ME_SKIP_DELTASEARCH;
      xvid_enc_frame.motion |= XVID_ME_FAST_MODEINTERPOLATE;
      xvid_enc_frame.motion |= XVID_ME_BFRAME_EARLYSTOP;
    }
#endif
  //xvid_enc_frame.bframe_threshold = _param.bframe_threshold;

  xvid_enc_frame.input.csp = XVID_CSP_PLANAR;
  xvid_enc_frame.input.stride[0] = in->GetPitch(PLANAR_Y);
  xvid_enc_frame.input.stride[2] = in->GetPitch(PLANAR_U);
  xvid_enc_frame.input.stride[1] = in->GetPitch(PLANAR_V);
  xvid_enc_frame.type = XVID_TYPE_AUTO;


  /* Set up motion estimation flags */
  xvid_enc_frame.input.plane[0] = YPLANE(in);
  xvid_enc_frame.input.plane[2] = UPLANE(in);
  xvid_enc_frame.input.plane[1] = VPLANE(in);
  
#if 0
  xvid_enc_frame.par_width = _param.par_width;
  xvid_enc_frame.par_height = _param.par_height;
  //printf("Using AR : %u x %u\n",xvid_enc_frame.par_width,xvid_enc_frame.par_height );
  if (xvid_enc_frame.par_width != xvid_enc_frame.par_height)
      xvid_enc_frame.par = XVID_PAR_EXT;
  else
      xvid_enc_frame.par = XVID_PAR_11_VGA;

  /* Custome matrices */
  if(_param.useCustomIntra) 
  {
  if(!xvid_enc_frame.quant_intra_matrix)  
      printf("[xvid] Using custom intra matrix\n");
      xvid_enc_frame.quant_intra_matrix=_param.intraMatrix;
  }
  if(_param.useCustomInter)
  {
    if(!xvid_enc_frame.quant_inter_matrix)
      printf("[xvid] Using custom inter matrix\n");
     xvid_enc_frame.quant_inter_matrix=_param.interMatrix;
  }
#endif
    if(xvid4Settings.params.mode==COMPRESS_CQ)
            xvid_enc_frame.quant=xvid4Settings.params.qz;
  return 1;
}
/**
    \fn postAmble
    \brief update after a frame has been succesfully encoded
*/
bool xvid4Encoder::postAmble (ADMBitstream * out,xvid_enc_stats_t *stat,int size)
{
  out->flags = 0;
  if (xvid_enc_frame.out_flags & XVID_KEYFRAME)
    {
      out->flags = AVI_KEY_FRAME;
    }
  else if (xvid_enc_stats.type == XVID_TYPE_BVOP)
    {
      out->flags = AVI_B_FRAME;

    }
  out->len=size;
  // update Pts/DTS
  currentRef=outFrameStatic;
  uint32_t myFrame;
  if(out->flags==AVI_B_FRAME)
    {
        myFrame=backRef+refIndex;
        refIndex++;
    }else
    {
        backRef=fwdRef;
        fwdRef=currentRef;
        myFrame=fwdRef;
        refIndex=1;
    }
  out->out_quantizer=stat->quant;
  aprintf("XvidQ:%d\n",(int)out->out_quantizer);
  aprintf("Popping flags=%x fnum=%d back=%d fwd=0%d index=%d => %d\n",
                (int)out->flags,
                (int)outFrameStatic,
                (int)backRef,
                (int)fwdRef,
                (int)refIndex,
                (int)myFrame);
#if 1
  getRealPtsFromInternal(myFrame,&(out->dts),&(out->pts)); 
#else
    out->dts=frameNum*source->getInfo()->frameIncrement;
    out->pts=out->dts+encoderDelay;
#endif
  return 1;
}
/**
    \fn hook
    \brief glue to retrieve frame number and get PTS/DTS later
*/
int xvid4Encoder::hook (void *handle, int opt, void *param1, void *param2)
{
  xvid_plg_data_t *data = (xvid_plg_data_t *) param1;
 //printf("plugin called with %u (%"PRIx64" %"PRIx64")\n",opt,param1,param2);
 
  if (opt==XVID_PLG_FRAME )//|| opt==XVID_PLG_FRAME)
    {
        outFrameStatic=data->frame_num;
    }
  return 0;
}
// EOF

