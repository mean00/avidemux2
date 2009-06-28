/***************************************************************************
  FAC_float.cpp
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
#include "DIA_fileSel.h"
//#include "prefs.h"
namespace ADM_GtkFactory
{
class diaElemFile : public diaElemFileBase
{

protected:
    
public:
  
  diaElemFile(uint32_t writeMode,char **filename,const char *toggleTitle,
              const char *defaultSuffix = 0,const char *tip=NULL);
  virtual ~diaElemFile() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  
  void   changeFile(void);
  void   enable(uint32_t onoff);
  int getRequiredLayout(void);
};
class diaElemDirSelect : public diaElemDirSelectBase
{

public:
  
  diaElemDirSelect(char **filename,const char *toggleTitle,const char *tip=NULL);
  virtual ~diaElemDirSelect() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  
  void changeFile(void);
  void   enable(uint32_t onoff);
  int getRequiredLayout(void);
};
static void fileRead(void *w,void *p);
static void dirSel(void *w,void *p);


diaElemFile::diaElemFile(uint32_t writemode,char **filename,const char *toggleTitle,
                         const char * defaultSuffix,const char *selectFileDesc)
  : diaElemFileBase()
    
{
  this->defaultSuffix=defaultSuffix;
  param=(void *)filename;
  paramTitle=toggleTitle;

  if (!selectFileDesc || strlen(selectFileDesc) == 0)
	  tip = toggleTitle;
  else
      tip = selectFileDesc;

  _write=writemode;
}

diaElemFile::~diaElemFile()
{
GtkWidget **wid=(GtkWidget **)myWidget;
        if(wid)
        {
            delete [] wid;
            myWidget=NULL;
        }
  
}
void diaElemFile::setMe(void *dialog, void *opaque,uint32_t line)
{
  GtkObject *adj;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *hbox1;
  GtkWidget *button;
  
  label = gtk_label_new_with_mnemonic (paramTitle);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show(label);
  
  gtk_table_attach (GTK_TABLE (opaque), label, 0, 1, line, line+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  /**/
  
  hbox1 = gtk_hbox_new (FALSE, 12);
  gtk_widget_show (hbox1);
  
  entry = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY(entry), TRUE);
  gtk_widget_show (entry);
  if(param)
  {
      char **val=(char **)param;
      gtk_entry_set_text (GTK_ENTRY (entry), *val);
  }
  gtk_label_set_mnemonic_widget (GTK_LABEL(label), entry);
  
  /* put entry in hbox */
  gtk_box_pack_start (GTK_BOX (hbox1), entry, TRUE, TRUE, 0); /* expand fill padding */
  
  /*  add button */
  
  button = gtk_button_new_with_mnemonic (QT_TR_NOOP("_Browse..."));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox1), button, FALSE, FALSE, 0);

  
  /**/
  gtk_table_attach (GTK_TABLE (opaque), hbox1, 1, 2, line, line+1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  
  /**/
  
  /* Add callback ...*/
  g_signal_connect(GTK_OBJECT(button), "clicked",
                    GTK_SIGNAL_FUNC(fileRead),  this);

  GtkWidget **w=new GtkWidget *[2];
  w[0]=entry;
  w[1]=button;
  myWidget=(void *)w;
  
}
void diaElemFile::getMe(void)
{
  GtkWidget **widget=(GtkWidget **)myWidget;
  char **name=(char **)param;
  if(*name) delete *name;
  *name =ADM_strdup(gtk_entry_get_text (GTK_ENTRY (widget[0])));
}

void diaElemFile::changeFile(void)
{
#define MAX_SEL 2040
  char buffer[MAX_SEL+1];
  uint8_t t=0;
  GtkWidget **wid=(GtkWidget **)myWidget;
  GtkWidget *widget=(GtkWidget *)wid[0];
  const char *txt;
  txt =gtk_entry_get_text (GTK_ENTRY (widget));
  
  if(_write)
  {
      char newname [1024];
      if (!*txt && defaultSuffix)
      {
          const char * lastfilename;
#if 0
#warning FIXME
          if (prefs->get(LASTFILES_FILE1,(ADM_filename **)&lastfilename))
          {
              strcpy (newname, lastfilename);
              char * cptr = newname + strlen (newname);
              while (cptr > newname)
              {
                  if (*cptr == '.')
                  {
                      strcpy (cptr + 1, defaultSuffix);
                      txt = newname;
                      printf ("Default output filename is %s based on "
                              "%s + %s\n",
                              txt, lastfilename, defaultSuffix);
                      break;
                  }
                  --cptr;
              }
          }
#endif
      }
      t = FileSel_SelectWrite(tip,buffer,MAX_SEL,txt);
  }
  else
  {
      t = FileSel_SelectRead(tip,buffer,MAX_SEL,txt);
  }
  if(t)
  {
    char **name=(char **)param;
    if(*name) delete [] *name;
    *name =ADM_strdup(buffer);
     gtk_entry_set_text (GTK_ENTRY (widget), *name);
  }
  
}

