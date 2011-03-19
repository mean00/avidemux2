#include "config.h"
#include "GUI_glade.h"
#include "ADM_toolkitGtk.h"

uint8_t DIA_about( void )
{
    admGlade glade;
    glade.init();
    if (!glade.loadFile("about.gtkBuilder")) return 0;
    GtkWidget *dialog = glade.getWidget("aboutdialog");

    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG (dialog), VERSION);

    gtk_register_dialog(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));	
    gtk_unregister_dialog(dialog);
    gtk_widget_destroy(dialog);
    return 1;
}
