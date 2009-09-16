/***************************************************************************
    copyright            : (C) 2006 by mean
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
#include <math.h>

#include <math.h>

#include "ADM_default.h"
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"

#include "audioencoder.h"
#include "audioencoderInternal.h"
#include "audioencoder_vorbis_param.h"
#include "audioencoder_vorbis.h"


#include "vorbis/vorbisenc.h"

#define OPTIONS (twolame_options_struct *)_twolameOptions

#define VD (((vorbisStruct *)_handle)->vd)
#define VI (((vorbisStruct *)_handle)->vi)
#define VB (((vorbisStruct *)_handle)->vb)
#define VC (((vorbisStruct *)_handle)->vc)
typedef struct vorbisStruct
{
	vorbis_info 	 vi ;
	vorbis_dsp_state vd ;
	vorbis_block     vb ;
	vorbis_comment   vc ;
}vorbisStruct;
//___________
static VORBIS_encoderParam vorbisParam=
{
    128,
    ADM_VORBIS_VBR,
    9
};

static bool configure(void);
/********************* Declare Plugin *****************************************************/
ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(AUDMEncoder_Vorbis);

static ADM_audioEncoder encoderDesc = {
  ADM_AUDIO_ENCODER_API_VERSION,
  create,			// Defined by macro automatically
  destroy,			// Defined by macro automatically
  configure,		//** put your own function here**
  "Vorbis",
  "Vorbis",
  "Vorbis encoder plugin Mean 2008",
  6,                    // Max channels
  1,0,0,                // Version
  WAV_OGG_VORBIS,
  200,                  // Priority
  getConfigurationData,  // Defined by macro automatically
  setConfigurationData,  // Defined by macro automatically

  getBitrate,           // Defined by macro automatically
  setBitrate,            // Defined by macro automatically

  NULL,         //** put your own function here**

  NULL
};
ADM_DECLARE_AUDIO_ENCODER_CONFIG(vorbisParam);

/******************* / Declare plugin*******************************************************/


//__________

AUDMEncoder_Vorbis::AUDMEncoder_Vorbis(AUDMAudioFilter * instream)  :ADM_AudioEncoder    (instream)
{
  printf("[Vorbis] Creating Vorbis\n");
  _handle=NULL;
  wavheader.encoding=WAV_OGG_VORBIS;
  _oldpos=0;
  _handle=(void *)new  vorbisStruct;
  outputChannelMapping[0] = ADM_CH_FRONT_LEFT;
  outputChannelMapping[1] = ADM_CH_FRONT_RIGHT;
  outputChannelMapping[2] = ADM_CH_REAR_LEFT;
  outputChannelMapping[3] = ADM_CH_REAR_RIGHT;
  outputChannelMapping[4] = ADM_CH_FRONT_CENTER;
  outputChannelMapping[5] = ADM_CH_LFE;
};

/**

*/
AUDMEncoder_Vorbis::~AUDMEncoder_Vorbis()
{
  printf("[Vorbis] Deleting Vorbis\n");
  if(_handle)
  {
    vorbis_block_clear(&VB);
    vorbis_dsp_clear(&VD);
    vorbis_info_clear(&VI);
    delete (vorbisStruct *)_handle;
  }
  _handle=NULL;

};

/**
    \fn initialize

*/
bool AUDMEncoder_Vorbis::initialize(void)
{
  int ret;
  VORBIS_encoderParam *vorbisConf=&vorbisParam;


  ogg_packet header1,header2,header3;
  int err;



  vorbis_info_init(&VI) ;

  switch(vorbisConf->mode)
  {

    case ADM_VORBIS_VBR:
                      err=vorbis_encode_init(&VI,
                              wavheader.channels,
                              wavheader.frequency,
                              -1, // Max bitrate
                              vorbisConf->bitrate*1000, //long nominal_bitrate,
                              -1 //long min_bitrate))
                            );
                      break;
    case  ADM_VORBIS_QUALITY :
                    err=vorbis_encode_init_vbr(&VI,
                                wavheader.channels,
                                wavheader.frequency,
                                vorbisConf->quality/10
                              );
                    break;

    default:
      ADM_assert(0);
  }
  if (err!=0)
  {
	  delete (vorbisStruct*)_handle;
	  _handle = NULL;

    printf("[vorbis] init error %d\n",err);
    return 0;
  }
  vorbis_analysis_init(&VD, &VI) ;
  vorbis_block_init(&VD, &VB);
  vorbis_comment_init(&VC);
  vorbis_comment_add_tag(&VC, "encoder", "AVIDEMUX2") ;

  vorbis_analysis_headerout(&VD, &VC, &header1,
                             &header2, &header3);


// Store all headers as extra data
// see ogg vorbis decode for details
// we need 3 packets

  _extraSize=header1.bytes+header2.bytes+header3.bytes+3*sizeof(uint32_t);
  _extraData=new uint8_t[_extraSize];

  uint32_t *ex=(uint32_t *)_extraData;
  uint8_t *d;
  d=_extraData+sizeof(uint32_t)*3;
  ex[0]=header1.bytes;
  ex[1]=header2.bytes;
  ex[2]=header3.bytes;
  memcpy(d,header1.packet,ex[0]);
  d+=ex[0];
  memcpy(d,header2.packet,ex[1]);
  d+=ex[1];
  memcpy(d,header3.packet,ex[2]);
  vorbis_comment_clear(&VC);

  printf("\n[Vorbis]Vorbis encoder initialized\n");
  switch(vorbisConf->mode)
  {
    case ADM_VORBIS_VBR:
      printf("[Vorbis]CBR Bitrate:%"LU"\n",vorbisConf->bitrate);
      break;
    case ADM_VORBIS_QUALITY: //FIXME FIXME FIXME
      printf("[Vorbis]VBR Quality:%.1f\n",vorbisConf->quality);
    break;
    default:
      ADM_assert(0);
  }

  printf("[Vorbis]Channels  :%"LU"\n",wavheader.channels);
  printf("[Vorbis]Frequency :%"LU"\n",wavheader.frequency);
  return 1;
}

