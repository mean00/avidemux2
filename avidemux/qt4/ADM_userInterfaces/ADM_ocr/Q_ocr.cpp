/***************************************************************************
                          Q_SRT.cpp  -  description
                             -------------------

    Handle the QT specific part of the fontsize & position dialog box
    copyright            : (C) 2002/2007 by mean
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

#include "Q_ocr.h"
#include "ADM_toolkitQt.h"

//*********************************************
     
void Ui_ocrWindow::setGlyph(admGlyph *glyph,admGlyph *head,char *decodedString)
{
	_glyph=glyph;
	_head=head;
	_decodedString=decodedString;
}

void Ui_ocrWindow::dialogReturn(ReplyType r)
{
	_reply=r;
	accept();
}

void Ui_ocrWindow::resizeSmall(uint32_t w,uint32_t h,uint8_t *smallData)
{
	smallCanvas->changeSize(w*2,h*2);
	smallCanvas->dataBuffer=smallData;
	QGraphicsView *graphicsView=ui.smallView;

	graphicsView->resize(w*2, h*2);
	smallCanvas->setMinimumSize(w*2,h*2);
	smallCanvas->resize(w*2, h*2); 
}

Ui_ocrWindow::Ui_ocrWindow(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	ui.textEdit->setReadOnly(TRUE);
	data=NULL;
	_w=_h=100;
	canvas=new ADM_QCanvas(ui.bigView,_w,_h);
	smallCanvas= new ADM_QCanvas(ui.smallView,_w,_h);
#define BUTTON(x)   connect(ui.x,SIGNAL(clicked(bool)),this,SLOT(x(bool)))
	BUTTON(pushButtonCalibrate);
	BUTTON(pushButtonSkipAll);
	BUTTON(pushButtonSkip);
	BUTTON(pushButtonIgnore);
	BUTTON(pushButtonOk);
	BUTTON(pushButtonClose);
	_glyph=_head=NULL;
	_decodedString=NULL;
}
//***********************************************

void  Ui_ocrWindow::pushButtonSkip(bool i)
  {
	  	dialogReturn(ReplySkip);
  }
void    Ui_ocrWindow::pushButtonSkipAll(bool i)
  {
	  	dialogReturn(ReplySkipAll);
  }
void  Ui_ocrWindow::pushButtonIgnore(bool i)
  {
	  	ADM_assert(_glyph);
	  	ADM_assert(_head);
	  	_glyph->code=NULL;
	  	insertInGlyphTree(_head,_glyph);
	  	dialogReturn(ReplyOk);
  }
void    Ui_ocrWindow::pushButtonClose(bool i)
  {
	  	dialogReturn(ReplyClose);
  }
void    Ui_ocrWindow::pushButtonCalibrate(bool i)
  {
	  	dialogReturn(ReplyCalibrate);
  }
void    Ui_ocrWindow::pushButtonOk(bool i)
  {
		// Retrieve content typed
	 		char *data=(ui.lineEdit->text()).toAscii().data(); // Memleak ??
	 		if(data&& strlen(data))
	                     {
	                         _glyph->code=ADM_strdup(data);
	                         insertInGlyphTree(_head,_glyph);
	                         strcat(_decodedString,_glyph->code);
	                     }
	  	dialogReturn(ReplyOk);
  }
//***********************************************
Ui_ocrWindow::~Ui_ocrWindow()
{
	delete canvas;
	canvas=NULL;
	
	delete smallCanvas;
	smallCanvas=NULL;
	
	if(data) delete [] data;
	data=NULL;	   
}

Ui_ocrWindow *gDialog=NULL;
/**
 * 	\fn ADM_ocrUpdateNbLines
 *  \brief Update the number of lines ocr'ed
 */
uint8_t ADM_ocrUpdateNbLines(void *ui,uint32_t cur,uint32_t total)
{
	return 1;
}
/**
 * 	\fn ADM_ocrUpdateNbGlyphs
 *  \brief Update the number of glyphs learnt (not used ATM)
 */

uint8_t ADM_ocrUpdateNbGlyphs(void *ui,uint32_t nbGlyphs)
{
	return 1;
}
/**
 * 	\fn ADM_ocrUpdateTextAndTime
 *  \brief Update the currently ocr'ed text from the current image
 */

uint8_t ADM_ocrUpdateTextAndTime(void *ui,char *decodedString,char *timeCode)
{
	Ui_ocrWindow *dialog=( Ui_ocrWindow *)ui;
	 ADM_assert(dialog==gDialog);
	 if(timeCode)
	 {
		 QLabel *labelTimecode=dialog->ui.labelTimecode;
		 labelTimecode->setText(timeCode);
	 }
	 if(decodedString)
	 {
		 QTextEdit *textedit=dialog->ui.textEdit;
		 textedit->clear();
		 textedit->textCursor().insertText(decodedString);
	 }
	 UI_purge();
	return 1;
}

