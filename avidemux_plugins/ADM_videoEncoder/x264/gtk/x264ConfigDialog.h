#ifndef x264ConfigDialog_h
#define x264ConfigDialog_h

#include <gtk/gtk.h>

#include "../options.h"

extern "C"
{
#include "ADM_vidEnc_plugin.h"
}

static GtkWidget *create_dialog1(void);
static int getCurrentEncodeMode(GtkWidget *dialog);
static void updateMode(GtkWidget *dialog, int encodeMode, int encodeModeParameter);
static void updateDeblockingFilter(GtkWidget *dialog);
static int signalReceiver(GtkObject* object, gpointer user_data);
static int entryTarget_changed(GtkObject* object, gpointer user_data);
static void loadOptions(GtkWidget *dialog, x264Options *options);
static void saveOptions(GtkWidget *dialog, x264Options *options);

#endif	// x264ConfigDialog_h
