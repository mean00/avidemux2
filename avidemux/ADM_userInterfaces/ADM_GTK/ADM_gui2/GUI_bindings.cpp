/***************************************************************************
                          GUI_binding.cpp  -  description
                             -------------------
    begin                : Fri Jan 17 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <math.h>
#include "ADM_toolkitGtk.h"

#include "../ADM_render/GUI_render.h"
#include "gui_action.hxx"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_UI
#include "ADM_osSupport/ADM_debug.h"

#include "ADM_codecs/ADM_codec.h"
#include "../ADM_commonUI/GUI_ui.h"

#include "DIA_fileSel.h"
#include "ADM_Video.h"

#include "prefs.h"
#include "../ADM_toolkit_gtk/gtkmarkscale.h"
#include "../ADM_toolkit_gtk/jogshuttle.h"
#include "../ADM_toolkit_gtk/ADM_jogshuttle.h"
#include "gtkgui.h"
#include "DIA_coreToolkit.h"
#include "ADM_userInterfaces/ADM_render/GUI_renderInternal.h"
#include "ADM_video/ADM_vidMisc.h"
#include "GUI_glade.h"

extern uint8_t UI_getPhysicalScreenSize(void* window, uint32_t *w,uint32_t *h);
extern void ADM_initUIGtk(GtkWidget *guiRootWindow);
extern  ADM_RENDER_TYPE UI_getPreferredRender(void);;

#define WOD(x) glade.getWidget (#x)

void frame2time(uint32_t frame, uint32_t fps, uint16_t * hh, uint16_t * mm,
	   uint16_t * ss, uint16_t * ms);


static admGlade glade;

GtkWidget        *guiRootWindow=NULL;
static GtkWidget *guiDrawingArea=NULL;
static GtkWidget *guiSlider=NULL;

static GtkWidget *guiCurFrame=NULL;
static GtkWidget *guiTotalFrame=NULL;
static GtkWidget *guiCurTime=NULL;
static GtkWidget *guiTotalTime=NULL;


static GtkWidget *guiVideoToggle=NULL;
static GdkCursor *guiCursorBusy=NULL;
static GdkCursor *guiCursorNormal=NULL;

static  GtkAdjustment *sliderAdjustment;

static int keyPressHandlerId=0;

static gint       jogChange( void );
static void volumeChange( void );
static char     *customNames[ADM_MAC_CUSTOM_SCRIPT];
static uint32_t ADM_nbCustom=0;
// Needed for DND
// extern int A_openAvi (char *name);
extern int A_appendAvi (const char *name);


static void on_audio_change(void);
static void on_video_change(void);
static void on_preview_change(void);
static void on_format_change(void);
static int update_ui=0;
void GUI_gtk_grow_off(int onff);
static void GUI_initCustom(void);
const char * GUI_getCustomScript(uint32_t nb);

uint32_t audioEncoderGetNumberOfEncoders(void);
const char  *audioEncoderGetDisplayName(uint32_t i);

extern uint32_t ADM_mx_getNbMuxers(void);
extern const char *ADM_mx_getDisplayName(uint32_t i);


#ifdef HAVE_AUDIO
extern uint8_t AVDM_setVolume(int volume);
#endif
extern void checkCrashFile(void);
#define AUDIO_WIDGET   "comboboxAudio"
#define VIDEO_WIDGET   "comboboxVideo"
#define FORMAT_WIDGET  "comboboxFormat"
#define PREVIEW_WIDGET "comboboxPreview"
//
enum
{
        TARGET_STRING,
        TARGET_ROOTWIN,
        TARGET_URL
};

static GtkTargetEntry target_table[] =
{
  { "STRING",     0, TARGET_STRING },
  { "text/plain", 0, TARGET_STRING },
  { "text/uri-list", 0, TARGET_URL },
  { "application/x-rootwin-drop", 0, TARGET_ROOTWIN }
};
// CYB 2005.02.22: DND (END)

static void DNDDataReceived( GtkWidget *widget, GdkDragContext *dc,
                             gint x, gint y, GtkSelectionData *selection_data, guint info, guint t);

extern aviInfo   *avifileinfo;
extern GtkWidget	*create_mainWindow (void);
extern void guiCallback(GtkMenuItem * menuitem, gpointer user_data);
extern void HandleAction(Action act);
extern gboolean UI_on_key_press(GtkWidget *widget, GdkEventKey* event, gpointer user_data);
extern char *actual_workbench_file;
extern void FileSel_ReadWrite(SELFILE_CB *cb, int rw, const char *name, const char *actual_workbench_file);

// To build vcodec
extern int encoderGetEncoderCount(void);
extern const char* encoderGetIndexedName(uint32_t i);
//
static uint8_t  bindGUI( void );
static gboolean destroyCallback(GtkWidget * widget,	  GdkEvent * event, gpointer user_data);
static gboolean  on_drawingarea1_expose_event(GtkWidget * widget,  GdkEventExpose * event, gpointer user_data);
// Currentframe taking/loosing focus
static int  UI_grabFocus( void);
static int  UI_loseFocus( void);
static void UI_focusAfterActivate(GtkMenuItem * menuitem, gpointer user_data);
static void GUI_initCursor( void );
 void UI_BusyCursor( void );
 void UI_NormalCursor( void );
// For checking if Slider shift key is pressed
bool SliderIsShifted = FALSE;
gboolean UI_SliderPressed(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean UI_SliderReleased(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean UI_returnFocus(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
// Global
GtkAccelGroup *accel_group;
#ifdef USE_JOG
PhysicalJogShuttle *physical_jog_shuttle;
#endif
//
void guiCallback(GtkMenuItem * menuitem, gpointer user_data);

typedef struct buttonCallBack_S
{
	const char *name;
	const char *signal;
	Action action;

} buttonCallBack_S;

buttonCallBack_S buttonCallback[]=
{
	{"buttonPlay"			,"clicked"		,ACT_PlayAvi},
	{"buttonStop"			,"clicked"		,ACT_StopAvi},
	{"buttonPrevFrame"		,"clicked"		,ACT_PreviousFrame},
	{"buttonNextFrame"		,"clicked"		,ACT_NextFrame},
	{"buttonPrevKFrame"		,"clicked"		,ACT_PreviousKFrame},
	{"buttonNextKFrame"		,"clicked"		,ACT_NextKFrame},
	{"buttonMarkA"			,"clicked"		,ACT_MarkA},
	{"buttonMarkB"			,"clicked"		,ACT_MarkB},
	{"buttonBegin"			,"clicked"		,ACT_Begin},
	{"buttonEnd"			,"clicked"		,ACT_End},
	{"menutoolbuttonOpen"	,"clicked"		,ACT_OpenAvi},
	{"toolbuttonInfo"		,"clicked"		,ACT_AviInfo},
	{"toolbuttonSave"		,"clicked"		,ACT_SaveAvi},

	{"buttonFilters"		,"clicked"		,ACT_VideoParameter},
	{"buttonAudioFilter"	,"clicked"		,ACT_AudioFilters},
	{"buttonConfV"			,"clicked"		,ACT_SetMuxParam},
	{"buttonConfA"			,"clicked"		,ACT_AudioCodec},
	{"buttonConfF"			,"clicked"		,ACT_SetMuxParam},

	{"buttonPrevBlack"		,"clicked"		,ACT_PrevBlackFrame},
	{"buttonNextBlack"		,"clicked"		,ACT_NextBlackFrame},
	{"buttonGotoA"			,"clicked"		,ACT_GotoMarkA},
	{"buttonGotoB"			,"clicked"		,ACT_GotoMarkB},
	{"toolbuttonCalc"		,"clicked"		,ACT_Bitrate},

	//{"boxCurFrame"			,"activate"		,ACT_JumpToFrame},
	//{"boxCurTime"			,"editing_done"		,ACT_TimeChanged},
    {"CheckButtonTimeshift"         ,"toggled"             ,ACT_TimeShift}
    // {"spinbuttonTimeShift"          ,"editing_done"       ,ACT_TimeShift}

};

#ifdef ENABLE_WINDOW_SIZING_HACK

// This is a hack to empirically determine the largest window for which there
// is room on the screen, allowing for panels and such.  There doesn't seem to
// be any way to query GTK for this information, so we create a window,
// maximize it and make it visible, wait for the real configure event that
// comes after it is actually maximized, and record the size that it had in
// that event.  That size is what we take to be the largest window that will
// fit.  We use that information later to calculate how much (and whether or
// not) to scale down the preview video in flyDialogs, via
// UI_calcZoomToFitScreen() in GUI_gtkRender.cpp.

int maxWindowWidth = -1;
int maxWindowHeight = -1;

gboolean tmpwin_configured (GtkWidget * widget, GdkEventConfigure * event,
                            gpointer user_data)
{
    printf ("tmpwin_configured: now %dx%d @ +%d+%d\n",
            event->width, event->height, event->x, event->y);

    // it always seems to get two events, one at a 200x200 dimension and then
    // the one at the maximized dimensions.  An alternate approach here would
    // be to ignore the first configure event and use the 2nd.  Yet another
    // way would be to do the maximize call here on the first event...

    if (event->width > 200)
    {
        maxWindowWidth = event->width;
        maxWindowHeight = event->height;
        gtk_widget_destroy (widget);
    }

    return TRUE;
}

void do_tmpwin_hack (void)
{
    GtkWidget * tmpwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (GTK_OBJECT (tmpwin), "configure-event",
                      GTK_SIGNAL_FUNC (tmpwin_configured),
                      gpointer (tmpwin));
    gtk_window_maximize (GTK_WINDOW(tmpwin));
    gtk_widget_show (tmpwin);
}

#endif

// This is part of a hack to allow the user to attach the Play and Stop
// actions to buttons.  The problem is that once we launch the Play action,
// HandleAction() doesn't return to its caller until avidemux stops playing,
// so if HandleAction() is called directly from the event handler, there is no
// way for the Stop button to be seen until something else has already stopped
// it.  However, if we have the event handler just set up a timer, which
// launches the play action, it's only the timer that is locked up, and since
// we only use the timer for the Play action (and not any of the other
// buttons), then everything (including the Stop button) works just fine.
// A GTK expert might know of a better way to solve this problem, but I'm not
// convinced that one exists in current GTK (2.10 on my FC7 system, but we
// need to support older ones, too).

static gboolean on_PlayButtonHackTimer (gpointer data)
{
    gdk_threads_enter();
    HandleAction (ACT_PlayAvi);
    gdk_threads_leave();
    return FALSE;
}

void jogButton (void *, unsigned short /* raw_button */, Action gui_action)
{
    // hack alert: see comment for on_PlayButtonHackTimer(), above
    if (gui_action == ACT_PlayAvi)
        g_timeout_add (10, on_PlayButtonHackTimer, NULL);
    else if (gui_action != ACT_INVALID)
        HandleAction (gui_action);
}

