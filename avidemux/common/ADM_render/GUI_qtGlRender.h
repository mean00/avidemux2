/***************************************************************************
    copyright            : (C) 2010 by gruntster
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef GUI_QTGLRENDER_H
#define GUI_QTGLRENDER_H

#include <QtOpenGL/QGLWidget>
#include <QtOpenGL/QGLShader>

#include "GUI_render.h"
#include "GUI_accelRender.h"
#include "ADM_colorspace.h"
/**
    \class QtGlAccelWidget
*/
class QtGlAccelWidget : public QGLWidget
{
private:
	int             imageWidth, imageHeight;
    int             displayWidth,displayHeight;
	bool            firstRun;
    

	QGLShaderProgram *glProgram;
	GLsizei textureRealWidths[3];
    GLsizei textureStrides[3];
	GLsizei textureHeights[3];
	uint8_t *textureOffsets[3];
    GLuint  textureName[3];

protected:
	void initializeGL();
	void paintGL() __attribute__((force_align_arg_pointer));
    void updateTexture(void);

public:
	QtGlAccelWidget(QWidget *parent, int imagew, int imageh);
    ~QtGlAccelWidget();
	bool setImage(ADMImage *pic);
    bool setDisplaySize(int width,int height);
};

/**
    \fn class QtGlRender
*/
class QtGlRender: public VideoRenderBase
{
      protected:
                            
                            GUI_WindowInfo  info;
                            QtGlAccelWidget *glWidget;
      public:
                             QtGlRender( void ) ;
              virtual        ~QtGlRender();
              virtual	bool init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom);
              virtual	bool stop(void);				
              virtual   bool displayImage(ADMImage *pic);
              virtual   bool changeZoom(renderZoom newzoom);
              virtual   bool refresh(void);
              virtual   bool usingUIRedraw(void) {return false;}; // We can! redraw by ourself
};

#endif
