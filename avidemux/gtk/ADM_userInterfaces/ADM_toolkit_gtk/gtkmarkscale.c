#include "gtkmarkscale.h"

static GtkWidgetClass *parent_class = NULL;

static gboolean gtk_markscale_expose (GtkWidget *widget, GdkEventExpose *event);

static void gtk_markscale_realize (GtkWidget *widget);

G_DEFINE_TYPE (GtkMarkScale, gtk_markscale, GTK_TYPE_SCALE)


uint32_t markA;
uint32_t markB;
uint32_t nbFrames;

static void
gtk_markscale_class_init (GtkMarkScaleClass *class)
{
  GtkWidgetClass *widget_class;
  GtkRangeClass *range_class;
  GtkScaleClass *scale_class;
  parent_class = g_type_class_peek_parent(class);
  widget_class = GTK_WIDGET_CLASS (class);
  range_class = GTK_RANGE_CLASS (class);
  scale_class = GTK_SCALE_CLASS (class);
  
  range_class->slider_detail = "hscale";
  
  widget_class->expose_event = gtk_markscale_expose;
  widget_class->realize = gtk_markscale_realize;
}

static void
gtk_markscale_init (GtkMarkScale *markscale)
{
  GtkRange *range;

  range = GTK_RANGE (markscale);

  range->orientation = GTK_ORIENTATION_HORIZONTAL;
  range->flippable = TRUE;
  markA = 0;
  markB = 0;
  nbFrames = 0;
}

void gtk_markscale_setA(GtkWidget *widget, uint32_t a)
{
	markA = a;
}

void gtk_markscale_setB(GtkWidget *widget, uint32_t b)
{
	markB = b;
	GdkRectangle rect;
	rect.x = widget->allocation.x;
	rect.y = widget->allocation.y;
	rect.width = widget->allocation.width;
	rect.height = widget->allocation.height;
	gdk_window_invalidate_rect(widget->window, &rect, FALSE);
}

void gtk_markscale_setNbFrames(GtkWidget *widget, uint32_t total)
{
	nbFrames = total;
}

GtkWidget*
gtk_markscale_new (GtkAdjustment *adjustment)
{
  return g_object_new (GTK_TYPE_MARKSCALE, "adjustment", adjustment, NULL);
}

static void gtk_markscale_realize (GtkWidget *widget)
{
	parent_class->realize(widget);
	gtk_widget_set_size_request(widget, widget->allocation.width, widget->allocation.height+10);
}

static gboolean
gtk_markscale_expose (GtkWidget      *widget,
                   GdkEventExpose *event)
{
  GtkScale *scale;
  
  scale = GTK_SCALE (widget);

  if (GTK_WIDGET_CLASS (gtk_markscale_parent_class)->expose_event)
    GTK_WIDGET_CLASS (gtk_markscale_parent_class)->expose_event (widget, event);

      gint x, y;
      GtkStateType state_type;
      state_type = GTK_STATE_NORMAL;
      if (!GTK_WIDGET_IS_SENSITIVE (scale))
        state_type = GTK_STATE_INSENSITIVE;
	
	if (nbFrames>1)
	{
		GdkGC *gc = gdk_gc_new(widget->window);
	
		int width = widget->allocation.width;
		int start = widget->allocation.x + (int)floor((width-1)*((float)markA/(nbFrames-1)));
		int end =  widget->allocation.x + (int)floor((width-1)*((float)markB/(nbFrames-1)));
		int top = widget->allocation.y + 1;
		int bottom = widget->allocation.y + widget->allocation.height - 2;
		
	//mark A:
		gdk_draw_line (widget->window,
					   gc,
					   start,
					   top,
					   start+4,
					   top);
		gdk_draw_line (widget->window,
					   gc,
					   start,
					   top,
					   start,
					   top+4);
	
		gdk_draw_line (widget->window,
					   gc,
					   start,
					   bottom,
					   start+4,
					   bottom);
		gdk_draw_line (widget->window,
					   gc,
					   start,
					   bottom - 4,
					   start,
					   bottom);
	
	//mark B:
		gdk_draw_line (widget->window,
					   gc,
					   end,
					   top,
					   end-4,
					   top);
		gdk_draw_line (widget->window,
					   gc,
					   end,
					   top,
					   end,
					   top+4);
	
		gdk_draw_line (widget->window,
					   gc,
					   end,
					   bottom,
					   end-4,
					   bottom);
		gdk_draw_line (widget->window,
					   gc,
					   end,
					   bottom-4,
					   end,
					   bottom);
	}
  return FALSE;
}


#define __GTK_MARKSCALE_C__