void jogDial (void *, signed short offset) // offset usually -1 or +1
{
    if (offset < 0)
        GUI_PrevFrame (uint32_t (-offset));
    else
        GUI_NextFrame (uint32_t (offset));
}

GtkWidget * lookup_jog_shuttle_widget (void)
{
    return glade.getWidget("jogg");
}

void jogRing (void *, gfloat angle) // angle is -1.0 to 0 to +1.0
{
    GtkWidget * jsw = lookup_jog_shuttle_widget();
    if (jsw)
        jog_shuttle_set_value (jsw, angle);
}

/**
        \fn initGUI
        \brief Create main window and bind to it
*/
uint8_t initGUI( void )
{
uint8_t ret=0;
uint32_t w,h;
        if(!glade.loadFile("main/gtk2_build.glade"))
        {
            GUI_Error_HIG("Glade","Cannot load glade file");
            ADM_assert(0);
        }
		// create top window
		guiRootWindow=glade.getWidget("mainWindow");

		if(!guiRootWindow) 
        {
            return 0;
        }

#ifdef ENABLE_WINDOW_SIZING_HACK
                do_tmpwin_hack();
#endif

		gtk_register_dialog(guiRootWindow);
		ADM_initUIGtk(guiRootWindow);
		// and seek global sub entity
		ret= bindGUI();
		if(ret) gtk_widget_show(guiRootWindow);
                UI_purge();
		// Set it as always low level
		//gtk_window_set_keep_below(GTK_WINDOW(guiRootWindow), 1);
		renderInit();
		GUI_initCursor(  );


                UI_getPhysicalScreenSize(guiRootWindow, &w, &h);
                printf("The screen seems to be %u x %u px\n",w,h);

                GUI_gtk_grow_off(1);
#ifdef USE_JOG
                physical_jog_shuttle = &(PhysicalJogShuttle::getInstance());
                physical_jog_shuttle->registerCBs (NULL, jogButton, jogDial, jogRing);
#endif



	return ret;
}
/**
    \fn destroyGUI
*/
void destroyGUI(void)
{
	renderDestroy();

	for(int i=0;i<ADM_nbCustom;i++)
		delete(customNames[i]);
#ifdef USE_JOG
        physical_jog_shuttle->deregisterCBs (NULL);
        delete physical_jog_shuttle;
#endif
}

