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

#define ADM_LEGACY_PROGGY // Dont clash with free/malloc etc..
#include "ADM_default.h"
#include "GUI_render.h"

#include "GUI_accelRender.h"
#include "GUI_qtGlRender.h"

static const char *yuvToRgb =
    "#extension GL_ARB_texture_rectangle: enable\n"

    "uniform sampler2DRect texY, texU, texV;\n"

    "uniform float height;\n"

    "const mat4 mytrix=mat4( 1.1643,   0,         1.5958,   0,"
                            "1.1643,  -0.39173,  -0.81290,  0,"
                            "1.1643,   2.017,      0,       0,"
                            "0,        0,     0,       1);\n"
    "const vec2 divby2=vec2( 0.5  ,0.5);\n"
    "const vec4 offsetx=vec4(-0.07276875,-0.5,-0.5,0);\n"
    

    "void main(void) {\n"
    "  float nx = gl_TexCoord[0].x;\n"
    "  float ny = height - gl_TexCoord[0].y;\n"
    "\n"
    "  vec2 coord=vec2(nx,ny);"
    "  vec2 coord2=coord*divby2;"
    "  float y = texture2DRect(texY, coord).r;\n"
    "  float u = texture2DRect(texU, coord2).r;\n"
    "  float v = texture2DRect(texV, coord2).r;\n"

    "  vec4 inx=vec4(y,u,v,1.0);\n"
    "  vec4 outx=(inx+offsetx)*mytrix;\n"
    "  gl_FragColor = outx;\n"
    "}\n";


static bool initedOnced=false;
static bool initedValue=false;

/**
    \fn initOnce
*/
static bool initOnce(QOpenGLWidget *widget)
{
    if(initedOnced) return initedValue;  
    initedOnced=initedValue=true;
    ADM_info("[GL Render] OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
    ADM_info("[GL Render] OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    ADM_info("[GL Render] OpenGL Version: %s\n", glGetString(GL_VERSION));
    ADM_info("[GL Render] OpenGL Extensions:\n");
    printf("%s\n",(const char *)glGetString(GL_EXTENSIONS)); // too long for ADM_info
    return true;
}
/**
    \fn ctor
*/

QtGlAccelWidget::QtGlAccelWidget(QWidget *parent, int w, int h,QtGlRender *glRender) : QOpenGLWidget(parent), ADM_coreQtGl(this,true)
{
    ADM_info("[QTGL]\t Creating glWidget\n");

    _parent=glRender;
    imageWidth = w;
    imageHeight = h;
    glProgram = NULL;
    renderFirstRun=true;
}
/**
        \fn dtor
*/
QtGlAccelWidget::~QtGlAccelWidget()
{
    ADM_info("[QTGL]\t Deleting glWidget\n");
    if(glProgram) 
    {
        glProgram->release();
        delete glProgram;
    }
    glProgram=NULL;
    if(_parent)
    {
        _parent->clearWidget();
    }
}
/**
    \fn setDisplaySize
*/
bool QtGlAccelWidget::setDisplaySize(int width,int height) 
{
    displayWidth=width;
    displayHeight=height;
    resize(displayWidth,displayHeight);
    renderFirstRun = true;
    return true;
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
  
    updateTexture(pic);
    return true;
}
/**
    \fn initializeGL
*/
void QtGlAccelWidget::initializeGL()
{
    int success = 1;

    if(!initTextures() || !initOnce(this))
    {
        ADM_warning("No QtGl support\n");
        success=false;
        return;
    }

    glProgram = new QOpenGLShaderProgram(this);

    if (success && !glProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, yuvToRgb))
    {
        success = 0;
        ADM_info("[GL Render] Fragment log: %s\n", glProgram->log().toUtf8().constData());
    }

    if (success && !glProgram->link())
    {
        success = 0;
        ADM_info("[GL Render] Link log: %s\n", glProgram->log().toUtf8().constData());
    }

    if (success && !glProgram->bind())
    {
        success = 0;
        ADM_info("[GL Render] Binding FAILED\n");
    }

    glProgram->setUniformValue("texY", 0);
    glProgram->setUniformValue("texU", 2);
    glProgram->setUniformValue("texV", 1);
    if(success==1)
        ADM_info("[GL Render] Init successful\n");
}
/**
    \fn updateTexture
*/
void QtGlAccelWidget::updateTexture(ADMImage *pic)
{   
    if (renderFirstRun)
    {
        glViewport(0, 0, width(), height());
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width(), 0, height(), -1, 1);
        glProgram->setUniformValue("height", (float)imageHeight);
        renderFirstRun=false;
    }
    uploadAllPlanes(pic);
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
    checkGlError("setUniformValue");
    
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
    checkGlError("draw");
}
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/**
    \fn ctor
*/
QtGlRender::QtGlRender(void)
{
    ADM_info("Creating GL Renderer\n");
    glWidget = NULL;

}
/**
    \fn dtor
*/
QtGlRender::~QtGlRender(void)
{
    ADM_info("Destroying GL Renderer\n");
    stop();
}

/**
    \fn stop
*/

bool QtGlRender::stop(void)
{
    ADM_info("[GL Render] Renderer closed\n");
    if (glWidget)
    {        
        glWidget->setParent(NULL);
        delete glWidget;
    }
    glWidget=NULL;
    return true;
}
/**
    \fn init
*/

bool QtGlRender::init( GUI_WindowInfo *window, uint32_t w, uint32_t h, float zoom)
{
    ADM_info("[GL Render] Initialising renderer\n");
    baseInit(w,h,zoom);
    glWidget=NULL;
#if 0
    if(false==QGLFormat::hasOpenGL())
    {
        ADM_warning("This platform has no openGL support \n");
        return false;
    }
#endif
    glWidget = new QtGlAccelWidget((QWidget*)window->widget, w, h,this);
    ADM_info("[GL Render] Setting widget display size to %d x %d\n",imageWidth,imageHeight);
    glWidget->setDisplaySize(displayWidth,displayHeight);
    glWidget->show();
    bool status = QOpenGLShaderProgram::hasOpenGLShaderPrograms(glWidget->context());
    if(!status)
        ADM_warning("[GL Render] Init failed : OpenGL Shader Program support\n");
    glWidget->doneCurrent();
    return status;
}
/**
    \fn displayImage
*/
bool QtGlRender::displayImage(ADMImage *pic)
{
    pic->shrinkColorRange();
    //printf("Gl paint\n");
    glWidget->makeCurrent();
    glWidget->setImage(pic);
    glWidget->update();
    glWidget->doneCurrent();
    return true;
}
/**
    \fn changeZoom
*/
bool QtGlRender::changeZoom(float newZoom)
{
    ADM_info("changing zoom, qtGl render.\n");
    glWidget->makeCurrent();
    calcDisplayFromZoom(newZoom);
    currentZoom=newZoom;
    glWidget->setDisplaySize(displayWidth,displayHeight);
    glWidget->update();
    glWidget->doneCurrent();
    return true;
}
/**
    \fn refresh
*/      
bool    QtGlRender::refresh(void)   
{
    //printf("Gl refresh\n");
    glWidget->update();
    return true;
}
/* Hook */
VideoRenderBase *RenderSpawnQtGl(void)
{
    return new QtGlRender();
}
// EOF
