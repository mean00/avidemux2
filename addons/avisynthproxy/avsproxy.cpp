// Avs2YUV by Loren Merritt

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.


#define __int64_t long long int
#define uint64_t long long unsigned int

#include "internal.h"
#include "sket.h"
#include "avsHeader.h"

#ifndef INT_MAX
#define INT_MAX 0x7fffffff
#endif

#define MY_VERSION "Avs2YUV 0.24 ADM_1.1"
#define MAX_FH 10
#include "winsock2.h"
#define Log printf
/******************************************/
int vid_width=-1;
int vid_height=-1;
int vid_fps1000=-1;
int vid_nbFrame=-1;
uint32_t currentFrame=0xFFFF0000;
char *Buffer;

PVideoFrame Aframe=NULL;
IScriptEnvironment* env =NULL;
PClip clip =NULL;
avsyInfo    info;

/*****************************************/
static int framePack(PVideoFrame p);
static void handleError(void);
static bool initAvisynth(const char *infile);


/**
	\fn main
*/
int __cdecl main(int argc, const char* argv[])
{
	const char* infile = NULL;
	char *audioBuffer=new char[48000*6*2];
	char *audioBufferData=audioBuffer+sizeof(avsAudioFrame);

	printf("AvsSocket Proxy, derivated from avs2yuv by  Loren Merritt \n");
	fflush(stdout);
	
	
		if(argc>=2)
		{
			infile = argv[1];
			const char *dot = strrchr(infile, '.');
			if(!dot || strcmp(".avs", dot))
			{
				fprintf(stderr, "infile (%s) doesn't look like an avisynth script\n", infile);
				fflush(stderr);
				infile=NULL;
			}
		}
		else
		{
			infile="toto.avs";
		}
	if(!infile) {
		fprintf(stderr, MY_VERSION "\n"
		"Usage: avs2yuv  in.avs \n");
		fflush(stderr);
		return 2;
	}
	if(!initAvisynth(infile))
	{
		printf("Avisynth initfailed\n");
		fflush(stdout);
		handleError();
		exit (-2);
	}
	//********************************
	
	WSADATA wsaData;
	int iResult;
		printf("Initializing WinSock\n");
		fflush(stdout);
		iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (iResult != NO_ERROR)
		{
			printf("Error at WSAStartup()\n");
			fflush(stdout);
			handleError();
			exit(-1);
		}	
		printf("WinSock ok\n");
		fflush(stdout);
		SetLastError(0);
		
	//********************************
		
	
				
		Buffer=new char[vid_width*vid_height*2]; // Too much but who cares
		
		//
		Sket *sket=new Sket();
		if(!sket->waitConnexion())
		{
			printf("Accept/listen error\n");
			fflush(stdout);
			handleError();
			exit(-1);
		}
		// We got a connection, now handle request
		// First be stupid
		uint32_t cmd,frame,len;
		uint32_t page=(vid_width*vid_height*3)>>1;
		uint8_t payload[1000]; // Never used normally...
		while(1)
		{
			if(!sket->receive(&cmd,&frame,&len,payload))
			{
				printf("Error in receive\n");
				fflush(stdout);
				handleError();
				exit(-1);
			}
			switch(cmd)
			{
					case AvsCmd_GetInfo:
							printf("Received get info...\n");
							fflush(stdout);
							if(len!=8)
							{
									// Version
									printf("This version of avsproxy is not compatible with the avidemux version you are using\n");
									fflush(stdout);
									exit( -1);
							}
							uint32_t api,ver;
							api=*(uint32_t *)payload;
							ver=*(uint32_t *)(payload+4);
							printf("Connection from avidemux, api=%d version=%d\n",api,ver);
							if(api!=AVSHEADER_API_VERSION)
							{
									printf("This version of avsproxy has api version %d, avidemux has version %d, exiting\n",AVSHEADER_API_VERSION,api);
									fflush(stdout);
									exit(-1);
							}
							sket->sendData(AvsCmd_SendInfo,0,sizeof(info),(uint8_t *)&info);
							break;
					case AvsCmd_GetAudio:
							// Read payload
							if(len!=sizeof(avsAudioFrame))
							{
									printf("get audio command, expected %d bytes , got %d \n",sizeof(avsAudioFrame),len);
									fflush(stdout);
									exit(-1);	
							}
							avsAudioFrame aFrame;
							memcpy(&aFrame,payload,len);
							// Read...
							try {
								clip->GetAudio(audioBufferData, aFrame.startSample, aFrame.sizeInFloatSample, env);
							}catch(AvisynthError err) 
							{
								fprintf(stderr, "Get Audio :Avisynth error:\n%s\n", err.msg);	
							    aFrame.sizeInFloatSample=0;
							}
							// Send reply...
							memcpy(audioBuffer,&aFrame,sizeof(aFrame));
							sket->sendData(AvsCmd_SendAudio,0,aFrame.sizeInFloatSample*2*info.channels+sizeof(aFrame),(uint8_t *)audioBuffer);
							break;
					case AvsCmd_GetFrame:
							if(currentFrame!=frame)
							{
								
							try{
								Log("Get frame %u (old:%u)\n",frame,currentFrame);
								fflush(stdout);
								Aframe= clip->GetFrame(frame, env);
								framePack(Aframe);	
								currentFrame=frame;
								}
								catch(AvisynthError err) 
								{		
									fprintf(stderr, "\nAvisynth error:\n%s\n", err.msg);
									fflush(stderr);
									handleError();
									return 1;
								}
							}
							sket->sendData(AvsCmd_SendFrame,frame,page,(uint8_t *)Buffer);
							if(frame<(uint32_t)vid_nbFrame-1)
							{
								frame++;
								Aframe= clip->GetFrame(frame, env);
								framePack(Aframe);	
								currentFrame=frame;
							}

							break;
					default:
							printf("Unknown command\n");
							fflush(stdout);
							handleError();
							exit(-1);


			}


		}	
}
/**
	\fn framePack
	\brief Pack a video frame inside our buffer
*/
int framePack(PVideoFrame p)
{
			int w,stride;
			const BYTE *data;
			char *out;
			
			
			data=p->GetReadPtr(PLANAR_Y);
			w=vid_width;
			stride=p->GetPitch(PLANAR_Y);
			out=Buffer;
			for(int i=0;i<vid_height;i++)
			{
				memcpy(out,data,w);
				data+=stride;
				out+=w;
			}

			data=p->GetReadPtr(PLANAR_V);
			w=vid_width>>1;
			stride=p->GetPitch(PLANAR_V);
			out=Buffer+(vid_width*vid_height);
			for(int i=0;i<vid_height>>1;i++)
			{
				memcpy(out,data,w);
				data+=stride;
				out+=w;
			}

			data=p->GetReadPtr(PLANAR_U);
			w=vid_width>>1;
			stride=p->GetPitch(PLANAR_U);
			out=Buffer+((5*vid_width*vid_height)>>2);
			for(int i=0;i<vid_height>>1;i++)
			{
				memcpy(out,data,w);
				data+=stride;
				out+=w;
			}
			return 1;
}
/**
	\fn handleError
*/
void handleError(void)
{
	DWORD er=GetLastError();
	printf("Err: %d\n",er);
	fflush(stdout);
	exit(-1);

}
/**
	\fn initAvisynth
*/
typedef IScriptEnvironment * __stdcall DLLFUNC(int);