/**
    \fn GUI_gtk_grow_off
    \brief allow main window to grow or not
*/
void GUI_gtk_grow_off(int onoff)
{
  gtk_window_set_policy(GTK_WINDOW ( guiRootWindow ),
                                             0, //gint allow_shrink,
                                             onoff, //gint allow_grow,
                                             1);//gint auto_shrink);
}
/**
    Get the custom entry

*/
const char * GUI_getCustomScript(uint32_t nb)
{
    ADM_assert(nb<ADM_nbCustom);
    return customNames[nb];

}
/**
     \fn bindGUI
     \brief Bind the GUI to our handling functions/callbacks
*/
uint8_t  bindGUI( void )
{

#define ADM_LOOKUP(a,b) a= glade.getWidget (#b);if(!a) return 0;


	ADM_LOOKUP(guiDrawingArea,guiDrawing);
	ADM_LOOKUP(guiSlider,sliderNavigate);

	sliderAdjustment=gtk_range_get_adjustment (GTK_RANGE(guiSlider));

	ADM_LOOKUP(guiCurFrame,boxCurFrame);
	ADM_LOOKUP(guiTotalFrame,labelTotalFrame);

	ADM_LOOKUP(guiCurTime,boxCurTime);
	ADM_LOOKUP(guiTotalTime,labelTotalTime);

#undef ADM_LOOKUP
  // bind menu
 #define CALLBACK(x,y) gtk_signal_connect(GTK_OBJECT( glade.getWidget(#x)), "activate", \
                      GTK_SIGNAL_FUNC(guiCallback),                   (void *) y)

 	#include "GUI_menumap.h"
  #undef CALLBACK
  /// /bind menu

// destroy
	 gtk_object_set_data_full(GTK_OBJECT(guiRootWindow),
			     "guiRootWindow",
			     guiRootWindow,
			     (GtkDestroyNotify) destroyCallback);


//	now add callbacks
	 gtk_widget_add_events(guiRootWindow, GDK_BUTTON_PRESS_MASK);
	 gtk_signal_connect(GTK_OBJECT(guiRootWindow), "button_press_event", GTK_SIGNAL_FUNC(UI_returnFocus), NULL);

	gtk_signal_connect(GTK_OBJECT(guiSlider), "button_press_event", GTK_SIGNAL_FUNC(UI_SliderPressed), NULL);
	gtk_signal_connect(GTK_OBJECT(guiSlider), "button_release_event", GTK_SIGNAL_FUNC(UI_SliderReleased), NULL);

	// Current Frame
	gtk_signal_connect(GTK_OBJECT(guiCurFrame), "focus_in_event", GTK_SIGNAL_FUNC(UI_grabFocus), (void *) NULL);
	gtk_signal_connect(GTK_OBJECT(guiCurFrame), "focus_out_event", GTK_SIGNAL_FUNC(UI_loseFocus), (void *) NULL);
	gtk_signal_connect(GTK_OBJECT(guiCurFrame), "activate", GTK_SIGNAL_FUNC(UI_focusAfterActivate), (void *) ACT_JumpToFrame);

    // Volume
    gtk_signal_connect(GTK_OBJECT(glade.getWidget("hscalVolume")), "value_changed", GTK_SIGNAL_FUNC(volumeChange), (void *) NULL);

    // Jog
    gtk_signal_connect(GTK_OBJECT(glade.getWidget("jogg")), "value_changed", GTK_SIGNAL_FUNC(jogChange), (void *) NULL);

	// Time Shift
	gtk_signal_connect(GTK_OBJECT(glade.getWidget("spinbuttonTimeShift")), "focus_in_event", GTK_SIGNAL_FUNC(UI_grabFocus), (void *) NULL);
	gtk_signal_connect(GTK_OBJECT(glade.getWidget("spinbuttonTimeShift")), "focus_out_event", GTK_SIGNAL_FUNC(UI_loseFocus), (void *) NULL);
	gtk_signal_connect(GTK_OBJECT(glade.getWidget("spinbuttonTimeShift")), "activate", GTK_SIGNAL_FUNC(UI_focusAfterActivate), (void *) ACT_TimeShift);

#define ADD_SIGNAL(a,b,c)  gtk_signal_connect(GTK_OBJECT(a), b, GTK_SIGNAL_FUNC(guiCallback), (void *) c);

   	ADD_SIGNAL(guiSlider,"value_changed",ACT_Scale);
	ADD_SIGNAL(glade.getWidget("spinbuttonTimeShift"),"value_changed",ACT_TimeShift);

	// Callbacks for buttons
		uint32_t nb=sizeof(buttonCallback)/sizeof(buttonCallBack_S);
		GtkWidget *bt;


		for(uint32_t i=0;i<nb;i++)
		{
			bt= glade.getWidget(buttonCallback[i].name);
			if(!bt)
			{
				printf("Binding failed for %s\n",buttonCallback[i].name);
				//ADM_assert(0);
			}
			ADD_SIGNAL(bt,buttonCallback[i].signal,buttonCallback[i].action);
			GTK_WIDGET_UNSET_FLAGS (bt, GTK_CAN_FOCUS);
		}

	GTK_WIDGET_SET_FLAGS (glade.getWidget("boxCurFrame"), GTK_CAN_FOCUS);

// set some tuning
    gtk_widget_set_usize(guiDrawingArea, 512, 288);

