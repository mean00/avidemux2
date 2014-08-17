/** *************************************************************************
        \file                  DIA_coreToolkit.cpp
 
		(c) 2008 Mean, fixounet@free.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "DIA_coreUI_internal.h"
#include "DIA_factoryStubs.h"
#include <stdarg.h>
static CoreToolkitDescriptor *Toolkit=NULL;
#define MAX_ALERT_SIZE 1024
/**
 * 	\fn DIA_toolkitInit
 *  \brief Hook a set of coreUIToolkit into the dll (needed for win32)
 */
uint8_t  DIA_toolkitInit(CoreToolkitDescriptor *d)
{
	uint32_t major,minor;
	Toolkit=d;
    Toolkit->getVersion(&major,&minor);
    printf("[UI Toolkit] Running version %02d:%02d\n",(int)major,(int)minor);
    if(major!=ADM_CORE_TOOLKIT_MAJOR || minor!=ADM_CORE_TOOLKIT_MINOR)
    {
        ADM_error("UI Toolkit version mistmatch, expected %02d:%02d\n",ADM_CORE_TOOLKIT_MAJOR,ADM_CORE_TOOLKIT_MINOR);
        ADM_assert(0);
    }
	return 1;
}
/**
 * 	\fn GUI_Info_HIG
 *  \brief Display a warning/info/debug message. The primary field is the "title" of the window, secondary format is printf like
 */

void            GUI_Info_HIG(const ADM_LOG_LEVEL level,const char *primary, const char *secondary_format, ...)
{
	char text[MAX_ALERT_SIZE+1] = {0};
	ADM_assert(Toolkit);
	va_list ap;
	
		va_start(ap, secondary_format);
		if (secondary_format)
		{
			 vsnprintf(text,
						MAX_ALERT_SIZE,
						secondary_format, 
						ap);
		}		
		va_end(ap);
		
	Toolkit->infoHig(level,primary,text);
	
}
/**
 * 	\fn GUI_Error_HIG
 *  \brief Display an error message. The primary field is the "title" of the window, secondary format is printf like
 */

void            GUI_Error_HIG(const char *primary, const char *secondary_format, ...)
{
	char text[MAX_ALERT_SIZE+1] = {0};
		ADM_assert(Toolkit);
		va_list ap;
		
			va_start(ap, secondary_format);
			if (secondary_format)
			{
				 vsnprintf(text,
							MAX_ALERT_SIZE,
							secondary_format, 
							ap);
			}		
			va_end(ap);
	Toolkit->errorHig(primary,text);	
}
/**
 * 	\fn GUI_Confirmation_HIG
 *  \brief ask for confirmation. The button_confirm will be the title of the button (accept, do it,...)
 */

int             GUI_Confirmation_HIG(const char *button_confirm, const char *primary, const char *secondary_format, ...)
{
	char text[MAX_ALERT_SIZE+1] = {0};
			ADM_assert(Toolkit);
			va_list ap;
			
				va_start(ap, secondary_format);
				if (secondary_format)
				{
					 vsnprintf(text,
								MAX_ALERT_SIZE,
								secondary_format, 
								ap);
				}		
				va_end(ap);
	return Toolkit->confirmationHig(button_confirm,primary,text);	
}
/**
 * 	\fn GUI_YesNo
 *  \brief Ask for yes/no. Yes will return 1, No will return 0
 */

int             GUI_YesNo(const char *primary, const char *secondary_format, ...)
{
	char text[MAX_ALERT_SIZE+1] = {0};
			ADM_assert(Toolkit);
			va_list ap;
			
				va_start(ap, secondary_format);
				if (secondary_format)
				{
					 vsnprintf(text,
								MAX_ALERT_SIZE,
								secondary_format, 
								ap);
				}		
				va_end(ap);
	return Toolkit->yesno(primary,text);	
}
/**
 * 	\fn GUI_Question
 *  \brief About the same as GUI_YesNo, the button will be ok/cancel
 */
