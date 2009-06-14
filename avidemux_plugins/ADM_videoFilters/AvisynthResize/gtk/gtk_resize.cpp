/*
	Resize GUI

*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <math.h>

#include "ADM_toolkitGtk.h"
#include "ADM_default.h"
#define aprintf(...) {}

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>


#include "ADM_gladeSupport.h"

#define FILL_ENTRY(widget_name,string) 		{r=-1;   \
gtk_editable_delete_text(GTK_EDITABLE(lookup_widget(dialog,#widget_name)), 0,-1);\
gtk_editable_insert_text(GTK_EDITABLE(lookup_widget(dialog,#widget_name)), string, strlen(string), &r);}
#define CHECKBOX(x,y) if(TRUE==gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog,#x))))  \
			y=1; else y=0;
#define READ_ENTRY(widget_name)   gtk_editable_get_chars(GTK_EDITABLE (lookup_widget(dialog,#widget_name)), 0, -1);

#define SPIN_GET(x,y) {y= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(WID(x))) ;}
#define SPIN_SET(x,y)  {gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(x)),(gfloat)y) ;}


/*----------------------------------------------*/
static GtkWidget	*create_dialog1 (void);
static void drag(GtkButton * button, gpointer user_data);
static void read (void );
static void write (void );
/*----------------------------------------------*/
static GtkWidget *dialog;
static uint32_t iw,ih,ow,oh;
static GtkAdjustment *adj_angle;
static gint  r;
static int sar;
static int dar;
static int resizeMethod;
static int roundBool;
static float erx,ery;
static double aspectRatio[2][3]={
								{1.,0.888888,1.19}, // NTSC 1:1 4:3 16:9
								{1.,1.066667,1.43} // PAL  1:1 4:3 16:9
							};
static int pal=1;
/*----------------------------------------------*/
uint8_t DIA_resize(uint32_t *width,uint32_t *height,uint32_t *alg,uint32_t originalw, uint32_t originalh,uint32_t fps1000)
{


	char str[100];
	uint8_t ret=0;


	if(fps1000>24600 && fps1000<25400)
	{
		aprintf("Pal\n");
		pal=1;
	}
	else
	{
		aprintf("NTSC\n");
		pal=0;
	}

	ow=originalw;
	oh=originalh;

	iw=*width;
	ih=*height;
	dialog=create_dialog1();
	//gtk_transient(dialog);
        gtk_register_dialog(dialog);
	erx=ery=0;

	double val;
	val=100.*iw;
	if(ow) val=val/ow;
	adj_angle=	gtk_range_get_adjustment (GTK_RANGE(WID(hscale1)));
	gtk_adjustment_set_value( GTK_ADJUSTMENT(adj_angle),(  gdouble  ) val );

	// remember algo
 	gtk_option_menu_set_history (GTK_OPTION_MENU (WID(optionmenu1)), *alg);


	#define CONNECT(w,x) gtk_signal_connect(GTK_OBJECT(lookup_widget(dialog,#w)), #x, \
		       GTK_SIGNAL_FUNC(drag), NULL)

		       	CONNECT(hscale1,drag_data_received);
			CONNECT(hscale1,drag_motion);
			CONNECT(hscale1,drag_data_get);
			CONNECT(hscale1,drag_begin);

			gtk_signal_connect(GTK_OBJECT(adj_angle),"value_changed",    GTK_SIGNAL_FUNC(drag), NULL);

	write();

	ret=0;
	uint8_t stop=0;
	while(!stop)
	{
		switch(gtk_dialog_run(GTK_DIALOG(dialog)))
		{
			case GTK_RESPONSE_OK:
				gchar *s;

                                SPIN_GET(spinbutton_width,*width);
                                SPIN_GET(spinbutton_height,*height);
				*alg= getRangeInMenu(lookup_widget(dialog,"optionmenu1"));
				ret=1;
				stop=1;
				break;
		default:			
		case GTK_RESPONSE_CANCEL:
				stop=1;
				break;
		case GTK_RESPONSE_APPLY:
				drag(NULL,NULL);
				break;
							
		}

	}
        gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);

	return ret;

}
/*
	Called when the hscale is changed
*/
void drag(GtkButton * button, gpointer user_data)
{
  UNUSED_ARG(button);
  UNUSED_ARG(user_data);

  double percent;
  float x,y;
  float sr_mul,dst_mul;

 int32_t xx,yy;



  percent = GTK_ADJUSTMENT(adj_angle)->value;
  if(percent<10.0) percent=10.;
  read();

  aprintf("drag called : %f \n",percent);
  x=ow;
  y=oh;
 erx=0;
 ery=0;
  sr_mul=1.;
  if(sar)
  	{  // source is 4/3 or 16/9
			sr_mul=aspectRatio[pal][sar];

	}

dst_mul=1.;
  if(dar)
  	{  // dst is 4/3 or 16/9

			dst_mul=1/aspectRatio[pal][dar];
	}
	aprintf("source mul %02.2f , dst mul : %02.2f\n",sr_mul,dst_mul);
	x=x*sr_mul*dst_mul;
	y=y;

	// normalize it to recover 100% width
	y=y/(x/ow);
	x=ow;

	aprintf("AR:x,y  : %03f %03f \n",x,y);

  	percent/=100.;
  	x=x*percent;
  	y=y*percent;


	aprintf("AR x,y  : %03f %03f \n",x,y);
  	xx=(uint32_t)floor(x+0.5);
	yy=(uint32_t)floor(y+0.5);

	if(xx&1) xx--;
	if(yy&1) yy--;


	if(roundBool)
	{
		int32_t ox=xx,oy=yy;
		xx=(xx +7) & 0xfffff0;
		yy=(yy +7) & 0xfffff0;

		erx=(xx-ox);
		erx=erx/xx;
		ery=(yy-oy);
		ery=ery/yy;

		aprintf("x: %d -> %d : err %f\n",ox,xx,erx);
		aprintf("y: %d -> %d : err %f\n",oy,yy,ery);
	}

	iw=xx;
	ih=yy;
	write();

  }
