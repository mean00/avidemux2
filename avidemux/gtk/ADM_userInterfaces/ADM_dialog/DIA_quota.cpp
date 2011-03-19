#include "ADM_toolkitGtk.h"
#include "GUI_glade.h"

// Return 1 if resume, 0 if ignore
uint8_t DIA_quota( char * msg )
{
    admGlade glade;
    glade.init();
    if (!glade.loadFile("DIA_alternate.gtkBuilder")) return 0;

    GtkWidget *dialog = glade.getWidget("dialogAlternate");
    GtkWidget *message = glade.getWidget("labelMessage");
    GtkWidget *button1 = glade.getWidget("button1");
    GtkWidget *button2 = glade.getWidget("button2");
    gtk_button_set_label(GTK_BUTTON(button1), QT_TR_NOOP("_Ignore"));
    gtk_button_set_label(GTK_BUTTON(button1), QT_TR_NOOP("_Resume"));
    gchar *str = g_strconcat("<span weight=\"bold\" size=\"larger\">",
                             QT_TR_NOOP("Filesystem full / quota exceeded"),
                             "</span>\n\n", 
                             msg, 
                             NULL);
    gtk_label_set_text(GTK_LABEL(message), str);
    g_free(str);

    uint8_t ret=2;

    while(ret==2)
    {
        gtk_register_dialog(dialog);
        int i = gtk_dialog_run(GTK_DIALOG(dialog));
        switch(i)
        {
            case GTK_RESPONSE_OK:
                ret=0;	// ignore
                break;
            case GTK_RESPONSE_CANCEL:
                ret=1;	// resume
                break;
            default:
                ret=2;	// continue;
                break;
        }
        gtk_unregister_dialog(dialog);
        gtk_widget_destroy(dialog);
    };
    return ret;
}
