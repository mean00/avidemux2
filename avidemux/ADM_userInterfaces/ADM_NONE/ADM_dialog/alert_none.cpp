#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include "ADM_default.h"

#include "prefs.h"

#include "DIA_coreToolkit.h"

#include "ADM_assert.h" 

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



void            GUI_Info_HIG(const ADM_LOG_LEVEL level,const char *primary, const char *secondary_format, ...)
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

        va_list ap;
        va_start(ap, secondary_format);
        
        
        char alertstring[1024];
        
        vsnprintf(alertstring,1023,secondary_format, ap);
        va_end(ap);
        boxAdd(alertstring);
        boxEnd();
        
}

void            GUI_Error_HIG(const char *primary, const char *secondary_format, ...)
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
          va_list ap;
          va_start(ap, secondary_format);

          char alertstring[1024];
          
          vsnprintf(alertstring,1023,secondary_format, ap);
          va_end(ap);
          boxAdd(alertstring);
          boxEnd();
        }
}
int             GUI_Confirmation_HIG(const char *button_confirm, const char *primary, const char *secondary_format, ...)
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
            va_list ap;
            va_start(ap, secondary_format);
            
            
            char alertstring[1024];
            
            vsnprintf(alertstring,1023,secondary_format, ap);
            va_end(ap);
            boxAdd(alertstring);
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

int             GUI_YesNo(const char *primary, const char *secondary_format, ...)
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
            va_list ap;
            va_start(ap, secondary_format);
            
            
            char alertstring[1024];
            
            vsnprintf(alertstring,1023,secondary_format, ap);
            va_end(ap);
            boxAdd(alertstring);
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

int      GUI_Alternate(char *title,char *choice1,char *choice2)
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
void InitCoreToolkit(void)
{
	
}
//EOF
