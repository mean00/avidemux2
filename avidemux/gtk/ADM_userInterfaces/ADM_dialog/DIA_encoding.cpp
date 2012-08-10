/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_inttype.h"
#include "DIA_encoding.h"
#include "ADM_inttype.h"
#include "ADM_toolkitGtk.h"
#include "GUI_glade.h"
#include "prefs.h"
#include "DIA_encoding.h"
#include "DIA_coreToolkit.h"
#include "ADM_vidMisc.h"
#include "ADM_coreUtils.h"

#define GW(x) glade.getWidget(#x)
#define WRITEM(x,y) gtk_label_set_text(GTK_LABEL(GW(x)), y)
#define WRITE(x)  WRITEM(x, stringMe)

static gulong id1, id2;
static int stopReq = 0;
static char stringMe[80];
extern bool ADM_slaveReportProgress(uint32_t percent);

/**
    \class DIA_encodingGtk
*/
class DIA_encodingGtk : public DIA_encodingBase
{
public:
    DIA_encodingGtk(uint64_t duration,bool useTray);
    ~DIA_encodingGtk();
    
protected:
    void setTotalSize(uint64_t size);
    void setAudioSize(uint64_t size);
    void setVideoSize(uint64_t size);
    void setPercent(uint32_t percent);
    void setFps(uint32_t fps);
    void setFrameCount(uint32_t nb);
    void setElapsedTimeMs(uint32_t nb);
    void setRemainingTimeMS(uint32_t nb);
    void setAverageQz(uint32_t nb);
    void setAverageBitrateKbits(uint32_t kb);
    admGlade glade;

public:    
    void setPhasis(const char *n);
    void setAudioCodec(const char *n);
    void setVideoCodec(const char *n);
    void setBitrate(uint32_t br,uint32_t globalbr);
    void setContainer(const char *container);
    void setQuantIn(int size);
    bool isAlive();
};

static void buttonPressed(GtkButton*, gpointer)
{
    printf("StopReq\n");
    stopReq = 1;
}

static gboolean windowClosed(GtkWidget*, GdkEvent*, gpointer)
{
    printf("windowClosed\n");
    stopReq = 1;
    return TRUE;
}

static void shutdownChanged(GtkToggleButton* togglebutton, gpointer) 
{
#ifndef _WIN32
    if (getuid() != 0)
    {
        g_signal_handler_block(togglebutton, id1);
        gtk_toggle_button_set_active(togglebutton, FALSE);
        g_signal_handler_unblock(togglebutton, id1);

        GUI_Error_HIG(QT_TR_NOOP("Root privileges are required for shutdown"), NULL);
   }
#endif
}

static void priorityChanged(GtkComboBox* combo, gpointer)
{
#ifndef _WIN32
    if (getuid() != 0)
    {
        g_signal_handler_block(combo, id2);
        gtk_combo_box_set_active(combo, 2);
        g_signal_handler_unblock(combo, id2);

        GUI_Error_HIG(QT_TR_NOOP("Root privileges are required for changing priority"), NULL);
        return;
    }
#endif

    gint priorityLevel = gtk_combo_box_get_active(combo);
#ifndef __HAIKU__
    setpriority(PRIO_PROCESS, 0, ADM_getNiceValue(priorityLevel));
#endif
}

/**
    \fn DIA_encodingGtk
*/
DIA_encodingGtk::DIA_encodingGtk(uint64_t duration,bool tray) : DIA_encodingBase(duration,tray)
{
    ADM_info("DIA_encodingGtk\n");
    stopReq=0;

    glade.init();
    if (!glade.loadFile("encoding.gtkBuilder"))
    {
        GUI_Error_HIG(QT_TR_NOOP("Cannot load dialog"), 
                      QT_TR_NOOP("File \"encoding.gtkBuilder\" could not be loaded."));
        return;
    }

    GtkWidget* combo = GW(comboboxPriority);

#ifndef _WIN32
    if (getuid() != 0)
    {
        // set priority to normal, regardless of preferences
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 2);
    }
#endif
    id1 = g_signal_connect(G_OBJECT(GW(checkbuttonShutdown)), "toggled", G_CALLBACK(shutdownChanged), NULL);
    g_signal_connect(G_OBJECT(GW(buttonPause)), "clicked", G_CALLBACK(buttonPressed), NULL);
    g_signal_connect(G_OBJECT(GW(windowEncoding)), "delete-event", G_CALLBACK(windowClosed), NULL);
    id2 = g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(priorityChanged), NULL);

    // set priority
    uint32_t priority;

    prefs->get(PRIORITY_ENCODING, &priority);

#ifndef _WIN32
    //check for root privileges
    if (getuid() == 0)
    {
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), priority);
    }
#else
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), priority);
#endif

    GtkWidget* window = GW(windowEncoding);
    GtkRequisition req;
    gtk_widget_size_request(window, &req);
    // avoid window resizing when the text labels change their width
    gtk_widget_set_size_request(window, req.width, -1);
    gtk_register_dialog(window);
    gtk_widget_show_all(window);
}

/**
    \fn ~DIA_encodingGtk
*/
DIA_encodingGtk::~DIA_encodingGtk()
{
    gtk_unregister_dialog(GW(windowEncoding));
    gtk_widget_destroy(GW(windowEncoding));
}

