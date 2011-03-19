/***************************************************************************
  FAC_threadCount.cpp
  Handle dialog factory element : ThreadCount
  (C) 2007 Gruntster
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_toolkitGtk.h"
#include "DIA_factory.h"


namespace ADM_GtkFactory
{

class diaElemThreadCount : public diaElem
{

public:
  
  diaElemThreadCount(uint32_t *value, const char *title, const char *tip = NULL);
  virtual ~diaElemThreadCount() ;
  void setMe(void *dialog, void *opaque, uint32_t line);
  void getMe(void);
  int getRequiredLayout(void);
};

static void customToggled(void *widget, void *userData);

diaElemThreadCount::diaElemThreadCount(uint32_t *value, const char *title, const char *tip) : diaElem(ELEM_THREAD_COUNT)
{
	param=(void *)value;
	paramTitle = title;
	this->tip=tip;
}

diaElemThreadCount::~diaElemThreadCount()
{
	GtkWidget *w = (GtkWidget*)myWidget;
	delete[] w;
	myWidget = NULL;
}

void diaElemThreadCount::getMe(void)
{
	GtkWidget **widgets = (GtkWidget**)myWidget;
	uint32_t *val = (uint32_t*)param;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets[0])))
		*val = 1;   // disabled
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets[1])))
		*val = 0;   // auto-detect
	else
		*val = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets[3]));   // custom
}

void diaElemThreadCount::setMe(void *dialog, void *opaque, uint32_t line)
{
	GtkWidget *label1;
	GtkWidget *hbox1;
	GtkWidget *radiobutton1;
	GSList *radiobutton_group = NULL;
	GtkWidget *radiobutton2;
	GtkWidget *hbox2;
	GtkWidget *radiobutton3;
	GtkAdjustment *spinbutton1_adj;
	GtkWidget *spinbutton1;

	label1 = gtk_label_new_with_mnemonic (paramTitle);
	gtk_widget_show (label1);
	gtk_misc_set_alignment (GTK_MISC (label1), 0.0, 0.5);

	gtk_table_attach (GTK_TABLE (opaque), label1, 0, 1, line, line+1,
					(GtkAttachOptions) (GTK_FILL),
					(GtkAttachOptions) (0), 0, 0);

	hbox1 = gtk_hbox_new (FALSE, 20);
	gtk_widget_show (hbox1);

	radiobutton1 = gtk_radio_button_new_with_mnemonic (NULL, QT_TR_NOOP("Disable"));
	gtk_widget_show (radiobutton1);
	gtk_box_pack_start (GTK_BOX (hbox1), radiobutton1, FALSE, FALSE, 0);

	radiobutton2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radiobutton1), QT_TR_NOOP("Auto-detect"));
	gtk_widget_show (radiobutton2);
	gtk_box_pack_start (GTK_BOX (hbox1), radiobutton2, FALSE, FALSE, 0);

	hbox2 = gtk_hbox_new (FALSE, 6);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (hbox1), hbox2, FALSE, FALSE, 0);

	radiobutton3 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radiobutton1), QT_TR_NOOP("Custom"));
	gtk_widget_show (radiobutton3);
	gtk_box_pack_start (GTK_BOX (hbox2), radiobutton3, TRUE, TRUE, 0);

	spinbutton1_adj = gtk_adjustment_new (2, 2, 32, 1, 10, 0);
	spinbutton1 = gtk_spin_button_new (spinbutton1_adj, 1, 0);
	gtk_widget_show (spinbutton1);
	gtk_box_pack_start (GTK_BOX (hbox2), spinbutton1, TRUE, TRUE, 0);
	gtk_entry_set_activates_default (GTK_ENTRY(spinbutton1), TRUE);

	gtk_table_attach (GTK_TABLE (opaque), hbox1, 1, 2, line, line+1,
					(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
					(GtkAttachOptions) (0), 0, 0);

	g_signal_connect (radiobutton3, "toggled", G_CALLBACK(customToggled), this);

	GtkWidget **w;
	w = new GtkWidget*[4];

	w[0] = radiobutton1;
	w[1] = radiobutton2;
	w[2] = radiobutton3;
	w[3] = spinbutton1;

	myWidget = (void *)w;

	uint32_t val = *(uint32_t *)param;

	gtk_widget_set_sensitive(GTK_WIDGET(spinbutton1), val > 1);

	if (val == 0)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton2), TRUE);
	else if (val == 1)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton1), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton3), TRUE);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON(spinbutton1), val);
	}
}

int diaElemThreadCount::getRequiredLayout(void) { return 0; }

void customToggled(void *widget, void *userData)
{
	diaElemThreadCount *elem = (diaElemThreadCount*)userData;

	GtkWidget **widgets = (GtkWidget**)(elem->myWidget);

	gtk_widget_set_sensitive(GTK_WIDGET(widgets[3]), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets[2])));
}
} // End of namesapce
diaElem  *gtkCreateThreadCount(uint32_t *value, const char *title, const char *tip)
{
	return new  ADM_GtkFactory::diaElemThreadCount(value,title,tip);
}
void gtkDestroyThreadCount(diaElem *e)
{
	ADM_GtkFactory::diaElemThreadCount *a=(ADM_GtkFactory::diaElemThreadCount *)e;
	delete a;
}
//EOF