//---------------------------------------------
// 	read all value from dialog
//---------------------------------------------
void read (void )
{
	// Read source and dest aspect ratio
 	sar=getRangeInMenu(WID(optionmenu_source));
	dar=getRangeInMenu(WID(optionmenu_dest));
	// Read resizing method
	resizeMethod=getRangeInMenu(WID(optionmenu1));
	// Read round to neareast 16
	CHECKBOX(checkbutton_16,roundBool);

}
//---------------------------------------------
// 	update dialog with new value
//---------------------------------------------

void write (void )
{
 char str[100];

        SPIN_SET(spinbutton_width,iw);
        SPIN_SET(spinbutton_height,ih);

	sprintf(str,"%2.2f",erx*100.);
	gtk_label_set_text(GTK_LABEL(WID(label_errorx)),str);

	sprintf(str,"%2.2f",ery*100.);
	gtk_label_set_text(GTK_LABEL(WID(label_errory)),str);

}

//----------------------------
GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *table2;
  GtkWidget *label1;
  GtkWidget *label2;
  GtkObject *spinbutton_width_adj;
  GtkWidget *spinbutton_width;
  GtkObject *spinbutton_height_adj;
  GtkWidget *spinbutton_height;
  GtkWidget *label5;
  GtkWidget *label6;
  GtkWidget *optionmenu_source;
  GtkWidget *menu1;
  GtkWidget *item1_1;
  GtkWidget *_4_1;
  GtkWidget *_16_1;
  GtkWidget *optionmenu_dest;
  GtkWidget *menu2;
  GtkWidget *menuitem1;
  GtkWidget *menuitem2;
  GtkWidget *menuitem3;
  GtkWidget *label3;
  GtkWidget *label_errorx;
  GtkWidget *label7;
  GtkWidget *label_errory;
  GtkWidget *checkbutton_16;
  GtkWidget *optionmenu1;
  GtkWidget *menu3;
  GtkWidget *bilinear1;
  GtkWidget *bicubic1;
  GtkWidget *lanczos1;
  GtkWidget *alignment1;
  GtkWidget *fixed1;
  GtkWidget *fixed2;
  GtkWidget *hscale1;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *applybutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Resize"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  table2 = gtk_table_new (4, 4, FALSE);
  gtk_widget_show (table2);
  gtk_box_pack_start (GTK_BOX (vbox1), table2, FALSE, FALSE, 0);

  label1 = gtk_label_new (QT_TR_NOOP(" Width "));
  gtk_widget_show (label1);
  gtk_table_attach (GTK_TABLE (table2), label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

  label2 = gtk_label_new (QT_TR_NOOP(" Height "));
  gtk_widget_show (label2);
  gtk_table_attach (GTK_TABLE (table2), label2, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

  spinbutton_width_adj = gtk_adjustment_new (2, 0, 3000, 1, 10, 10);
  spinbutton_width = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_width_adj), 1, 0);
  gtk_widget_show (spinbutton_width);
  gtk_table_attach (GTK_TABLE (table2), spinbutton_width, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton_width), TRUE);

  spinbutton_height_adj = gtk_adjustment_new (1, 0, 3000, 1, 10, 10);
  spinbutton_height = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_height_adj), 1, 0);
  gtk_widget_show (spinbutton_height);
  gtk_table_attach (GTK_TABLE (table2), spinbutton_height, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton_height), TRUE);

  label5 = gtk_label_new (QT_TR_NOOP("Source :"));
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table2), label5, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  label6 = gtk_label_new (QT_TR_NOOP("Destination"));
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table2), label6, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  optionmenu_source = gtk_option_menu_new ();
  gtk_widget_show (optionmenu_source);
  gtk_table_attach (GTK_TABLE (table2), optionmenu_source, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  menu1 = gtk_menu_new ();

  item1_1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("1:1"));
  gtk_widget_show (item1_1);
  gtk_container_add (GTK_CONTAINER (menu1), item1_1);

  _4_1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("4:3"));
  gtk_widget_show (_4_1);
  gtk_container_add (GTK_CONTAINER (menu1), _4_1);

  _16_1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("16:9"));
  gtk_widget_show (_16_1);
  gtk_container_add (GTK_CONTAINER (menu1), _16_1);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu_source), menu1);

  optionmenu_dest = gtk_option_menu_new ();
  gtk_widget_show (optionmenu_dest);
  gtk_table_attach (GTK_TABLE (table2), optionmenu_dest, 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  menu2 = gtk_menu_new ();

  menuitem1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("1:1"));
  gtk_widget_show (menuitem1);
  gtk_container_add (GTK_CONTAINER (menu2), menuitem1);

  menuitem2 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("4:3"));
  gtk_widget_show (menuitem2);
  gtk_container_add (GTK_CONTAINER (menu2), menuitem2);

  menuitem3 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("16:9"));
  gtk_widget_show (menuitem3);
  gtk_container_add (GTK_CONTAINER (menu2), menuitem3);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu_dest), menu2);

  label3 = gtk_label_new (QT_TR_NOOP(" Error X:"));
  gtk_widget_show (label3);
  gtk_table_attach (GTK_TABLE (table2), label3, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);

  label_errorx = gtk_label_new (QT_TR_NOOP("0"));
  gtk_widget_show (label_errorx);
  gtk_table_attach (GTK_TABLE (table2), label_errorx, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_errorx), 0, 0.5);

  label7 = gtk_label_new (QT_TR_NOOP(" Error Y:"));
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table2), label7, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

  label_errory = gtk_label_new (QT_TR_NOOP("0"));
  gtk_widget_show (label_errory);
  gtk_table_attach (GTK_TABLE (table2), label_errory, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_errory), 0, 0.5);

  checkbutton_16 = gtk_check_button_new_with_mnemonic (QT_TR_NOOP("16 round up"));
  gtk_widget_show (checkbutton_16);
  gtk_table_attach (GTK_TABLE (table2), checkbutton_16, 3, 4, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  optionmenu1 = gtk_option_menu_new ();
  gtk_widget_show (optionmenu1);
  gtk_table_attach (GTK_TABLE (table2), optionmenu1, 3, 4, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  menu3 = gtk_menu_new ();

  bilinear1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("Bilinear"));
  gtk_widget_show (bilinear1);
  gtk_container_add (GTK_CONTAINER (menu3), bilinear1);

  bicubic1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("Bicubic"));
  gtk_widget_show (bicubic1);
  gtk_container_add (GTK_CONTAINER (menu3), bicubic1);

  lanczos1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("Lanczos3"));
  gtk_widget_show (lanczos1);
  gtk_container_add (GTK_CONTAINER (menu3), lanczos1);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu1), menu3);

  alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment1);
  gtk_table_attach (GTK_TABLE (table2), alignment1, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  fixed1 = gtk_fixed_new ();
  gtk_widget_show (fixed1);
  gtk_container_add (GTK_CONTAINER (alignment1), fixed1);

  fixed2 = gtk_fixed_new ();
  gtk_widget_show (fixed2);
  gtk_table_attach (GTK_TABLE (table2), fixed2, 2, 3, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hscale1 = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (100, 0, 100, 1, 1, 0)));
  gtk_widget_show (hscale1);
  gtk_box_pack_start (GTK_BOX (vbox1), hscale1, FALSE, FALSE, 0);
  gtk_scale_set_digits (GTK_SCALE (hscale1), 0);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), cancelbutton1, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton1, GTK_CAN_DEFAULT);

  applybutton1 = gtk_button_new_from_stock ("gtk-apply");
  gtk_widget_show (applybutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), applybutton1, GTK_RESPONSE_APPLY);
  GTK_WIDGET_SET_FLAGS (applybutton1, GTK_CAN_DEFAULT);

  okbutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), okbutton1, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);
