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

class diaElemReadOnlyText : public diaElem
{

public:
  
  diaElemReadOnlyText(const char *readyOnly,const char *toggleTitle,const char *tip=NULL);
  virtual ~diaElemReadOnlyText() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  int getRequiredLayout(void);
};

class diaElemText : public diaElem
{

public:
  
  diaElemText(char **text,const char *toggleTitle,const char *tip=NULL);
  virtual ~diaElemText() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  void enable(uint32_t onoff);
  int getRequiredLayout(void);
};


diaElemReadOnlyText::diaElemReadOnlyText(const char *readyOnly,const char *toggleTitle,const char *tip)
  : diaElem(ELEM_ROTEXT)
{
  param=(void *)ADM_strdup(readyOnly);
  paramTitle=ADM_strdup(toggleTitle);
  this->tip=tip;
}

diaElemReadOnlyText::~diaElemReadOnlyText()
{
  ADM_dealloc(param);
  ADM_dealloc(paramTitle);
}
void diaElemReadOnlyText::setMe(void *dialog, void *opaque,uint32_t line)
{
  GtkWidget *widget;
  GtkObject *adj;
  GtkWidget *label,*label2;
  
  label = gtk_label_new_with_mnemonic (paramTitle);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show(label);
  
  gtk_table_attach (GTK_TABLE (opaque), label, 0, 1, line, line+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  label2 = gtk_label_new_with_mnemonic ((char *)param);
  gtk_misc_set_alignment (GTK_MISC (label2), 0.0, 0.5);
  gtk_widget_show(label2);
  
 
  
  gtk_table_attach (GTK_TABLE (opaque), label2, 1, 2, line, line+1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  gtk_label_set_mnemonic_widget (GTK_LABEL(label), label2);
  
  myWidget=(void *)label2;
  
}
void diaElemReadOnlyText::getMe(void)
{
 
  
}

int diaElemReadOnlyText::getRequiredLayout(void) { return 0; }

/***************************************************************/
diaElemText::diaElemText(char **text,const char *toggleTitle,const char *tip)
  : diaElem(ELEM_ROTEXT)
{
  if(!*text) *text=ADM_strdup("");
  param=(void *)text;
  paramTitle=ADM_strdup(toggleTitle);
  
  this->tip=tip;
}

diaElemText::~diaElemText()
{
  ADM_dealloc(paramTitle);
}
void diaElemText::setMe(void *dialog, void *opaque,uint32_t line)
{
  GtkWidget *widget;
  GtkObject *adj;
  GtkWidget *label,*label2;
  char **input=(char **)param;
  label = gtk_label_new_with_mnemonic (paramTitle);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show(label);
  
  gtk_table_attach (GTK_TABLE (opaque), label, 0, 1, line, line+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  label2 = gtk_entry_new ();
  gtk_write_entry_string(label2, *input);
  gtk_widget_show(label2);
  
 
  
  gtk_table_attach (GTK_TABLE (opaque), label2, 1, 2, line, line+1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  gtk_label_set_mnemonic_widget (GTK_LABEL(label), label2);
  
  myWidget=(void *)label2;
  
}
void diaElemText::getMe(void)
{
  char **input=(char **)param;
  if(*input) ADM_dealloc(*input);
  *input=NULL;
  *input=ADM_strdup(gtk_editable_get_chars(GTK_EDITABLE (myWidget), 0, -1));
  
}

void diaElemText::enable(uint32_t onoff)
{
  GtkWidget *widget=(GtkWidget *)myWidget;
  gtk_widget_set_sensitive(GTK_WIDGET(myWidget),onoff);
}

int diaElemText::getRequiredLayout(void) { return 0; }
} // End of namespace
//****************************Hoook*****************

diaElem  *gtkCreateRoText(const char *text,const char *toggleTitle, const char *tip)
{
	return new  ADM_GtkFactory::diaElemReadOnlyText(text,toggleTitle,tip);
}
void gtkDestroyRoText(diaElem *e)
{
	ADM_GtkFactory::diaElemReadOnlyText *a=(ADM_GtkFactory::diaElemReadOnlyText *)e;
	delete a;
}

diaElem  *gtkCreateText(char **text,const char *toggleTitle, const char *tip)
{
	return new  ADM_GtkFactory::diaElemText(text,toggleTitle,tip);
}
void gtkDestroyText(diaElem *e)
{
	ADM_GtkFactory::diaElemText *a=(ADM_GtkFactory::diaElemText *)e;
	delete a;
}
//EOF
