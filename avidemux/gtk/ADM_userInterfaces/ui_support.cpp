#include <stdio.h>
#include <gtk/gtk.h>

#ifdef _WIN32
#include <gdk/gdkwin32.h>
#elif defined(__APPLE__)
extern "C" int getMainNSWindow(void);
#else
#include <gdk/gdkx.h>
#endif

#include "config.h"
#include "ADM_inttype.h"
#include "ADM_files.h"
#include "DIA_uiTypes.h"
#include "ADM_default.h"

extern GtkWidget *guiRootWindow;

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
	sprintf(desc, "GTK+ (%i.%i.%i)", gtk_major_version, gtk_minor_version, gtk_micro_version);
}

ADM_UI_TYPE UI_GetCurrentUI(void)
{
  return ADM_UI_GTK;
}

const char* getNativeRendererDesc(void)
{
	return QT_TR_NOOP("GTK+");
}

void getMainWindowHandles(long int *handle,long int *nativeHandle)
{
	*handle = (long int)guiRootWindow;

#ifdef _WIN32
	*nativeHandle = (long int)gtk_widget_get_window(guiRootWindow);
#elif defined(__APPLE__)
	*nativeHandle = (long int)getMainNSWindow();
#else
	*nativeHandle = (long int)gtk_widget_get_window(guiRootWindow);
#endif
}

void GUI_OpenApplicationLog() { }
void GUI_OpenApplicationDataFolder() { }
