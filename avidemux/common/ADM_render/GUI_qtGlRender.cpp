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
***************************************************************************///
#define GL_GLEXT_PROTOTYPES

#include <QtGui/QPainter>

#ifdef __APPLE__
#	include <OpenGL/gl.h>
#	include <OpenGL/glext.h>
#	define GL_TEXTURE_RECTANGLE_NV GL_TEXTURE_RECTANGLE_EXT
#else
#	include <GL/gl.h>
#	include <GL/glext.h>
#endif
#define ADM_LEGACY_PROGGY // Dont clash with free/malloc etc..
#include "ADM_default.h"
#include "GUI_render.h"

#include "GUI_accelRender.h"
#include "GUI_qtGlRender.h"

static const char *yuvToRgb =
	"#extension GL_ARB_texture_rectangle: enable\n"
	"uniform sampler2DRect texY, texU, texV;\n"
	"uniform float height;\n"

	"void main(void) {\n"
	"  float nx = gl_TexCoord[0].x;\n"
	"  float ny = height - gl_TexCoord[0].y;\n"
	"  float y = texture2DRect(texY, vec2(nx, ny)).r;\n"
	"  float u = texture2DRect(texU, vec2(nx / 2.0, ny / 2.0)).r;\n"
	"  float v = texture2DRect(texV, vec2(nx / 2.0, ny / 2.0)).r;\n"

	"  y = 1.1643 * (y - 0.0625);\n"
	"  u = u - 0.5;\n"
	"  v = v - 0.5;\n"

	"  float r = y + 1.5958 * v;\n"
	"  float g = y - 0.39173 * u - 0.81290 * v;\n"
	"  float b = y + 2.017 * u;\n"

	"  gl_FragColor = vec4(r, g, b, 1.0);\n"
	"}\n";

/**

*/

QtGlAccelWidget::QtGlAccelWidget(QWidget *parent, int w, int h) : QGLWidget(parent)
{
	memset(textureRealWidths, 0, sizeof(textureRealWidths));
    memset(textureStrides, 0, sizeof(textureStrides));
	memset(textureHeights, 0, sizeof(textureHeights));
	memset(textureOffsets, 0, sizeof(textureOffsets));

	imageWidth = w;
	imageHeight = h;
	firstRun = true;
	glProgram = NULL;
}
/**

*/
bool QtGlAccelWidget::setDisplaySize(int width,int height)
{
    displayWidth=width;
    displayHeight=height;
    resize(displayWidth,displayHeight);
    return true;
}
/**

*/
QtGlAccelWidget::~QtGlAccelWidget()
{
}
/**

*/