// hscale
    GTK_WIDGET_UNSET_FLAGS (guiSlider, GTK_CAN_FOCUS);
    gtk_widget_show(guiSlider);
    // And, the size now scales to the width of the window.
    gtk_widget_set_usize(guiSlider, 0, 0);
    // Plus, two-decimal precision.
    gtk_scale_set_digits(GTK_SCALE(guiSlider), 2);
    // And continuous updates!
    gtk_range_set_update_policy (GTK_RANGE (guiSlider), GTK_UPDATE_CONTINUOUS);

    gtk_range_set_range(GTK_RANGE(guiSlider),0,100.00);

    // keyboard events


 	gtk_signal_connect(GTK_OBJECT(guiDrawingArea), "expose_event",
		       GTK_SIGNAL_FUNC(on_drawingarea1_expose_event),
		       NULL);


	// Finally add video codec...
	uint32_t nbVid;
	const char *name;
        GtkComboBox     *combo_box;

                nbVid=encoderGetEncoderCount();
                combo_box=GTK_COMBO_BOX(glade.getWidget(VIDEO_WIDGET));
                gtk_combo_box_remove_text(combo_box,0);
                printf("Found %d video encoder\n",nbVid);
                for(uint32_t i=0;i<nbVid;i++)
                {
                        name=encoderGetIndexedName(i);
                        gtk_combo_box_append_text      (combo_box,QT_TR_NOOP(name));
                }

        gtk_combo_box_set_active(combo_box,0);
        on_video_change();
        // And A codec
        // Finally add video codec...
        uint32_t nbAud;
                nbAud=audioEncoderGetNumberOfEncoders();
                combo_box=GTK_COMBO_BOX(glade.getWidget(AUDIO_WIDGET));
                gtk_combo_box_remove_text(combo_box,0);
                printf("Found %d audio encoder\n",nbAud);
                for(uint32_t i=0;i<nbAud;i++)
                {
                        name=audioEncoderGetDisplayName(i); //audioFilterGetIndexedName(i);
                        printf("Found %d %s audio encoder\n",i,name);
                        gtk_combo_box_append_text      (combo_box,QT_TR_NOOP(name));
                }
        gtk_combo_box_set_active(combo_box,0);
        on_audio_change();
        /*   Fill in output format window */
        uint32_t nbFormat;

        nbFormat=ADM_mx_getNbMuxers();
        combo_box=GTK_COMBO_BOX(glade.getWidget(FORMAT_WIDGET));
        ADM_assert(combo_box);
        gtk_combo_box_remove_text(combo_box,0);
        printf("Found %d Format \n",nbFormat);
        for(uint32_t i=0;i<nbFormat;i++)
        {
                name=ADM_mx_getDisplayName(i);
                gtk_combo_box_append_text      (combo_box,QT_TR_NOOP(name));
        }
        gtk_combo_box_set_active(combo_box,0);



        /* File in preview mode combobox */
            const char *previewText[]=
                {
                    QT_TR_NOOP("Input"),
                    QT_TR_NOOP("Output"),
                    QT_TR_NOOP("Side"),
                    QT_TR_NOOP("Top"),
                    QT_TR_NOOP("Separate")
                };

                combo_box=GTK_COMBO_BOX(glade.getWidget(PREVIEW_WIDGET));
                gtk_combo_box_remove_text(combo_box,0);
                for(uint32_t i=0;i<sizeof(previewText)/sizeof(char*);i++)
                {
                        name=previewText[i];
                        gtk_combo_box_append_text      (combo_box,(name));
                }
        gtk_combo_box_set_active(combo_box,0);
        // Format
                 gtk_combo_box_set_active(GTK_COMBO_BOX(glade.getWidget(FORMAT_WIDGET)),0);

    //
        gtk_signal_connect(GTK_OBJECT(glade.getWidget(VIDEO_WIDGET)), "changed",
                       GTK_SIGNAL_FUNC(on_video_change),
                       NULL);
        gtk_signal_connect(GTK_OBJECT(glade.getWidget(AUDIO_WIDGET)), "changed",
                       GTK_SIGNAL_FUNC(on_audio_change),
                       NULL);
        gtk_signal_connect(GTK_OBJECT(glade.getWidget(PREVIEW_WIDGET)), "changed",
                       GTK_SIGNAL_FUNC(on_preview_change),
                       NULL);
        gtk_signal_connect(GTK_OBJECT(glade.getWidget(FORMAT_WIDGET)), "changed",
                       GTK_SIGNAL_FUNC(on_format_change),
                       NULL);

        // Add initial recent files
        UI_updateRecentMenu(  );
    //
    //CYB 2005.02.22: DND (START)
    // Set up avidemux as an available drag'n'drop target.
    gtk_drag_dest_set(guiRootWindow,
        GTK_DEST_DEFAULT_ALL,
        target_table,sizeof(target_table)/sizeof(GtkTargetEntry),
        (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_DEFAULT));
    g_signal_connect(GTK_OBJECT(guiRootWindow), "drag_data_received",
        GTK_SIGNAL_FUNC(DNDDataReceived),NULL);
    //CYB 2005.02.22: DND (END)

     // Allow shrink
   //GTK_WINDOW ( guiRootWindow ) ->allow_shrink = FALSE;
   //GTK_WINDOW ( guiDrawingArea ) ->allow_shrink = FALSE;

   // By default enable arrow keys
   UI_arrow_enabled();
  // Add custom menu
 GUI_initCustom();
    return 1;

}
/**
    \fn GUI_initCustom
*/
void GUI_initCustom(void )
{
  GtkWidget *menuEntry=glade.getWidget("custom1");
  char *customdir=ADM_getCustomDir();
  if(!menuEntry)
  {
      printf("No custom menu...\n");
      return;
  }
  if(!customdir)
  {
      printf("No custom dir...\n");
      return;
  }
  /* Collect the name */
   if(! buildDirectoryContent(&ADM_nbCustom,customdir, customNames,ADM_MAC_CUSTOM_SCRIPT,".js"))
    {
      printf("Failed to build custom dir content");
      return;
    }
  if(!ADM_nbCustom)
  {
      printf("No custom script\n");
  }
  printf("Found %u custom scripts, adding them\n",ADM_nbCustom);
 GtkWidget *go,*menu;
 int rank;

  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuEntry), menu);

#define CALLBACK(x,y) gtk_signal_connect(GTK_OBJECT(x), "activate", \
                      GTK_SIGNAL_FUNC(guiCallback),                   (void *) y)

  for(int i=0;i<ADM_nbCustom;i++)
  {
    go = gtk_menu_item_new_with_mnemonic (ADM_GetFileName(customNames[i]));
    gtk_widget_show (go);
    gtk_container_add (GTK_CONTAINER (menu), go);
    rank=ACT_CUSTOM_BASE+i;
    CALLBACK( go                 ,rank);
  }

  #undef CALLBACK
  printf("Menu built\n");
}
gboolean  on_drawingarea1_expose_event(GtkWidget * widget,  GdkEventExpose * event, gpointer user_data)
{
UNUSED_ARG(widget);
UNUSED_ARG(event);
UNUSED_ARG(user_data);
        renderExpose();
        return true;
}

/**
        \fn getDrawWidget
        \brief Return the widget containing the image
*/
GtkWidget *getDrawWidget( void )
{
	return guiDrawingArea;

}
/**
        \fn UI_setScale
        \briefSet slider position
*/
static int _upd_in_progres=0;
void UI_setScale( double val )
{
if(_upd_in_progres) return;
 _upd_in_progres++;
   gtk_adjustment_set_value( GTK_ADJUSTMENT(sliderAdjustment),(  gdouble  ) val );
   gtk_signal_emit_by_name (GTK_OBJECT (sliderAdjustment), "changed");
 _upd_in_progres--;

}

