//
// C++ Implementation: ADM_codecfaad
//
// Description: class ADM_faad, interface to libfaad2 .
//
// The init can be done 2 ways
// 1- some info are used as extradata (esds atom ot whatever)
// 2- the init is done when decoding the actual raw stream
//
// The big drawback if that in some case you can eat up a lot of the stream
// before finding a packet start code to enable correct init of the codec
//
// Author: mean <fixounet@free.fr>, (C) 2004/2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <faad.h>

#include "ADM_default.h"
#include "ADM_ad_plugin.h"

#define FAAD_BUFFER (10*1024)
/**
    \class ADM_faad
*/
class ADM_faad : public     ADM_Audiocodec
{
	protected:
		uint8_t _inited;
		void	*_instance;
		uint8_t  _buffer[FAAD_BUFFER*2];
		uint32_t head, tail;
        bool     monoFaadBug; // if true, the stream is mono, but faad outputs stereo
        uint32_t fq;
    protected:
        uint8_t     extraData[16]; // usually 2 bytes...
        int         extraDataSize;
        bool        initFaad(WAVHeader *info,uint32_t l,uint8_t *d);
	public:
		ADM_faad(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d);
		virtual	~ADM_faad();
		virtual	void purge(void);
		virtual	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
		virtual	uint8_t isCompressed(void) {return 1;}
		virtual	uint8_t isDecompressable(void) {return 1;}
		virtual	bool     resetAfterSeek(void);
        virtual uint32_t getOutputFrequency(void);
};
// Supported formats + declare our plugin
//*******************************************************

static  ad_supportedFormat Formats[]={
        {WAV_AAC,AD_MEDIUM_QUAL},
        {WAV_MP4,AD_MEDIUM_QUAL},
        {0x706D,AD_MEDIUM_QUAL},
};
DECLARE_AUDIO_DECODER(ADM_faad,						// Class
			0,0,1, 												// Major, minor,patch
			Formats, 											// Supported formats
			"Faad2 decoder plugin for avidemux (c) Mean\n"); 	// Desc
