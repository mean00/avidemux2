#include <stdio.h>

#include "config.h"
#include "ADM_inttype.h"
#include "ADM_files.h"
#include "DIA_uiTypes.h"

#ifdef HAVE_GETTEXT
#include <libintl.h>

extern void initGetText(void);

void initTranslator(void)
{
	initGetText();
}

const char* translate(const char *__domainname, const char *__msgid)
{
	return (const char*)dgettext(PACKAGE, __msgid);
}
#else
void initTranslator(void) {}

const char* translate(const char *__domainname, const char *__msgid)
{
	return __msgid;
}
#endif

void getUIDescription(char* desc)
{
	sprintf(desc, "CLI");
}

const char* getNativeRendererDesc(void)
{
	return "";
}

ADM_UI_TYPE UI_GetCurrentUI(void)
{
  return ADM_UI_CLI;
}

void getMainWindowHandles(long int *handle, long int *nativeHandle)
{
	*handle = 0;
	*nativeHandle = 0;
}
