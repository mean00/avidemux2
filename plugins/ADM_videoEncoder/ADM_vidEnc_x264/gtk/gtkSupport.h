#ifndef gtkSupport_h
#define gtkSupport_h

#include <gtk/gtk.h>

GtkWidget *lookup_widget(GtkWidget *widget, const gchar *widget_name);

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

#endif	// gtkSupport_h
