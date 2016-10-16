/***************************************************************************
 * \file    main.cpp
 * \brief   Initialize the env.
 * \author  mean/fixounet@free.fr (C) 2002/2016 by mean
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
#include "config.h"
#include "ADM_default.h"
#include "ADM_threads.h"
#include "DIA_uiTypes.h"
#include "ADM_preview.h"
#include "ADM_win32.h"
#include "ADM_crashdump.h"
#include "ADM_memsupport.h"
#include "ADM_script2/include/ADM_script.h"
#include "ADM_ffmp43.h"
#include "ADM_coreVideoFilterFunc.h"
#include "ADM_coreDemuxer.h"
#include "ADM_muxerProto.h"

#define __DECLARE__
#include "avi_vars.h"

#include "prefs.h"
#include "audio_out.h"
#include "ADM_assert.h"
#include "adm_main.h"

void abortExitHandler(void);

#ifdef main
extern "C"
{
int main(int _argc, char *_argv[]);
}
#endif // main

int main(int _argc, char *_argv[])
{
	ADM_initBaseDir(_argc, _argv);

#if defined(_WIN32) && (ADM_UI_TYPE_BUILD == ADM_UI_GTK || ADM_UI_TYPE_BUILD == ADM_UI_QT4)
	// redirect output before registering exception handler so error dumps are captured
	redirectStdoutToFile();
#endif

	installSigHandler();

	char **argv;
	int argc;

#ifdef _WIN32
	getUtf8CommandLine(&argc, &argv);
#else
	argv = _argv;
	argc = _argc;
#endif

#if !defined(NDEBUG) && defined(FIND_LEAKS)
	new_progname = argv[0];
#endif


	int exitVal = startAvidemux(argc, argv);

#ifdef _WIN32
	freeUtf8CommandLine(argc, argv);
#endif

	uninstallSigHandler();

	return exitVal;
}
/**
 * \fn getUISpecifSubfolder
 * @return
 */
static const char *getUISpecifSubfolder()
{
    switch(UI_GetCurrentUI())
    {
        case ADM_UI_QT4:
#if ADM_UI_TYPE_BUILD == ADM_UI_QT4
            return QT_FLAVOR;
#else
            return "qt4";
#endif
            break;
        case ADM_UI_CLI:
            return "cli";
            break;
        case ADM_UI_GTK:
            return "gtk";
            break;
        default:
            break;
    }
    return "unknown";
}

#ifdef USE_SDL
static bool sdlProbe(void)
{
    std::string drv;
    printf("Probing for SDL...\n");
    std::string sdlDriver=std::string("dummy");
    if(prefs->get(FEATURES_SDLDRIVER,drv))
    {
        if(drv.size())
        {
                sdlDriver=drv;
        }
    }
    printf("Calling initSDL with driver=%s\n",sdlDriver.c_str());
    initSdl(sdlDriver);
    return true;
}
/**
 *
 * @param argc
 * @param argv
 * @return
 */
static bool fakeInitSdl()
{
    return true;
}
#endif
#if 0
void run(const char *foo)
{
  std::string r,ext;
  ADM_PathSplit(std::string(foo),r,ext);
  printf("%s => %s  + <%s>\n",foo,r.c_str(),ext.c_str());
}
void testCase(void)
{
  run("/");
  run("/foobar");
  run("/foobar/meuh");
  run("/foobar/meuh.ext");
  run("/foobar/meuh.");
  run("/foobar/meuh.ext.bar");

}
#endif
/**
 *
 */
