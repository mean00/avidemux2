/***************************************************************************
  FAC_integer.cpp
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
class diaElemInteger : public diaElem
{

public:
  int32_t min,max;
  diaElemInteger(int32_t *intValue,const char *toggleTitle, int32_t min, int32_t max,const char *tip=NULL);
  virtual ~diaElemInteger() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
};
/* Same but unsigned */
class diaElemUInteger : public diaElem
{

public:
  uint32_t min,max;
  diaElemUInteger(uint32_t *intValue,const char *toggleTitle, uint32_t min, uint32_t max,const char *tip=NULL);
  virtual ~diaElemUInteger() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
  
};

diaElemInteger::diaElemInteger(int32_t *intValue,const char *toggleTitle, int32_t min, int32_t max,const char *tip)
  : diaElem(ELEM_INTEGER)
{
  param=(void *)intValue;
  paramTitle=toggleTitle;
  this->min=min;
  this->max=max;
  this->tip=tip;
}

diaElemInteger::~diaElemInteger()
{
  
}
void diaElemInteger::setMe(void *dialog, void *opaque,uint32_t line)
{
  GtkWidget *widget;
  GtkWidget *label;
  
  label = gtk_label_new_with_mnemonic (paramTitle);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show(label);
  
  gtk_table_attach (GTK_TABLE (opaque), label, 0, 1, line, line+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  int32_t val=*(int32_t *)param;
  widget = gtk_spin_button_new_with_range(min,max,1);
  gtk_entry_set_activates_default (GTK_ENTRY(widget), TRUE);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(widget),TRUE);
  gtk_spin_button_set_digits  (GTK_SPIN_BUTTON(widget),0);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(widget),val);
  
  gtk_widget_show (widget);
  
  gtk_table_attach (GTK_TABLE (opaque), widget, 1, 2, line, line+1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  gtk_label_set_mnemonic_widget (GTK_LABEL(label), widget);
  
  myWidget=(void *)widget;
  if(readOnly)
    gtk_widget_set_sensitive(widget,0);
   if(tip)
   {
      gtk_widget_set_tooltip_text (widget, tip);
   }
 
}
void diaElemInteger::getMe(void)
{
  GtkWidget *widget=(GtkWidget *)myWidget;
  int32_t *val=(int32_t *)param;
  ADM_assert(widget);
  *val=gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
  if(*val<min) *val=min;
  if(*val>max) *val=max;
  
}

void diaElemInteger::enable(uint32_t onoff)
{
  GtkWidget *widget=(GtkWidget *)myWidget;
  gtk_widget_set_sensitive(GTK_WIDGET(myWidget),onoff);
}

int diaElemInteger::getRequiredLayout(void) { return 0; }

//**********************************************************************************

diaElemUInteger::diaElemUInteger(uint32_t *intValue,const char *toggleTitle, uint32_t min, uint32_t max,const char *tip)
  : diaElem(ELEM_INTEGER)
{
  param=(void *)intValue;
  paramTitle=toggleTitle;
  this->min=min;
  this->max=max;
  this->tip=tip;
}

diaElemUInteger::~diaElemUInteger()
{
  
}
void diaElemUInteger::setMe(void *dialog, void *opaque,uint32_t line)
{
  GtkWidget *widget;
  GtkWidget *label;
  
  label = gtk_label_new_with_mnemonic (paramTitle);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show(label);
  
  gtk_table_attach (GTK_TABLE (opaque), label, 0, 1, line, line+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  uint32_t val=*(uint32_t *)param;
  widget = gtk_spin_button_new_with_range(min,max,1);
  gtk_entry_set_activates_default (GTK_ENTRY(widget), TRUE);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(widget),TRUE);
  gtk_spin_button_set_digits  (GTK_SPIN_BUTTON(widget),0);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(widget),val);
  
  gtk_widget_show (widget);
  
  gtk_table_attach (GTK_TABLE (opaque), widget, 1, 2, line, line+1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  gtk_label_set_mnemonic_widget (GTK_LABEL(label), widget);
  
  myWidget=(void *)widget;

  if(tip)
  {
      gtk_widget_set_tooltip_text (widget, tip);
  }
  
  if(readOnly)
      gtk_widget_set_sensitive(widget,0);

}
void diaElemUInteger::getMe(void)
{
  GtkWidget *widget=(GtkWidget *)myWidget;
  uint32_t *val=(uint32_t *)param;
  ADM_assert(widget);
  *val=gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
  if(*val<min) *val=min;
  if(*val>max) *val=max;

}
void diaElemUInteger::enable(uint32_t onoff)
{
  GtkWidget *widget=(GtkWidget *)myWidget;
  gtk_widget_set_sensitive(GTK_WIDGET(myWidget),onoff);
}

int diaElemUInteger::getRequiredLayout(void) { return 0; }

//****************************************************
} // End of namespace
//****************************Hoook*****************

diaElem  *gtkCreateInteger(int32_t *intValue,const char *toggleTitle,int32_t min, int32_t max,const char *tip)
{
	return new  ADM_GtkFactory::diaElemInteger(intValue,toggleTitle,min,max,tip);
}
void gtkDestroyInteger(diaElem *e)
{
	ADM_GtkFactory::diaElemInteger *a=(ADM_GtkFactory::diaElemInteger *)e;
	delete a;
}
diaElem  *gtkCreateUInteger(uint32_t *intValue,const char *toggleTitle,uint32_t min, uint32_t max,const char *tip)
{
	return new  ADM_GtkFactory::diaElemUInteger(intValue,toggleTitle,min,max,tip);
}
void gtkDestroyUInteger(diaElem *e)
{
	ADM_GtkFactory::diaElemUInteger *a=(ADM_GtkFactory::diaElemUInteger *)e;
	delete a;
}

//EOF
