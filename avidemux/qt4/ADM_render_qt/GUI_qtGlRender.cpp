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
        glWidget = NULL;
    }
    return true;
}
/**
    \fn init
*/

bool QtGlRender::init(GUI_WindowInfo *window, uint32_t w, uint32_t h, float zoom)
{
    ADM_info("[GL Render] Initialising renderer\n");
    baseInit(w, h, zoom);
    glWidget = NULL;
#if 0
    if(false==QGLFormat::hasOpenGL())
    {
        ADM_warning("This platform has no openGL support \n");
        return false;
    }
#endif
    glWidget = new QtGlAccelWidget((QWidget *)window->widget, w, h);
    ADM_info("[GL Render] Setting widget display size to %d x %d\n", imageWidth, imageHeight);
    glWidget->setDisplaySize(displayWidth, displayHeight);
    glWidget->show();
    bool status = QOpenGLShaderProgram::hasOpenGLShaderPrograms(glWidget->context());
    if (!status)
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
    // printf("Gl paint\n");
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
    currentZoom = newZoom;
    glWidget->setDisplaySize(displayWidth, displayHeight);
    glWidget->update();
    glWidget->doneCurrent();
    return true;
}
/**
    \fn refresh
*/
bool QtGlRender::refresh(void)
{
    // printf("Gl refresh\n");
    glWidget->update();
    return true;
}
/* Hook */
VideoRenderBase *RenderSpawnQtGl(void)
{
    return new QtGlRender();
}
// EOF
