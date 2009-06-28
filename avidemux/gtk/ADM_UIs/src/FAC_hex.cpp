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
class diaElemHex : public diaElem
{
  uint32_t dataSize;
  uint8_t  *data;
  
public:
  
  diaElemHex(const char *toggleTitle, uint32_t dataSize,uint8_t *data);
  virtual ~diaElemHex() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void) ;
  void finalize(void);
  int getRequiredLayout(void);
};

#define HEX_NB_LINE   8
#define HEX_NB_COLUMN 16

typedef struct hexStruct
{ 
    GtkWidget *grid;
    GtkWidget *entry[HEX_NB_LINE];
    uint8_t   *data;
    uint32_t  size;
    uint32_t  curOffset;
}hexStruct;

static void updateMe(hexStruct *s);
static void prev(void *z,hexStruct *s);
static void next(void *z,hexStruct *s);

diaElemHex::diaElemHex(const char *toggleTitle, uint32_t dataSize,uint8_t *data)
  : diaElem(ELEM_HEXDUMP)
{
  param=NULL;
  paramTitle=toggleTitle;
  this->tip=NULL;
  this->data=data;
  this->dataSize=dataSize;
   setSize(3);
}

diaElemHex::~diaElemHex()
{
  if(myWidget)
  {
     hexStruct *s=(hexStruct *)myWidget;
     delete s;
     myWidget=NULL;
  }
}
void diaElemHex::setMe(void *dialog, void *opaque,uint32_t line)
{
  GtkWidget *hexTable;
  GtkWidget *buttonP, *alignment1, *hbox1, *image1, *label1;
  GtkWidget *buttonN, *alignment2, *hbox2, *image2, *label2;
  uint8_t *tail=data+dataSize;
  
  hexTable=gtk_table_new(1,HEX_NB_LINE,0);
  gtk_widget_show(hexTable);
  
  
  gtk_table_attach (GTK_TABLE (opaque), hexTable, 0, 2, line, line+1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  hexStruct *s=new hexStruct;
  s->grid=hexTable;
  s->data=data;
  s->size=dataSize;
  s->curOffset=0;
  for(int i=0;i<HEX_NB_LINE;i++)
  {
    s->entry[i]=gtk_label_new("");
    gtk_label_set_selectable(GTK_LABEL(s->entry[i]), TRUE);
    gtk_misc_set_alignment (GTK_MISC (s->entry[i]), 0, 1);
    gtk_widget_show(s->entry[i]);
     gtk_table_attach (GTK_TABLE (hexTable), s->entry[i], 0, 1, i, i+1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  }
  myWidget=(void *)s;
  //*************************
  buttonP = gtk_button_new_with_mnemonic (QT_TR_NOOP("_Previous"));
  gtk_widget_show (buttonP);
  gtk_table_attach (GTK_TABLE (opaque), buttonP, 0, 1, line+1, line+2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
   g_signal_connect(GTK_OBJECT(buttonP), "clicked",
                    GTK_SIGNAL_FUNC(prev), s);
   
  buttonN = gtk_button_new_with_mnemonic (QT_TR_NOOP("_Next"));
  gtk_widget_show (buttonN);
  gtk_table_attach (GTK_TABLE (opaque), buttonN, 1, 2, line+1, line+2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
   g_signal_connect(GTK_OBJECT(buttonN), "clicked",
                    GTK_SIGNAL_FUNC(next), s);
#if 0
  alignment1 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment1);
  

  hbox1 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (alignment1), hbox1);

  
  
  GTK_WIDGET_SET_FLAGS (buttonP, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX (hbox1), buttonP, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (buttonP), alignment1);
  
  image1 = gtk_image_new_from_stock ("gtk-go-back", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox1), image1, FALSE, FALSE, 0);

  label1 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Previous"));
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox1), label1, FALSE, FALSE, 0);

  gtk_table_attach (GTK_TABLE (opaque), hbox1, 0, 1, line+1, line+2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  g_signal_connect(GTK_OBJECT(buttonP), "clicked",
                    GTK_SIGNAL_FUNC(prev), s);


  alignment2 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (buttonN), alignment2);

  hbox2 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox2);
  gtk_container_add (GTK_CONTAINER (alignment2), hbox2);

  buttonN = gtk_button_new ();
  gtk_widget_show (buttonN);
  gtk_box_pack_start (GTK_BOX (hbox2), buttonN, FALSE, FALSE, 0);
  GTK_WIDGET_SET_FLAGS (buttonN, GTK_CAN_DEFAULT);

  
  image2 = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image2);
  gtk_box_pack_start (GTK_BOX (hbox2), image2, FALSE, FALSE, 0);

  label2 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Next"));
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (hbox2), label2, FALSE, FALSE, 0);

  gtk_table_attach (GTK_TABLE (opaque), hbox2, 0, 1, line+2, line+3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  g_signal_connect(GTK_OBJECT(buttonN), "clicked",
                    GTK_SIGNAL_FUNC(next),  s);
#endif
  updateMe(s);
}
void prev(void *z,hexStruct *s)
{
  if(s->curOffset>=HEX_NB_COLUMN*4)
      s->curOffset-=HEX_NB_COLUMN*4;
  updateMe(s);
  
}
void next(void *z,hexStruct *s)
{
      s->curOffset+=HEX_NB_COLUMN*4;
  updateMe(s);
}
void updateMe(hexStruct *s)
{
  uint8_t *tail=s->data+s->size;
  uint8_t *cur;
  char string[3000];
  char *ptr;
  for(int i=0;i<HEX_NB_LINE;i++)
  {
     cur=s->data+HEX_NB_COLUMN*i+s->curOffset;
     
     sprintf(string,"%06x:",HEX_NB_COLUMN*i+s->curOffset);
     ptr=string+strlen(string);
     *ptr=0;
     for(int j=0;j<HEX_NB_COLUMN;j++)
     {
       if(cur<tail)
       {
          sprintf(ptr,"%02X ",*cur++);
       }
	   else
       {
		  sprintf(ptr,"XX ");
       }

	   ptr+=3;
     }

     gtk_label_set_text(GTK_LABEL(s->entry[i]),string);     
  }
}
void diaElemHex::getMe(void)
{
 
}
void diaElemHex::finalize(void) 
{

};

int diaElemHex::getRequiredLayout(void) { return 0; }

} // End of namespace
//****************************Hoook*****************

diaElem  *gtkCreateHex(const char *toggleTitle, uint32_t dataSize,uint8_t *data)
{
	return new  ADM_GtkFactory::diaElemHex(toggleTitle,dataSize,data);
}
void gtkDestroyHex(diaElem *e)
{
	ADM_GtkFactory::diaElemHex *a=(ADM_GtkFactory::diaElemHex *)e;
	delete a;
}
//EOF