/**
    \fn UI_readJog
    \brief Returns value of jog
*/
int32_t UI_readJog(void)
{
  GtkWidget *wid;
  float val;

        wid=glade.getWidget("jogg");
        val=jog_shuttle_get_value(wid);
        //printf("Jog : %f\n",val);
        val=val*100;
        return (int32_t )val;

}
/**
        \fn UI_setTitle
*/
void UI_setTitle(const char *name)
{
	char *title;
	const char* defaultTitle = "Avidemux";

	if (name && strlen(name) > 0)
	{
		title = new char[strlen(defaultTitle) + strlen(name) + 3 + 1];

		strcpy(title, name);
		strcat(title, " - ");
		strcat(title, defaultTitle);
	}
	else
	{
		title = new char[strlen(defaultTitle) + 1];

		strcpy(title, defaultTitle);
	}

	gtk_window_set_title(GTK_WINDOW (guiRootWindow), title);
	delete [] title;
}
/**
    \fn UI_setFrameType
*/
void UI_setFrameType( uint32_t frametype,uint32_t qp)
{
GtkLabel *wid=(GtkLabel *)glade.getWidget("labelFrameType");
static char string[100];
char	c='?';
	switch(frametype)
	{
		case AVI_KEY_FRAME: c='I';break;
		case AVI_B_FRAME: c='B';break;
		case 0: c='P';break;
		default:c='?';break;

	}
	sprintf(string,QT_TR_NOOP("Frame:%c(%02d)"),c,qp);
	gtk_label_set_text( wid,string);

}
/**
    \fn UI_readScale
	\brief Get slider position, return double between 0.0 and 1.0
*/
double  UI_readScale( void )
{

	return (double)GTK_ADJUSTMENT(sliderAdjustment)->value;

}
/**
    \fn UI_updateFrameCount
	
*/

void UI_updateFrameCount(uint32_t curFrame)
{
    //char text[80];
    // frames
    //sprintf(text, "%"LU" ", curFrame);
//    gtk_label_set_text((GtkLabel *) guiCurFrame, text);
	gtk_write_entry(guiCurFrame,curFrame);

}
/**
    \fn UI_setFrameCount
	
*/

void UI_setFrameCount(uint32_t curFrame,uint32_t total)
{
    char text[80];
    if(total) total--; // We display from 0 to X
    // frames
   // sprintf(text, "%"LU" ", curFrame);
   // gtk_label_set_text((GtkLabel *) guiCurFrame, text);
    gtk_write_entry(guiCurFrame,curFrame);
      sprintf(text, "/ %"LU"", total);
    gtk_label_set_text((GtkLabel *) guiTotalFrame, text);

}
/**
    \fn UI_setTotalTime
    \brief SEt the total duration of video
*/
void UI_setTotalTime(uint64_t curTime)
{
  char text[80];
 uint16_t mm,hh,ss,ms;
 uint32_t shorty=(uint32_t)(curTime/1000);

    ms2time(shorty,&hh,&mm,&ss,&ms);
  	sprintf(text, "/%02d:%02d:%02d.%03d", hh, mm, ss, ms);
    gtk_label_set_text((GtkLabel *) guiTotalTime, text);


}
/**
    \fn UI_setCurrentTime
    \brief Set current PTS of displayed video
*/
void UI_setCurrentTime(uint64_t curTime)
{
  char text[80];
 uint16_t mm,hh,ss,ms;
 uint32_t shorty=(uint32_t)(curTime/1000);

    ms2time(shorty,&hh,&mm,&ss,&ms);
  	sprintf(text, "%02d:%02d:%02d.%03d", hh, mm, ss, ms);
	gtk_write_entry_string(guiCurTime,text);

}

void UI_updateTimeCount(uint32_t curFrame,uint32_t fps)
{
  char text[80];
 uint16_t mm,hh,ss,ms;

 	frame2time(curFrame,fps, &hh, &mm, &ss, &ms);
  	sprintf(text, "%02d:%02d:%02d.%03d", hh, mm, ss, ms);
//     gtk_label_set_text((GtkLabel *) guiCurTime, text);
	gtk_write_entry_string(guiCurTime,text);

}
///
/// Called upon destroy event
/// just cleanly exit the application
///

gboolean destroyCallback(GtkWidget * widget,
				  GdkEvent * event, gpointer user_data)
{
    UNUSED_ARG(widget);
    UNUSED_ARG(event);
    UNUSED_ARG(user_data);
    HandleAction(ACT_Exit);
    return 1;
}
/**
    \fn UI_grabFocus
*/
int UI_grabFocus( void)
{
#define RM(x,y)   gtk_widget_remove_accelerator (glade.getWidget(#x), accel_group, \
                              y, (GdkModifierType) 0  );
#define RMCTRL(x,y)   gtk_widget_remove_accelerator (glade.getWidget(#x), accel_group, \
                              y, (GdkModifierType) GDK_CONTROL_MASK  );

	RM(next_frame1, GDK_KP_6);
	RM(previous_frame1, GDK_KP_4);
	RM(next_intra_frame1, GDK_KP_8);
	RM(previous_intra_frame1, GDK_KP_2);

	RM(buttonNextFrame, GDK_KP_6);
	RM(buttonPrevFrame, GDK_KP_4);
	RM(buttonNextKFrame, GDK_KP_8);
	RM(buttonPrevKFrame, GDK_KP_2);

	RM(delete1, GDK_Delete);
	RM(set_marker_a1,GDK_bracketleft);
	RM(set_marker_b1,GDK_bracketright);

	RMCTRL(paste1,GDK_V);
	RMCTRL(copy1,GDK_C);
	RMCTRL(cut1,GDK_X);

	UI_arrow_disabled();

	return FALSE;

}
/**
    \fn UI_loseFocus
*/
int UI_loseFocus( void)
{
#define ADD(x,y)  gtk_widget_add_accelerator (glade.getWidget(#x), "clicked", accel_group, \
                              y, (GdkModifierType) 0, GTK_ACCEL_VISIBLE);
#define ADD_ACT(x,y)  gtk_widget_add_accelerator (glade.getWidget(#x), "activate", accel_group, \
                              y, (GdkModifierType) 0, GTK_ACCEL_VISIBLE);
