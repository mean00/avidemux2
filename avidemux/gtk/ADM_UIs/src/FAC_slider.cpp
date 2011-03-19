/***************************************************************************
  FAC_slider.cpp
  Handle dialog factory elements : Integer, Slider, and variations thereupon
  (C) 2006 Mean Fixounet@free.fr 
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


class diaElemUSlider : public diaElemSliderBase
{
  protected:
	  uint32_t min,max,incr;
public:
	diaElemUSlider(uint32_t *value,const char *toggleTitle, uint32_t min,uint32_t max,uint32_t incr = 1, const char *tip=NULL);
  virtual   ~diaElemUSlider() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
  
};
class diaElemSlider : public diaElemSliderBase
{
  protected:
    
	  int32_t min,max,incr;
    
public:
	diaElemSlider(int32_t *value,const char *toggleTitle, int32_t min,int32_t max,int32_t incr = 1, const char *tip=NULL);
  virtual   ~diaElemSlider() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
};

//****************************************************



diaElemSlider::diaElemSlider(int32_t *value,const char *toggleTitle, int32_t min,int32_t max,
				int32_t incr,const char *tip)
    : diaElemSliderBase()
    {
      this->min=min;
      this->max=max;
      this->incr=incr;
      incr=0;
    param = (void *)value;
    paramTitle = toggleTitle;
    this->tip = tip;
    size = 2;
}

diaElemSlider::~diaElemSlider()
{
}

void diaElemSlider::setMe(void *dialog, void *opaque,uint32_t line)
{
  GtkWidget *label = gtk_label_new_with_mnemonic (paramTitle);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show(label);
  
  gtk_table_attach (GTK_TABLE (opaque), label, 0, 2, line, line+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  int32_t val=*(int32_t  *)param;

  GtkAdjustment * adj = (GtkAdjustment *) gtk_adjustment_new (val, min, max, incr, incr, 0);

  GtkWidget *spinner = gtk_spin_button_new (adj, 1, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(spinner), TRUE);
  gtk_spin_button_set_digits  (GTK_SPIN_BUTTON(spinner), digits);
  
  GtkWidget *slider = gtk_hscale_new (adj);
  gtk_scale_set_draw_value (GTK_SCALE (slider), FALSE);
  gtk_scale_set_digits (GTK_SCALE (slider), digits);
  
  GtkWidget *hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (hbox), slider, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), spinner, FALSE, FALSE, 0);

  gtk_table_attach (GTK_TABLE (opaque), hbox, 0, 2, line+1, line+2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  gtk_label_set_mnemonic_widget (GTK_LABEL(label), hbox);
  gtk_widget_show (hbox);
  gtk_widget_show (spinner);
  gtk_widget_show (slider);
  
  myWidget=(void *)slider;
  if(readOnly)
  {
    gtk_widget_set_sensitive(spinner,0);
    gtk_widget_set_sensitive(slider,0);
  }
  if(tip)
  {
      gtk_widget_set_tooltip_text (spinner, tip);
      gtk_widget_set_tooltip_text (slider, tip);
  }
}

void diaElemSlider::getMe(void)
{
  GtkWidget *widget=(GtkWidget *)myWidget;
  uint32_t *val=(uint32_t *)param;
  ADM_assert(widget);
  GtkAdjustment *adj = gtk_range_get_adjustment (GTK_RANGE(widget));
  *val = (uint32_t)gtk_adjustment_get_value(adj);
  if(*val<min) *val=min;
  if(*val>max) *val=max;
}

void diaElemSlider::enable(uint32_t onoff)
{
  GtkWidget *widget=(GtkWidget *)myWidget;
  gtk_widget_set_sensitive(GTK_WIDGET(myWidget),onoff);
}

int diaElemSlider::getRequiredLayout(void) { return 0; }

//********************** USLIDER***********


diaElemUSlider::diaElemUSlider(uint32_t *value,const char *toggleTitle, 
		uint32_t min,uint32_t max,uint32_t incr,const char *tip)

    : diaElemSliderBase()
{
      this->min=min;
      this->max=max;
      this->incr=incr;
      digits=0;
    param = (void *)value;
    paramTitle = toggleTitle;
    this->tip = tip;
    size = 2;
}

diaElemUSlider::~diaElemUSlider()
{
}

void diaElemUSlider::setMe(void *dialog, void *opaque,uint32_t line)
{
  GtkWidget *label = gtk_label_new_with_mnemonic (paramTitle);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show(label);
  
  gtk_table_attach (GTK_TABLE (opaque), label, 0, 2, line, line+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  uint32_t val=*(uint32_t *)param;

  GtkAdjustment * adj = (GtkAdjustment *) gtk_adjustment_new (val, min, max, incr, incr, 0);

  GtkWidget *spinner = gtk_spin_button_new (adj, 1, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(spinner), TRUE);
  gtk_spin_button_set_digits  (GTK_SPIN_BUTTON(spinner), digits);
  
  GtkWidget *slider = gtk_hscale_new (adj);
  gtk_scale_set_draw_value (GTK_SCALE (slider), FALSE);
  gtk_scale_set_digits (GTK_SCALE (slider), digits);
  
  GtkWidget *hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (hbox), slider, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), spinner, FALSE, FALSE, 0);

  gtk_table_attach (GTK_TABLE (opaque), hbox, 0, 2, line+1, line+2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  gtk_label_set_mnemonic_widget (GTK_LABEL(label), hbox);
  gtk_widget_show (hbox);
  gtk_widget_show (spinner);
  gtk_widget_show (slider);
  
  myWidget=(void *)slider;
  if(readOnly)
  {
    gtk_widget_set_sensitive(spinner,0);
    gtk_widget_set_sensitive(slider,0);
  }
  if(tip)
  {
      gtk_widget_set_tooltip_text (spinner, tip);
      gtk_widget_set_tooltip_text (slider, tip);
  }
}

void diaElemUSlider::getMe(void)
{
  GtkWidget *widget=(GtkWidget *)myWidget;
  uint32_t *val=(uint32_t *)param;
  ADM_assert(widget);
  GtkAdjustment *adj = gtk_range_get_adjustment (GTK_RANGE(widget));
  *val = (uint32_t)gtk_adjustment_get_value(adj);
  if(*val<min) *val=min;
  if(*val>max) *val=max;
}

void diaElemUSlider::enable(uint32_t onoff)
{
  GtkWidget *widget=(GtkWidget *)myWidget;
  gtk_widget_set_sensitive(GTK_WIDGET(myWidget),onoff);
}

int diaElemUSlider::getRequiredLayout(void) { return 0; }

//****************************************************
} // End of namespace
//****************************Hoook*****************

diaElem  *gtkCreateSlider(int32_t *value,const char *toggleTitle, int32_t min,int32_t max,int32_t incr , const char *tip)
{
	return new  ADM_GtkFactory::diaElemSlider(value,toggleTitle,min,max,incr,tip);
}
void gtkDestroySlider(diaElem *e)
{
	ADM_GtkFactory::diaElemSlider *a=(ADM_GtkFactory::diaElemSlider *)e;
	delete a;
}
diaElem  *gtkCreateUSlider(uint32_t *value,const char *toggleTitle, uint32_t min,uint32_t max,uint32_t incr , const char *tip)
{
	return new  ADM_GtkFactory::diaElemUSlider(value,toggleTitle,min,max,incr,tip);
}
void gtkDestroyUSlider(diaElem *e)
{
	ADM_GtkFactory::diaElemUSlider *a=(ADM_GtkFactory::diaElemUSlider *)e;
	delete a;
}
//****************************************************
//EOF