bool initAvisynth(const char *infile)
{
		HMODULE instance;
		DLLFUNC *CreateScriptEnvironment=NULL;

		printf("Loading Avisynth.dll \n");
		fflush(stdout);
		instance= LoadLibrary("avisynth.dll");
		if(!instance) 
		{
			handleError();
			fprintf(stderr, "failed to load avisynth.dll\n"); 
			fflush(stderr);
			return false;
		}
		printf("Avisynth.dll loaded\n");
		fflush(stdout);
		CreateScriptEnvironment			= (DLLFUNC *) GetProcAddress(instance, "CreateScriptEnvironment");
		printf("Env created\n");
		fflush(stdout);
		if(!CreateScriptEnvironment)
		{
			fprintf(stderr, "failed to load CreateScriptEnvironment()\n"); 
			fflush(stderr); 
			return false;
		}
try{
		env = CreateScriptEnvironment(AVISYNTH_INTERFACE_VERSION);
		if(!env)
		{
			fprintf(stderr, "Env failed\n"); 
			fflush(stderr); 
			return false;
		}
		AVSValue args[]={infile};
		printf("Importing..\n");
		fflush(stdout);
		PClip dummy(env->Invoke("Import", AVSValue(args, 1)).AsClip());
		clip=dummy;

		VideoInfo inf = clip->GetVideoInfo();
	    memset(&info,0,sizeof(info));
		info.version=AVSHEADER_API_VERSION;
		// Video part
		info.width=vid_width=inf.width;
		info.height=vid_height=inf.height;
		info.fps1000=vid_fps1000=(inf.fps_numerator*1000)/inf.fps_denominator;
		printf("%d / %d\n",inf.fps_numerator,inf.fps_denominator);
		fflush(stdout);
		float f=(float)inf.fps_numerator;
		f*=1000;
		f/=inf.fps_denominator;
		vid_fps1000=info.fps1000=(uint32_t)ceil(f);
		info.nbFrames=vid_nbFrame=inf.num_frames;
		// audio part
		info.frequency=inf.SamplesPerSecond(); // 0 means no audio
		info.channels=inf.AudioChannels();
		if(inf.SampleType()!=SAMPLE_INT16)
		{
			printf("Only int16 for audio and not %d!\n",(int)inf.SampleType());
			fflush(stdout);
			handleError();
			return false;	
		}
		if(!inf.IsYV12()) 
		{
			printf("Only yv12!\n");
			fflush(stdout);
			handleError();
			return false;
		}
		if(!inf.IsYV12())
			{fprintf(stderr, "Couldn't convert input to YV12\n"); fflush(stderr); return false;}
		if(inf.IsFieldBased())
			{fprintf(stderr, "Needs progressive input\n"); fflush(stderr); return false;}

	
		// Incoming ready
		printf("Clip Info\n");
		printf("_________\n");
		printf("Width     :%d \n",vid_width);
		printf("Height    :%d \n",vid_height);
		printf("Fps1K     :%d \n",vid_fps1000);
		printf("NbFrame   :%d \n",vid_nbFrame);
		printf("Frequency :%d \n",info.frequency);
		printf("#channels :%d \n",info.channels);
		fflush(stdout);
		return true;
}
catch(AvisynthError err) {
		
			fprintf(stderr, "\nAvisynth error:\n%s\n", err.msg);
			fflush(stderr);
		return false;
	}



		return true;
}

//EOF

