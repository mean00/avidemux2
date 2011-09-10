/***************************************************************************
                          main.cpp  -  description
                             -------------------
	Initialize the env.

    begin                : Sat Feb 2 2002
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
#include "ADM_cpp.h"

#ifdef __MINGW32__
#define UNICODE
#include <winbase.h>
#include <windows.h>
#include <excpt.h>
#endif

#include "config.h"
#include "ADM_default.h"
#include "ADM_threads.h"
#include "DIA_uiTypes.h"
#include "ADM_preview.h"
#define __DECLARE__
#include "avi_vars.h"

#include "prefs.h"
#include "audio_out.h"
#include "ADM_assert.h"


static bool setPrefsDefault(void);
extern void  ADM_lavInit();
extern void  ADM_lavDestroy();
extern void  ADM_lavFormatInit(void);
extern bool  vdpauProbe(void);
extern "C" {
     extern uint8_t     ADM_InitMemcpy(void);
};

#ifdef USE_SDL
extern "C" {
	#ifdef __HAIKU__
	#include "SDL/SDL.h"
	#else
	#include "SDL.h"
	#endif
}

#include "ADM_render/GUI_sdlRender.h"
#endif

#include "ADM_script2/include/ADM_scriptIf.h"
void onexit( void );
//extern void automation(int argc, char **argv);

extern void registerVideoFilters( void );
extern void filterCleanUp( void );
extern void register_Encoders( void )  ;

extern uint8_t initGUI( void );
extern void destroyGUI(void);
extern uint8_t initFileSelector(void);
extern void AUDMEncoder_initDither(void);
extern void ADM_memStat( void );
extern void ADM_memStatInit( void );
extern void ADM_memStatEnd( void );
extern void getUIDescription(char*);
extern uint8_t ADM_ad_loadPlugins(const char *path);
extern uint8_t ADM_vf_loadPlugins(const char *path);
extern uint8_t ADM_vd6_loadPlugins(const char *path);
extern uint8_t ADM_av_loadPlugins(const char *path);
extern uint8_t ADM_ae_loadPlugins(const char *path);
extern uint8_t ADM_dm_loadPlugins(const char *path);
extern uint8_t ADM_mx_loadPlugins(const char *path);
extern uint8_t ADM_ve6_loadPlugins(const char *path);

extern bool ADM_ad_cleanup(void);
extern bool ADM_ae_cleanup(void);
extern bool ADM_mx_cleanup(void);
extern bool ADM_vf_cleanup(void);
extern bool ADM_dm_cleanup(void);
extern void ADM_ve6_cleanup(void);
extern bool ADM_vf_clearFilters(void);

extern bool vdpauProbe(void);
extern bool vdpauCleanup(void);

extern void loadPlugins(void);
extern void InitFactory(void);
extern void InitCoreToolkit(void);
#ifdef __MINGW32__
extern EXCEPTION_DISPOSITION exceptionHandler(struct _EXCEPTION_RECORD* pExceptionRec, void* pEstablisherFrame, struct _CONTEXT* pContextRecord, void* pDispatcherContext);
extern int wideCharStringToUtf8(const wchar_t *wideCharString, int wideCharStringLength, char *utf8String);
#else
extern void installSigHandler(void);
#endif

#ifdef __WIN32
extern bool getWindowsVersion(char* version);
extern void redirectStdoutToFile(void);
#endif

extern uint8_t  quotaInit(void);
extern void ADMImage_stat( void );
extern uint8_t win32_netInit(void);

extern int UI_Init(int nargc,char **nargv);
extern int UI_RunApp(void);
extern bool UI_End(void);
extern void renderDestroy(void);
extern bool ADM_jobInit(void);
extern bool ADM_jobShutDown(void);
// Spidermonkey/Scripting stuff  
#if defined(ADM_DEBUG) && defined(FIND_LEAKS)
extern const char* new_progname;
extern int check_leaks();
#endif

int main(int argc, char *argv[])
{
#if defined(__WIN32) && defined(USE_SDL)
//	redirectStdoutToFile();
#endif

#if defined(ADM_DEBUG) && defined(FIND_LEAKS)
	new_progname = argv[0];
#endif

#ifndef __MINGW32__
	// thx smurf uk :)
    installSigHandler();
#endif

    printf("*************************\n");
    printf("  Avidemux v" VERSION);
  	if(ADM_SUBVERSION)
	{
		printf(" (r%04u)", ADM_SUBVERSION);
	}
    printf("\n*************************\n");
    printf(" http://www.avidemux.org\n");
    printf(" Code      : Mean, JSC, Gruntster \n");
    printf(" GFX       : Nestor Di , nestordi@augcyl.org\n");
    printf(" Design    : Jakub Misak\n");
    printf(" FreeBSD   : Anish Mistry, amistry@am-productions.biz\n");
    printf(" Audio     : Mihail Zenkov\n");
    printf(" MacOsX    : Kuisathaverat\n");
    printf(" Win32     : Gruntster\n\n");

#ifdef __GNUC__
	printf("Compiler: GCC %s\n", __VERSION__);
#endif

	printf("Build Target: ");

#if defined(__WIN32)
	printf("Microsoft Windows");
#elif defined(__APPLE__)
	printf("Apple");
#else
	printf("Linux");
#endif

#if defined(ADM_CPU_X86_32)
	printf(" (x86)");
#elif defined(ADM_CPU_X86_64)
	printf(" (x86-64)");
#elif defined(ADM_CPU_PPC)
	printf(" (PowerPC)");
#endif

	printf("\n");

	char uiDesc[15];
	getUIDescription(uiDesc);
	printf("User Interface: %s\n", uiDesc);

#ifdef __WIN32
	char version[250];

	if (getWindowsVersion(version))
		printf("Operating System: %s\n", version);
#endif

#if defined(__USE_LARGEFILE) && defined(__USE_LARGEFILE64)
	printf("\nLarge file available: %d offset\n", __USE_FILE_OFFSET64);
#endif
        // getTime
        printf("Current time :%s\n",ADM_epochToString(ADM_getSecondsSinceEpoch()));
	// Start counting memory
	ADM_memStatInit();
    ADM_InitMemcpy();
	printf("\nInitialising prefs\n");
	initPrefs();
	if(false==prefs->load()) // no prefs, set some sane default
    {
        setPrefsDefault();
    }
    CpuCaps::init();

#ifdef USE_SDL
	uint32_t videoDevice = RENDER_LAST;

	prefs->get(VIDEODEVICE, &videoDevice);

	initSdl(videoDevice);
#endif

	atexit(onexit);

#ifdef __MINGW32__
    win32_netInit();
#endif

    // unicode command line...
    char    **Aargv=NULL;
    int     Aargc=0;
#if _WIN32
     wchar_t **wargv = CommandLineToArgvW (GetCommandLineW (), &Aargc);
     if (wargv && Aargc)
     {
        Aargv=new char *[Aargc];
        for(int i=0;i<Aargc;i++)
        {
            char *utf8;
            wchar_t   *w=wargv[i];
            int     lin=wcslen(w);
            int     lout= wideCharStringToUtf8(w, lin,NULL);
            utf8=new char[lout+1];
            memset(utf8,0,lout+1);
            wideCharStringToUtf8(w,lin,utf8);
            Aargv[i]=utf8;
        }
        for(int i=0;i<Aargc;i++)
            printf("%d : %s\n",i,Aargv[i]);
        
     }
#else
    Aargv=argv;
    Aargc=argc;

#endif


    UI_Init(Aargc,Aargv);
    AUDMEncoder_initDither();

    // Hook our UI...
    InitFactory();
    InitCoreToolkit();
    initFileSelector();


	// Load .avidemuxrc
    quotaInit();

    video_body = new ADM_Composer;

    ADM_lavFormatInit();
	//***************Plugins *********************
	// Load system wide audio decoder plugin
#ifdef __APPLE__
    const char *startDir="../Resources/lib";
#else
    const char *startDir=ADM_RELATIVE_LIB_DIR;
#endif
    char *adPlugins = ADM_getInstallRelativePath(startDir,"ADM_plugins6","audioDecoder");
    char *avPlugins = ADM_getInstallRelativePath(startDir,"ADM_plugins6","audioDevices");    
    char *aePlugins = ADM_getInstallRelativePath(startDir,"ADM_plugins6","audioEncoders");    
    char *dmPlugins = ADM_getInstallRelativePath(startDir,"ADM_plugins6","demuxers");    
    char *mxPlugins = ADM_getInstallRelativePath(startDir,"ADM_plugins6","muxers");    
    char *vePlugins = ADM_getInstallRelativePath(startDir,"ADM_plugins6","videoEncoders");    
    char *vdPlugins = ADM_getInstallRelativePath(startDir,"ADM_plugins6","videoDecoders");    
    char *vfPlugins = ADM_getInstallRelativePath(startDir,"ADM_plugins6","videoFilters");

    ADM_mx_loadPlugins(mxPlugins);
    delete [] mxPlugins;

	ADM_ad_loadPlugins(adPlugins);
	delete [] adPlugins;

    ADM_av_loadPlugins(avPlugins);
    delete [] avPlugins;

    ADM_ae_loadPlugins(aePlugins);
    delete [] aePlugins;

	ADM_dm_loadPlugins(dmPlugins);
    delete [] dmPlugins;

    ADM_ve6_loadPlugins(vePlugins);
    delete [] vePlugins;

    ADM_vf_loadPlugins(vfPlugins);
    delete [] vfPlugins;

    ADM_vd6_loadPlugins(vdPlugins);
    delete [] vdPlugins;


    // load local audio decoder plugins
	adPlugins=ADM_getHomeRelativePath("plugins6","audioDecoder");
	ADM_ad_loadPlugins(adPlugins);
	delete [] adPlugins;

	// load local video filter plugins
	vfPlugins=ADM_getHomeRelativePath("plugins6","videoFilter");
	ADM_vf_loadPlugins(vfPlugins);
	delete [] vfPlugins;
	

	//***************Plugins *********************

	if(!initGUI())
	{
		printf("\n Fatal : could not init GUI\n");
		exit(-1);
	}

    ADM_lavInit();
    AVDM_audioInit();

    if(SpidermonkeyInit() == true)
        printf("Spidermonkey initialized.\n");
    else
		ADM_assert(0); 

#ifdef __MINGW32__
	__try1(exceptionHandler);
#endif

#if defined( USE_VDPAU) 
  #if (ADM_UI_TYPE_BUILD!=ADM_UI_CLI)
    printf("Probing for VDPAU...\n");
    if(vdpauProbe()==true) printf("VDPAU available\n");
        else printf("VDPAU not available\n");
  #else
    printf("Cannot use VDPAU in cli mode %d,%d\n",ADM_UI_TYPE_BUILD,ADM_UI_CLI);
  #endif
#endif

    // Init jobs
    ADM_jobInit();

    UI_RunApp();

#ifdef __MINGW32__
	__except1(exceptionHandler);
#endif

    printf("Normal exit\n");
    return 0;
}
extern uint8_t GUI_close(void);
void onexit( void )
{
	printf("Cleaning up\n");
    if(video_body) video_body->cleanup ();  
    delete video_body;	
    // wait for thread to finish executing
    SpidermonkeyExit();
//    filterCleanUp();
	ADM_lavDestroy();

#ifdef USE_SDL
	quitSdl();
#endif

#ifdef HAVE_AUDIO
	AVDM_cleanup();
#endif

	destroyGUI();
    destroyPrefs();
    
    admPreview::destroy();
    UI_End();
    //renderDestroy();

    ADM_ad_cleanup();
    ADM_ae_cleanup();
    ADM_mx_cleanup();
    ADM_vf_clearFilters();
    ADM_vf_cleanup();
    ADM_dm_cleanup();
    ADM_ve6_cleanup();
#if defined( USE_VDPAU) 
  #if (ADM_UI_TYPE_BUILD!=ADM_UI_CLI)
    printf("cleaning VDPAU...\n");
    vdpauCleanup();
  #else
    printf("Cannot use VDPAU in cli mode %d,%d\n",ADM_UI_TYPE_BUILD,ADM_UI_CLI);
  #endif
#endif

    printf("--End of cleanup--\n");
    ADMImage_stat();
    ADM_memStat();
    ADM_memStatEnd();
    
    ADM_info("\nGoodbye...\n\n");

#if defined(ADM_DEBUG) && defined(FIND_LEAKS)
	check_leaks();
#endif
}
/**
    \fn setPrefsDefault
*/
bool setPrefsDefault(void)
{
#ifdef __MINGW32__
        prefs->set(AUDIO_DEVICE_AUDIODEVICE,"Win32");        
    #ifdef USE_OPENGL
        prefs->set(VIDEODEVICE,(uint32_t)5); // QTGL
    #endif
#endif        
#ifdef __linux__
            prefs->set(AUDIO_DEVICE_AUDIODEVICE,"PulseAudioS");
    #ifdef USE_VDPAU
            prefs->set(VIDEODEVICE,(uint32_t)4); // VDPAU
    #else
            prefs->set(VIDEODEVICE,(uint32_t)1); // XV
    #endif
#endif
}
extern void checkCrashFile(void);
void dummyXref(void)
{
    checkCrashFile();
}
//EOF
