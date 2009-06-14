#ifndef __GTK_MARKSCALE_H__
#define __GTK_MARKSCALE_H__


#include <gdk/gdk.h>
#include <gtk/gtkscale.h>
#include <stdint.h>

G_BEGIN_DECLS

#define GTK_TYPE_MARKSCALE            (gtk_markscale_get_type ())
#define GTK_MARKSCALE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_MARKSCALE, GtkMarkScale))
#define GTK_MARKSCALE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_MARKSCALE, GtkMarkScaleClass))
#define GTK_IS_MARKSCALE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_MARKSCALE))
#define GTK_IS_MARKSCALE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_MARKSCALE))
#define GTK_MARKSCALE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_MARKSCALE, GtkMarkScaleClass))


typedef struct _GtkMarkScale       GtkMarkScale;
typedef struct _GtkMarkScaleClass  GtkMarkScaleClass;

struct _GtkMarkScale
{
  GtkScale scale;
};

struct _GtkMarkScaleClass
{
  GtkScaleClass parent_class;
};


GType      gtk_markscale_get_type       (void) G_GNUC_CONST;
GtkWidget* gtk_markscale_new            (GtkAdjustment *adjustment);
GtkWidget* gtk_markscale_new_with_range (gdouble        min,
                                      gdouble        max,
                                      gdouble        step);

void gtk_markscale_setA(GtkWidget *widget, uint32_t a);
void gtk_markscale_setB(GtkWidget *widget, uint32_t b);
void gtk_markscale_setNbFrames(GtkWidget *widget, uint32_t total);

G_END_DECLS

#endif /* __GTK_MARKSCALE_H__ */
