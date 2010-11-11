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

class ADM_faad : public     ADM_Audiocodec
{
	protected:
		uint8_t _inited;
		void	*_instance;
		uint8_t  _buffer[FAAD_BUFFER*2];
		uint32_t head, tail;
        bool     monoFaadBug; // if true, the stream is mono, but faad outputs stereo

	public:
		ADM_faad(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d);
		virtual	~ADM_faad();
		virtual	void purge(void);
		virtual	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
		virtual	uint8_t isCompressed(void) {return 1;}
		virtual	uint8_t isDecompressable(void) {return 1;}
		virtual	uint8_t beginDecompress(void);
		virtual	uint8_t endDecompress(void);
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

ADM_faad::ADM_faad( uint32_t fourcc ,WAVHeader *info,uint32_t l,uint8_t *d) :   ADM_Audiocodec(fourcc)
{
faacDecConfigurationPtr conf;
#ifdef FAAD_OLD_PROTOTYPE
unsigned long int srate;
#else
uint32_t srate;
#endif
unsigned char chan;
		_inited=0;
		_instance=NULL;
        head=tail=0;
        monoFaadBug=false;
		_instance = faacDecOpen();
		conf=faacDecGetCurrentConfiguration(_instance);
		// Update the field we know about
		conf->outputFormat=FAAD_FMT_FLOAT;
		conf->defSampleRate=info->frequency;
  	    conf->defObjectType =LC;
		faacDecSetConfiguration(_instance, conf);
                printf("[FAAD] using %u bytes of extradata\n",l);
		// if we have some extra data, it means we can init it from it
		if(l)
		{
			_inited = 1;
			faacDecInit2(_instance, d,l, &srate,&chan);
			printf("[FAAD]Found :%"LU" rate %"LU" channels\n",(uint32_t)srate,(uint32_t)chan);
                        if(srate!=info->frequency)
                        {
                            printf("[FAAD]Frequency mismatch!!! %d to %"LU" (SBR ?)\n",info->frequency,(uint32_t)srate);
                            info->frequency=srate;
                        }
                        if(chan!=info->channels) // Ask for stereo !
                        {
                            printf("[FAAD]channel mismatch!!! %d to %d \n",info->channels,chan);
                            if(info->channels==1 && chan==2) 
                            {
                                    ADM_warning("Workaround Faad mono stream handling... \n");
                                    monoFaadBug=true;
                            }
                            
                        }
		}
		else // we will init it later on...
		{
			_inited=0;
			printf("No conf header, will try to init later\n");

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

		printf("[FAAD]Faad decoder created\n");
}

ADM_faad::~ADM_faad()
{
	if(_instance)
		faacDecClose(_instance);
	_instance=NULL;
	printf("Faad decoder closed\n");
}

void ADM_faad::purge(void)
{
	head=tail=0;
	faacDecPostSeekReset(_instance, 0);
}
uint8_t ADM_faad:: beginDecompress( void )
{
	head=tail=0;
    return 1;
}
uint8_t ADM_faad::endDecompress( void )
{
	 head=tail=0;
	 faacDecPostSeekReset(_instance, 0);
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
			printf("Trying with %d bytes\n",nbIn);
			res=(long int)faacDecInit(_instance,inptr,nbIn,&srate,&chan);
			if(res>=0)
			{
				printf("Faad Inited : rate:%d chan:%d off:%ld\n",(int)srate,(int)chan,res);
				_inited=1;
				first=1;
				head=tail=0;
				inptr+=res;
				nbIn-=res;
			}
		}
		if(!_inited)
		{
			printf("No dice...\n");
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
				ADM_warning("Bye consumed %"LLU", bytes dropped %"LU" \n",info.bytesconsumed,tail-head);

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
			if(consumed>(tail-head)) {head=tail=0;}
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