void   diaElemFile::enable(uint32_t onoff)
{
GtkWidget **wid=(GtkWidget **)myWidget;
     gtk_widget_set_sensitive(GTK_WIDGET(wid[0]),onoff);  
     gtk_widget_set_sensitive(GTK_WIDGET(wid[1]),onoff);  
}

int diaElemFile::getRequiredLayout(void) { return 0; }

void fileRead(void *w,void *p)
{
  diaElemFile *me=(diaElemFile *)p;
  me->changeFile();
}



/*********************************************************************/
diaElemDirSelect::diaElemDirSelect(char **filename,const char *toggleTitle,const char *selectDirDesc)
  : diaElemDirSelectBase()
{
  param=(void *)filename;
  paramTitle=toggleTitle;

  if (!selectDirDesc || strlen(selectDirDesc) == 0)
	  tip = toggleTitle;
  else
      tip = selectDirDesc;
}

diaElemDirSelect::~diaElemDirSelect()
{
  GtkWidget **wid=(GtkWidget **)myWidget;
        if(wid)
        {
            delete [] wid;
            myWidget=NULL;
        }
}
void diaElemDirSelect::setMe(void *dialog, void *opaque,uint32_t line)
{
  GtkObject *adj;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *hbox1;
  GtkWidget *button;
  
  label = gtk_label_new_with_mnemonic (paramTitle);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show(label);
  
  gtk_table_attach (GTK_TABLE (opaque), label, 0, 1, line, line+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  /**/
  
  hbox1 = gtk_hbox_new (FALSE, 12);
  gtk_widget_show (hbox1);
  
  entry = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY(entry), TRUE);
  gtk_widget_show (entry);
  if(param)
  {
      char **val=(char **)param;
      gtk_entry_set_text (GTK_ENTRY (entry), *val);
  }
  gtk_label_set_mnemonic_widget (GTK_LABEL(label), entry);
  
  /* put entry in hbox */
  gtk_box_pack_start (GTK_BOX (hbox1), entry, TRUE, TRUE, 0); /* expand fill padding */
  
  /*  add button */
  
  button = gtk_button_new_with_mnemonic (QT_TR_NOOP("_Browse..."));
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox1), button, FALSE, FALSE, 0);

  
  /**/
  gtk_table_attach (GTK_TABLE (opaque), hbox1, 1, 2, line, line+1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  
  /**/
  
  /* Add callback ...*/
  g_signal_connect(GTK_OBJECT(button), "clicked",
                    GTK_SIGNAL_FUNC(dirSel),  this);

  
   GtkWidget **w=new GtkWidget *[2];
  w[0]=entry;
  w[1]=button;
  myWidget=(void *)w;
  
}
void diaElemDirSelect::getMe(void)
{
GtkWidget **wid=(GtkWidget **)myWidget;
  GtkWidget *widget=wid[0];
  char **name=(char **)param;
  if(*name) delete *name;
  *name=NULL;
  *name =ADM_strdup(gtk_entry_get_text (GTK_ENTRY (widget)));
}

void diaElemDirSelect::changeFile(void)
{
#define MAX_SEL 2040
  char buffer[MAX_SEL+1];
  GtkWidget **wid=(GtkWidget **)myWidget;
  GtkWidget *widget=wid[0];

  
  const char *txt;
  txt =gtk_entry_get_text (GTK_ENTRY (widget));
  
  if(FileSel_SelectDir(this->tip,buffer,MAX_SEL,txt))
  {
    char **name=(char **)param;
    if(*name) delete [] *name;
    *name=NULL;
    *name =ADM_strdup(buffer);
     gtk_entry_set_text (GTK_ENTRY (widget), *name);
  }
  
}
void   diaElemDirSelect::enable(uint32_t onoff)
{
    GtkWidget **wid=(GtkWidget **)myWidget;
     gtk_widget_set_sensitive(GTK_WIDGET(wid[0]),onoff);  
     gtk_widget_set_sensitive(GTK_WIDGET(wid[1]),onoff);  

}

int diaElemDirSelect::getRequiredLayout(void) { return 0; }

void dirSel(void *w,void *p)
{
  diaElemDirSelect *me=(diaElemDirSelect *)p;
  me->changeFile();
}
} // End of namespace
//****************************Hoook*****************

diaElem  *gtkCreateFile(uint32_t writeMode,char **filename,const char *toggleTitle,
        const char *defaultSuffix ,const char *tip)
{
	return new  ADM_GtkFactory::diaElemFile(writeMode,filename,toggleTitle,defaultSuffix ,tip);
}
void gtkDestroyFile(diaElem *e)
{
	ADM_GtkFactory::diaElemFile *a=(ADM_GtkFactory::diaElemFile *)e;
	delete a;
}

diaElem  *gtkCreateDir(char **filename,const char *toggleTitle,const char *tip)
{
	return new  ADM_GtkFactory::diaElemDirSelect(filename,toggleTitle,tip);
}
void gtkDestroyDir(diaElem *e)
{
	ADM_GtkFactory::diaElemDirSelect *a=(ADM_GtkFactory::diaElemDirSelect *)e;
	delete a;
}
//EOF
