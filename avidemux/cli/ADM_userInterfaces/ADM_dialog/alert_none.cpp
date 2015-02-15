#include <ctype.h>

#include "config.h"

#include "ADM_default.h"
#include "prefs.h"
#include "DIA_coreToolkit.h"
#include "DIA_coreUI_internal.h"

#define BOX_SIZE 80
static int beQuiet=0;
static void boxStart(void)
{
  for(int i=0;i<BOX_SIZE;i++) fprintf(stderr,"*");
  fprintf(stderr,"\n"); 
}
static void boxEnd(void)
{
  for(int i=0;i<BOX_SIZE;i++) fprintf(stderr,"*");
  fprintf(stderr,"\n"); 
}
static void boxAdd(const char *str)
{
  int l=strlen(str);
  fprintf(stderr, "* %s",str);
  if(l+4<BOX_SIZE)
  {
    l=BOX_SIZE-l-4;
      for(int i=0;i<BOX_SIZE;i++) fprintf(stderr,"");
  }
  fprintf(stderr," *\n"); 
}


namespace ADM_CliCoreUIToolkit
{
extern DIA_workingBase    *createWorking(const char *title);
extern DIA_encodingBase   *createEncoding(uint64_t duration,bool tray);
extern DIA_audioTrackBase *createAudioTrack( PoolOfAudioTracks *pool, ActiveAudioTracks *active );
void            GUI_Info_HIG(const ADM_LOG_LEVEL level,const char *primary, const char *secondary_format)
{
  uint32_t msglvl=2;

        prefs->get(MESSAGE_LEVEL,&msglvl);

        boxStart();
        boxAdd("Info");
        boxAdd(primary);   
        
        if(! secondary_format)
        {
                boxEnd();
                return;
        }

        boxAdd(secondary_format);
        boxEnd();
        
}

void            GUI_Error_HIG(const char *primary, const char *secondary_format)
{
        boxStart();
        boxAdd("Error");
        boxAdd(primary);   
        if(!secondary_format)
        {
                boxEnd();
                return;
        }else
        {
          boxAdd(secondary_format);
          boxEnd();
        }
}
int             GUI_Confirmation_HIG(const char *button_confirm, const char *primary, const char *secondary_format)
{
   uint32_t msglvl=2;

        prefs->get(MESSAGE_LEVEL,&msglvl);

        boxStart();
        boxAdd("Question");

        boxAdd(button_confirm);
        if(! secondary_format)
        {
                
                boxEnd();
        }
        else
        {
            boxAdd(secondary_format);
            boxEnd();
        }
        if (beQuiet)
        {
          boxAdd("--> First one\n");
          return 0;
        }
        else
        {
          int x;
          while(1)
          {
            printf("Are you sure (Y/y or N/n ):\n");
            x=tolower(getchar());
            if(x=='y') return 1;
            if(x=='n') return 0;
          }
        }
        return 0; 
}

int             GUI_YesNo(const char *primary, const char *secondary_format)
{
   uint32_t msglvl=2;

        prefs->get(MESSAGE_LEVEL,&msglvl);

        boxStart();
        boxAdd("Question");
        boxAdd(primary);

        
        if(! secondary_format)
        {
                boxEnd();
        }
        else
        {
            boxAdd(secondary_format);
            boxEnd();
        }
        if (beQuiet)
        {
          boxAdd("--> First one\n");
          return 0;
        }
        else
        {
          int x;
          while(1)
          {
            printf("Yes or No (Y/y or N/n) :\n");
            x=tolower(getchar());
            if(x=='y') return 1;
            if(x=='n') return 0;
          }
          return 0; 
        }
}

int             GUI_Question(const char *alertstring)
{
  
  boxStart();
  boxAdd("Question");
  boxAdd(alertstring);
  boxEnd();
  
  while(1)
          {
            printf("Yes or No (Y/y or N/n) :\n");
            int x=tolower(getchar());
            if(x=='y') return 1;
            if(x=='n') return 0;
          }
}

int      GUI_Alternate(const char *title,const char *choice1,const char *choice2)
{
  boxStart();
  boxAdd("Choice");
  boxAdd(title);
  boxEnd();
  
  while(1)
          {
            printf("0->%s 1->%s :\n",choice1,choice2);
            int x=tolower(getchar());
            if(x=='0') return 0;
            if(x=='1') return 1;
          }
}

uint8_t  GUI_getDoubleValue(double *valye, float min, float max, const char *title)
{
  boxStart();
  boxAdd("DOUBLEVALUE stub");
  boxAdd(title);
  boxEnd();
  
  return 0; 
}

uint8_t		GUI_isQuiet(void)
{
    return beQuiet;
}
void            GUI_Verbose(void)
{
    beQuiet=0;
}
void            GUI_Quiet(void)
{
  beQuiet=1;
}
int32_t UI_readJog(void)
{
 return 0; 
}
void getVersion(uint32_t *maj,uint32_t *minor)
{
    *maj=ADM_CORE_TOOLKIT_MAJOR;
    *minor=ADM_CORE_TOOLKIT_MINOR;
}

void UI_purge( void )
{
}

class cliProcessing: public DIA_processingBase
{
public:
        cliProcessing(const char *title, uint64_t _totalToProcess ) : DIA_processingBase(title,_totalToProcess)
        {
            
        }
        ~cliProcessing()
        {
            
        }
        
};
 DIA_processingBase *createProcessing(const char *title,uint64_t totalToProcess)
 {
     return new cliProcessing(title,totalToProcess);cd 
 }

}; // namespace
static CoreToolkitDescriptor CliCoreToolkitDescriptor=
{
		&ADM_CliCoreUIToolkit::getVersion,
		&ADM_CliCoreUIToolkit::GUI_Info_HIG,
		&ADM_CliCoreUIToolkit::GUI_Error_HIG,
		&ADM_CliCoreUIToolkit::GUI_Confirmation_HIG,
		&ADM_CliCoreUIToolkit::GUI_YesNo,
		&ADM_CliCoreUIToolkit::GUI_Question,
		&ADM_CliCoreUIToolkit::GUI_Alternate,
		&ADM_CliCoreUIToolkit::GUI_Verbose,
		&ADM_CliCoreUIToolkit::GUI_Quiet,
		&ADM_CliCoreUIToolkit::GUI_isQuiet,
                &ADM_CliCoreUIToolkit::createWorking,
                &ADM_CliCoreUIToolkit::createEncoding,
                &ADM_CliCoreUIToolkit::createAudioTrack,
                &ADM_CliCoreUIToolkit::UI_purge,         
                &ADM_CliCoreUIToolkit::createProcessing
};


void InitCoreToolkit(void)
{
	DIA_toolkitInit(&CliCoreToolkitDescriptor);
}
//EOF
