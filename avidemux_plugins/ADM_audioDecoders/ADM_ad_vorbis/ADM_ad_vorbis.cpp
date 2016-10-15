/***************************************************************************
                          audiocodec_ogg.cpp  -  description
                             -------------------
    begin                : Thu May 23 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr

    Strongly inspired by mplayer ogg vorbis decoder
    (ripp off should be more appropriate)

    We expect the first packet as extraData. It contains needed info such
    as frequency/channels etc...

    How to deal with comment and codebook ?
    Mmmmm

 ***************************************************************************/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <vorbis/codec.h>

#include "ADM_default.h"
#include "ADM_ad_plugin.h"
#include "ADM_ad_vorbis.h"
#include "ADM_coreUtils.h"


/**
 * \class ADM_vorbis
 */
class ADM_vorbis : public     ADM_Audiocodec
{
    protected:
        oggVorbis           _context;

    public:
                            ADM_vorbis(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d);
        virtual             ~ADM_vorbis();
        virtual    uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
        virtual    uint8_t isCompressed(void) {return 1;}
        virtual    uint8_t isDecompressable(void) {return 1;}
        virtual    bool    resetAfterSeek(void) ;
};

// Supported formats + declare our plugin
//*******************************************************
static  ad_supportedFormat Formats[]={
        {WAV_OGG_VORBIS,AD_MEDIUM_QUAL},
};
DECLARE_AUDIO_DECODER(ADM_vorbis,                        // Class
            0,0,1,                                                 // Major, minor,patch
            Formats,                                             // Supported formats
            "libVorbis decoder plugin for avidemux (c) Mean\n");     // Desc