/**
    \fn    setFps(uint32_t fps)
    \brief Display encoding speed in frames per second
*/
void DIA_encodingGtk::setFps(uint32_t fps)
{
    snprintf(stringMe, 79, "%"PRIu32, fps);
    WRITE(labelFps);
}

/**
    \fn    setPhasis(const char *n)
    \brief Display parameters as phase
*/
void DIA_encodingGtk::setPhasis(const char *n)
{
    gtk_window_set_title(GTK_WINDOW(GW(windowEncoding)), n);
}

/**
    \fn    setFrameCount
    \brief display the # of processed frames
*/
void DIA_encodingGtk::setFrameCount(uint32_t nb)
{
    snprintf(stringMe, 79, "%"PRIu32, nb);
    WRITE(labelFrames);
}

/**
    \fn    setPercent
    \brief display percent of saved file
*/
void DIA_encodingGtk::setPercent(uint32_t p)
{
    printf("Percent: %u\n", p);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(GW(progressbar)), (gdouble)p/100.0);
    ADM_slaveReportProgress(p);
    UI_purge();
}

/**
    \fn    setAudioCodec(const char *n)
    \brief Display parameters as audio codec
*/
void DIA_encodingGtk::setAudioCodec(const char *n)
{
    WRITEM(labelAudioCodec, n);
}

/**
    \fn    setVideoCodec(const char *n)
    \brief Display parameters as video codec
*/
void DIA_encodingGtk::setVideoCodec(const char *n)
{
    WRITEM(labelVideoCodec, n);
}

/**
    \fn    setBitrate(uint32_t br,uint32_t globalbr)
    \brief Display parameters as instantaneous bitrate and average bitrate
*/
void DIA_encodingGtk::setBitrate(uint32_t br,uint32_t globalbr)
{
    snprintf(stringMe, 79, "%"PRIu32" kb/s", br);
    WRITE(labelBitrate);
}

/**
    \fn    setContainer(const char *container)
    \brief Display parameter as container field
*/
void DIA_encodingGtk::setContainer(const char *container)
{
    WRITEM(labelContainer, container);
}

/**
    \fn    setQuantIn(int size)
    \brief display parameter as quantizer
*/
void DIA_encodingGtk::setQuantIn(int size)
{
    sprintf(stringMe, "%"PRIu32, size);
    WRITE(labelQuantiser);
}

/**
    \fn    setTotalSize(int size)
    \brief display parameter as total size
*/
void DIA_encodingGtk::setTotalSize(uint64_t size)
{
    uint64_t mb = size>>20;
    sprintf(stringMe, "%"PRIu32" MB", (int)mb);
    WRITE(labelTotalSize);
}

/**
    \fn    setVideoSize(int size)
    \brief display parameter as total size
*/
void DIA_encodingGtk::setVideoSize(uint64_t size)
{
    uint64_t mb = size>>20;
    sprintf(stringMe, "%"PRIu32" MB", (int)mb);
    WRITE(labelVideoSize);
}

/**
    \fn    setAudioSizeIn(int size)
    \brief display parameter as audio size
*/
void DIA_encodingGtk::setAudioSize(uint64_t size)
{
    uint64_t mb = size>>20;
    sprintf(stringMe, "%"PRIu32" MB", (int)mb);
    WRITE(labelAudioSize);
}

/**
    \fn    setElapsedTimeMs(uint32_t nb)
    \brief display elapsed time since saving start
*/
void DIA_encodingGtk::setElapsedTimeMs(uint32_t nb)
{
    uint32_t h, m, s, mms;
    ms2time(nb, &h, &m, &s, &mms);
    sprintf(stringMe, "%02"PRIu32":%02"PRIu32":%02"PRIu32, h, m, s);
    WRITE(labelElapsed);
}

/**
    \fn    setRemainingTimeMS
    \brief display remaining time (ETA)
*/
void DIA_encodingGtk::setRemainingTimeMS(uint32_t nb)
{
    uint32_t h, m, s, mms;
    ms2time(nb, &h, &m, &s, &mms);
    sprintf(stringMe, "%02"PRIu32":%02"PRIu32":%02"PRIu32, h, m, s);
    WRITE(labelRemaining);
}

/**
    \fn    setAverageQz(int size)
    \brief display average quantizer used
*/
void DIA_encodingGtk::setAverageQz(uint32_t nb)
{
    snprintf(stringMe, 79, "%"PRIu32, nb);
    WRITE(labelQuantiser);
}

/**
    \fn    setAverageBitrateKbits(int size)
    \brief display average bitrate in kb/s
*/
void DIA_encodingGtk::setAverageBitrateKbits(uint32_t kb)
{
    snprintf(stringMe, 79, "%"PRIu32" kb/s", kb);
    WRITE(labelBitrate);
}

/**
    \fn    isAlive()
    \brief return 0 if the window was killed or cancel button press, 1 otherwise
*/
bool DIA_encodingGtk::isAlive()
{
    if (stopReq)
    {
        if (GUI_Confirmation_HIG(QT_TR_NOOP("_Resume"), 
                                 QT_TR_NOOP("The encoding is paused"), 
                                 QT_TR_NOOP("Do you want to resume or cancel the encoding?")))
        {
            stopReq=0;
        }
    }

    if (!stopReq) return true;       

    return false;
}

/**
        \fn createEncoding
*/
namespace ADM_GtkCoreUIToolkit
{
DIA_encodingBase *createEncoding(uint64_t duration,bool tray)
{
        return new DIA_encodingGtk(duration,tray);
}
}