//********************************************************
/**
    \fn getOutputFrequency
*/
uint32_t ADM_faad::getOutputFrequency(void)
{
    return fq;

}
/**
    \fn initFaad

*/
bool ADM_faad::initFaad(WAVHeader *info,uint32_t l,uint8_t *d)
{
faacDecConfigurationPtr conf;
#ifdef FAAD_OLD_PROTOTYPE
unsigned long int srate;
#else
uint32_t srate;
#endif
unsigned char chan;

    _instance = faacDecOpen();
    conf=faacDecGetCurrentConfiguration(_instance);
    // Update the field we know about
    conf->outputFormat=FAAD_FMT_FLOAT;
    conf->defSampleRate=info->frequency;
    conf->defObjectType =LC;
    fq=info->frequency;
    //
    faacDecSetConfiguration(_instance, conf);
    ADM_info("[FAAD] using %u bytes of extradata\n",l);
    if(l)
    {
        for(int i=0;i<l;i++) ADM_info("%02x ",d[i]);
        ADM_info("\n");
    }
    // if we have some extra data, it means we can init it from it
    if(l)
    {
        
        faacDecInit2(_instance, d,l, &srate,&chan);
        ADM_info("[FAAD]Found :%"LU" rate %"LU" channels\n",(uint32_t)srate,(uint32_t)chan);
        if(srate!=info->frequency)
        {
            ADM_info("[FAAD]Frequency mismatch!!! %d to %"LU" (SBR ?)\n",info->frequency,(uint32_t)srate);
            if(srate==2*info->frequency)
            {
                ADM_info("Sbr detected\n");
                fq=srate;
            }
            
            //info->frequency=srate;
        }
        if(chan!=info->channels) // Ask for stereo !
        {
            ADM_info("[FAAD]channel mismatch!!! %d to %d \n",info->channels,chan);
            if(info->channels==1 && chan==2) 
            {
                    ADM_warning("Workaround Faad mono stream handling... \n");
                    monoFaadBug=true;
            }
            
        }
        ADM_assert(l<16);
        memcpy(extraData,d,l);
        extraDataSize=l;
    }
    
}
/**
    \fn ctor
*/
ADM_faad::ADM_faad( uint32_t fourcc ,WAVHeader *info,uint32_t l,uint8_t *d) :   ADM_Audiocodec(fourcc,*info)
{
        extraDataSize=0;
		_inited=0;
		_instance=NULL;
        head=tail=0;
        monoFaadBug=false;
        initFaad(info,l,d);
        if(l)
        {
            _inited=1;
            ADM_assert(l<16);
            memcpy(extraData,d,l);
            extraDataSize=l;
            
        }

        // Give our channel configuration
        switch(info->channels)
        {
            case 1: channelMapping[0] = ADM_CH_FRONT_CENTER;
                    break;
            case 2: channelMapping[0] = ADM_CH_FRONT_LEFT;
                    channelMapping[1] = ADM_CH_FRONT_RIGHT;
                    break;
            default:
            {
                CHANNEL_TYPE *p=channelMapping;
                *p++ = ADM_CH_FRONT_CENTER;
                *p++ = ADM_CH_FRONT_LEFT;
                *p++ = ADM_CH_FRONT_RIGHT;
                *p++ = ADM_CH_REAR_LEFT;
                *p++ = ADM_CH_REAR_RIGHT;                
                *p++ = ADM_CH_LFE;
              }
                break;
        }

		ADM_info("[FAAD]Faad decoder created\n");
}
/**
    \fn dtor

*/
ADM_faad::~ADM_faad()
{
	if(_instance)
		faacDecClose(_instance);
	_instance=NULL;
	ADM_info("Faad decoder closed\n");
}
/**
    \fn purge
*/
void ADM_faad::purge(void)
{
	head=tail=0;
	faacDecPostSeekReset(_instance, 0);
}
/**
    \fn resetAfterSeek
*/
bool ADM_faad::resetAfterSeek( void )
{
	 head=tail=0;
	 faacDecPostSeekReset(_instance, 0);
     // close then reopen, it's the only way to get rid of the glitch
#if 1
     if(extraDataSize)
     {
           
            faacDecClose(_instance);
            ADM_info("Resetting faad\n");
            initFaad(&wavHeader,extraDataSize,extraData);
     }
#endif
     return 1;
}
/**
    \fn run
*/
uint8_t ADM_faad::run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut)
{
uint32_t consumed;
long int res;
void *outbuf;
faacDecFrameInfo info;
int max=0;
#ifdef FAAD_OLD_PROTOTYPE
unsigned long int srate;
#else
uint32_t srate;
#endif
unsigned char chan=0;
uint8_t first=0;


		ADM_assert(_instance);
		*nbOut=0;
		if(!_inited) // we still have'nt initialized the codec
		{
			// Try
			ADM_info("Trying with %d bytes\n",nbIn);
			res=(long int)faacDecInit(_instance,inptr,nbIn,&srate,&chan);
			if(res>=0)
			{
				ADM_info("Faad Inited : rate:%d chan:%d off:%ld\n",(int)srate,(int)chan,res);
				_inited=1;
				first=1;
				head=tail=0;
				inptr+=res;
				nbIn-=res;
			}
		}
		if(!_inited)
		{
			ADM_info("No dice...\n");
			return 1;
		}
		// The codec is initialized, feed him
		do
		{
            // Shrink ? The buffer is 2*FAAD_BUFFER
            if(tail>FAAD_BUFFER && head)
            {
                memmove(_buffer,_buffer+head,tail-head);
                tail-=head;
                head=0;
            }
            // Add incoming data to our own buffer
			max=FAAD_BUFFER*2-tail;
			if(nbIn<max) max=nbIn;
			memcpy(_buffer+tail,inptr,max);
			inptr+=max;
			nbIn-=max;
			tail+=max;
			memset(&info,0,sizeof(info));
			//printf("%x %x\n",_buffer[0],_buffer[1]);
		 	outbuf = faacDecDecode(_instance, &info, _buffer+head, tail-head);
			if(info.error)
			{
				ADM_warning("Faad: Error %d :%s\n",info.error,faacDecGetErrorMessage(info.error));
				ADM_warning("Bytes consumed %"LLU", bytes dropped %"LU" \n",info.bytesconsumed,tail-head);

                head=tail=0; // Purge buffer
				return 1;
			}
			if(first)
			{
				printf("Channels : %d\n",info.channels);
				printf("Frequency: %"LLU"\n",info.samplerate);
				printf("SBR      : %d\n",info.sbr);
			}
			consumed=info.bytesconsumed ;
			if(consumed>(tail-head)) 
            {
                ADM_warning("Too much data consumed %d vs %d\n",(int)consumed,(int)(tail-head));
                head=tail=0;
                }
                else head+=consumed;
			if(info.samples)
			{
                if(monoFaadBug)
                {
                    uint32_t n=info.samples/2;
                    float *f=(float *)outbuf;
                    for(int i=0;i<n;i++)
                    {
                        *outptr++=*f; // drop one out of two samples
                        f+=2;
                    }
                    *nbOut+=n;
                }else   
                {
                    *nbOut+= info.samples;
                    memcpy(outptr,outbuf,info.samples*sizeof(float));
                    outptr+=info.samples;
                }
			}
		}while(nbIn || (tail-head));
		return 1;
}

