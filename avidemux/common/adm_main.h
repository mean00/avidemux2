
#ifdef USE_SDL
extern "C" {
	#ifdef __HAIKU__
	#include "SDL2/SDL.h"
	#else
	#include "SDL2/SDL.h"
	#endif
}

#include "ADM_render/GUI_sdlRender.h"
#endif
extern uint8_t GUI_close(void);
static bool setPrefsDefault(void);
extern bool  vdpauProbe(void);
extern bool  libvaProbe(void);

extern void registerVideoFilters( void );
extern void filterCleanUp( void );
extern void register_Encoders( void )  ;

extern uint8_t initGUI(const vector<IScriptEngine*>& engines);
extern void destroyGUI(void);
extern void initFileSelector(void);
extern void getUIDescription(char*);
extern uint8_t ADM_ad_loadPlugins(const char *path);
extern uint8_t ADM_vf_loadPlugins(const char *path,const char *subFolder);
extern uint8_t ADM_vd6_loadPlugins(const char *path);
extern uint8_t ADM_av_loadPlugins(const char *path);
extern uint8_t ADM_ae_loadPlugins(const char *path);
extern uint8_t ADM_ve6_loadPlugins(const char *path,const char *subFolder);

extern bool ADM_ad_cleanup(void);
extern bool ADM_ae_cleanup(void);
extern bool ADM_vf_cleanup(void);
extern void ADM_ve6_cleanup(void);

extern bool vdpauProbe(void);
extern bool vdpauCleanup(void);

extern bool xvbaProbe(void);
extern bool xvbaCleanup(void);

extern void loadPlugins(void);
extern void InitFactory(void);
extern void InitCoreToolkit(void);
extern uint8_t  quotaInit(void);

extern int UI_Init(int nargc,char **nargv);
extern int UI_RunApp(void);
extern bool UI_End(void);
extern void cleanUp (void);

extern ADM_UI_TYPE UI_GetCurrentUI(void);

typedef bool (*initFunc_t)    (void);
std::vector<initFunc_t> listOfHwInit;    

#if !defined(NDEBUG) && defined(FIND_LEAKS)
extern const char* new_progname;
#endif

void ADM_ExitCleanup(void);
int startAvidemux(int argc, char *argv[]);
bool isPortableMode(int argc, char *argv[]);


#ifdef USE_VDPAU
extern bool initVDPAUDecoder(void);
#endif
#ifdef USE_LIBVA
extern bool initLIBVADecoder(void);
#endif

#define PROBE_HW_ACCEL(probe,name,initFunc) {   \
    printf("Probing for "#name"...\n"); \
    if(probe()==true)\
    {\
        printf(#name" available\n"); \
        listOfHwInit.push_back(initFunc);\
    }\
      else \
        printf(#name" not available\n");}

#ifdef __APPLE__
    const char *startDir="../lib";
#else
    const char *startDir=ADM_RELATIVE_LIB_DIR;
#endif

       
#define loadPlugins(subdir, func)\
{\
     char *p = ADM_getInstallRelativePath(startDir, ADM_PLUGIN_DIR,subdir );\
     func(p);\
     delete [] p;p=NULL;\
}            

#define loadPluginsEx(subdir, func)\
{\
     char *p = ADM_getInstallRelativePath(startDir, ADM_PLUGIN_DIR,subdir );\
     func(p,getUISpecifSubfolder());\
     delete [] p;p=NULL;\
}            

#define loadPluginsMyEx(subdir, func)\
{\
     char *p = ADM_getHomeRelativePath("plugins6",subdir); \
     func(p,getUISpecifSubfolder());\
     delete [] p;p=NULL;\
}  