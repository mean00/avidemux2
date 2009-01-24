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
class diaElemFrame : public diaElemFrameBase
{
  
public:
  
  diaElemFrame(const char *toggleTitle, const char *tip=NULL);
  virtual ~diaElemFrame() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void) ;
  void swallow(diaElem *widget);
  void enable(uint32_t onoff);
  void finalize(void);
};
diaElemFrame::diaElemFrame(const char *toggleTitle, const char *tip)
  : diaElemFrameBase()
{
  param=NULL;
  paramTitle=toggleTitle;
  this->tip=tip;
  nbElems=0;
  frameSize=0;
  setSize(1);
}
void diaElemFrame::swallow(diaElem *widget)
{
  elems[nbElems]=widget;
  frameSize+=widget->getSize();
 // setSize(frameSize);
  nbElems++;
  ADM_assert(nbElems<DIA_MAX_FRAME); 
  
}
diaElemFrame::~diaElemFrame()
{
  
}
void diaElemFrame::setMe(void *dialog, void *opaque,uint32_t line)
{
  
  GtkWidget *label;
  GtkWidget *table;
  GtkWidget *alignment;
  GtkWidget *vbox;
  char str[200];
 
  sprintf(str,"<b>%s</b>",paramTitle);
  label = gtk_label_new (str);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_widget_show(label);
  
  vbox = gtk_vbox_new (0, 0);
  alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 6, 0, 18, 0);
  
   table = gtk_table_new (frameSize, 2, FALSE);
   gtk_container_add (GTK_CONTAINER (alignment), table);
   
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  
  gtk_box_pack_start (GTK_BOX(vbox), label, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(vbox), alignment, FALSE, FALSE, 0);
  gtk_widget_show(table);
  gtk_widget_show(vbox);
   

   gtk_box_pack_start (GTK_BOX(opaque), vbox, FALSE, FALSE, 0);
    
    
  uint32_t v=0;
  for(int i=0;i<nbElems;i++)
  {
    elems[i]->setMe(dialog,table,v); 
    v+=elems[i]->getSize();
  }
  myWidget=(void *)table;
}
void diaElemFrame::getMe(void)
{
   for(int i=0;i<nbElems;i++)
  {
    elems[i]->getMe(); 
  }
}
void diaElemFrame::finalize(void)
{
   for(int i=0;i<nbElems;i++)
  {
    elems[i]->finalize(); 
  }
}
void diaElemFrame::enable(uint32_t onoff)
{
   GtkWidget *widget=(GtkWidget *)myWidget;
   gtk_widget_set_sensitive(GTK_WIDGET(myWidget),onoff);
}
} // End of namespace
//****************************Hoook*****************

diaElem  *gtkCreateFrame(const char *toggleTitle, const char *tip)
{
	return new  ADM_GtkFactory::diaElemFrame(toggleTitle,tip);
}
void gtkDestroyFrame(diaElem *e)
{
	ADM_GtkFactory::diaElemFrame *a=(ADM_GtkFactory::diaElemFrame *)e;
	delete a;
}
//EOF
//EOF
