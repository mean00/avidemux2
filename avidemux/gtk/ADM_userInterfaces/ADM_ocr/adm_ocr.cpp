/***************************************************************************
                         
        Very simple OCR engine

        We do it in 3 passes
        
                Ask the vobsub decoder for a bitmap
                Try to split the bitmap in lines
                For each lines try to split in glyph (i.e. horizontal line)
                Detour the glyph
                If the detoured glyph has a width much less inferiror to its width
                it probably means we have a italic or kerning.
                In that case use the detouring to isolate the glyphs


         A bit of warning. 
                        The UI code is ugly.
                        Bottom is the last actual line so to get height you have to to last-first +1 !

    begin                : Jan 2005
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
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

#include "DIA_coreToolkit.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "DIA_fileSel.h"

#if 0
//#include "ADM_videoFilter/ADM_vobsubinfo.h"
//#include "ADM_videoFilter/ADM_vidVobSub.h"
#include "ADM_ocr/ADM_leftturn.h"
#include "DIA_enter.h"

#include "ADM_ocr/ADM_ocr.h"
#include "ADM_ocr/ADM_ocrInternal.h"
/******************************/

#define TESTSUB "/home/fx/usbstick/subs/vts_01_0.idx"
#define CONNECT(x,y,z) 	gtk_signal_connect(GTK_OBJECT(WID(x)), #y,GTK_SIGNAL_FUNC(z),   NULL);


/* Minimal size of the current glyph display window */
#define MAX_W 65
#define MAX_H 65


extern  uint8_t DIA_vobsub(vobSubParam *param);
//********************************************
extern  GtkWidget *DIA_ocr(void);


//********************************************




typedef enum
{
    actionLoadVob=10,
    actionSaveSub,
    actionGo,
    actionLoadGlyph,
    actionSaveGlyph,
    actionSkip,
    actionSkipAll,
    actionAccept,
    actionIgnore,
    actionCalibrate
}ocrAction;


/************************ UI Dependant part ********************/
/**
        Search throught the existing glyphs , if not present create it
        and append the text to decodedString
*/

static GtkWidget *dialog=NULL;
static GtkWidget *mainDisplay=NULL;
static GtkWidget *smallDisplay=NULL;

static uint32_t clipW=0,clipH=0;

static gboolean   gui_draw( void );
static gboolean   gui_draw_small( void );
static void       displaySmall( admGlyph *glyph );
static int        cb_accept(GtkObject * object, gpointer user_data);


ReplyType glyphToText(admGlyph *glyph,admGlyph *head,char *decodedString)
{
 admGlyph *cand;
            //printf("2t: %d x %d\n",glyph->width,glyph->height);
            if(glyph->width<2 && glyph->height<2)
            {
                delete glyph;
                return ReplyOk;
            }
            cand=searchGlyph(head,glyph);
            if(!cand) // New glyph
            {
                char *string;
                // Draw it
                displaySmall(glyph); 
                gtk_label_set_text(GTK_LABEL(WID(labelText)),decodedString);
                gtk_editable_delete_text(GTK_EDITABLE(WID(entry)), 0,-1);
                
                //gtk_widget_set_sensitive(WID(buttonAccept),1);
                //gtk_widget_set_sensitive(WID(buttonSkip),1);
                //gtk_widget_set_sensitive(WID(entryEntry),1);
                
                gtk_widget_grab_focus (WID(entry));
                gtk_widget_grab_default (WID(buttonOk));
                
                //printf("i\n");
                switch(gtk_dialog_run(GTK_DIALOG(dialog)))
                {
                case actionIgnore:
                        glyph->code=NULL;
                        insertInGlyphTree(head,glyph);
                        //*nbGl++;
                        break;
                case actionCalibrate: return ReplyCalibrate;
                case actionAccept:
                    string =gtk_editable_get_chars(GTK_EDITABLE (WID(entry)), 0, -1);
                    if(string&& strlen(string))
                    {
                        glyph->code=ADM_strdup(string);
                        insertInGlyphTree(head,glyph);
                        //printf("New glyph:%s\n",glyph->code);
                        strcat(decodedString,glyph->code);
                        //*nbGl++;
                       
                    }
                    else delete glyph;
                    break;
                case actionSkip: //SKIP
                    return ReplySkip;
                    break;
                case actionSkipAll:
                    return ReplySkipAll;
                    break;
                case GTK_RESPONSE_CLOSE:
                  if(GUI_Question(QT_TR_NOOP("Sure ?"))) return ReplyClose;
                    break; // Abort
                    
                }
                gtk_editable_delete_text(GTK_EDITABLE(WID(entry)), 0,-1);
                //gtk_widget_set_sensitive(WID(buttonAccept),0);
                //gtk_widget_set_sensitive(WID(buttonSkip),0);
                //gtk_widget_set_sensitive(WID(entryEntry),0);
            }
            else
            {
                //printf("Glyph known :%s \n",cand->code);
                if(cand->code)
                    strcat(decodedString,cand->code);
                delete glyph;
            }
           return ReplyOk;  

}
static int sx=0,sy=0,sw=0,sh=0;
static int redraw_x=0,redraw_y=0;
static uint8_t *drawing=NULL;
static uint8_t *sdata=NULL;
uint8_t ADM_ocrSetRedrawSize(void *ui,uint32_t w,uint32_t h)
{
		redraw_x=w;
		redraw_y=h;
		return 1;
}