/*
  g_signal_connect ((gpointer) _4_1, "activate",
                    G_CALLBACK (on_4_1_activate),
                    NULL);
  g_signal_connect ((gpointer) _16_1, "activate",
                    G_CALLBACK (on_16_1_activate),
                    NULL);
  g_signal_connect ((gpointer) menuitem2, "activate",
                    G_CALLBACK (on_4_1_activate),
                    NULL);
  g_signal_connect ((gpointer) menuitem3, "activate",
                    G_CALLBACK (on_16_1_activate),
                    NULL);
  g_signal_connect ((gpointer) bilinear1, "activate",
                    G_CALLBACK (on_bilinear1_activate),
                    NULL);
  g_signal_connect ((gpointer) bicubic1, "activate",
                    G_CALLBACK (on_bicubic1_activate),
                    NULL);
  g_signal_connect ((gpointer) lanczos1, "activate",
                    G_CALLBACK (on_lanczos1_activate),
                    NULL);
*/
  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, table2, "table2");
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT (dialog1, label2, "label2");
  GLADE_HOOKUP_OBJECT (dialog1, spinbutton_width, "spinbutton_width");
  GLADE_HOOKUP_OBJECT (dialog1, spinbutton_height, "spinbutton_height");
  GLADE_HOOKUP_OBJECT (dialog1, label5, "label5");
  GLADE_HOOKUP_OBJECT (dialog1, label6, "label6");
  GLADE_HOOKUP_OBJECT (dialog1, optionmenu_source, "optionmenu_source");
  GLADE_HOOKUP_OBJECT (dialog1, menu1, "menu1");
  GLADE_HOOKUP_OBJECT (dialog1, item1_1, "item1_1");
  GLADE_HOOKUP_OBJECT (dialog1, _4_1, "_4_1");
  GLADE_HOOKUP_OBJECT (dialog1, _16_1, "_16_1");
  GLADE_HOOKUP_OBJECT (dialog1, optionmenu_dest, "optionmenu_dest");
  GLADE_HOOKUP_OBJECT (dialog1, menu2, "menu2");
  GLADE_HOOKUP_OBJECT (dialog1, menuitem1, "menuitem1");
  GLADE_HOOKUP_OBJECT (dialog1, menuitem2, "menuitem2");
  GLADE_HOOKUP_OBJECT (dialog1, menuitem3, "menuitem3");
  GLADE_HOOKUP_OBJECT (dialog1, label3, "label3");
  GLADE_HOOKUP_OBJECT (dialog1, label_errorx, "label_errorx");
  GLADE_HOOKUP_OBJECT (dialog1, label7, "label7");
  GLADE_HOOKUP_OBJECT (dialog1, label_errory, "label_errory");
  GLADE_HOOKUP_OBJECT (dialog1, checkbutton_16, "checkbutton_16");
  GLADE_HOOKUP_OBJECT (dialog1, optionmenu1, "optionmenu1");
  GLADE_HOOKUP_OBJECT (dialog1, menu3, "menu3");
  GLADE_HOOKUP_OBJECT (dialog1, bilinear1, "bilinear1");
  GLADE_HOOKUP_OBJECT (dialog1, bicubic1, "bicubic1");
  GLADE_HOOKUP_OBJECT (dialog1, lanczos1, "lanczos1");
  GLADE_HOOKUP_OBJECT (dialog1, alignment1, "alignment1");
  GLADE_HOOKUP_OBJECT (dialog1, fixed1, "fixed1");
  GLADE_HOOKUP_OBJECT (dialog1, fixed2, "fixed2");
  GLADE_HOOKUP_OBJECT (dialog1, hscale1, "hscale1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, applybutton1, "applybutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}