//********************************************************


 ADM_vorbis::~ADM_vorbis()
 {
    if(_init)
    {
        vorbis_block_clear(&(_context.vblock));
        vorbis_info_clear(&(_context.vinfo));
    }
    _init=false;
 }
 /**
  * 
  * @param name
  * @param pack
  */
 static void printPacket(const char *name, ogg_packet *pack)
 {
     ADM_warning(" sending %s packet of size %d\n",name,pack->bytes);
     mixDump(pack->packet,pack->bytes);
     ADM_warning("\n");
 }
 /**
  * 
  * @param name
  * @param error
  */
 static void printError(const char *name, int error)
 {
     ADM_warning(" Error %d when processing %s\n",error,name);
     
#define VERR(x) case x: ADM_warning(#x"\n")     ;break;
     
     switch(error)
     {
VERR(OV_EREAD)
VERR(OV_EFAULT     )
VERR(OV_EIMPL      )
VERR(OV_EINVAL     )
VERR(OV_ENOTVORBIS )
VERR(OV_EBADHEADER )
VERR(OV_EVERSION   )
VERR(OV_ENOTAUDIO  )
VERR(OV_EBADPACKET )
VERR(OV_EBADLINK   )
VERR(OV_ENOSEEK    )

         default: ADM_warning("Unknown error\n");
                  break;
     }
     
 }
 /**
  * 
  * @param fcc
  * @param info
  * @param extra
  * @param extraData
  */
 ADM_vorbis::ADM_vorbis(uint32_t fcc, WAVHeader *info, uint32_t extra, uint8_t *extraData)
     : ADM_Audiocodec(fcc,*info)
 {
 ogg_packet packet;
 vorbis_comment comment;
 oggVorbis *vrbis;
 uint8_t *hdr,*cmt,*code;
 uint32_t size_hdr,size_cmt, size_code;
 uint32_t *ptr;
 int error;
#define MANAGE_ERROR(st,er) {if(er<0) {printError(st,er); return ;}}
     _init=0;
     ADM_info("Trying to initialize vorbis codec with %d bytes of header data\n",(int)extra);

     _init=false;
    memset(&(_context),0,sizeof(_context));

    // init everything
    vorbis_info_init(&(_context.vinfo));
    vorbis_comment_init(&(_context.vcomment));

    // split extradata as header/comment/code
    ptr=(uint32_t *)extraData;
    size_hdr=*ptr++;
    size_cmt=*ptr++;
    size_code=*ptr++;

    hdr=extraData;
    hdr+=3*sizeof(uint32_t);
    cmt=hdr+size_hdr;
    code=cmt+size_cmt;

        
        
     // Feed header passed as extraData
    packet.bytes=size_hdr;
    packet.packet=hdr;
    packet.b_o_s=1; // yes, it is a new stream
    printPacket("1st packet",&packet);
    error=vorbis_synthesis_headerin(&(_context.vinfo),&comment,&packet);
    MANAGE_ERROR("1st packet",error);
    // update some info in header this is the only place to get them
    // especially frequency.
/*
    info->channels=STRUCT->vinfo.channels;
    info->frequency=STRUCT->vinfo.rate;
*/
    info->byterate=_context.vinfo.bitrate_nominal>>3;

    if(!info->byterate)
    {
        ADM_warning("Mmm, no nominal bitrate...\n");
        info->byterate=16000;
    }
    // now unpack comment
    packet.bytes=size_cmt;
    packet.packet=cmt;
    packet.b_o_s=0; // Not new
    printPacket("2nd packet",&packet);

    error=vorbis_synthesis_headerin(&(_context.vinfo),&comment,&packet);        
    MANAGE_ERROR("2nd packet",error);

    // and codebook
    packet.bytes=size_code;
    packet.packet=code;
    packet.b_o_s=0; // Not new
    printPacket("3rd packet",&packet);
        
    error=vorbis_synthesis_headerin(&(_context.vinfo),&comment,&packet);
    MANAGE_ERROR("3rd packet",error);
        
    vorbis_comment_clear(&comment);
    vorbis_synthesis_init(&(_context.vdsp),&(_context.vinfo));
    vorbis_block_init(&(_context.vdsp),&(_context.vblock));
    ADM_info("Vorbis init successfull\n");
    _context.ampscale=1;
    _init=1;
  CHANNEL_TYPE *p_ch_type = channelMapping;
#define DOIT(y) *(p_ch_type++)=ADM_CH_##y;
    switch(_context.vinfo.channels)
    {
    case 1:
    case 2:
    {
        DOIT(FRONT_LEFT);
        DOIT(FRONT_RIGHT);
        break;
    }
    default:
    case 5:
    {
        DOIT(FRONT_LEFT);
        DOIT(FRONT_CENTER);
        DOIT(FRONT_RIGHT);
        
        
        DOIT(REAR_LEFT);
        DOIT(REAR_RIGHT);
        DOIT(LFE);
        break;
    }
    }
 }
 /**
  * 
  * @param inptr
  * @param nbIn
  * @param outptr
  * @param nbOut
  * @return 
  */

 uint8_t ADM_vorbis::run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut)
{
ogg_packet packet;
float **sample_pcm;
int    nb_synth;

    *nbOut=0;
    if(!_init) return 0;
    packet.b_o_s=0;
    packet.e_o_s=0;
    packet.bytes=nbIn;
    packet.packet=inptr;
        packet.granulepos=-1;
    if(!vorbis_synthesis(&(_context.vblock),&packet))
    {
          vorbis_synthesis_blockin(&(_context.vdsp),&(_context.vblock));
     }
     nb_synth=vorbis_synthesis_pcmout(&(_context.vdsp),&sample_pcm);
     if(nb_synth<0)
     {
         printf("error decoding vorbis %d\n",nb_synth);
        return 0;
     }

     for (uint32_t samp = 0; samp < nb_synth; samp++)
         for (uint8_t chan = 0; chan < _context.vinfo.channels; chan++)
            *outptr++ = sample_pcm[chan][samp] * _context.ampscale;

     *nbOut = _context.vinfo.channels * nb_synth;

    // Puge them
     vorbis_synthesis_read(&_context.vdsp,nb_synth);
     //printf("This round : in %d bytes, out %d bytes synthetized:%d\n",nbIn,*nbOut,nb_synth);
    return 1;

}
/**
    \fn resetAfterSeek
    \brief  Try to flush the buffer
            unsuccessfully :(

*/
  bool    ADM_vorbis::resetAfterSeek(void) 
  {
  float **sample_pcm;
  ogg_packet packet;

    ADM_info("Vorbis-Resetting\n");
    vorbis_synthesis_pcmout(&(_context.vdsp),&sample_pcm);
    vorbis_synthesis_restart(&(_context.vdsp));
    return true;
  }