#define ADDCTRL(x,y) gtk_widget_add_accelerator (glade.getWidget(#x), "activate", accel_group, \
                              y, (GdkModifierType) GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	ADD_ACT(next_frame1, GDK_KP_6);
	ADD_ACT(previous_frame1, GDK_KP_4);
	ADD_ACT(next_intra_frame1, GDK_KP_8);
	ADD_ACT(previous_intra_frame1, GDK_KP_2);

	ADD(buttonNextFrame, GDK_KP_6);
	ADD(buttonPrevFrame, GDK_KP_4);
	ADD(buttonNextKFrame, GDK_KP_8);
	ADD(buttonPrevKFrame, GDK_KP_2);

	ADD_ACT(delete1, GDK_Delete);
	ADD_ACT(set_marker_a1,GDK_bracketleft);
	ADD_ACT(set_marker_b1,GDK_bracketright);

	ADDCTRL(paste1,GDK_V);
	ADDCTRL(copy1,GDK_C);
	ADDCTRL(cut1,GDK_X);

	UI_arrow_enabled();

	return FALSE;
}
/**
    \fn UI_focusAfterActivate
*/
void UI_focusAfterActivate(GtkMenuItem * menuitem, gpointer user_data)
{
	// return focus to window once Enter has been pressed
	UNUSED_ARG(menuitem);
    Action act;
    uint32_t aint;
    if(update_ui) return; // no event sent

    aint = (long int) user_data;
    act = (Action) aint;

	HandleAction(act);

	gtk_widget_grab_focus(glade.getWidget( "menuBar"));
}
/**
    \fn UI_returnFocus
*/
gboolean UI_returnFocus(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	gtk_widget_grab_focus(glade.getWidget( "menuBar"));
	return FALSE;
}
/**
    \fn UI_setMarkers
*/
void UI_setMarkers(uint64_t a, uint64_t b )
{
char text[500];
  uint16_t hh,mm,ss,ms;
  uint32_t timems;
    a/=1000;
    b/=1000;
    timems=(uint32_t)(a);
    ms2time(timems,&hh,&mm,&ss,&ms);
	snprintf(text,79,"%02"LU":%02"LU":%02"LU".%02"LU,hh,mm,ss,ms);
	
    gtk_label_set_text(GTK_LABEL(glade.getWidget("labelMarkA")),text);

	timems=(uint32_t)(b);
    ms2time(timems,&hh,&mm,&ss,&ms);
	snprintf(text,79,"%02"LU":%02"LU":%02"LU".%02"LU,hh,mm,ss,ms);
	gtk_label_set_text(GTK_LABEL(glade.getWidget("labelMarkB")),text);
    //gtk_markscale_setA(guiSlider, a);
    //gtk_markscale_setB(guiSlider, b);
}

/**
	Set/unset the toggle button audio process
*/
void UI_setAProcessToggleStatus( uint8_t status )
{
gint b;
	 if(status) b=TRUE;
  	else			b=FALSE;

	if(b!=gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(glade.getWidget("togglebuttonAudio"))))
     		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(glade.getWidget("togglebuttonAudio")),b);
	aprintf("++ audio toggle : %d(%d)\n",b,status);

}
/**
    \fn UI_setVProcessToggleStatus
	\brief Set/unset the toggle button video process
*/
void UI_setVProcessToggleStatus( uint8_t status )
{
gint b;
	 if(status) 		b=TRUE;
  	else			b=FALSE;
	if(b!=gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(glade.getWidget("togglebuttonVideo"))))
	{
     		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(glade.getWidget("togglebuttonVideo")),b);
		aprintf("Changing it to %d\n",b);
	}
	aprintf("++ video toggle : %d\n",b);

}

/**
            \fn guiCallback
            \brief This is a relay function to do UI events -> gtk_gui.cpp
*/
void guiCallback(GtkMenuItem * menuitem, gpointer user_data)
{
    UNUSED_ARG(menuitem);
    Action act;
    uint32_t aint;
    if(update_ui) return; // no event sent

    aint = (long int) user_data;
    act = (Action) aint;
    if(act==ACT_Scale)
    {
    	if( _upd_in_progres) return;
	_upd_in_progres++;
   	 HandleAction(act);
	_upd_in_progres--;
   	return;
    }
     HandleAction(act);
}
/**
    \fn UI_JumpDone
*/
void UI_JumpDone(void )
{


}
/**
    \fn UI_readCurFrame
*/

int UI_readCurFrame( void )
{
	int i = gtk_read_entry(guiCurFrame);

	if(i < 0)
		i = 0;

	return i;
}
/**
    \fn UI_readCurTime
*/
int UI_readCurTime(uint16_t &hh, uint16_t &mm, uint16_t &ss, uint16_t &ms)
{
	return 0;
}

/**
		in=0 -> arts1
		in=1 -> alsa
		in=2->oss

*/

void UI_iconify( void )
{
	gtk_window_iconify(GTK_WINDOW(guiRootWindow));
}
void UI_deiconify( void )
{
	gtk_window_deiconify(GTK_WINDOW(guiRootWindow));
}
void GUI_initCursor( void )
{

	guiCursorBusy=gdk_cursor_new(GDK_WATCH);
	guiCursorNormal=gdk_cursor_new(GDK_X_CURSOR);
}
// Change cursor and drop all events
void UI_BusyCursor( void )
{
	 gdk_window_set_cursor((guiRootWindow->window),
                                          guiCursorBusy);

}
/**
    \fn on_video_change
*/
void on_video_change(void)
{
int enable;
        if(update_ui)
        {
                printf("Updating\n");
                 return;
        }

        if(!UI_getCurrentVCodec()) // copy ?
        {
                enable=0;
        }
        else enable=1;
        gtk_widget_set_sensitive(glade.getWidget("buttonConfV"),enable);
        gtk_widget_set_sensitive(glade.getWidget("buttonFilters"),enable);
        HandleAction(ACT_VideoCodecChanged);
}
/**
    \fn on_audio_change
*/

void on_audio_change(void)
{
int enable;
       if(update_ui) return;
        if(!UI_getCurrentACodec()) // copy ?
        {
                enable=0;
        }
        else enable=1;
        gtk_widget_set_sensitive(glade.getWidget("buttonConfA"),enable);
        gtk_widget_set_sensitive(glade.getWidget("buttonAudioFilter"),enable);
        HandleAction(ACT_AudioCodecChanged);

}
/**
    \fn on_preview_change
*/
void on_preview_change(void)
{
int enable;
       if(update_ui) return;
        HandleAction(ACT_PreviewChanged);

}
/**
    \fn on_format_change
*/

void on_format_change(void)
{


}