#define ROUNDMAX 3000
/**
    \fn encode

*/
bool	AUDMEncoder_Vorbis::encode(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
  uint32_t nbout;
  uint32_t consumed=0;
  float **float_samples;
    int channels=wavheader.channels;
  ogg_packet op ;

  *len = 0;
  _chunk=1024*channels;
  int count=ROUNDMAX;
// Check that we have packet from previous pass
  while(count--)
  {
    if(!refillBuffer(_chunk ))
    {
      return 0;
    }

    if(tmptail-tmphead<_chunk)
    {
      return 0;
    }

	//printf("Round %d\n",ROUNDMAX-count);
    if(vorbis_analysis_blockout(&VD, &VB) == 1)
    {
      vorbis_analysis(&VB, NULL);
      vorbis_bitrate_addblock(&VB) ;
	//printf("Blockout\n");

      if(vorbis_bitrate_flushpacket(&VD, &op))
      {
        memcpy(dest, op.packet,op.bytes);
        *len=op.bytes;
        *samples=op.granulepos-_oldpos;
        _oldpos=op.granulepos;
        //  aprintf("1st packet :sampl:%lu len :%lu sample:%lu abs:%llu\n",*samples,op.bytes,total,op.granulepos);
        return 1;
      }
    }


    uint32_t nbSample=(tmptail-tmphead)/channels;
    if(nbSample>1024) nbSample=1024;
    float_samples=vorbis_analysis_buffer(&VD, nbSample) ;
    int index=tmphead;
    // Put our samples in incoming buffer
    reorderChannels(&(tmpbuffer[tmphead]), nbSample,_incoming->getChannelMapping(),outputChannelMapping);
    for (int i = 0; i < nbSample; i++)
      for (int j = 0; j < channels; j++) {
      float_samples[j][i] = tmpbuffer[index++];
      if (float_samples[j][i] > 1) float_samples[j][i] = 1;
      if (float_samples[j][i] < -1) float_samples[j][i] = -1;
      }
      // Buffer full, go go go
      vorbis_analysis_wrote(&VD, nbSample) ;
      tmphead+=nbSample*channels;
  }
  return 0;

}
#define SZT(x) sizeof(x)/sizeof(diaMenuEntry )
#define PX(x) &(lameParam->x)
#define BITRATE(x) {x,QT_TR_NOOP(#x)}
/**
      \fn configure
      \brief Dialog to set vorbis settings
      @return 1 on success, 0 on failure

*/

bool configure(void)
{

    uint32_t mmode,ppreset;
    ELEM_TYPE_FLOAT qqual;


   VORBIS_encoderParam *vParam=&vorbisParam;

    mmode=vParam->mode;
    qqual=(ELEM_TYPE_FLOAT)vParam->quality;

    diaMenuEntry channelMode[]={
                             {ADM_VORBIS_VBR,      QT_TR_NOOP("VBR"),NULL},
                             {ADM_VORBIS_QUALITY,   QT_TR_NOOP("Quality based"),NULL}};

    diaElemMenu menuMode(&mmode,   QT_TR_NOOP("_Mode:"), SZT(channelMode),channelMode);


    diaMenuEntry bitrateM[]={
                              BITRATE(56),
                              BITRATE(64),
                              BITRATE(80),
                              BITRATE(96),
                              BITRATE(112),
                              BITRATE(128),
                              BITRATE(160),
                              BITRATE(192),
                              BITRATE(224)
                          };
    diaElemMenu bitrate(&(vParam->bitrate),   QT_TR_NOOP("_Bitrate:"), SZT(bitrateM),bitrateM);

    diaElemFloat quality(&qqual,QT_TR_NOOP("_Quality:"),-1.,10.);




      diaElem *elems[]={&menuMode,&bitrate,&quality};

  if( diaFactoryRun(QT_TR_NOOP("Vorbis Configuration"),3,elems))
  {
    vParam->mode=(ADM_VORBIS_MODE)mmode;
    vParam->quality=(float)qqual;
    return 1;
  }
  return 0;
}

// EOF
