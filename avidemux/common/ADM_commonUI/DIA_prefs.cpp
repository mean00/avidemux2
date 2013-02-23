/***************************************************************************
  DIA_prefs.cpp
  (C) 2007 Mean Fixounet@free.fr 
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
# include "prefs.h"

#include "audio_out.h"
#include "ADM_assert.h"
#include "ADM_render/GUI_render.h"


#include "DIA_factory.h"

#ifdef _WIN32
#include "ADM_render/GUI_sdlRender.h"
#endif

extern void 		AVDM_audioPref( void );
extern const char* getNativeRendererDesc();



uint8_t DIA_Preferences(void);

/**
      \fn DIA_Preferences
      \brief Handle preference dialog
*/
uint8_t DIA_Preferences(void)
{
uint32_t olddevice,newdevice;

bool     use_odml=0;
uint32_t autosplit=0;
uint32_t render;
bool     useTray=0;


bool     useSwap=0;

uint32_t lavcThreads=0;
uint32_t encodePriority=2;
uint32_t indexPriority=2;
uint32_t playbackPriority=0;
uint32_t downmix;
bool     mpeg_no_limit=0;
uint32_t msglevel=2;
uint32_t trust=0;

uint32_t mixer=0;

char     *alsaDevice=NULL;

bool     balternate_mp3_tag=true;

uint32_t pp_type=3;
uint32_t pp_value=5;

bool     bvdpau=false;
bool     hzd,vzd,dring;
bool     capsMMX,capsMMXEXT,caps3DNOW,caps3DNOWEXT,capsSSE,capsSSE2,capsSSE3,capsSSSE3,capsAll;
bool     hasOpenGl=false;

bool     askPortAvisynth=false;
uint32_t defaultPortAvisynth = 9999;

#ifdef USE_OPENGL
          prefs->get(FEATURES_ENABLE_OPENGL,&hasOpenGl);
#endif

	olddevice=newdevice=AVDM_getCurrentDevice();

        // Default pp
         prefs->get(FEATURES_TRUSTPTS,&trust);
         if(!prefs->get(DEFAULT_POSTPROC_TYPE,&pp_type)) pp_type=3;
         if(!prefs->get(DEFAULT_POSTPROC_VALUE,&pp_value)) pp_value=3;
#define DOME(x,y) y=!!(pp_type & x)
    
    DOME(1,hzd);
    DOME(2,vzd);
    DOME(4,dring);
     
// Cpu caps
#define CPU_CAPS(x)    	if(CpuCaps::myCpuMask & ADM_CPUCAP_##x) caps##x=1; else caps##x=0;
    
    	if(CpuCaps::myCpuMask==ADM_CPUCAP_ALL) capsAll=1; else capsAll=0;
    	CPU_CAPS(MMX);
    	CPU_CAPS(MMXEXT);
    	CPU_CAPS(3DNOW);
    	CPU_CAPS(3DNOWEXT);
    	CPU_CAPS(SSE);
    	CPU_CAPS(SSE2);
    	CPU_CAPS(SSE3);
    	CPU_CAPS(SSSE3);

    	//Avisynth
    	if(!prefs->get(AVISYNTH_AVISYNTH_ALWAYS_ASK, &askPortAvisynth))
        {
    		ADM_info("Always ask not set\n");
    		askPortAvisynth=0;
    	}

    	if(!prefs->get(AVISYNTH_AVISYNTH_DEFAULTPORT, &defaultPortAvisynth))
        {
    			printf("Port not set\n");
                        defaultPortAvisynth=9999;
    	}
    	ADM_info("Avisynth port: %d\n",defaultPortAvisynth);
    
        // Alsa
#ifdef ALSA_SUPPORT
        if( prefs->get(DEVICE_AUDIO_ALSA_DEVICE, &alsaDevice) != RC_OK )
                alsaDevice = ADM_strdup("plughw:0,0");
#endif
        // vdpau
        prefs->get(FEATURES_VDPAU,&bvdpau);
        
        // Alternate mp3 tag (haali)
        prefs->get(FEATURES_ALTERNATE_MP3_TAG,&balternate_mp3_tag);
        
        // Video renderer
        if(prefs->get(VIDEODEVICE,&render)!=RC_OK)
        {       
                render=(uint32_t)RENDER_GTK;
        }
        // SysTray
        if(!prefs->get(FEATURES_USE_SYSTRAY,&useTray)) 
                useTray=0;
        // Accept mpeg for DVD when fq!=48 kHz
        if(!prefs->get(FEATURES_MPEG_NO_LIMIT,&mpeg_no_limit)) mpeg_no_limit=0;

        // Multithreads
        prefs->get(FEATURES_THREADING_LAVC, &lavcThreads);


		// Encoding priority
		if(!prefs->get(PRIORITY_ENCODING, &encodePriority))
                encodePriority=2;
		// Indexing / unpacking priority
		if(!prefs->get(PRIORITY_INDEXING, &indexPriority))
                indexPriority=2;
		// Playback priority
		if(!prefs->get(PRIORITY_PLAYBACK, &playbackPriority))
                playbackPriority=0;

        // VCD/SVCD split point		
        if(!prefs->get(MPEGSPLIT_AUTOSPLIT, &autosplit))
                autosplit=690;		
                        
        // Open DML (Gmv)
        if(!prefs->get(FEATURES_USE_ODML, &use_odml))
          use_odml=0;
#if defined(ALSA_SUPPORT) || defined (OSS_SUPPORT)
		// Master or PCM for audio
        if(!prefs->get(FEATURES_AUDIOBAR_USES_MASTER, &useMaster))
                useMaster=0;
#endif

        // SWAP A&B if A>B
        if(!prefs->get(FEATURES_SWAP_IF_A_GREATER_THAN_B, &useSwap))
                useSwap=0;
        // Get level of message verbosity
        prefs->get(MESSAGE_LEVEL,&msglevel);
        // Downmix default
        if(prefs->get(DEFAULT_DOWNMIXING,&downmix)!=RC_OK)
        {       
            downmix=0;
        }
        olddevice=newdevice=AVDM_getCurrentDevice();
        // Audio device
        /************************ Build diaelems ****************************************/
        diaElemToggle useVdpau(&bvdpau,QT_TR_NOOP("Decode video using VDPAU"));
        diaElemToggle useOpenGl(&hasOpenGl,QT_TR_NOOP("Enable openGl support"));
#ifndef USE_OPENGL
        //useOpenGl.enable(0);
#endif
        
        diaElemToggle useSysTray(&useTray,QT_TR_NOOP("_Use systray while encoding"));
        diaElemToggle allowAnyMpeg(&mpeg_no_limit,QT_TR_NOOP("_Accept non-standard audio frequency for DVD"));
        diaElemToggle openDml(&use_odml,QT_TR_NOOP("Create _OpenDML files"));

        

        diaElemFrame frameSimd(QT_TR_NOOP("SIMD"));

		diaElemToggle capsToggleAll(&capsAll,QT_TR_NOOP("Enable all SIMD"));
        diaElemToggle capsToggleMMX(&capsMMX, QT_TR_NOOP("Enable MMX"));
		diaElemToggle capsToggleMMXEXT(&capsMMXEXT, QT_TR_NOOP("Enable MMXEXT"));
		diaElemToggle capsToggle3DNOW(&caps3DNOW, QT_TR_NOOP("Enable 3DNOW"));
		diaElemToggle capsToggle3DNOWEXT(&caps3DNOWEXT, QT_TR_NOOP("Enable 3DNOWEXT"));
		diaElemToggle capsToggleSSE(&capsSSE, QT_TR_NOOP("Enable SSE"));
		diaElemToggle capsToggleSSE2(&capsSSE2, QT_TR_NOOP("Enable SSE2"));
		diaElemToggle capsToggleSSE3(&capsSSE3, QT_TR_NOOP("Enable SSE3"));
		diaElemToggle capsToggleSSSE3(&capsSSSE3, QT_TR_NOOP("Enable SSSE3"));

		capsToggleAll.link(0, &capsToggleMMX);
		capsToggleAll.link(0, &capsToggleMMXEXT);
		capsToggleAll.link(0, &capsToggle3DNOW);
		capsToggleAll.link(0, &capsToggle3DNOWEXT);
		capsToggleAll.link(0, &capsToggleSSE);
		capsToggleAll.link(0, &capsToggleSSE2);
		capsToggleAll.link(0, &capsToggleSSE3);
		capsToggleAll.link(0, &capsToggleSSSE3);

		frameSimd.swallow(&capsToggleAll);
		frameSimd.swallow(&capsToggleMMX);
		frameSimd.swallow(&capsToggleMMXEXT);
		frameSimd.swallow(&capsToggle3DNOW);
		frameSimd.swallow(&capsToggle3DNOWEXT);
		frameSimd.swallow(&capsToggleSSE);
		frameSimd.swallow(&capsToggleSSE2);
		frameSimd.swallow(&capsToggleSSE3);
		frameSimd.swallow(&capsToggleSSSE3);

		diaElemThreadCount lavcThreadCount(&lavcThreads, QT_TR_NOOP("_lavc threads:"));

		diaElemFrame frameThread(QT_TR_NOOP("Multi-threading"));
		frameThread.swallow(&lavcThreadCount);

		diaMenuEntry priorityEntries[] = {
                             {0,       QT_TR_NOOP("High"),NULL}
                             ,{1,      QT_TR_NOOP("Above normal"),NULL}
                             ,{2,      QT_TR_NOOP("Normal"),NULL}
							 ,{3,      QT_TR_NOOP("Below normal"),NULL}
							 ,{4,      QT_TR_NOOP("Low"),NULL}
        };
		diaElemMenu menuEncodePriority(&encodePriority,QT_TR_NOOP("_Encoding priority:"), sizeof(priorityEntries)/sizeof(diaMenuEntry), priorityEntries,"");
		diaElemMenu menuIndexPriority(&indexPriority,QT_TR_NOOP("_Indexing/unpacking priority:"), sizeof(priorityEntries)/sizeof(diaMenuEntry), priorityEntries,"");
		diaElemMenu menuPlaybackPriority(&playbackPriority,QT_TR_NOOP("_Playback priority:"), sizeof(priorityEntries)/sizeof(diaMenuEntry), priorityEntries,"");

		diaElemFrame framePriority(QT_TR_NOOP("Prioritisation"));
		framePriority.swallow(&menuEncodePriority);
		framePriority.swallow(&menuIndexPriority);
		framePriority.swallow(&menuPlaybackPriority);

        diaElemUInteger autoSplit(&autosplit,QT_TR_NOOP("_Split MPEG files every (MB):"),10,4096);
        
        diaElemToggle   togTagMp3(&balternate_mp3_tag,QT_TR_NOOP("_Use alternative tag for MP3 in .mp4"));
        
        diaMenuEntry videoMode[]={
                             {RENDER_GTK, getNativeRendererDesc(), NULL}
#ifdef USE_XV
                             ,{RENDER_XV,   QT_TR_NOOP("XVideo (best)"),NULL}
#endif
#ifdef USE_VDPAU
                             ,{RENDER_VDPAU,   QT_TR_NOOP("VDPAU (best)"),NULL}
#endif
#ifdef USE_OPENGL
                             ,{RENDER_QTOPENGL,   QT_TR_NOOP("OpenGL (best)"),NULL}
#endif

#ifdef USE_SDL
#ifdef _WIN32
                             ,{RENDER_SDL,      QT_TR_NOOP("MS Windows GDI (SDL)"),NULL}
							 ,{RENDER_DIRECTX,      QT_TR_NOOP("MS Windows DirectX (SDL)"),NULL}
#else
							 ,{RENDER_SDL,      QT_TR_NOOP("SDL (good)"),NULL}
#endif
#endif
        };        
        diaElemMenu menuVideoMode(&render,QT_TR_NOOP("Video _display:"), sizeof(videoMode)/sizeof(diaMenuEntry),videoMode,"");
        
        
        
        diaMenuEntry msgEntries[]={
                             {0,       QT_TR_NOOP("No alerts"),NULL}
                             ,{1,      QT_TR_NOOP("Display only error alerts"),NULL}
                             ,{2,      QT_TR_NOOP("Display all alerts"),NULL}
        };
        diaMenuEntry trustEntries[]={
                             {0,       QT_TR_NOOP("Trust all"),NULL}
                             ,{1,      QT_TR_NOOP("Partially trust"),NULL}
                             ,{2,      QT_TR_NOOP("Dont trust"),NULL}
        };
        diaElemMenu menuMessage(&msglevel,QT_TR_NOOP("_Message level:"), sizeof(msgEntries)/sizeof(diaMenuEntry),msgEntries,"");
        diaElemMenu menuTrust(&trust,QT_TR_NOOP("Timestamp confidence:"), sizeof(trustEntries)/sizeof(diaMenuEntry),trustEntries,"");
        
        
#if defined(ALSA_SUPPORT) || defined (OSS_SUPPORT)
        diaMenuEntry volumeEntries[]={
                             {0,       QT_TR_NOOP("PCM"),NULL}
                             ,{1,      QT_TR_NOOP("Master"),NULL}};
        diaElemMenu menuVolume(&useMaster,QT_TR_NOOP("_Volume control:"), sizeof(volumeEntries)/sizeof(diaMenuEntry),volumeEntries,"");
#endif
        
        
         diaMenuEntry mixerEntries[]={
                             {0,       QT_TR_NOOP("No downmixing"),NULL}
                             ,{1,       QT_TR_NOOP("Stereo"),NULL}
                             ,{2,      QT_TR_NOOP("Pro Logic"),NULL}
                              ,{3,      QT_TR_NOOP("Pro Logic II"),NULL}
         };
        diaElemMenu menuMixer(&downmix,QT_TR_NOOP("_Local playback downmixing:"), sizeof(mixerEntries)/sizeof(diaMenuEntry),mixerEntries,"");
//*********** AV_
		
//***AV
        uint32_t nbAudioDevice=ADM_av_getNbDevices();
        diaMenuEntryDynamic **audioDeviceItems=new diaMenuEntryDynamic *[nbAudioDevice+1];
        audioDeviceItems[0]=new diaMenuEntryDynamic(0,"Dummy","Dummy");
        for(int i=0;i<nbAudioDevice;i++)
        {
            const char *name;
            uint32_t major,minor,patch;
            ADM_av_getDeviceInfo(i, &name, &major,&minor,&patch);
            audioDeviceItems[i+1]=new diaMenuEntryDynamic(i+1,name,name);
        }
        diaElemMenuDynamic menuAudio(&newdevice,QT_TR_NOOP("_AudioDevice"), nbAudioDevice+1, 
                    audioDeviceItems,NULL);
        // default Post proc
     diaElemToggle     fhzd(&hzd,QT_TR_NOOP("_Horizontal deblocking"));
     diaElemToggle     fvzd(&vzd,QT_TR_NOOP("_Vertical deblocking"));
     diaElemToggle     fdring(&dring,QT_TR_NOOP("De_ringing"));
     diaElemUInteger   postProcStrength(&pp_value,QT_TR_NOOP("_Strength:"),0,5);
     diaElemFrame      framePP(QT_TR_NOOP("Default Postprocessing"));
     
     framePP.swallow(&fhzd);
     framePP.swallow(&fvzd);
     framePP.swallow(&fdring);
     framePP.swallow(&postProcStrength);
     

        /* User Interface */
        diaElem *diaUser[]={&useSysTray,&menuMessage,&menuTrust};
        diaElemTabs tabUser(QT_TR_NOOP("User Interface"),3,diaUser);
        
         /* Automation */
        
        
        /* Output */
        diaElem *diaOutput[]={&autoSplit,&openDml,&allowAnyMpeg,&togTagMp3};
        diaElemTabs tabOutput(QT_TR_NOOP("Output"),4,(diaElem **)diaOutput);
        
        /* Audio */

#if 0 //defined(ALSA_SUPPORT)
        diaElem *diaAudio[]={&menuMixer,&menuVolume,&menuAudio,&entryAlsaDevice};
        diaElemTabs tabAudio(QT_TR_NOOP("Audio"),4,(diaElem **)diaAudio);
//#elif defined(OSS_SUPPORT)
        diaElem *diaAudio[]={&menuMixer,&menuVolume,&menuAudio};
        diaElemTabs tabAudio(QT_TR_NOOP("Audio"),3,(diaElem **)diaAudio);
#endif

#if 1
        diaElem *diaAudio[]={&menuMixer,&menuAudio};
        diaElemTabs tabAudio(QT_TR_NOOP("Audio"),2,(diaElem **)diaAudio);
#endif

        
        /* Video */
        diaElem *diaVideo[]={&menuVideoMode,&framePP,&useVdpau,&useOpenGl};
        diaElemTabs tabVideo(QT_TR_NOOP("Video"),4,(diaElem **)diaVideo);
        
        /* CPU tab */
		diaElem *diaCpu[]={&frameSimd};
		diaElemTabs tabCpu(QT_TR_NOOP("CPU"),1,(diaElem **)diaCpu);

        /* Threading tab */
		diaElem *diaThreading[]={&frameThread, &framePriority};
		diaElemTabs tabThreading(QT_TR_NOOP("Threading"),2,(diaElem **)diaThreading);

		/* Avisynth tab */
		diaElemToggle togAskAvisynthPort(&askPortAvisynth,"_Always ask which port to use");
		diaElemUInteger uintDefaultPortAvisynth(&defaultPortAvisynth,"Default port to use",1024,65535);
		diaElem *diaAvisynth[]={&togAskAvisynthPort, &uintDefaultPortAvisynth};
		diaElemTabs tabAvisynth("Avisynth",2,(diaElem **)diaAvisynth);

        /* Global Glyph tab */

                                    
// SET
        diaElemTabs *tabs[]={&tabUser,&tabOutput,&tabAudio,&tabVideo,&tabCpu,&tabThreading, &tabAvisynth};
        if( diaFactoryRunTabs(QT_TR_NOOP("Preferences"),7,tabs))
	{
        	//
#ifdef USE_OPENGL
          prefs->set(FEATURES_ENABLE_OPENGL,hasOpenGl);
#endif
        	// cpu caps
        		if(capsAll)
        		{
        			CpuCaps::myCpuMask=ADM_CPUCAP_ALL;
        		}else
        		{
        			CpuCaps::myCpuMask=0;
#undef CPU_CAPS
#define CPU_CAPS(x)    	if(caps##x) CpuCaps::myCpuMask|= ADM_CPUCAP_##x;        	    	
        	    	CPU_CAPS(MMX);
        	    	CPU_CAPS(MMXEXT);
        	    	CPU_CAPS(3DNOW);
        	    	CPU_CAPS(3DNOWEXT);
        	    	CPU_CAPS(SSE);
        	    	CPU_CAPS(SSE2);
        	    	CPU_CAPS(SSE3);
        	    	CPU_CAPS(SSSE3);
        		}
        		prefs->set(FEATURES_CPU_CAPS,CpuCaps::myCpuMask);

                // Postproc
                #undef DOME
                #define DOME(x,y) if(y) pp_type |=x;
                pp_type=0;
                DOME(1,hzd);
                DOME(2,vzd);
                DOME(4,dring);
                prefs->set(DEFAULT_POSTPROC_TYPE,pp_type);
                prefs->set(DEFAULT_POSTPROC_VALUE,pp_value);

                // Alsa
#ifdef ALSA_SUPPORT
                if(alsaDevice)
                {
                   prefs->set(DEVICE_AUDIO_ALSA_DEVICE, alsaDevice);
                   ADM_dealloc(alsaDevice);
                   alsaDevice=NULL;
                }
#endif
                // Device
                //printf("[AudioDevice] Old : %d, new :%d\n",olddevice,newdevice);
                if(olddevice!=newdevice)
                {
                      AVDM_switch((AUDIO_DEVICE)newdevice); // Change current device
                      AVDM_audioSave();                     // Save it in prefs
                      AVDM_audioInit();                     // Respawn
                }
                // Downmixing (default)
                prefs->set(DEFAULT_DOWNMIXING,downmix);
#if defined(ALSA_SUPPORT) || defined (OSS_SUPPORT)
                // Master or PCM
                prefs->set(FEATURES_AUDIOBAR_USES_MASTER, useMaster);
#endif
                // allow non std audio fq for dvd
                prefs->set(FEATURES_MPEG_NO_LIMIT, mpeg_no_limit);
                // Video render
                prefs->set(VIDEODEVICE,render);
                // Odml
                prefs->set(FEATURES_USE_ODML, use_odml);
				// Split
                prefs->set(MPEGSPLIT_AUTOSPLIT, autosplit);
                
                // number of threads
                prefs->set(FEATURES_THREADING_LAVC, lavcThreads);
                prefs->set(FEATURES_TRUSTPTS,trust);
                // Encoding priority
                prefs->set(PRIORITY_ENCODING, encodePriority);
                // Indexing / unpacking priority
                prefs->set(PRIORITY_INDEXING, indexPriority);
                // Playback priority
                prefs->set(PRIORITY_PLAYBACK, playbackPriority);

                // Auto swap A/B
                prefs->set(FEATURES_SWAP_IF_A_GREATER_THAN_B, useSwap);
                //
                prefs->set(MESSAGE_LEVEL,msglevel);
                // Use tray while encoding
                prefs->set(FEATURES_USE_SYSTRAY,useTray);

                // VDPAU
                prefs->set(FEATURES_VDPAU,bvdpau);
                // Alternate mp3 tag (haali)
                prefs->set(FEATURES_ALTERNATE_MP3_TAG,balternate_mp3_tag);

			
                // Avisynth
                prefs->set(AVISYNTH_AVISYNTH_DEFAULTPORT,defaultPortAvisynth);
                prefs->set(AVISYNTH_AVISYNTH_ALWAYS_ASK, askPortAvisynth);
#if defined(_WIN32) && defined(USE_SDL)
                // Initialise SDL again as driver may have changed
                initSdl(render);
#endif
                
	}
        for(int i=0;i<nbAudioDevice+1;i++)
        {
            
            delete audioDeviceItems[i];
        }
        delete [] audioDeviceItems;



	return 1;
}
extern int DIA_getMPParams( uint32_t *pplevel, uint32_t *ppstrength,bool *swap);
void setpp(void)
{
        uint32_t type,strength;
        bool uv=0;

        if(!prefs->get(DEFAULT_POSTPROC_TYPE,&type)) type=3;
        if(!prefs->get(DEFAULT_POSTPROC_VALUE,&strength)) strength=3;
        if( DIA_getMPParams( &type,&strength,&uv))
        {
                prefs->set(DEFAULT_POSTPROC_TYPE,type);
                prefs->set(DEFAULT_POSTPROC_VALUE,strength);
        }
//	video_body->setPostProc(type, strength, uv);
}
//EOF 
