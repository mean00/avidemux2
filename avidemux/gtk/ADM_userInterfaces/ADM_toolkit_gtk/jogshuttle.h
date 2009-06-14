#ifndef __JOG_SHUTTLE_H__
#define __JOG_SHUTTLE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define JOG_TYPE_SHUTTLE		(jog_shuttle_get_type ())
#define JOG_SHUTTLE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), JOG_TYPE_SHUTTLE, JogShuttle))
#define JOG_SHUTTLE_CLASS(obj)	(G_TYPE_CHECK_CLASS_CAST ((obj), JOG_SHUTTLE, JogShuttleClass))
#define JOG_IS_SHUTTLE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), JOG_TYPE_SHUTTLE))
#define JOG_IS_SHUTTLE_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE ((obj), JOG_TYPE_SHUTTLE))
#define JOG_SHUTTLE_GET_CLASS	(G_TYPE_INSTANCE_GET_CLASS ((obj), JOG_TYPE_SHUTTLE, JogShuttleClass))

typedef struct _JogShuttle        JogShuttle;
typedef struct _JogShuttleClass   JogShuttleClass;

struct _JogShuttle
{
	GtkDrawingArea parent;
};

struct _JogShuttleClass
{
	GtkDrawingAreaClass parent_class;
	void (* value_changed) (JogShuttle *wheel);
};

GtkWidget *jog_shuttle_new (void);
gfloat jog_shuttle_get_value (GtkWidget *wheel);
void jog_shuttle_set_value (GtkWidget *wheel, gfloat value);

G_END_DECLS

#endif
