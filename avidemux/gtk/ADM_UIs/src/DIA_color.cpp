#include "ADM_toolkitGtk.h"

int DIA_colorSel(uint8_t *r, uint8_t *g, uint8_t *b)
{
    GtkWidget *dialog = gtk_color_selection_dialog_new(QT_TR_NOOP("Select Color"));
    int ret = 0;
    GdkColor color;

    color.red = *r<<8;
    color.green = *g<<8;
    color.blue = *b<<8;

    GtkWidget *sel = gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(dialog));
    gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(sel), &color);

    if(GTK_RESPONSE_OK == gtk_dialog_run(GTK_DIALOG(dialog)))
    {
        gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(sel), &color);
        *r = color.red>>8;
        *g = color.green>>8;
        *b = color.blue>>8;
        ret = 1;
    }

    gtk_widget_destroy(dialog);
    return ret;
}
