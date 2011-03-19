// This is for the simulated jog/shuttle control on the main window, but it
// can also be controlled by jog/shuttle controllers that exist in the
// physical world (which are handled in ADM_jogshuttle.cpp).

#include <gtk/gtk.h>
#include "jogshuttle.h"

#define JOG_SHUTTLE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), JOG_TYPE_SHUTTLE, JogShuttlePrivate))

G_DEFINE_TYPE (JogShuttle, jog_shuttle, GTK_TYPE_DRAWING_AREA);

static gboolean jog_shuttle_expose (GtkWidget *wheel, GdkEventExpose *event);
static gboolean jog_shuttle_button_press (GtkWidget *wheel, GdkEventButton *event);
static gboolean jog_shuttle_button_release (GtkWidget *wheel, GdkEventButton *event);
static gboolean jog_shuttle_motion_notify (GtkWidget *wheel, GdkEventMotion *event);

typedef struct _JogShuttlePrivate JogShuttlePrivate;

struct _JogShuttlePrivate
{
	gfloat pos[6];
	gboolean pressed;
	gboolean external_control;
	gfloat start;
	gfloat offset;
	gfloat value;
};

enum
{
	VALUE_CHANGED,
	LAST_SIGNAL
};

static guint jog_shuttle_signals[LAST_SIGNAL] = { 0 };

static void
jog_shuttle_class_init (JogShuttleClass *class)
{
#if 0
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

 	obj_class = G_OBJECT_CLASS (class);
	widget_class = GTK_WIDGET_CLASS (class);

	widget_class->button_press_event = jog_shuttle_button_press;
	widget_class->button_release_event = jog_shuttle_button_release;
	widget_class->motion_notify_event = jog_shuttle_motion_notify;
	widget_class->expose_event = jog_shuttle_expose;
	
	jog_shuttle_signals[VALUE_CHANGED] = g_signal_new (
		"value-changed",
		G_OBJECT_CLASS_TYPE (obj_class),
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET (JogShuttleClass, value_changed),
		NULL, NULL,
		gtk_marshal_NONE__NONE,
		G_TYPE_NONE, 0);

	g_type_class_add_private (obj_class, sizeof (JogShuttlePrivate));
#endif
}