static void convertBWtoRGB32(uint32_t w,uint32_t h,uint8_t *in,uint8_t *out)
{
	for(int y=0;y<h;y++)
		for(int x=0;x<w;x++)
		{
			uint8_t a=*in++;
			*(out+0)=a;
			*(out+1)=a;
			*(out+2)=a;
			*(out+3)=255; // alpha
			out+=4;
		}
}
static void convertBWtoRGB32Zoom(uint32_t w,uint32_t h,uint32_t strideW,uint32_t strideH,uint8_t *in,uint8_t *out)
{
	ADM_assert(strideW>=w);
	memset(out,0,strideW*strideH*4*4);
	for(int y=0;y<h;y++)
	{
		for(int x=0;x<w;x++)
		{
			uint8_t a=*in++;
			*(out+0)=a;
			*(out+1)=a;
			*(out+2)=a;
			*(out+3)=255; // alpha
			*(out+4)=a;
			*(out+5)=a;
			*(out+6)=a;
			*(out+7)=255; // alpha
			out+=8;
		}
		out+=(strideW-w)*8;
		memcpy(out,out-8*strideW,8*w);
		out+=8*strideW;
			
	}
}
/**
 * 	\fn ADM_ocrDrawFull
 *  \brief Redraw the full image
 *  @param data : uin8_t image to redraw (1 byte per pixel)
 */

uint8_t ADM_ocrDrawFull(void *d,uint8_t *data)
{
	Ui_ocrWindow *dialog=( Ui_ocrWindow *)d;
	ADM_assert(dialog==gDialog);
	uint8_t *out=dialog->data;
	uint8_t *in=data;
	uint32_t w=dialog->_w,h=dialog->_h;
	
	convertBWtoRGB32(w,h,in,out);
	
	// Paint!
	
	dialog->canvas->dataBuffer=dialog->data;
	
	
	QGraphicsView *graphicsView=dialog->ui.bigView;
	
	graphicsView->resize(w, h);
	dialog->canvas->setMinimumSize(w,h);
	dialog->canvas->resize(w, h);
	
	
	
	dialog->canvas->repaint();
	return 1;	
}
/**
 * 	\fn ADM_ocrSetRedrawSize
 *  \brief Set the new image dimensions. Redraw is disabled until ocrDrawFull is called with new datas
 *  @param w New image width
 *  @param h New image height
 */

uint8_t ADM_ocrSetRedrawSize(void *d,uint32_t w,uint32_t h)
{
	Ui_ocrWindow *dialog=( Ui_ocrWindow *)d;
	ADM_assert(dialog==gDialog);
	dialog->_w=w;
	dialog->_h=h;
	
	dialog->canvas->changeSize(w,h);
	
	if(dialog->data) delete [] dialog->data;
	dialog->data=NULL;
	dialog->data=new uint8_t[w*h*4];
	

	return 1;
}
/**
 * 	\fn ADM_ocrUiEnd
 *  \brief Destroy the UI for OCR
 */


uint8_t ADM_ocrUiEnd(void *d)
{
	   Ui_ocrWindow *dialog=( Ui_ocrWindow *)d;
	   ADM_assert(dialog==gDialog);

	   qtUnregisterDialog(dialog);

	   gDialog=NULL;
	   delete dialog;
	   return 1;	
}
/**
 * 	\fn ADM_ocrUiSetup
 *  \brief Create OCR UI
 */

void 	*ADM_ocrUiSetup(void)
{
	   Ui_ocrWindow *dialog=new Ui_ocrWindow(qtLastRegisteredDialog());
	   qtRegisterDialog(dialog);
	   dialog->setModal(TRUE);
	   dialog->show();
	   gDialog=dialog;
	   return dialog;
}
/**
 * 	\fn glyphToText
 *  \brief OCR one gluph
 *  @param glyph : Glyph to OCR
 *  @param head  : Pointer to the head of known glyph
 *  @param decodedString : String containing the current ocred text so far
 *  FIXME : Add ui as parameter!!
 */
#define MAX_W 32
#define MAX_H 32

ReplyType glyphToText(admGlyph *glyph,admGlyph *head,char *decodedString)
{
	// First draw small glyph
	uint32_t w,h,clipW,clipH;
	w=glyph->width;
	h=glyph->height;
	
	// Known glyph ?
	  if(glyph->width<2 && glyph->height<2)
	  {
	         // ????delete glyph;
	         return ReplyOk;
	   }
	  admGlyph *cand=NULL;
	  cand=searchGlyph(head,glyph);
	  if(cand) // New glyph?
	  {
		   if(cand->code)
			   		strcat(decodedString,cand->code);
		   	delete glyph;
		   	return ReplyOk;
	  }
	  // Yes, it is a new one
	//
#ifndef MAX
        #define MAX(a,b) a>b?a:b
#endif
	 clipW=MAX(MAX_W,w);
	 clipH=MAX(MAX_H,h);
	// Set datas too
	uint8_t smallData[clipW*clipH*4*4];
	// upscale & convert to RGB32 for display
	convertBWtoRGB32Zoom(w,h,clipW,clipH,glyph->data,smallData);
	//
	gDialog->resizeSmall(clipW,clipH,smallData);
	//
	gDialog->setGlyph(glyph,head,decodedString);
	//
	gDialog->ui.lineEdit->clear();
	gDialog->ui.lineEdit->setFocus(Qt::OtherFocusReason);
	ADM_ocrUpdateTextAndTime(gDialog,decodedString,NULL); // put ocred text so far
	//
    gDialog->exec();
    
    gDialog->smallCanvas->dataBuffer=NULL;
    // ???delete glyph;
	return gDialog->_reply;
}

//____________________________________
// EOF