void UI_refreshCustomMenu(void) {}

int  UI_getCurrentPreview(void)
 {

        return gtk_combo_box_get_active(GTK_COMBO_BOX(glade.getWidget(PREVIEW_WIDGET)));

 }
void   UI_setCurrentPreview(int ne)
 {

        gtk_combo_box_set_active(GTK_COMBO_BOX(glade.getWidget(PREVIEW_WIDGET)),ne);

 }

 int 	UI_getCurrentACodec(void)
 {
        //return getRangeInMenu(lookup_widget(guiRootWindow,AUDIO_WIDGET));
        return gtk_combo_box_get_active(GTK_COMBO_BOX(glade.getWidget(AUDIO_WIDGET)));

 }
 int 	UI_getCurrentVCodec(void)
 {

 	//return getRangeInMenu(lookup_widget(guiRootWindow,VIDEO_WIDGET));
        return gtk_combo_box_get_active(GTK_COMBO_BOX(glade.getWidget(VIDEO_WIDGET)));

 }
/**
    \fn UI_setAudioCodec
*/
void UI_setAudioCodec( int i)
{
        //gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(guiRootWindow,AUDIO_WIDGET)), i);
        update_ui=1;
        gtk_combo_box_set_active(GTK_COMBO_BOX(glade.getWidget(AUDIO_WIDGET)),i);
        gtk_widget_set_sensitive(glade.getWidget("buttonConfA"),i);
        gtk_widget_set_sensitive(glade.getWidget("buttonAudioFilter"),i);
        update_ui=0;
}
/**
    \fn UI_setVideoCodec
*/

void UI_setVideoCodec( int i)
{
        //gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(guiRootWindow,VIDEO_WIDGET)), i);
        update_ui=1;
        gtk_combo_box_set_active(GTK_COMBO_BOX(glade.getWidget(VIDEO_WIDGET)),i);
        gtk_widget_set_sensitive(glade.getWidget("buttonConfV"),i);
        gtk_widget_set_sensitive(glade.getWidget("buttonFilters"),i);
        update_ui=0;

}

void UI_NormalCursor( void )
{
//	gtk_widget_set_events(guiRootWindow,guiCursorEvtMask);
	gdk_window_set_cursor((guiRootWindow->window),
                                          NULL); //guiCursorNormal);


}
/**
    \fn UI_GetCurrentFormat
    \brief Returns index of the currently selected container

*/
uint32_t UI_GetCurrentFormat( void )
{

	return (uint32_t)gtk_combo_box_get_active(GTK_COMBO_BOX(glade.getWidget(FORMAT_WIDGET)));
}
/**
    \fn UI_GetCurrentFormat
    \brief Returns index of the currently selected container

*/

bool UI_SetCurrentFormat( uint32_t  fmt )
{

	 gtk_combo_box_set_active(GTK_COMBO_BOX(glade.getWidget(FORMAT_WIDGET)),fmt);
	return true;
}
/**
    \fn DNDmerge
    \brief Extract the actual filename from the DNDstring file://abcd and store it into where
*/
static void DNDmerge(char **where, char *start, char *end)
{
	char urlFile[end - start + 1];
	char *tail = urlFile + (end - start);

	memcpy(urlFile, start, end - start);

	do
	{
		*tail = 0;
		tail--;
	} while (*tail == '\r' || *tail == '\n');

	gchar *filename = g_filename_from_uri(urlFile, NULL, NULL);

	*where = new char[strlen(filename) + 1];
	strcpy(*where, filename);

	g_free(filename);
}
// DND CYB
#define MAX_DND_FILES 20
/**
    \fn DNDDataReceived
    \brief DnDrop file handling, code by CYB, modified by mean
        The datas are incoming like this file://abcd\rfile://xyz\r...
*/
void DNDDataReceived( GtkWidget *widget, GdkDragContext *dc,
                                  gint x, gint y, GtkSelectionData *selection_data, guint info, guint t)
{
   char *start,*cur,*old;
   char *names[MAX_DND_FILES];

    memset(names,0,sizeof(char *)*MAX_DND_FILES);
    if (info != TARGET_STRING && info != TARGET_URL)
    {
      gtk_drag_finish(dc,TRUE,FALSE,t);
      return;
    }

    int current=0;
    start=(char*)selection_data->data;
    old=start;
    while(current<MAX_DND_FILES)
    {
     cur = strstr(start,"file://");
     if(!cur) // Not found
     {
        if(!current)
          break;
        DNDmerge(&(names[current-1]),old,start+strlen(start));
        current++;
        break;
     }
     // Add
     if(current)
     {
        DNDmerge(&(names[current-1]),old,cur);
     }
     current++;
     old=cur;
     start=cur+1;
    }

    // Cleanup
    for(int i=0;i<current-1;i++)
    {
      printf("DND : %d %s\n",i,names[i]);

      const char *filename = names[i];

	  if (avifileinfo)
		  FileSel_ReadWrite(reinterpret_cast <void (*)(const char *)> (A_appendAvi), 0, filename, actual_workbench_file);
	  else
		  FileSel_ReadWrite(reinterpret_cast <void (*)(const char *)> (A_openAvi), 0, filename, actual_workbench_file);

      ADM_dealloc(names[i]);
    }
    gtk_drag_finish(dc,TRUE,FALSE,t);
}
/**
    \fn UI_toogleMain
*/
void UI_toogleMain(void)
{
static int show=1;
        show=show^1;
        if(!show)
                gtk_widget_hide(GTK_WIDGET(glade.getWidget("toolbar2")) );
        else
                gtk_widget_show(GTK_WIDGET(glade.getWidget("toolbar2")) );
}
/**
    \fn UI_toogleSide
*/
void UI_toogleSide(void)
{
static int show=1;
        show=show^1;
        if(!show)
                gtk_widget_hide(GTK_WIDGET(glade.getWidget("vbox9")));
        else
                gtk_widget_show(GTK_WIDGET(glade.getWidget("vbox9")));
}
/**
    \fn UI_updateRecentMenu
*/
uint8_t UI_updateRecentMenu( void )
{
const char **names;
uint32_t nb_item=0;
GtkWidget *button,*menu,*item[4];
static Action recent[4]={ACT_RECENT0,ACT_RECENT1,ACT_RECENT2,ACT_RECENT3};

        names=prefs->get_lastfiles();
// count
        for( nb_item=0;nb_item<4;nb_item++)
        {
                if(!names[nb_item]) break;
        }
        button=glade.getWidget("menutoolbuttonOpen");
        if(!nb_item)
        {
                gtk_menu_tool_button_set_menu   (GTK_MENU_TOOL_BUTTON(button),NULL);
                return 1;
        }
        menu=gtk_menu_new();
        for(int i=0;i<nb_item;i++)
        {
                item[i]=gtk_menu_item_new_with_label(names[i]);
                gtk_menu_attach(GTK_MENU(menu),item[i],0,1,i,i+1);
                 gtk_signal_connect (GTK_OBJECT (item[i]), "activate", GTK_SIGNAL_FUNC (guiCallback),
                                (gpointer) recent[i]);
                gtk_widget_show (item[i]);
        }
        gtk_menu_tool_button_set_menu   (GTK_MENU_TOOL_BUTTON(button),menu);
        return 1;
}