static void
jog_shuttle_init (JogShuttle *wheel)
{
#if 0
	JogShuttlePrivate *priv;
	priv = JOG_SHUTTLE_GET_PRIVATE (wheel);
	
	priv->pressed = FALSE;
	priv->external_control = FALSE;
	priv->offset = 0;
	priv->value = 0;
	gtk_widget_add_events (GTK_WIDGET (wheel), GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
#endif
}

static void reset(GtkWidget *wheel)
{
#if 0
	JogShuttlePrivate *priv;
	priv = JOG_SHUTTLE_GET_PRIVATE (wheel);
	gfloat width = wheel->allocation.width;
	
	int i;
	for (i=0; i<6; i++)
	{
		priv->pos[i] = width / 6 * i;
	}
#endif
}

static gboolean
jog_shuttle_button_press (GtkWidget *wheel, GdkEventButton *event)
{
#if 0
	JogShuttlePrivate *priv;
	priv = JOG_SHUTTLE_GET_PRIVATE (wheel);
	
	priv->start = event->x;
	priv->pressed = TRUE;
#endif
	return FALSE;
}

static gboolean
jog_shuttle_button_release (GtkWidget *wheel, GdkEventButton *event)
{
#if 0
	JogShuttlePrivate *priv;
	priv = JOG_SHUTTLE_GET_PRIVATE (wheel);
	
	priv->pressed = FALSE;
	priv->offset = 0;
	GtkWidget *widget;
	GdkRegion *region;
	
	widget = GTK_WIDGET (wheel);
	
	if (!widget->window) return;

	region = gdk_drawable_get_clip_region (widget->window);
	gdk_window_invalidate_region (widget->window, region, TRUE);
	gdk_window_process_updates (widget->window, TRUE);

	gdk_region_destroy (region);
#endif

	return FALSE;
}

static emit_value_changed_signal (JogShuttle *wheel)
{
#if 0
	g_signal_emit (wheel, jog_shuttle_signals[VALUE_CHANGED], 0);
#endif
}

static gboolean
jog_shuttle_motion_notify (GtkWidget *wheel, GdkEventMotion *event)
{
#if 0
	JogShuttlePrivate *priv;
	priv = JOG_SHUTTLE_GET_PRIVATE (wheel);
	
	gfloat width = wheel->allocation.width;
	gfloat height = wheel->allocation.height;
	
	if (!priv->pressed) return TRUE;
	GtkWidget *widget;
	GdkRegion *region;
	
	widget = GTK_WIDGET (wheel);
	
	priv->offset = event->x - priv->start;
	priv->start = event->x;

	if (!widget->window) return;
	
	if ((priv->offset+priv->pos[3]) > width || (priv->offset+priv->pos[3]) < 0) return;
	
	region = gdk_drawable_get_clip_region (widget->window);
	gdk_window_invalidate_region (widget->window, region, TRUE);
	gdk_window_process_updates (widget->window, TRUE);

	gdk_region_destroy (region);
#endif
}

static void
draw_borders (GtkWidget *wheel, cairo_t *cr)
{
#if 0
	gfloat width = wheel->allocation.width;
	gfloat height = wheel->allocation.height;
	gfloat middle = width/2.0;
	
	cairo_set_line_width (cr, 1);
	cairo_set_source_rgb (cr, 0.53, 0.53, 0.53);
	cairo_line_to (cr, 1, 0);
	cairo_line_to (cr, width-1, 0);
	cairo_stroke (cr);
	cairo_line_to (cr, 1, height-1);
	cairo_line_to (cr, width-1, height-1);
	cairo_stroke (cr);
	cairo_line_to (cr, 1, 1);
	cairo_line_to (cr, 1, height-1);
	cairo_stroke (cr);
	cairo_line_to (cr, width, 1);
	cairo_line_to (cr, width, height-1);
	cairo_stroke (cr);
	
	cairo_pattern_t *cp1 = cairo_pattern_create_linear (1, 1, middle-6, 1);
	cairo_pattern_add_color_stop_rgb (cp1, 0, 0.00, 0.00, 0.00);
	cairo_pattern_add_color_stop_rgb (cp1, 1, 0.98, 0.98, 0.98);
	cairo_pattern_t *cp2 = cairo_pattern_create_linear (width-1, 1, middle+6, 1);
	cairo_pattern_add_color_stop_rgb (cp2, 0, 0.00, 0.00, 0.00);
	cairo_pattern_add_color_stop_rgb (cp2, 1, 0.98, 0.98, 0.98);
	cairo_pattern_t *cp3 = cairo_pattern_create_linear (1, height-2, middle-6, height-2);
	cairo_pattern_add_color_stop_rgb (cp3, 1, 0.98, 0.98, 0.98);
	cairo_pattern_add_color_stop_rgb (cp3, 0, 0.00, 0.00, 0.00);
	cairo_pattern_t *cp4 = cairo_pattern_create_linear (width-1, height-2, middle+6, height-2);
	cairo_pattern_add_color_stop_rgb (cp4, 1, 0.98, 0.98, 0.98);
	cairo_pattern_add_color_stop_rgb (cp4, 0, 0.00, 0.00, 0.00);

	cairo_set_line_width (cr, 1);
	cairo_set_source (cr, cp1);
	cairo_line_to (cr, 1, 1);
	cairo_line_to (cr, middle, 1);
	cairo_stroke (cr);
	cairo_set_source (cr, cp3);
	cairo_line_to (cr, 1, height-2);
	cairo_line_to (cr, middle, height-2);
	cairo_stroke (cr);
	cairo_set_source (cr, cp2);
	cairo_line_to (cr, middle, 1);
	cairo_line_to (cr, width-1, 1);
	cairo_stroke (cr);
	cairo_set_source (cr, cp4);
	cairo_line_to (cr, middle, height-2);
	cairo_line_to (cr, width-1, height-2);
	cairo_stroke (cr);
	cairo_pattern_destroy(cp1);
	cairo_pattern_destroy(cp2);
	cairo_pattern_destroy(cp3);
	cairo_pattern_destroy(cp4);
#endif
}

static void
draw_background (GtkWidget *wheel, cairo_t *cr)
{
#if 0
	gfloat width = wheel->allocation.width;
	gfloat height = wheel->allocation.height;
	gfloat middle = width/2.0;

	cairo_set_line_width (cr, 0);
	cairo_pattern_t *base1 = cairo_pattern_create_linear (1, 1, middle-5, 1);
	cairo_pattern_add_color_stop_rgb (base1, 0, 0.33, 0.33, 0.33);
	cairo_pattern_add_color_stop_rgb (base1, 1, 0.88, 0.88, 0.88);
	cairo_set_source (cr, base1);
	cairo_rectangle (cr, 1, 0, middle-1, height-1);
	cairo_fill_preserve (cr);
	cairo_stroke (cr);
	cairo_pattern_t *base2 = cairo_pattern_create_linear (width-1, 1, middle+5, 1);
	cairo_pattern_add_color_stop_rgb (base2, 0, 0.33, 0.33, 0.33);
	cairo_pattern_add_color_stop_rgb (base2, 1, 0.88, 0.88, 0.88);
	cairo_set_source (cr, base2);
	cairo_rectangle (cr, middle, 0, middle-1, height-1);
	cairo_fill_preserve (cr);
	cairo_stroke (cr);
	cairo_pattern_destroy(base1);
	cairo_pattern_destroy(base2);
#endif
}

static void
draw_edges (GtkWidget *wheel, cairo_t *cr)
{
#if 0
	gfloat width = wheel->allocation.width;
	gfloat height = wheel->allocation.height;

	cairo_set_line_width (cr, 0);
	cairo_pattern_t *cp5 = cairo_pattern_create_linear (1, 2, 6, 2);
	cairo_pattern_add_color_stop_rgb (cp5, 0, 0.00, 0.00, 0.00);
	cairo_pattern_add_color_stop_rgb (cp5, 1, 0.23, 0.23, 0.23);
	cairo_set_source (cr, cp5);
	cairo_rectangle (cr, 1, 2, 6, height-4);
	cairo_fill_preserve (cr);
	cairo_stroke (cr);
	cairo_set_line_width (cr, 0);
	cairo_pattern_t *cp6 = cairo_pattern_create_linear (width-1, 2, width-6, 2);
	cairo_pattern_add_color_stop_rgb (cp6, 0, 0.00, 0.00, 0.00);
	cairo_pattern_add_color_stop_rgb (cp6, 1, 0.23, 0.23, 0.23);
	cairo_set_source (cr, cp6);
	cairo_rectangle (cr, width-7, 2, 6, height-4);
	cairo_fill_preserve (cr);
	cairo_stroke (cr);
	cairo_pattern_destroy(cp5);
	cairo_pattern_destroy(cp6);
#endif
}	

static void
draw_lines (GtkWidget *wheel, cairo_t *cr)
{
#if 0
	JogShuttlePrivate *priv;
	priv = JOG_SHUTTLE_GET_PRIVATE (wheel);
	
	gfloat width = wheel->allocation.width;
	gfloat height = wheel->allocation.height;
	gfloat middle = width/2.0;

	int i;
	for (i=0; i<6; i++)
	{
		priv->pos[i] += priv->offset;
		if (priv->pos[i] > width)
		{
			priv->pos[i] -= width;
		}
		else if (priv->pos[i] < 0)
		{
			priv->pos[i] = width + priv->pos[i];
		}
		
		gfloat color;
		if (priv->pos[i] <= middle)
		{
			color = priv->pos[i] / middle * 0.85;
		}
		else
		{
			color = (width-priv->pos[i]) / middle * 0.85;
		}
		if (i==3)
		{
 			cairo_set_source_rgb (cr, color, 0, 0);
 			cairo_set_line_width (cr, 2);
		}
		else
		{
			cairo_set_source_rgb (cr, color, color, color);
			cairo_set_line_width (cr, 1);
		}
		cairo_line_to (cr, priv->pos[i], 2);
		cairo_line_to (cr, priv->pos[i], height-2);
		cairo_stroke (cr);
	}
	
	priv->value = (priv->pos[3]-middle) / middle;
        //printf ("pos[3] = %f\n", priv->pos[3]);
 	emit_value_changed_signal (JOG_SHUTTLE(wheel));
#endif
}

static gboolean
jog_shuttle_expose (GtkWidget *wheel, GdkEventExpose *event)
{
#if 0
	JogShuttlePrivate *priv;
	priv = JOG_SHUTTLE_GET_PRIVATE (wheel);
	
	if (!priv->pressed && !priv->external_control)
	{
		reset(wheel);
	}
	
	cairo_t *cr;
	cr = gdk_cairo_create (wheel->window);

	cairo_rectangle (cr, event->area.x, event->area.y, event->area.width, event->area.height);
	cairo_clip (cr);
	cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
	draw_background (wheel, cr);
	draw_lines (wheel, cr);
	draw_borders (wheel, cr);
	draw_edges (wheel, cr);

	cairo_destroy (cr);
#endif
	return FALSE;
}

gfloat 
jog_shuttle_get_value (GtkWidget *wheel)
{
#if 0
	if (!wheel)
	    return 0;
	JogShuttlePrivate *priv;
	priv = JOG_SHUTTLE_GET_PRIVATE (wheel);
        if (!priv->pressed && !priv->external_control)
            return 0;
	return (priv->value);
#endif
}

void
jog_shuttle_set_value (GtkWidget *wheel, gfloat value)
{
#if 0
	JogShuttlePrivate *priv;
	priv = JOG_SHUTTLE_GET_PRIVATE (wheel);
        priv->external_control = (value < -0.001 || value > 0.001);
	priv->offset = 0;

	gfloat width = wheel->allocation.width;
        gfloat offset = value * (width / 2);
	
        gfloat was = priv->pos[3];
	int i;
	for (i=0; i<6; i++)
	{
		priv->pos[i] = width / 6 * i + offset;
	}

        //printf ("value = %f, offset = %f, pos[3] = %f (was %f)\n", value, offset, priv->pos[3], was);

	GtkWidget * widget = GTK_WIDGET (wheel);
	if (!widget->window)
            return;

	GdkRegion * region = gdk_drawable_get_clip_region (widget->window);
	gdk_window_invalidate_region (widget->window, region, TRUE);
	gdk_window_process_updates (widget->window, TRUE);
	gdk_region_destroy (region);
#endif
}

GtkWidget *
jog_shuttle_new (void)
{
	return g_object_new (JOG_TYPE_SHUTTLE, NULL);
}