bool QtGlAccelWidget::setImage(ADMImage *pic)
{
    int imageWidth=pic->_width;
    int imageHeight=pic->_height;

    
	this->imageWidth = imageWidth;
	this->imageHeight = imageHeight;

	textureRealWidths[0] = imageWidth;
    textureStrides[0]=pic->GetPitch(PLANAR_Y);
	textureHeights[0] = imageHeight;
	textureOffsets[0] = pic->GetReadPtr(PLANAR_Y);

	textureRealWidths[1] = imageWidth / 2;
    textureStrides[1]=pic->GetPitch(PLANAR_V);
	textureHeights[1] = imageHeight / 2;
	textureOffsets[1] = pic->GetReadPtr(PLANAR_V);

	
	textureRealWidths[2] = imageWidth / 2;
    textureStrides[2]=pic->GetPitch(PLANAR_U);
	textureHeights[2] = imageHeight / 2;
	textureOffsets[2] = pic->GetReadPtr(PLANAR_U);

    updateTexture();

    return true;
}
/**

*/
void QtGlAccelWidget::initializeGL()
{
	int success = 1;

#ifndef QT_OPENGL_ES
	glActiveTexture = (_glActiveTexture)this->context()->getProcAddress(QLatin1String("glActiveTexture"));

	if (!glActiveTexture)
	{
		success = 0;
		printf("[GL Render] Active Texture function not found!\n");
	}
#endif

	printf("[GL Render] OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
	printf("[GL Render] OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
	printf("[GL Render] OpenGL Version: %s\n", glGetString(GL_VERSION));
	printf("[GL Render] OpenGL Extensions: %s\n", glGetString(GL_EXTENSIONS));

	glProgram = new QGLShaderProgram(this);

	if (success && !glProgram->addShaderFromSourceCode(QGLShader::Fragment, yuvToRgb))
	{
		success = 0;
		printf("[GL Render] Fragment log: %s\n", glProgram->log().toUtf8().constData());
	}

	if (success && !glProgram->link())
	{
		success = 0;
		printf("[GL Render] Link log: %s\n", glProgram->log().toUtf8().constData());
	}

	if (success && !glProgram->bind())
	{
		success = 0;
		printf("[GL Render] Binding FAILED\n");
	}

	glProgram->setUniformValue("texY", 0);
	glProgram->setUniformValue("texU", 1);
	glProgram->setUniformValue("texV", 2);
}
/**

*/
void QtGlAccelWidget::updateTexture()
{
	if (!textureOffsets[0])
	{
		printf("[Render] Buffer not set\n");
		return;
	}

	if (firstRun)
	{
		glViewport(0, 0, width(), height());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, width(), 0, height(), -1, 1);
		glProgram->setUniformValue("height", (float)imageHeight);
	}

	// U
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, 1);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (firstRun)
		glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, textureStrides[1], textureHeights[1], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureOffsets[1]);
	else
		glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, textureStrides[1], textureHeights[1], GL_LUMINANCE, GL_UNSIGNED_BYTE, textureOffsets[1]);

	// V
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, 2);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (firstRun)
		glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, textureStrides[2], textureHeights[2], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureOffsets[2]);
	else
		glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, textureStrides[2], textureHeights[2], GL_LUMINANCE, GL_UNSIGNED_BYTE, textureOffsets[2]);

	// Y
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, 3);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (firstRun)
	{
		glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, textureStrides[0], textureHeights[0], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureOffsets[0]);
		firstRun = false;
	}
	else
		glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, textureStrides[0], textureHeights[0], GL_LUMINANCE, GL_UNSIGNED_BYTE, textureOffsets[0]);
}
/**

*/
void QtGlAccelWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex2i(0, 0);
	glTexCoord2i(imageWidth, 0);
	glVertex2i(width(), 0);
	glTexCoord2i(imageWidth, imageHeight);
	glVertex2i(width(), height());
	glTexCoord2i(0, imageHeight);
	glVertex2i(0, height());
	glEnd();
}
/**
    \fn ctor
*/
QtGlRender::QtGlRender(void)
{
	glWidget = NULL;

}
/**
    \fn stop
*/

bool QtGlRender::stop(void)
{
	printf("[GL Render] Renderer closed\n");

	if (glWidget)
		delete glWidget;
    glWidget=NULL;
    return true;
}
/**
    \fn init
*/

bool QtGlRender::init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom)
{
	printf("[GL Render] Initialising renderer\n");
    baseInit(w,h,zoom);
    glWidget=NULL;
	bool status= QGLFormat::hasOpenGL() && QGLShaderProgram::hasOpenGLShaderPrograms();
    if(false==status)
    {
        ADM_warning("[GL Render] Init failed : No QFl support or no GLShareProgram\n");
        ADM_warning("[GL Render] hasOpenGl : %d, hasOpenGLShaderProgram %d\n",(int)QGLFormat::hasOpenGL(),(int)QGLShaderProgram::hasOpenGLShaderPrograms());
        return false;
    }

	glWidget = new QtGlAccelWidget((QWidget*)window->widget, w, h);
    glWidget->setDisplaySize(displayWidth,displayHeight);
	glWidget->show();
    return true;
}
/**
    \fn displayImage
*/
bool QtGlRender::displayImage(ADMImage *pic)
{
	glWidget->setImage(pic);
	glWidget->repaint();
	return true;
}
/**
    \fn changeZoom
*/
bool QtGlRender::changeZoom(renderZoom newZoom)
{
    ADM_info("changing zoom, qtGl render.\n");
    calcDisplayFromZoom(newZoom);
    currentZoom=newZoom;
    glWidget->setDisplaySize(displayWidth,displayHeight);
    glWidget->repaint();
    return true;
}
/**
    \fn refresh
*/      
bool    QtGlRender::refresh(void)   
{
    glWidget->repaint();
	return true;
}
VideoRenderBase *RenderSpawnQtGl(void)
{
    return new QtGlRender();
}
// EOF