static bool admDummyHwCleanup()
{
    return true;
}

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int startAvidemux(int argc, char *argv[])
{

    printf("*************************\n");
    printf("  Avidemux v%s", ADM_VERSION);

#if defined( ADM_SUBVERSION )
#define MKSTRING(x) x
         printf(" (%s) .", MKSTRING(ADM_SUBVERSION));
#endif

    printf("\n*************************\n");
    printf(" http://www.avidemux.org\n");
    printf(" Code      : Mean, JSC, Grant Pedersen\n");
    printf(" GFX       : Nestor Di, nestordi@augcyl.org\n");
    printf(" Design    : Jakub Misak\n");
    printf(" FreeBSD   : Anish Mistry, amistry@am-productions.biz\n");
    printf(" Audio     : Mihail Zenkov\n");
    printf(" Mac OS X  : Kuisathaverat, Harry van der Wolf\n");
    printf(" Win32     : Grant Pedersen\n\n");

#ifdef __GNUC__
	printf("Compiler: GCC %s\n", __VERSION__);
#endif

	printf("Build Target: ");

#if defined(_WIN32)
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
#endif

	char uiDesc[15];
	getUIDescription(uiDesc);
	printf("\nUser Interface: %s\n", uiDesc);

#ifdef _WIN32
	char version[250];

	if (getWindowsVersion(version))
		printf("Operating System: %s\n", version);
#endif

#if defined(__USE_LARGEFILE) && defined(__USE_LARGEFILE64)
	printf("\nLarge file available: %d offset\n", __USE_FILE_OFFSET64);
#endif

    printf("Time: %s\n", ADM_epochToString(ADM_getSecondsSinceEpoch()));

    for(int i = 0; i < argc; i++)
    {
            printf("%d: %s\n", i, argv[i]);
    }

#ifndef __APPLE__
    ADM_InitMemcpy();
#endif
    printf("\nInitialising prefs\n");
    initPrefs();
    if(false==prefs->load()) // no prefs, set some sane default
    {
        setPrefsDefault();
    }
    uint32_t cpuMask;
     if(!prefs->get(FEATURES_CPU_CAPS,&cpuMask))
    {
       cpuMask=0xffffffff;
    }

    CpuCaps::init();
    CpuCaps::setMask(cpuMask);


#ifdef _WIN32
    win32_netInit();
#endif

    video_body = new ADM_Composer;

    UI_Init(argc, argv);
    AUDMEncoder_initDither();

    // Hook our UI...
    InitFactory();
    InitCoreToolkit();
    initFileSelector();

    // Load .avidemuxrc
    quotaInit();

    ADM_lavFormatInit();


    {
        std::string scriptFolder=ADM_getPluginDir("scriptEngines");
        if(!initGUI(initialiseScriptEngines(scriptFolder.c_str(), video_body,getUISpecifSubfolder())))
        {
                printf("\n Fatal : could not init GUI\n");
                exit(-1);
        }
    }




#if (ADM_UI_TYPE_BUILD!=ADM_UI_CLI)
#if defined( USE_VDPAU)
    extern bool admVdpau_exitCleanup();
    PROBE_HW_ACCEL(vdpauProbe,VDPAU,initVDPAUDecoder, admVdpau_exitCleanup)
#endif

#if defined( USE_LIBVA)
    extern bool admLibVa_exitCleanup();
    PROBE_HW_ACCEL(libvaProbe,LIBVA,initLIBVADecoder,admLibVa_exitCleanup)
#endif

#endif // !CLI

#ifdef USE_SDL
    PROBE_HW_ACCEL(sdlProbe,SDL,fakeInitSdl,admDummyHwCleanup)
#endif
    //


   ADM_lavInit();

    loadPlugins( "audioDecoder",   ADM_ad_loadPlugins);
    loadPlugins( "audioDevices",   ADM_av_loadPlugins);
    loadPlugins( "audioEncoders",  ADM_ae_loadPlugins);
    loadPlugins( "demuxers",       ADM_dm_loadPlugins);
    loadPlugins( "muxers",         ADM_mx_loadPlugins);
    loadPlugins( "videoDecoders",  ADM_vd6_loadPlugins);

    loadPluginsEx( "videoEncoders",  ADM_ve6_loadPlugins);
    loadPluginsEx( "videoFilters",   ADM_vf_loadPlugins);


    AVDM_audioInit();

    int n=listOfHwInit.size();
    for(int i=0;i<n;i++)
    {
        listOfHwInit[i]();
    }
    atexit(abortExitHandler);
    UI_RunApp();
    cleanUp();

    printf("Normal exit\n");
    return 0;
}
void abortExitHandler(void)
{
    static bool done=false;
    int n=listOfHwCleanup.size();
    if(!done && n)
    {
        done=true;
        ADM_info("Abnormal exit handler, trying to clean up \n");
        for(int i=0;i<n;i++)
        {
            listOfHwCleanup[i]();
        }
        listOfHwCleanup.clear();
    }else
    {
        ADM_info("already done, nothing to do\n");
    }
}
/**
 *
 */
void ADM_ExitCleanup( void )
{
    printf("Cleaning up\n");
    admPreview::destroy();
    if(video_body)
        video_body->cleanup ();
    delete video_body;
    video_body=NULL;
    // wait for thread to finish executing
    ADM_setCrashHook(NULL,NULL);
    destroyScriptEngines();
//    filterCleanUp();
    ADM_lavDestroy();

#ifdef USE_SDL
	quitSdl();
#endif




    AVDM_cleanup();


    destroyGUI();
    destroyPrefs();

    UI_End();
    ADM_vf_clearFilters();

		int n=listOfHwCleanup.size();
		for(int i=0;i<n;i++)
		{
				listOfHwCleanup[i]();
		}
		listOfHwCleanup.clear();


    ADM_ad_cleanup();
    ADM_ae_cleanup();
    ADM_mx_cleanup();
    ADM_vf_cleanup();
    ADM_dm_cleanup();
    ADM_vd6_cleanup();
    ADM_ve6_cleanup();


    printf("--End of cleanup--\n");
    ADMImage_stat();

    ADM_info("\nGoodbye...\n\n");
}
/**
    \fn setPrefsDefault
*/
bool setPrefsDefault(void)
{
#ifdef __MINGW32__
        prefs->set(AUDIO_DEVICE_AUDIODEVICE,std::string("Win32"));
    #ifdef USE_OPENGL
        prefs->set(VIDEODEVICE,(uint32_t)5); // QTGL
    #endif
#endif
#ifdef __linux__
            prefs->set(AUDIO_DEVICE_AUDIODEVICE,std::string("PulseAudioS"));
    #ifdef USE_VDPAU
            prefs->set(VIDEODEVICE,(uint32_t)4); // VDPAU
    #else
            prefs->set(VIDEODEVICE,(uint32_t)1); // XV
    #endif
#endif
    return true;
}
//EOF
