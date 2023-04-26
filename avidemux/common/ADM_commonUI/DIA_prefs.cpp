/***************************************************************************
  DIA_prefs.cpp
  (C) 2007/2016 Mean Fixounet@free.fr
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

#include "audio_out.h"
#include "ADM_render/GUI_render.h"
#include "ADM_pp.h"

#include "DIA_factory.h"
#ifdef USE_VDPAU
#include "ADM_coreVdpau/ADM_coreVdpau.h"
#endif
#ifdef USE_SDL
#include "ADM_render/GUI_sdlRender.h"
#endif

extern void 		AVDM_audioPref( void );
extern const char* getNativeRendererDesc(int type);

#if defined(USE_DXVA2) || defined(USE_VDPAU) || defined(USE_LIBVA) || defined(USE_VIDEOTOOLBOX)
    #define HW_ACCELERATED_DECODING
#endif

uint8_t DIA_Preferences(void);

/**
      \fn DIA_Preferences
      \brief Handle preference dialog
*/
uint8_t DIA_Preferences(void)
{
uint32_t olddevice,newdevice;

uint32_t render;

bool     useSwap=0;

uint32_t lavcThreads=0;
uint32_t encodePriority=2;
//uint32_t indexPriority=2;
uint32_t playbackPriority=0;
uint32_t downmix;
bool     mpeg_no_limit=0;
uint32_t msglevel=2;

uint32_t mixer=0;
bool     doAutoUpdate=false;
bool     loadDefault=false;
char     *alsaDevice=NULL;

bool     lastReadDirAsTarget=false;
bool     multiPassStatsAutoDelete=false;
bool     altKeyboardShortcuts=false;
bool     swapUpDown=false;
bool     swapWheel=false;

uint32_t pp_type=3;
uint32_t pp_value=5;

bool     useCustomFragmentSize=false;
uint32_t customFragmentSize=4000;
bool     loadPicsInReverseOrder=false;
uint32_t loadPicsFpsType=1;

uint32_t editor_cache_size=16;
bool     editor_use_shared_cache=false;

#ifdef USE_DXVA2
bool     bdxva2=false;
bool     bdxva2_override_version=false;
bool     bdxva2_override_profile=false;
#endif
bool     bvdpau=false;
bool     blibva=false;
#ifdef USE_VIDEOTOOLBOX
bool     bvideotoolbox=false;
#endif
bool     hzd,vzd,dring;
bool     capsMMX,capsMMXEXT,caps3DNOW,caps3DNOWEXT,capsSSE,capsSSE2,capsSSE3,capsSSSE3,capsSSE4,capsSSE42,capsAVX,capsAVX2,capsFMA3,capsAll;
bool     hasOpenGl=false;
bool     filterPreviewAccel=true;

bool     refreshCapEnabled=false;
uint32_t refreshCapValue=100;

bool     askPortAvisynth=false;
uint32_t defaultPortAvisynth = 9999;

uint32_t toneMappingHDR = 1;
uint32_t outOfGamutHDR = 0;
float    targetLumHDR = DEFAULT_TARGET_LUMINANCE_HDR;

#ifdef USE_SDL
std::string currentSdlDriver=getSdlDriverName();
#endif

#ifdef USE_OPENGL
        if(!prefs->get(FEATURES_ENABLE_OPENGL,&hasOpenGl))
            hasOpenGl = false;
        if(!prefs->get(FEATURES_FILTER_PREVIEW_CANVAS_OPENGL,&filterPreviewAccel))
            filterPreviewAccel = true;
#endif

	olddevice=newdevice=AVDM_getCurrentDevice();

        prefs->get(FEATURES_CAP_REFRESH_ENABLED,&refreshCapEnabled);
        prefs->get(FEATURES_CAP_REFRESH_VALUE,&refreshCapValue);


        // Default pp
         if(!prefs->get(DEFAULT_POSTPROC_TYPE,&pp_type)) pp_type=0;
         if(!prefs->get(DEFAULT_POSTPROC_VALUE,&pp_value)) pp_value=0;
#define DOME(x,y) y=!!(pp_type & x);

    DOME(ADM_POSTPROC_HORIZ_DEBLOCK,hzd)
    DOME(ADM_POSTPROC_VERT_DEBLOCK,vzd)
    DOME(ADM_POSTPROC_DERING,dring)

// Cpu caps
#define CPU_CAPS(x) \
    if((cpuCaps & cpuMask) & ADM_CPUCAP_##x) \
        caps##x = true; \
    else \
        caps##x = false;

        uint32_t cpuMask=CpuCaps::getMask();
        uint32_t cpuCaps=CpuCaps::getCaps();
    	if(cpuMask==ADM_CPUCAP_ALL) capsAll=1; else capsAll=0;
    	CPU_CAPS(MMX);
    	CPU_CAPS(MMXEXT);
    	CPU_CAPS(3DNOW);
    	CPU_CAPS(3DNOWEXT);
    	CPU_CAPS(SSE);
    	CPU_CAPS(SSE2);
    	CPU_CAPS(SSE3);
    	CPU_CAPS(SSSE3);
    	CPU_CAPS(SSE4);
    	CPU_CAPS(SSE42);
    	CPU_CAPS(AVX);
    	CPU_CAPS(AVX2);
        CPU_CAPS(FMA3);

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

    	// HDR
    	if (!prefs->get(HDR_TONEMAPPING,&toneMappingHDR)) toneMappingHDR=1;
    	if (!prefs->get(HDR_TARGET_LUMINANCE,&targetLumHDR)) targetLumHDR = DEFAULT_TARGET_LUMINANCE_HDR;
        if (!prefs->get(HDR_OUT_OF_GAMUT_HANDLING,&outOfGamutHDR)) outOfGamutHDR=0;

        // Alsa
#ifdef ALSA_SUPPORT
        if( prefs->get(DEVICE_AUDIO_ALSA_DEVICE, &alsaDevice) != RC_OK )
                alsaDevice = ADM_strdup("plughw:0,0");
#endif
        // Auto-append
        if(!prefs->get(DEFAULT_MULTILOAD_USE_CUSTOM_SIZE,&useCustomFragmentSize))
            useCustomFragmentSize=false;
        if(!prefs->get(DEFAULT_MULTILOAD_CUSTOM_SIZE_M,&customFragmentSize))
            customFragmentSize=4000;
        // Should the pics demuxer be used to reverse video?
        if(!prefs->get(PICTURES_LOAD_IN_REVERSE_ORDER,&loadPicsInReverseOrder))
            loadPicsInReverseOrder = false;
        // Which timebase should the pics demuxer use?
        if(!prefs->get(PICTURES_FPS_TYPE,&loadPicsFpsType))
            loadPicsFpsType = 1;

        // Video cache
        prefs->get(FEATURES_CACHE_SIZE,&editor_cache_size);
        prefs->get(FEATURES_SHARED_CACHE,&editor_use_shared_cache);
#ifdef USE_DXVA2
        // dxva2
        prefs->get(FEATURES_DXVA2,&bdxva2);
        prefs->get(FEATURES_DXVA2_OVERRIDE_BLACKLIST_VERSION,&bdxva2_override_version);
        prefs->get(FEATURES_DXVA2_OVERRIDE_BLACKLIST_PROFILE,&bdxva2_override_profile);
#endif
#ifdef USE_VDPAU
        // vdpau
        prefs->get(FEATURES_VDPAU,&bvdpau);
#endif
#ifdef USE_LIBVA
        // libva
        prefs->get(FEATURES_LIBVA,&blibva);
#endif
#ifdef USE_VIDEOTOOLBOX
        // VideoToolbox
        prefs->get(FEATURES_VIDEOTOOLBOX,&bvideotoolbox);
#endif
        // Video renderer
        if(prefs->get(VIDEODEVICE,&render)!=RC_OK)
        {
                render=(uint32_t)RENDER_GTK;
        }
        // Accept mpeg for DVD when fq!=48 kHz
        if(!prefs->get(FEATURES_MPEG_NO_LIMIT,&mpeg_no_limit)) mpeg_no_limit=0;

        prefs->get(UPDATE_ENABLED,&doAutoUpdate);

        prefs->get(RESET_ENCODER_ON_VIDEO_LOAD,&loadDefault);

        // Make users happy who prefer the output dir to be the same as the input dir
        prefs->get(FEATURES_USE_LAST_READ_DIR_AS_TARGET,&lastReadDirAsTarget);

        // Get the default state of the checkbox in the encoding dialog to auto-delete stats files
        prefs->get(DEFAULT_DELETE_FIRST_PASS_LOG_FILES,&multiPassStatsAutoDelete);

        // PgUp and PgDown are cumbersome to reach on some laptops, offer alternative kbd shortcuts
        prefs->get(KEYBOARD_SHORTCUTS_USE_ALTERNATE_KBD_SHORTCUTS,&altKeyboardShortcuts);

        // Optionally reverse UP and DOWN keys for navigation
        prefs->get(KEYBOARD_SHORTCUTS_SWAP_UP_DOWN_KEYS,&swapUpDown);
        
        // Optionally reverse mouse wheel for navigation
        prefs->get(FEATURES_SWAP_MOUSE_WHEEL,&swapWheel);

        // Multithreads
        prefs->get(FEATURES_THREADING_LAVC, &lavcThreads);


        // Encoding priority
        if(!prefs->get(PRIORITY_ENCODING, &encodePriority))
        encodePriority=2;
        // Indexing / unpacking priority
        //if(!prefs->get(PRIORITY_INDEXING, &indexPriority))
        //indexPriority=2;
        // Playback priority
        if(!prefs->get(PRIORITY_PLAYBACK, &playbackPriority))
        playbackPriority=0;

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
        if(!prefs->get(DEFAULT_DOWNMIXING,&downmix))
        {
            downmix=1; // Stereo
        }
        olddevice=newdevice=AVDM_getCurrentDevice();
        // Audio device
        /************************ Build diaelems ****************************************/
        diaElemToggle allowAnyMpeg(&mpeg_no_limit,QT_TRANSLATE_NOOP("adm","_Accept non-standard audio frequency for DVD"));
        diaElemToggle resetEncoder(&loadDefault,QT_TRANSLATE_NOOP("adm","_Revert to saved default output settings on video load"));
        diaElemToggle enableAltShortcuts(&altKeyboardShortcuts,QT_TRANSLATE_NOOP("adm","_Enable alternative keyboard shortcuts"));
        diaElemToggle swapUpDownKeys(&swapUpDown,QT_TRANSLATE_NOOP("adm","Re_verse UP and DOWN arrow keys for navigation"));
        diaElemToggle swapMouseWheel(&swapWheel,QT_TRANSLATE_NOOP("adm","Reverse mouse _wheel for navigation"));
        diaElemToggle swapMarkers(&useSwap,QT_TRANSLATE_NOOP("adm","_Swap markers if marker A is set past marker B or marker B before A in video"));
        diaElemToggle checkForUpdate(&doAutoUpdate,QT_TRANSLATE_NOOP("adm","_Check for new release"));


        diaElemFrame frameSimd(QT_TRANSLATE_NOOP("adm","SIMD"));

        diaElemToggle capsToggleAll(&capsAll,QT_TRANSLATE_NOOP("adm","Enable all SIMD"));
        diaElemToggle capsToggleMMX(&capsMMX, QT_TRANSLATE_NOOP("adm","Enable MMX"));
        diaElemToggle capsToggleMMXEXT(&capsMMXEXT, QT_TRANSLATE_NOOP("adm","Enable MMXEXT"));
        diaElemToggle capsToggle3DNOW(&caps3DNOW, QT_TRANSLATE_NOOP("adm","Enable 3DNOW"));
        diaElemToggle capsToggle3DNOWEXT(&caps3DNOWEXT, QT_TRANSLATE_NOOP("adm","Enable 3DNOWEXT"));
        diaElemToggle capsToggleSSE(&capsSSE, QT_TRANSLATE_NOOP("adm","Enable SSE"));
        diaElemToggle capsToggleSSE2(&capsSSE2, QT_TRANSLATE_NOOP("adm","Enable SSE2"));
        diaElemToggle capsToggleSSE3(&capsSSE3, QT_TRANSLATE_NOOP("adm","Enable SSE3"));
        diaElemToggle capsToggleSSSE3(&capsSSSE3, QT_TRANSLATE_NOOP("adm","Enable SSSE3"));
        diaElemToggle capsToggleSSE4(&capsSSE4, QT_TRANSLATE_NOOP("adm","Enable SSE4"));
        diaElemToggle capsToggleSSE42(&capsSSE42, QT_TRANSLATE_NOOP("adm","Enable SSE4.2"));
        diaElemToggle capsToggleAVX(&capsAVX, QT_TRANSLATE_NOOP("adm","Enable AVX"));
        diaElemToggle capsToggleAVX2(&capsAVX2, QT_TRANSLATE_NOOP("adm","Enable AVX2"));
        diaElemToggle capsToggleFMA3(&capsFMA3, QT_TRANSLATE_NOOP("adm","Enable FMA3"));

#define CPU_CAP_AVAIL(x) \
    if(cpuCaps & ADM_CPUCAP_##x) \
        capsToggleAll.link(0, &capsToggle##x);

        CPU_CAP_AVAIL(MMX)
        CPU_CAP_AVAIL(MMXEXT)
        CPU_CAP_AVAIL(3DNOW)
        CPU_CAP_AVAIL(3DNOWEXT)
        CPU_CAP_AVAIL(SSE)
        CPU_CAP_AVAIL(SSE2)
        CPU_CAP_AVAIL(SSE3)
        CPU_CAP_AVAIL(SSSE3)
        CPU_CAP_AVAIL(SSE4)
        CPU_CAP_AVAIL(SSE42)
        CPU_CAP_AVAIL(AVX)
        CPU_CAP_AVAIL(AVX2)
        CPU_CAP_AVAIL(FMA3)
#undef CPU_CAP_AVAIL

        frameSimd.swallow(&capsToggleAll);
#define CPU_CAP_AVAIL(x) \
    if(cpuCaps & ADM_CPUCAP_##x) \
        frameSimd.swallow(&capsToggle##x);

        CPU_CAP_AVAIL(MMX)
        CPU_CAP_AVAIL(MMXEXT)
        CPU_CAP_AVAIL(3DNOW)
        CPU_CAP_AVAIL(3DNOWEXT)
        CPU_CAP_AVAIL(SSE)
        CPU_CAP_AVAIL(SSE2)
        CPU_CAP_AVAIL(SSE3)
        CPU_CAP_AVAIL(SSSE3)
        CPU_CAP_AVAIL(SSE4)
        CPU_CAP_AVAIL(SSE42)
        CPU_CAP_AVAIL(AVX)
        CPU_CAP_AVAIL(AVX2)
        CPU_CAP_AVAIL(FMA3)
#undef CPU_CAP_AVAIL

        diaElemThreadCount lavcThreadCount(&lavcThreads, QT_TRANSLATE_NOOP("adm","_lavc threads:"));

        diaElemReadOnlyText lavcMultiThreadHwText(NULL,
            QT_TRANSLATE_NOOP("adm","Multi-threading is disabled internally if HW accelerated decoding is enabled, "
                                    "restart application to apply changes"),NULL);

        diaElemFrame frameThread(QT_TRANSLATE_NOOP("adm","Multi-threading"));
        frameThread.swallow(&lavcThreadCount);
        frameThread.swallow(&lavcMultiThreadHwText);


#define NB_ITEMS(x) sizeof(x)/sizeof(diaMenuEntry)        
        diaMenuEntry priorityEntries[] = {
#ifdef _WIN32        
                    {0,       QT_TRANSLATE_NOOP("adm","High"),NULL},
                    {1,      QT_TRANSLATE_NOOP("adm","Above normal"),NULL},
#endif
                    {2,      QT_TRANSLATE_NOOP("adm","Normal"),NULL},
                    {3,      QT_TRANSLATE_NOOP("adm","Below normal"),NULL},
                    {4,      QT_TRANSLATE_NOOP("adm","Low"),NULL}
        };

        diaElemFrame framePriority(QT_TRANSLATE_NOOP("adm","Prioritisation"));
        diaElemMenu menuEncodePriority(&encodePriority, QT_TRANSLATE_NOOP("adm","_Encoding priority:"), NB_ITEMS(priorityEntries), priorityEntries);
        framePriority.swallow(&menuEncodePriority);
        //diaElemMenu menuIndexPriority(&indexPriority, QT_TRANSLATE_NOOP("adm","_Indexing/unpacking priority:"), NB_ITEMS(priorityEntries), priorityEntries);
        //framePriority.swallow(&menuIndexPriority);
#ifdef _WIN32        
        diaElemMenu menuPlaybackPriority(&playbackPriority, QT_TRANSLATE_NOOP("adm","_Playback priority:"), NB_ITEMS(priorityEntries), priorityEntries);
        framePriority.swallow(&menuPlaybackPriority);
#else
        diaElemReadOnlyText encodePriorityNixText(NULL,
            /* advanced users know how to renice, we cannot do this ourselves */
            QT_TRANSLATE_NOOP("adm","Priority can be only lowered and applies to the entire application for the current session"),
            NULL);
        framePriority.swallow(&encodePriorityNixText);
#endif
        
        diaElemToggle useLastReadAsTarget(&lastReadDirAsTarget,QT_TRANSLATE_NOOP("adm","_Default to the directory of the last read file for saving"));
        diaElemToggle firstPassLogFilesAutoDelete(&multiPassStatsAutoDelete,QT_TRANSLATE_NOOP("adm","De_lete first pass log files by default"));

        // Multiload
        diaElemFrame frameMultiLoad(QT_TRANSLATE_NOOP("adm","Auto-Append Settings"));
        diaElemToggle multiLoadUseCustomFragmentSize(&useCustomFragmentSize,QT_TRANSLATE_NOOP("adm","_Use custom fragment size for auto-append of MPEG-TS files"));
        diaElemUInteger multiLoadCustomFragmentSize(&customFragmentSize,QT_TRANSLATE_NOOP("adm","_Fragment size:"),250,8196);
        frameMultiLoad.swallow(&multiLoadUseCustomFragmentSize);
        frameMultiLoad.swallow(&multiLoadCustomFragmentSize);
        multiLoadUseCustomFragmentSize.link(1,&multiLoadCustomFragmentSize);

        // Pictures
        diaElemFrame framePics(QT_TRANSLATE_NOOP("adm","Pictures")); // the purpose of this frame is to fix tab order
        diaElemToggle toggleReversePicsOrder(&loadPicsInReverseOrder, QT_TRANSLATE_NOOP("adm","_Load sequentially named pictures in reverse order"));
        diaMenuEntry standardFpsEntries[] = {
            {0, QT_TRANSLATE_NOOP("adm","23.976 (Film)"), NULL },
            {1, QT_TRANSLATE_NOOP("adm","25 (PAL)"), NULL },
            {2, QT_TRANSLATE_NOOP("adm","29.970 (NTSC)"), NULL },
            {3, QT_TRANSLATE_NOOP("adm","30"), NULL },
            {4, QT_TRANSLATE_NOOP("adm","50 (PAL)"), NULL },
            {5, QT_TRANSLATE_NOOP("adm","59.940 (NTSC)"), NULL },
            {6, QT_TRANSLATE_NOOP("adm","60"), NULL }
        };
        diaElemMenu menuStandardFps(&loadPicsFpsType, QT_TRANSLATE_NOOP("adm","_Assign following standard frame rate:"), NB_ITEMS(standardFpsEntries), standardFpsEntries);
        framePics.swallow(&toggleReversePicsOrder);
        framePics.swallow(&menuStandardFps);

        // Avisynth
        diaElemFrame frameAvisynth(QT_TRANSLATE_NOOP("adm","Avisynth"));
        diaElemToggle togAskAvisynthPort(&askPortAvisynth,QT_TRANSLATE_NOOP("adm","_Always ask which port to use"));
        diaElemUInteger uintDefaultPortAvisynth(&defaultPortAvisynth,QT_TRANSLATE_NOOP("adm","Default port to use"),1024,65535);
        frameAvisynth.swallow(&togAskAvisynthPort);
        frameAvisynth.swallow(&uintDefaultPortAvisynth);

        // Editor cache
        diaElemFrame frameCache(QT_TRANSLATE_NOOP("adm","Caching of decoded pictures"));
        diaElemUInteger cacheSize(&editor_cache_size,QT_TRANSLATE_NOOP("adm","_Cache size:"),8,16);
        diaElemToggle toggleSharedCache(&editor_use_shared_cache,QT_TRANSLATE_NOOP("adm","Use _shared cache"));
        frameCache.swallow(&cacheSize);
        frameCache.swallow(&toggleSharedCache);

        diaMenuEntry videoMode[]={
            {RENDER_GTK, getNativeRendererDesc(0), QT_TRANSLATE_NOOP("adm","This renderer provides no hardware acceleration")}
#ifdef USE_XV
            ,{RENDER_XV, QT_TRANSLATE_NOOP("adm","XVideo"),NULL}
#endif
#ifdef USE_VDPAU
            ,{RENDER_VDPAU, QT_TRANSLATE_NOOP("adm","VDPAU"),NULL}
#endif
#ifdef USE_DXVA2
            ,{RENDER_DXVA2, QT_TRANSLATE_NOOP("adm","DXVA2"),NULL}
#endif
#ifdef USE_OPENGL
            ,{RENDER_QTOPENGL, QT_TRANSLATE_NOOP("adm","OpenGL"),NULL}
#endif
#ifdef USE_LIBVA
            ,{RENDER_LIBVA, QT_TRANSLATE_NOOP("adm","LibVA"),NULL}
#endif
#ifdef USE_SDL
            ,{RENDER_SDL, QT_TRANSLATE_NOOP("adm","SDL"),NULL}
#endif
        };
        diaElemMenu menuVideoMode(&render,QT_TRANSLATE_NOOP("adm","Video _display:"),NB_ITEMS(videoMode),videoMode);
#ifdef USE_SDL
        const std::vector<sdlDriverInfo> &listOfSdl=getListOfSdlDrivers();
        int nbSDL=listOfSdl.size();
        diaElemMenuDynamic *sdlMenu=NULL;
        diaMenuEntryDynamic **sdlMenuEntries=NULL;
        uint32_t sdlMenuIndex=0;
        int current=0;
        if(nbSDL)
        {
            sdlMenuEntries=new diaMenuEntryDynamic*[nbSDL];
            for(int i=0;i<nbSDL;i++)
            {
                if(!currentSdlDriver.compare(listOfSdl[i].driverName))
                {
                    current=i;
                }
                sdlMenuEntries[i]=new diaMenuEntryDynamic(i,listOfSdl[i].driverName.c_str(),NULL);
            }
            sdlMenuIndex=current;
            sdlMenu=new diaElemMenuDynamic(&sdlMenuIndex, QT_TRANSLATE_NOOP("adm","Sdl driver"),nbSDL,  sdlMenuEntries);
        }else
        {
            sdlMenu=new diaElemMenuDynamic(&sdlMenuIndex, QT_TRANSLATE_NOOP("adm","Sdl driver"),0,  NULL);
        }
#endif


        diaMenuEntry msgEntries[]={
                             {0,       QT_TRANSLATE_NOOP("adm","No alerts"),NULL}
                             ,{1,      QT_TRANSLATE_NOOP("adm","Display only error alerts"),NULL}
                             ,{2,      QT_TRANSLATE_NOOP("adm","Display all alerts"),NULL}
        };
        diaElemMenu menuMessage(&msglevel,QT_TRANSLATE_NOOP("adm","_Message level:"),NB_ITEMS(msgEntries),msgEntries);


#if defined(ALSA_SUPPORT) || defined (OSS_SUPPORT)
        diaMenuEntry volumeEntries[]={
                             {0,       QT_TRANSLATE_NOOP("adm","PCM"),NULL}
                             ,{1,      QT_TRANSLATE_NOOP("adm","Master"),NULL}};
        diaElemMenu menuVolume(&useMaster,QT_TRANSLATE_NOOP("adm","_Volume control:"),NB_ITEMS(volumeEntries),volumeEntries);
#endif


         diaMenuEntry mixerEntries[]={
                             {0,       QT_TRANSLATE_NOOP("adm","No downmixing"),NULL}
                             ,{1,       QT_TRANSLATE_NOOP("adm","Stereo"),NULL}
                             ,{2,       QT_TRANSLATE_NOOP("adm","Stereo headphones"),NULL}
                             ,{3,      QT_TRANSLATE_NOOP("adm","Pro Logic"),NULL}
                             ,{4,      QT_TRANSLATE_NOOP("adm","Pro Logic II"),NULL}
                             ,{5,       QT_TRANSLATE_NOOP("adm","Surround headphones"),NULL}
         };
        diaElemMenu menuMixer(&downmix,QT_TRANSLATE_NOOP("adm","_Local playback downmixing:"),NB_ITEMS(mixerEntries),mixerEntries);
//*********** AV_

//***AV
        uint32_t nbAudioDevice=ADM_av_getNbDevices();
        diaMenuEntryDynamic **audioDeviceItems=new diaMenuEntryDynamic *[nbAudioDevice+1];
        audioDeviceItems[0]=new diaMenuEntryDynamic(0,"Dummy",NULL);
        for(int i=0;i<nbAudioDevice;i++)
        {
            std::string name;
            uint32_t major,minor,patch;
            ADM_av_getDeviceInfo(i, name, &major,&minor,&patch);
            audioDeviceItems[i+1]=new diaMenuEntryDynamic(i+1,name.c_str(),NULL);
        }
        diaElemMenuDynamic menuAudio(&newdevice,QT_TRANSLATE_NOOP("adm","_AudioDevice"), nbAudioDevice+1,
                    audioDeviceItems,NULL);

//  -- select language
        typedef struct  { const char *lang;const char *desc;}languageDescriptor;
        uint32_t languageIndex=0;
        languageDescriptor myLanguages[]={
                   {"auto",QT_TRANSLATE_NOOP("adm","System language")},
                {"da","Dansk"},
                {"de","Deutsch"},
                {"en","English"},
                {"es","Español"},
                {"fr","Français"},
                {"it","Italiano"},
                {"hu","Magyar"},
                {"pl","Polski"},
                {"ru","Русский"},
        };
        uint32_t nbLanguages=sizeof(myLanguages)/sizeof(languageDescriptor);
        std::string currentLanguage;
        int currentIndex=0;
        if(!prefs->get(DEFAULT_LANGUAGE,currentLanguage)) currentLanguage=std::string("auto");

        diaMenuEntryDynamic **languagesMenuItems=new diaMenuEntryDynamic *[nbLanguages+1];
        for(int i=0;i<nbLanguages;i++)
        {
            languageDescriptor *lg=myLanguages+i;
            if(!strcmp(lg->lang,currentLanguage.c_str()))
                currentIndex=i;
            languagesMenuItems[i]=new diaMenuEntryDynamic(i,lg->desc,lg->lang);
        }
        languageIndex=currentIndex;
        diaElemMenuDynamic menuLanguage(&languageIndex,QT_TRANSLATE_NOOP("adm","_Language"), nbLanguages,
                    languagesMenuItems,NULL);
//--
#define NB_ELEM(x) sizeof(x)/sizeof(diaElem *)
        /* User Interface */
        diaElem *diaUser[]={&menuMessage, &menuLanguage, &resetEncoder, &enableAltShortcuts, &swapUpDownKeys, &swapMouseWheel, &swapMarkers, &checkForUpdate};
        diaElemTabs tabUser(QT_TRANSLATE_NOOP("adm","User Interface"),NB_ELEM(diaUser),diaUser);

         /* Automation */

        /* Import */
        diaElem *diaImport[]={&frameMultiLoad, &framePics, &frameAvisynth};
        diaElemTabs tabImport(QT_TRANSLATE_NOOP("adm","Import"),NB_ELEM(diaImport),diaImport);

        /* Output */
        diaElem *diaOutput[]={&allowAnyMpeg, &useLastReadAsTarget, &firstPassLogFilesAutoDelete};
        diaElemTabs tabOutput(QT_TRANSLATE_NOOP("adm","Output"),NB_ELEM(diaOutput),diaOutput);

        /* Audio */

#if 0 //defined(ALSA_SUPPORT)
        diaElem *diaAudio[]={&menuMixer,&menuVolume,&menuAudio,&entryAlsaDevice};
        diaElemTabs tabAudio(QT_TRANSLATE_NOOP("adm","Audio"),4,(diaElem **)diaAudio);
//#elif defined(OSS_SUPPORT)
        diaElem *diaAudio[]={&menuMixer,&menuVolume,&menuAudio};
        diaElemTabs tabAudio(QT_TRANSLATE_NOOP("adm","Audio"),3,(diaElem **)diaAudio);
#endif

#if 1
        diaElem *diaAudio[]={&menuMixer,&menuAudio};
        diaElemTabs tabAudio(QT_TRANSLATE_NOOP("adm","Audio"),NB_ELEM(diaAudio),diaAudio);
#endif

        /* Post-Processing */
        diaElemToggle fhzd(&hzd,QT_TRANSLATE_NOOP("adm","_Horizontal deblocking"));
        diaElemToggle fvzd(&vzd,QT_TRANSLATE_NOOP("adm","_Vertical deblocking"));
        diaElemToggle fdring(&dring,QT_TRANSLATE_NOOP("adm","De_ringing"));
        diaElemUInteger postProcStrength(&pp_value,QT_TRANSLATE_NOOP("adm","_Strength:"),0,5);

        diaElemFrame framePP(QT_TRANSLATE_NOOP("adm","Default Postprocessing"));
        framePP.swallow(&fhzd);
        framePP.swallow(&fvzd);
        framePP.swallow(&fdring);
        framePP.swallow(&postProcStrength);

        // HDR
        diaMenuEntry toneMapEntries[] = {
             {0,    QT_TRANSLATE_NOOP("adm","Disabled"),NULL}
            ,{1,    QT_TRANSLATE_NOOP("adm","Fast YUV"),NULL}
            ,{2,    QT_TRANSLATE_NOOP("adm","RGB clipping"),NULL}
            ,{3,    QT_TRANSLATE_NOOP("adm","RGB soft limit"),NULL}
            ,{4,    QT_TRANSLATE_NOOP("adm","RGB Reinhard"),NULL}
            ,{5,    QT_TRANSLATE_NOOP("adm","RGB Hable"),NULL}
            //,{2,    QT_TRANSLATE_NOOP("adm","TODO"),NULL}
        };
        diaElemMenu menuToneMapHDR(&toneMappingHDR,QT_TRANSLATE_NOOP("adm","Default _tone mapping method:"),NB_ITEMS(toneMapEntries),toneMapEntries);

        ELEM_TYPE_FLOAT dTargetLumHDR = targetLumHDR;
        diaElemFloatResettable floatTargetLumHDR(&dTargetLumHDR,QT_TRANSLATE_NOOP("adm","Target peak luminance (nits):"),0.,1000.,DEFAULT_TARGET_LUMINANCE_HDR);
        diaMenuEntry outOfGamutEntries[] = {
             {0,    QT_TRANSLATE_NOOP("adm","Clipping"),NULL}
            ,{1,    QT_TRANSLATE_NOOP("adm","Compression"),NULL}
        };
        diaElemMenu menuOutOfGamutHDR(&outOfGamutHDR,QT_TRANSLATE_NOOP("adm","Default out of gamut handling:"),NB_ITEMS(outOfGamutEntries),outOfGamutEntries);

        diaElemFrame frameHDR(QT_TRANSLATE_NOOP("adm","HDR"));
        frameHDR.swallow(&menuToneMapHDR);
        frameHDR.swallow(&menuOutOfGamutHDR);
        frameHDR.swallow(&floatTargetLumHDR);

        diaElem *diaPostProc[] = { &framePP, &frameHDR };
        diaElemTabs tabPostProc(QT_TRANSLATE_NOOP("adm","Post-Processing"),NB_ELEM(diaPostProc),diaPostProc);

        /* Display */
        diaElemToggle togDisplayRefreshCap(&refreshCapEnabled,QT_TRANSLATE_NOOP("adm","_Limit Refresh Rate"));
        diaElemUInteger displayRefreshCap(&refreshCapValue,QT_TRANSLATE_NOOP("adm","Refresh Rate Cap (ms)"),10,1000);

#ifdef USE_OPENGL
        diaElemToggle useOpenGl(&hasOpenGl,QT_TRANSLATE_NOOP("adm","Enable OpenGL support"));
        diaElemToggle filterPreviewAccelToggle(&filterPreviewAccel,QT_TRANSLATE_NOOP("adm","Enable OpenGL display in filter preview dialogs"));
        diaElemReadOnlyText openGlText(NULL,QT_TRANSLATE_NOOP("adm","OpenGL video display and filters require "
            "OpenGL to be enabled and working at application startup to be available"));

        diaElemFrame frameOpenGL(QT_TRANSLATE_NOOP("adm","OpenGL"));
        frameOpenGL.swallow(&useOpenGl);
        frameOpenGL.swallow(&filterPreviewAccelToggle);
        frameOpenGL.swallow(&openGlText);
#endif
        diaElemFrame frameRC(QT_TRANSLATE_NOOP("adm","GUI Rendering Options")); // a hack to fix tabbing order

        // Packing the following elements into frameRC rectifies otherwise wrong tabbing order:
        // framePP got constructed before the refresh rate spinbox, but after the display refresh
        // toggle resulting in a tabbing order 1-6-7-2-3-4-5-8, counting elements from top to bottom.
        // With this extra frame we get 1-2-3-4-5-6-7-8 (video mode, hor. deblocking, vert. delocking,
        // deringing, deringing strength, OpenGL toggle, refr. rate cap toggle, refr. rate spinbox).
        frameRC.swallow(&togDisplayRefreshCap);
        frameRC.swallow(&displayRefreshCap);

        diaElem *diaVideo[] = {
            &menuVideoMode,
#ifdef USE_OPENGL
            &frameOpenGL,
#endif
#ifdef USE_SDL
            sdlMenu,
#endif
            &frameRC,
            &frameCache
        };
        diaElemTabs tabVideo(QT_TRANSLATE_NOOP("adm","Display"),NB_ELEM(diaVideo),diaVideo);

        /* HW accel */
#ifdef HW_ACCELERATED_DECODING
    #if defined(USE_DXVA2)
        diaElemToggle useDxva2(&bdxva2,QT_TRANSLATE_NOOP("adm","Decode video using DXVA2 (windows)"));
        diaElemToggle dxva2OverrideVersion(&bdxva2_override_version,QT_TRANSLATE_NOOP("adm","Ignore driver blacklist (Intel)"));
        diaElemToggle dxva2OverrideProfile(&bdxva2_override_profile,QT_TRANSLATE_NOOP("adm","Ignore codec blacklist (Intel, HEVC 10bit)"));
    #elif defined(USE_VIDEOTOOLBOX)
        diaElemToggle useVideoToolbox(&bvideotoolbox,QT_TRANSLATE_NOOP("adm","Decode video using VideoToolbox (macOS)"));
    #else
        diaElemToggle useVdpau(&bvdpau,QT_TRANSLATE_NOOP("adm","Decode video using VDPAU (NVIDIA)"));
        diaElemToggle useLibVA(&blibva,QT_TRANSLATE_NOOP("adm","Decode video using LIBVA (INTEL)"));
    #endif
    #ifndef USE_VIDEOTOOLBOX
        diaElemReadOnlyText hwAccelText(NULL,QT_TRANSLATE_NOOP("adm","If you use Hw decoding, it is better to use the matching display driver"),NULL);
    #endif
        diaElemReadOnlyText hwAccelMultiThreadText(NULL,QT_TRANSLATE_NOOP("adm","Enabling Hw decoding disables multi-threading, restart application to apply changes"),NULL);

        diaElem *diaHwDecoding[] = {
    #ifdef USE_DXVA2
            &useDxva2,
            &dxva2OverrideVersion,
            &dxva2OverrideProfile,
    #elif defined(USE_VIDEOTOOLBOX)
            &useVideoToolbox,
    #else
            &useVdpau,
            &useLibVA,
    #endif
            &hwAccelMultiThreadText
    #ifndef USE_VIDEOTOOLBOX
            ,&hwAccelText
    #endif
        };
        diaElemTabs tabHwDecoding(QT_TRANSLATE_NOOP("adm","HW Accel"),NB_ELEM(diaHwDecoding),diaHwDecoding);
#endif

        /* CPU tab */
        diaElem *diaCpu[]={&frameSimd};
        diaElemTabs tabCpu(QT_TRANSLATE_NOOP("adm","CPU"),NB_ELEM(diaCpu),diaCpu);

        /* Threading tab */
        diaElem *diaThreading[]={&frameThread, &framePriority};
        diaElemTabs tabThreading(QT_TRANSLATE_NOOP("adm","Threading"),NB_ELEM(diaThreading),diaThreading);

        diaElemTabs *tabs[] = {
            &tabUser,
            &tabImport,
            &tabOutput,
            &tabAudio,
            &tabVideo,
            &tabPostProc,
#ifdef HW_ACCELERATED_DECODING
            &tabHwDecoding,
#endif
            &tabCpu,
            &tabThreading
        };

#undef NB_ELEM
#define NB_ELEM(x) sizeof(x)/sizeof(diaElemTabs *)
        void *factoryCookiez=diaFactoryRunTabsPrepare(QT_TRANSLATE_NOOP("adm","Preferences"),NB_ELEM(tabs),tabs);
// Now we can disable stuff if needed
#if defined(HW_ACCELERATED_DECODING) && !defined(USE_VIDEOTOOLBOX) && !defined(USE_DXVA2)
    #ifndef USE_VDPAU
        useVdpau.enable(false);
    #endif
    #ifndef USE_LIBVA
        useLibVA.enable(false);
    #endif
#endif

        uint8_t dialogAccepted=0;
        if( diaFactoryRunTabsFinish(factoryCookiez))
        {
#ifdef USE_OPENGL
            prefs->set(FEATURES_ENABLE_OPENGL,hasOpenGl);
            prefs->set(FEATURES_FILTER_PREVIEW_CANVAS_OPENGL,filterPreviewAccel);
#endif
    // cpu caps
            uint32_t cpuMaskOut;
            if(capsAll)
            {
                    cpuMaskOut=ADM_CPUCAP_ALL;
            }else
            {
                    cpuMaskOut=0;
    #undef CPU_CAPS
    #define CPU_CAPS(x)    	if(caps##x) cpuMaskOut|= ADM_CPUCAP_##x;
                    CPU_CAPS(MMX);
                    CPU_CAPS(MMXEXT);
                    CPU_CAPS(3DNOW);
                    CPU_CAPS(3DNOWEXT);
                    CPU_CAPS(SSE);
                    CPU_CAPS(SSE2);
                    CPU_CAPS(SSE3);
                    CPU_CAPS(SSSE3);
                    CPU_CAPS(SSE4);
                    CPU_CAPS(SSE42);
                    CPU_CAPS(AVX);
                    CPU_CAPS(AVX2);
                    CPU_CAPS(FMA3);
            }
            prefs->set(FEATURES_CPU_CAPS,cpuMaskOut);
            CpuCaps::setMask(cpuMaskOut);
            //
            prefs->set(FEATURES_CAP_REFRESH_ENABLED,refreshCapEnabled);
            prefs->set(FEATURES_CAP_REFRESH_VALUE,refreshCapValue);

            // Postproc
            #undef DOME
            #define DOME(x,y) if(y) pp_type |=x;
            pp_type=0;
            DOME(ADM_POSTPROC_HORIZ_DEBLOCK,hzd)
            DOME(ADM_POSTPROC_VERT_DEBLOCK,vzd)
            DOME(ADM_POSTPROC_DERING,dring)
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
            //

            prefs->set(UPDATE_ENABLED,doAutoUpdate);
            // Video render
            prefs->set(VIDEODEVICE,render);
            // Auto-append
            prefs->set(DEFAULT_MULTILOAD_USE_CUSTOM_SIZE, useCustomFragmentSize);
            prefs->set(DEFAULT_MULTILOAD_CUSTOM_SIZE_M, customFragmentSize);
            // Auto-load pictures in reverse order
            prefs->set(PICTURES_LOAD_IN_REVERSE_ORDER, loadPicsInReverseOrder);
            // Standard FPS to use when auto-loading pictures
            prefs->set(PICTURES_FPS_TYPE, loadPicsFpsType);
            // Video cache
            prefs->set(FEATURES_CACHE_SIZE, editor_cache_size);
            prefs->set(FEATURES_SHARED_CACHE, editor_use_shared_cache);
            // Encoding priority
            prefs->set(PRIORITY_ENCODING, encodePriority);
            // Indexing / unpacking priority
            //prefs->set(PRIORITY_INDEXING, indexPriority);
            // Playback priority
            prefs->set(PRIORITY_PLAYBACK, playbackPriority);

            // Auto swap A/B vs reset the other marker
            prefs->set(FEATURES_SWAP_IF_A_GREATER_THAN_B, useSwap);
            //
            prefs->set(MESSAGE_LEVEL,msglevel);
            // Discard changes to output config on video load
            prefs->set(RESET_ENCODER_ON_VIDEO_LOAD, loadDefault);
#ifdef USE_VDPAU
            // VDPAU
            prefs->set(FEATURES_VDPAU,bvdpau);
            if(bvdpau) lavcThreads=1; // disable multi-threaded decoding
#endif
#ifdef USE_DXVA2
            // DXVA2
            prefs->set(FEATURES_DXVA2,bdxva2);
            prefs->set(FEATURES_DXVA2_OVERRIDE_BLACKLIST_VERSION,bdxva2_override_version);
            prefs->set(FEATURES_DXVA2_OVERRIDE_BLACKLIST_PROFILE,bdxva2_override_profile);
            if(bdxva2) lavcThreads=1;
#endif
#ifdef USE_LIBVA
            // LIBVA
            prefs->set(FEATURES_LIBVA,blibva);
            if(blibva) lavcThreads=1;
#endif
#ifdef USE_VIDEOTOOLBOX
            // VideoToolbox
            prefs->set(FEATURES_VIDEOTOOLBOX,bvideotoolbox);
            if(bvideotoolbox) lavcThreads=1;
#endif
            // number of threads
            prefs->set(FEATURES_THREADING_LAVC, lavcThreads);
            // Make users happy who prefer the output dir to be the same as the input dir
            prefs->set(FEATURES_USE_LAST_READ_DIR_AS_TARGET,lastReadDirAsTarget);
            // Store the default state of the encoding dialog checkbox to auto-delete first pass stats
            prefs->set(DEFAULT_DELETE_FIRST_PASS_LOG_FILES,multiPassStatsAutoDelete);
            // Enable alternate keyboard shortcuts
            prefs->set(KEYBOARD_SHORTCUTS_USE_ALTERNATE_KBD_SHORTCUTS,altKeyboardShortcuts);
            // Allow to use the UP key to navigate back, DOWN to navigate forward
            prefs->set(KEYBOARD_SHORTCUTS_SWAP_UP_DOWN_KEYS,swapUpDown);
            // Optional reversed mouse wheel for navigation
            prefs->set(FEATURES_SWAP_MOUSE_WHEEL,swapWheel);

            prefs->set(DEFAULT_LANGUAGE,std::string(myLanguages[languageIndex].lang));

            // Avisynth
            prefs->set(AVISYNTH_AVISYNTH_DEFAULTPORT,defaultPortAvisynth);
            prefs->set(AVISYNTH_AVISYNTH_ALWAYS_ASK, askPortAvisynth);

            // HDR
            prefs->set(HDR_TONEMAPPING, toneMappingHDR);
            targetLumHDR = dTargetLumHDR;
            prefs->set(HDR_TARGET_LUMINANCE, targetLumHDR);
            prefs->set(HDR_OUT_OF_GAMUT_HANDLING, outOfGamutHDR);

                // Initialise SDL again as driver may have changed
#ifdef USE_SDL
            std::string driverName=listOfSdl[sdlMenuIndex].driverName;
            setSdlDriverByName(driverName);
            prefs->set(FEATURES_SDLDRIVER,driverName.c_str());
#endif
            dialogAccepted=1;
        }
#ifdef USE_SDL
        if(sdlMenu)
        {
            if(nbSDL&&sdlMenuEntries)
            {
                for(int i=0;i<nbSDL;i++)
                {
                    delete sdlMenuEntries[i];
                }
                delete [] sdlMenuEntries;
                sdlMenuEntries=NULL;
            }
            delete sdlMenu;
            sdlMenu=NULL;
        }
#endif
        for(int i=0;i<nbAudioDevice+1;i++)
        {

            delete audioDeviceItems[i];
        }
        delete [] audioDeviceItems;

        for(int i=0;i<nbLanguages;i++)
        {
            delete languagesMenuItems[i];
        }
        delete [] languagesMenuItems;


        return dialogAccepted;
}
//EOF
