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
#include "swresize.h"
#include <gtk/gtk.h>

#define FILL_ENTRY(widget_name,string) 		{r=-1;   \
gtk_editable_delete_text(GTK_EDITABLE(widget_name), 0,-1);\
gtk_editable_insert_text(GTK_EDITABLE(widget_name), string, strlen(string), &r);}
#define CHECKBOX(x,y) if(TRUE==gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(x)))  \
			y=1; else y=0;
#define READ_ENTRY(widget_name)   gtk_editable_get_chars(GTK_EDITABLE (widget_name), 0, -1);

#define SPIN_GET(x,y) {y= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(x)) ;}
#define SPIN_SET(x,y)  {gtk_spin_button_set_value(GTK_SPIN_BUTTON(x),(gfloat)y) ;}


/*----------------------------------------------*/
static GtkWidget	*create_dialog1 (void);
static void drag(GtkWidget * scale, gpointer user_data);
static void read (void );
static void write (void );
/*----------------------------------------------*/
static GtkWidget *dialog;
static GtkWidget *menu_algo;
static GtkWidget *menu_source;
static GtkWidget *menu_dest;
static GtkWidget *hscale1;
static GtkWidget *checkbutton1;
static GtkWidget *label_errorx;
static GtkWidget *label_errory;
static GtkWidget *spinbutton_width;
static GtkWidget *spinbutton_height;
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
/**
    \fn DIA_resize
    \brief Dialog for resizer, gtk flavor
*/
bool         DIA_resize(uint32_t originalWidth,uint32_t originalHeight,uint32_t fps1000,swresize *resize)
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

	ow=originalWidth;
	oh=originalHeight;

	iw=resize->width;
	ih=resize->height;
	dialog=create_dialog1();
	//gtk_transient(dialog);
        gtk_register_dialog(dialog);
	erx=ery=0;

	double val;
	val=100.*iw;
	if(ow) val=val/ow;
	adj_angle=gtk_range_get_adjustment (GTK_RANGE(hscale1));
	gtk_adjustment_set_value( GTK_ADJUSTMENT(adj_angle),(  gdouble  ) val );

	// remember algo
	gtk_combo_box_set_active (GTK_COMBO_BOX (menu_algo), resize->algo);
    gtk_combo_box_set_active (GTK_COMBO_BOX (menu_source), resize->sourceAR);
    gtk_combo_box_set_active (GTK_COMBO_BOX (menu_dest), resize->targetAR);


	#define CONNECT(w,x) g_signal_connect(w, #x, G_CALLBACK(drag), NULL)

			CONNECT(hscale1,drag_data_received);
			CONNECT(hscale1,drag_motion);
			CONNECT(hscale1,drag_data_get);
			CONNECT(hscale1,drag_begin);

			g_signal_connect(adj_angle,"value_changed", G_CALLBACK(drag), NULL);

	write();

	ret=0;
	uint8_t stop=0;
	while(!stop)
	{
		switch(gtk_dialog_run(GTK_DIALOG(dialog)))
		{
			case GTK_RESPONSE_OK:
				gchar *s;

                SPIN_GET(spinbutton_width,resize->width);
                SPIN_GET(spinbutton_height,resize->height);
                resize->algo= gtk_combo_box_get_active(GTK_COMBO_BOX(menu_algo));
                resize->sourceAR=gtk_combo_box_get_active(GTK_COMBO_BOX(menu_source));
                resize->targetAR=gtk_combo_box_get_active(GTK_COMBO_BOX(menu_dest));
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
void drag(GtkWidget * scale, gpointer user_data)
{
  UNUSED_ARG(scale);
  UNUSED_ARG(user_data);

  double percent;
  float x,y;
  float sr_mul,dst_mul;

 int32_t xx,yy;



  percent = gtk_adjustment_get_value (adj_angle);
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
	y=y*1;

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
 	sar=gtk_combo_box_get_active(GTK_COMBO_BOX(menu_source));
	dar=gtk_combo_box_get_active(GTK_COMBO_BOX(menu_dest));
	// Read resizing method
	resizeMethod=gtk_combo_box_get_active(GTK_COMBO_BOX(menu_algo));
	// Read round to neareast 16
	CHECKBOX(checkbutton1,roundBool);
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
	gtk_label_set_text(GTK_LABEL(label_errorx),str);

	sprintf(str,"%2.2f",ery*100.);
	gtk_label_set_text(GTK_LABEL(label_errory),str);

}

//----------------------------
GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *table1;
  GtkWidget *table2;
  GtkWidget *label1;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *label5;
  GtkWidget *label6;
  GtkWidget *label7;
  GtkAdjustment *spinbutton_width_adj;
  GtkAdjustment *spinbutton_height_adj;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *applybutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Resize"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_resizable (GTK_WINDOW (dialog1), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (dialog1), 6);

  dialog_vbox1 = gtk_dialog_get_content_area (GTK_DIALOG (dialog1));
  gtk_box_set_spacing (GTK_BOX (dialog_vbox1), 12);
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 6);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);
  gtk_widget_show (vbox1);

  table1 = gtk_table_new (3, 4, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 12);
  gtk_box_pack_start (GTK_BOX (vbox1), table1, FALSE, FALSE, 0);
  gtk_widget_show (table1);

  label1 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Width:"));
  gtk_table_attach (GTK_TABLE (table1), label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);
  gtk_widget_show (label1);
   
  spinbutton_width_adj = gtk_adjustment_new (2, 0, 3000, 1, 10, 10);
  spinbutton_width = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_width_adj), 1, 0);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label1), spinbutton_width);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton_width), TRUE);
  gtk_table_attach (GTK_TABLE (table1), spinbutton_width, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (spinbutton_width);

  label2 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Height:"));
  gtk_table_attach (GTK_TABLE (table1), label2, 2, 3, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);
  gtk_widget_show (label2);

  spinbutton_height_adj = gtk_adjustment_new (1, 0, 3000, 1, 10, 10);
  spinbutton_height = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_height_adj), 1, 0);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label2), spinbutton_height);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton_height), TRUE);
  gtk_table_attach (GTK_TABLE (table1), spinbutton_height, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (spinbutton_height);

  hscale1 = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (100, 0, 100, 1, 1, 0)));
  gtk_scale_set_draw_value (GTK_SCALE (hscale1), FALSE);
  gtk_table_attach (GTK_TABLE (table1), hscale1, 0, 4, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (hscale1);

  label3 = gtk_label_new (QT_TR_NOOP("Error X:"));
  gtk_table_attach (GTK_TABLE (table1), label3, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);
  gtk_widget_show (label3);

  label_errorx = gtk_label_new (QT_TR_NOOP("0"));
  gtk_table_attach (GTK_TABLE (table1), label_errorx, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_errorx), 0, 0.5);
  gtk_widget_show (label_errorx);
  
  label4 = gtk_label_new (QT_TR_NOOP("Error Y:"));
  gtk_table_attach (GTK_TABLE (table1), label4, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);
  gtk_widget_show (label4);

  label_errory = gtk_label_new (QT_TR_NOOP("0"));
  gtk_table_attach (GTK_TABLE (table1), label_errory, 3, 4, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_errory), 0, 0.5);
  gtk_widget_show (label_errory);

  table2 = gtk_table_new (4, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 12);
  gtk_box_pack_start (GTK_BOX (vbox1), table2, FALSE, FALSE, 0);
  gtk_widget_show (table2);

  checkbutton1 = gtk_check_button_new_with_mnemonic (QT_TR_NOOP("16 _round up"));
  gtk_table_attach (GTK_TABLE (table2), checkbutton1, 0, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (checkbutton1);
  
  label5 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Source:"));
  gtk_table_attach (GTK_TABLE (table2), label5, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);
  gtk_widget_show (label5);

  menu_source = gtk_combo_box_text_new ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (label5), menu_source);
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (menu_source), QT_TR_NOOP("1:1"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (menu_source), QT_TR_NOOP("4:3"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (menu_source), QT_TR_NOOP("16:9"));
  gtk_table_attach (GTK_TABLE (table2), menu_source, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (menu_source);

  label6 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Destination:"));
  gtk_table_attach (GTK_TABLE (table2), label6, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);
  gtk_widget_show (label6);

  menu_dest = gtk_combo_box_text_new ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (label6), menu_dest);
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (menu_dest), QT_TR_NOOP("1:1"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (menu_dest), QT_TR_NOOP("4:3"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (menu_dest), QT_TR_NOOP("16:9"));
  gtk_table_attach (GTK_TABLE (table2), menu_dest, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (menu_dest);

  label7 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Method:"));
  gtk_table_attach (GTK_TABLE (table2), label7, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);
  gtk_widget_show (label7);

  menu_algo = gtk_combo_box_text_new ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (label7), menu_algo);
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (menu_algo), QT_TR_NOOP("Bilinear"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (menu_algo), QT_TR_NOOP("Bicubic"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (menu_algo), QT_TR_NOOP("Lanczos3"));
  gtk_table_attach (GTK_TABLE (table2), menu_algo, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (menu_algo);

  dialog_action_area1 = gtk_dialog_get_action_area (GTK_DIALOG (dialog1));
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);
  gtk_widget_show (dialog_action_area1);

  cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), cancelbutton1, GTK_RESPONSE_CANCEL);
  gtk_widget_show (cancelbutton1);

  applybutton1 = gtk_button_new_from_stock ("gtk-apply");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), applybutton1, GTK_RESPONSE_APPLY);
  gtk_widget_show (applybutton1);

  okbutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), okbutton1, GTK_RESPONSE_OK);
  gtk_widget_show (okbutton1);

  return dialog1;
}