//*****************************************************************************************
gboolean gui_draw( void )
{
static int lastx=0,lasty=0;
	if(!drawing) return true;
    if(lastx!=redraw_x || lasty!=redraw_y)
        gtk_widget_set_usize(mainDisplay, redraw_x, redraw_y);
    lastx=redraw_x;
    lasty=redraw_y;
    
    gdk_draw_gray_image(mainDisplay->window, mainDisplay->style->fg_gc[GTK_STATE_NORMAL],
                        0,                          // X
                        0,                          // y
                        redraw_x,                          //width
                        redraw_y,                          //h*2, // heigth
                        GDK_RGB_DITHER_NONE,
                        drawing, // buffer
                        redraw_x );
    return true;
}
//*****************************************************************************************
 
void displaySmall( admGlyph *glyph)
{
    if(sw!=glyph->width || sh!=glyph->height)
    {
    	
        if(sdata) delete [] sdata;
        sdata=NULL;
        sw=glyph->width;
        sh=glyph->height;
        sdata=new uint8_t[(sw*2+2)*(sh*2+2)];
        clipW= sw*2+2;
        clipH=  sh*2+2;
#ifndef MAX
        #define MAX(a,b) a>b?a:b
#endif
        clipW=MAX(MAX_W,clipW);
        clipH=MAX(MAX_H,clipH);
        gtk_widget_set_usize(smallDisplay, clipW,clipH);
    }
    uint32_t stride=sw*2+2;
    uint8_t *in=glyph->data;
    uint8_t *out=sdata;
    
    memset(out,0,stride);
    out+=stride;
    for(uint32_t y=0;y<sh;y++)
    {
      *(out++)=0;
      for(uint32_t x=0;x<sw;x++)
      {
        out[1]=out[0]=out[stride]=out[stride+1]=*in;
        out+=2; 
        in++;
        
      } 
      *(out++)=0;
      out+=stride;      
    }
    memset(out,0,stride);
    //memcpy(sdata,glyph->data,sw*sh);
    gui_draw_small();
} 

gboolean gui_draw_small(void)
{ 
	if(clipW && clipH)
	{
		 gdk_draw_rectangle(smallDisplay->window,
				 				smallDisplay->style->fg_gc[GTK_STATE_NORMAL],
				 				1, // Filled
		                        0,                          // X
		                        0,                          // y
		                        clipW,
		                        clipH);
		
	
	}
 if(sw && sh && sdata)
 {
    gdk_draw_gray_image(smallDisplay->window, smallDisplay->style->fg_gc[GTK_STATE_NORMAL],
                        1,                          // X
                        1,                          // y
                        sw*2+1,                          //width
                        sh*2+1,                          //h*2, // heigth
                        GDK_RGB_DITHER_NONE,
                        sdata, // buffer
                        sw*2+2 );
 }
    return true;
}


/**
    \fn cb_accept
    \brief Bridge between dialog/Accept and gtk signals
*/
int cb_accept(GtkObject * object, gpointer user_data)
{
        //printf("Hopla\n");
        gtk_signal_emit_by_name(GTK_OBJECT(WID(buttonOk)),"clicked",(gpointer)1);
        return 0;
}
uint8_t ADM_ocrUpdateNbLines(void *ui,uint32_t cur,uint32_t total)
{
char text[50];         
         snprintf(text,50,"%03d/%03d",cur,total);
         gtk_label_set_text(GTK_LABEL(WID(labelNbLines)),text);
         return 1;
}
uint8_t ADM_ocrUpdateNbGlyphs(void *ui,uint32_t nbGlyphs)
{
char text[50];         
         snprintf(text,50,"%03d",nbGlyphs);
         gtk_label_set_text(GTK_LABEL(WID(labelNbGlyphs)),text);
         return 1;
}
uint8_t ADM_ocrUpdateTextAndTime(void *ui,char *decodedString,char *timeCode)
{
             gtk_label_set_text(GTK_LABEL(WID(labelText)),decodedString);
             gtk_label_set_text(GTK_LABEL(WID(labelTime)),timeCode);
             return 1;
}            
uint8_t ADM_ocrDrawFull(void *d,uint8_t *data)
{
		    drawing=data;;
			UI_purge();
             gui_draw();
             UI_purge();
             return 1;
}

uint8_t ADM_ocrUiEnd(void *d)
{
	GtkWidget *dialog=(GtkWidget *)d;
	ADM_assert(dialog);
		// Final round
	    gtk_widget_set_sensitive(WID(frameBitmap),0);
	   // gtk_widget_set_sensitive(WID(Current_Glyph),0);     
	  
	  
	    gtk_unregister_dialog(dialog);
	    gtk_widget_destroy(dialog);

}
void *ADM_ocrUiSetup(void)
{
	// Create UI && prepare callback
	 	dialog=NULL;
	    mainDisplay=NULL;
	    smallDisplay=NULL;
	    drawing=NULL;
	    sdata=NULL;
	    clipW=0;
	    clipH=0;
	    dialog=DIA_ocr();
	    gtk_register_dialog(dialog);
	#define ASSOCIATE(x,y)   gtk_dialog_add_action_widget (GTK_DIALOG (dialog), WID(x),y)
	    
	    ASSOCIATE(buttonOk,   actionAccept);
	    ASSOCIATE(buttonSkip,     actionSkip);
	    ASSOCIATE(buttonSkipAll,     actionSkipAll);
	    ASSOCIATE(buttonIgnore,   actionIgnore);
	    ASSOCIATE(buttonCalibrate,   actionCalibrate);
	    
	   
	    gtk_widget_show(dialog);
	    //
	    
	//  disable
	    
	    mainDisplay=WID(drawingareaBitmap);
	    smallDisplay=WID(drawingareaSmall);
	    
	   
	    
	    CONNECT(drawingareaBitmap,expose_event,gui_draw);
	    CONNECT(drawingareaSmall,expose_event,gui_draw_small);

	    CONNECT(entry,activate,cb_accept);
	    return (void *)dialog;
	
}
#endif
//;
