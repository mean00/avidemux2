/***************************************************************************
  FAC_toggle.cpp
  Handle dialog factory element : Toggle
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


class diaElemNotch : public diaElem
{
  uint32_t yesno;
public:
  
  diaElemNotch(uint32_t yes,const char *toggleTitle, const char *tip=NULL);
  virtual ~diaElemNotch() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void) {};
};


diaElemNotch::diaElemNotch(uint32_t yes,const char *toggleTitle, const char *tip)
  : diaElem(ELEM_NOTCH)
{
  yesno=yes;
  paramTitle=toggleTitle;
  this->tip=tip;
}

diaElemNotch::~diaElemNotch()
{
  
}
void diaElemNotch::setMe(void *dialog, void *opaque,uint32_t line)
{
  GtkWidget *widget;
  
  if(yesno)
    widget = gtk_image_new_from_stock ("gtk-apply", GTK_ICON_SIZE_BUTTON);
  else
    widget = gtk_image_new_from_stock ("gtk-cancel", GTK_ICON_SIZE_BUTTON);
  
  gtk_widget_show (widget);
  myWidget=(void *)widget;
  
  gtk_table_attach (GTK_TABLE (opaque), widget, 0, 1, line, line+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  GtkWidget *label;
  
  label = gtk_label_new_with_mnemonic (paramTitle);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show(label);
  
  gtk_table_attach (GTK_TABLE (opaque), label, 1, 2, line, line+1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
}
//****************************************************
} // End of namespace
//****************************Hoook*****************

diaElem  *gtkCreateNotch(uint32_t yes,const char *toggleTitle, const char *tip)
{
	return new  ADM_GtkFactory::diaElemNotch(yes,toggleTitle, tip);
}
void gtkDestroyNotch(diaElem *e)
{
	ADM_GtkFactory::diaElemNotch *a=(ADM_GtkFactory::diaElemNotch *)e;
	delete a;
}
//EOF