int             GUI_Question(const char *alertstring)
{
	ADM_assert(Toolkit);
	return Toolkit->question(alertstring);	
}

//
//    Sleep for n ms
//
void GUI_Sleep(uint32_t ms)
{
    if (ms < 10)
	return;
    ADM_usleep(ms*1000);
}

/**
 * 	\fn GUI_Alternate
 *  \brief Ask to choose between choice1 and choice2
 * 
 */
int             GUI_Alternate(const char *title,const char *choice1,const char *choice2)
{
	ADM_assert(Toolkit);
	return Toolkit->alternate(title,choice1,choice2);	
}

/**
 * 	\fn GUI_Verbose
 *  \brief  Set ui in verbose mode (default). Show all popup & questions
 */
void            GUI_Verbose(void)
{
	ADM_assert(Toolkit);
	Toolkit->verbose();	
}
/**
 * 	\fn GUI_Quiet
 *  \brief Set the ui in silent mode. All popups & questions will be answered with their default value
 */
void            GUI_Quiet(void)
{
	ADM_assert(Toolkit);
	Toolkit->quiet();	
}
/**
 * 	\fn GUI_isQuiet
 *  \brief  Is the UI in quiet mode ?
 */
uint8_t			GUI_isQuiet(void)
{
	ADM_assert(Toolkit);
	return Toolkit->isQuiet();	
}

// Some obsolete functions ....

uint8_t                 GUI_getIntegerValue(int *valye, int min, int max, const char *title)
{
        return DIA_GetIntegerValue(valye,min,max,title,title);
}
/**
      \fn DIA_GetIntegerValue( 
      \brief Popup a window asking for a value between min & max (int)
      @return 1 on success, 0 on failure

*/

uint8_t  DIA_GetIntegerValue(int *value, int min, int max, const char *title, const char *legend)
{

    int32_t val=*value;;
     
    diaElemInteger     fval(&val,legend,min,max);
    
      diaElem *elems[1]={&fval};

   if(diaFactoryRun(title,1,elems))
  {
    *value=(int)val;
    return 1;
  }
  return 0;     
}
/**
      \fn DIA_GetFloatValue
      \brief Popup a window asking for a value between min & max
      @return 1 on success, 0 on failure

*/
uint8_t  DIA_GetFloatValue(float *value, float min, float max, const char *title, const char *legend)
{
  ELEM_TYPE_FLOAT val=*value;;
     
    diaElemFloat     fval(&val,legend,min,max);
    
      diaElem *elems[1]={&fval};

   if(diaFactoryRun(title,1,elems))
  {
    *value=(float)val;
    return 1;
  }
  return 0;     
}
/**
    \fn createWorking
    \brief
*/
DIA_workingBase *createWorking(const char *title)
{
    if(Toolkit->createWorking) return Toolkit->createWorking(title);
    return NULL;
}

/**
    \fn createWorking
    \brief
*/
DIA_processingBase *createProcessing(const char *title)
{
    if(Toolkit->createProcessing) return Toolkit->createProcessing(title);
    return NULL;
}
/**
    \fn createWorking
    \brief
*/
DIA_audioTrackBase *createAudioTrack(PoolOfAudioTracks *pool, ActiveAudioTracks *active )
{
    if(Toolkit->createAudioTrack) return Toolkit->createAudioTrack(pool,active);
    return NULL;
}
/**
    \fn createEncoding
*/
DIA_encodingBase *createEncoding(uint64_t duration,bool tray)
{
    if(Toolkit->createEncoding) return Toolkit->createEncoding(duration,tray );
//    return new DIA_encodingDummy(duration);
    return NULL;
} 
/**
    \fn UI_Purge
*/
void UI_purge(void)
{
    if(Toolkit->uiPurge) Toolkit->uiPurge();
}
// EOF