// Override arrow keys to quickly navigate
uint8_t UI_arrow_enabled(void)
{
	keyPressHandlerId = g_signal_connect(GTK_OBJECT(guiRootWindow), "key_press_event", GTK_SIGNAL_FUNC(UI_on_key_press), NULL);
}

uint8_t UI_arrow_disabled(void)
{
	g_signal_handler_disconnect(GTK_OBJECT(guiRootWindow), keyPressHandlerId);
}

gboolean UI_SliderPressed(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	gtk_widget_grab_focus(glade.getWidget( "menuBar"));

	if(event->state&GDK_SHIFT_MASK) SliderIsShifted=TRUE;
	return FALSE;

}
gboolean UI_SliderReleased(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	SliderIsShifted=FALSE;
	return FALSE;
}
/****/
extern int global_argc;
extern char **global_argv;
typedef gboolean GCALL       (void *);
extern int automation(void );
//********************************************
extern void initTranslator(void);
static const UI_FUNCTIONS_T UI_Hooks=
    {
        ADM_RENDER_API_VERSION_NUMBER,
        UI_purge,
        UI_getWindowInfo,
        UI_updateDrawWindowSize,
        UI_rgbDraw,
        UI_getDrawWidget,
        UI_getPreferredRender

    };

/**
 *      \fn UI_Init
 *      \brief Entry point. Initialize renderLib.
 *
 */
int UI_Init(int argc, char **argv)
{
	initTranslator();

  uint32_t w,h;
    if(!g_thread_supported())
        g_thread_init(NULL);
    gdk_threads_init();
    gdk_threads_enter();
    gtk_set_locale();
    global_argc=argc;
    global_argv=argv;



    ADM_renderLibInit(&UI_Hooks);

    gtk_init(&global_argc, &global_argv);
    gdk_rgb_init();


}
static void trampoline(void);
static void gtk_fatalFunction(const char *title, const char *info)
{
    GUI_Info_HIG(ADM_LOG_IMPORTANT,title,info);
}
extern void saveCrashProject(void);
int UI_RunApp(void)
{
    if (global_argc >= 2)
    {
      g_timeout_add(200,(GCALL *)trampoline,NULL);
    }
    // Install our crash handler
    ADM_setCrashHook(&saveCrashProject, &gtk_fatalFunction);
    checkCrashFile();
    gtk_main();
    gdk_threads_leave();

}


void trampoline(void)
{
    ADM_usleep(100000); // let gtk start
    gdk_threads_enter();
    automation();
    gdk_threads_leave();
}
/*****************************/
/*
 * This is the wheel position <->time array
 * */
static uint32_t jogScale[10]={
50,
40,
30,
20,  // 5
10,
5,
3,
2,
1,
0
};
static int nbTimer=0;
static int jogLock=0;
static int count=0;
/**
        \fn tickToTime
        \brief  convert tick (i.e. wheel value) between 0 and 9  to a number of tick to wait in 10 ms units. Should be exp/log TODO
  */
static uint32_t tickToTime(uint32_t tick)
{
        if(tick>9) tick=9;
        return jogScale[tick];
}
/**
                \fn jogTimer
                \brief Function that is called every 10 ms and handle the fwd/bwd depending on wheel position
 */
static int jogTimer(void)
{
   int32_t v;
   uint32_t r;
    v=UI_readJog();
    r=abs(v);
    r=r/10;
    //printf ("r %d v %d\n", r, v);
    if(!r)
    {
      return FALSE;
    }
    //printf("Arm %u\n",count);
    if(count)     count--;
    if(count)     return TRUE;
    count=tickToTime(r);
    //printf ("r %d v %d count %d\n", r, v, count);
    if(jogLock) return FALSE;

    jogLock++;
 //   printf("Arm Call\n");
    gdk_threads_enter();
    if(v>0)
         g_signal_emit_by_name (GTK_OBJECT (glade.getWidget("next_intra_frame1")), "activate",G_TYPE_NONE );
      else
        g_signal_emit_by_name (GTK_OBJECT (glade.getWidget("previous_intra_frame1")), "activate",G_TYPE_NONE );
      gdk_threads_leave();
    jogLock--;
      return TRUE;
}
/**
 *      \fn jogDel
 *      \brief Called when timer is deleted
*/
void jogDel(void *)
{
      nbTimer=0;
      count=0;
//      printf("Arm off\n");
}
/**
 *      \fn jogChange
 *      \brief Handler for wheel value changed event
 * */
gint jogChange(void)
{
  int32_t v;
   uint32_t r;
   if(nbTimer) return FALSE; // Already armed

   // Process First move
      v=UI_readJog()/10;
      r=abs(v);
      if(!r)
      {
        return FALSE;
      }
      if(!jogLock)
      {
        jogLock++;
        if(v>0)
          gtk_signal_emit_by_name (GTK_OBJECT (glade.getWidget("next_intra_frame1")), "activate" );

        else
          gtk_signal_emit_by_name (GTK_OBJECT (glade.getWidget("previous_intra_frame1")), "activate" );
        jogLock--;
        nbTimer=1;
        count=0;//tickToTime(r);
        g_timeout_add_full(G_PRIORITY_DEFAULT,10,(GCALL *)jogTimer,NULL,jogDel);

      }
   // Armed!
   return FALSE;
}
/**
  \fn volumeChange
  \brief Called when the volume slider is moved
*/
void volumeChange( void )
{
#ifdef HAVE_AUDIO
GtkWidget *wid;
GtkAdjustment *adj;
int vol;


if(_upd_in_progres) return;
 _upd_in_progres++;

        wid=glade.getWidget("hscalVolume");
        adj=gtk_range_get_adjustment (GTK_RANGE(wid));
        vol=(int)floor(adj->value+0.5);
        AVDM_setVolume( vol);
 _upd_in_progres--;
#endif
}

// EOF
