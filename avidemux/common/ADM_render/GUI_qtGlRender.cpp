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
    "const mat4 mytrix=mat4( 1,   0,         1.5958,   0,"
                            "1,  -0.39173,  -0.81290,  0,"
                            "1,   2.017,      0,       0,"
                            "0,        0,     0,       1);\n"
                            
    "const vec4 offsetx=vec4(-0.07276875,-0.5,-0.5,0);\n"
    "const vec4 factorx=vec4(1.1643,1,1,1);\n"

	"void main(void) {\n"
	"  float nx = gl_TexCoord[0].x;\n"
	"  float ny = height - gl_TexCoord[0].y;\n"
	"  float y = texture2DRect(texY, vec2(nx, ny)).r;\n"
	"  float u = texture2DRect(texU, vec2(nx / 2.0, ny / 2.0)).r;\n"
	"  float v = texture2DRect(texV, vec2(nx / 2.0, ny / 2.0)).r;\n"

    "  vec4 inx=vec4(y,u,v,1.0);\n"
    "  vec4 inx2=(factorx*inx)+offsetx;\n"
    "  vec4 outx=inx2*mytrix;\n"
	"  gl_FragColor = outx;\n"
	"}\n";

/**
    \fn ctor
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
    glGenTextures(3,textureName);
}
/**
    \fn setDisplaySize
*/
bool QtGlAccelWidget::setDisplaySize(int width,int height)
{
    displayWidth=width;
    displayHeight=height;
    resize(displayWidth,displayHeight);
    firstRun = true;
    return true;
}
/**
        \fn dtor
*/
QtGlAccelWidget::~QtGlAccelWidget()
{
    if(glProgram) delete glProgram;
    glProgram=NULL;
    if(textureName[0])
        glDeleteTextures(3,textureName);
    textureName[0]=0;
}
/**
    \fn setImage
*/

bool QtGlAccelWidget::setImage(ADMImage *pic)
{
    int imageWidth=pic->_width;
    int imageHeight=pic->_height;

    
	this->imageWidth = imageWidth;
	this->imageHeight = imageHeight;

    for(int i=0;i<3;i++)
    {
        ADM_PLANE plane=(ADM_PLANE)i;
        textureRealWidths[i] = pic->GetWidth(plane);
        textureStrides[i]    = pic->GetPitch(plane);
        textureHeights[i]    = pic->GetHeight(plane);
        textureOffsets[i]    = pic->GetReadPtr(plane);

    }
    updateTexture();
    return true;
}
/**
    \fn initializeGL
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
	glProgram->setUniformValue("texU", 2);
	glProgram->setUniformValue("texV", 1);
    if(success==1)
        printf("[GL Render] Init successful\n");
}
/**
    \fn updateTexture
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

//--
      // Activate texture unit "tex"
        for(int xplane=2;xplane>=0;xplane--)
        {
            glActiveTexture(GL_TEXTURE0+xplane);
            ADM_PLANE plane=(ADM_PLANE)xplane;
            glBindTexture(GL_TEXTURE_RECTANGLE_NV, textureName[xplane]); // Use tex engine "texNum"
            glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

                if(!firstRun)
                {
                    glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, 
                                    textureStrides[xplane],
                                    textureHeights[xplane],
                                    0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                                    textureOffsets[xplane]);
                }else
                {
                    glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 
                        textureStrides[xplane],
                        textureHeights[xplane],
                        GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                        textureOffsets[xplane]);
                }
        }

	if (firstRun)
	{
		firstRun = false;
	}
    
}
/**
    \fn paintGL
*/
void QtGlAccelWidget::paintGL()
{
	glProgram->setUniformValue("texY", 0);
	glProgram->setUniformValue("texU", 2);
	glProgram->setUniformValue("texV", 1);
    glProgram->setUniformValue("height", (float)imageHeight);

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
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/**
    \fn ctor
*/
QtGlRender::QtGlRender(void)
{
	glWidget = NULL;

}
/**
    \fn dtor
*/
QtGlRender::~QtGlRender(void)
{
	stop();
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
    if(false==QGLFormat::hasOpenGL())
    {
        ADM_warning("This platform has no openGL support \n");
        return false;
    }
    glWidget = new QtGlAccelWidget((QWidget*)window->widget, w, h);
	bool status= QGLShaderProgram::hasOpenGLShaderPrograms(glWidget->context());
    if(false==status)
    {
        delete glWidget;
        glWidget=NULL;
        ADM_warning("[GL Render] Init failed : OpenGL Shader Program support\n");
        return false;
    }

	printf("[GL Render] Setting widget display size to %d x %d\n",imageWidth,imageHeight);
    glWidget->setDisplaySize(displayWidth,displayHeight);
	glWidget->show();
    return true;
}
/**
    \fn displayImage
*/
bool QtGlRender::displayImage(ADMImage *pic)
{
    glWidget->makeCurrent();
	glWidget->setImage(pic);
	glWidget->repaint();
    glWidget->doneCurrent();
	return true;
}
/**
    \fn changeZoom
*/
bool QtGlRender::changeZoom(renderZoom newZoom)
{
    ADM_info("changing zoom, qtGl render.\n");
    glWidget->makeCurrent();
    calcDisplayFromZoom(newZoom);
    currentZoom=newZoom;
    glWidget->setDisplaySize(displayWidth,displayHeight);
    glWidget->repaint();
    glWidget->doneCurrent();
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