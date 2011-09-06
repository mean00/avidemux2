/***************************************************************************
    copyright            : (C) 2001 by mean
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

#include "config.h"
#ifdef USE_OPENGL

#include "ADM_inttype.h"
#include "T_openGL.h"

#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtGui/QKeyEvent>
#include <QtGui/QGraphicsView>

#include <QtOpenGL/QGLWidget>
#include "Q_dummyWidget.h"


#include "T_openGLFilter.h"
#include "ADM_default.h"
dummyGLWidget *topGlWidget=NULL;
dummyGLWidget *topGlWidgetRoot=NULL;
extern QWidget *VuMeter;
extern bool ADM_glHasARB(void);
/**
    \fn UI_Qt4InitGl
*/
void UI_Qt4InitGl(void)
{
    ADM_info("Creating openGl dummy widget\n");
    topGlWidgetRoot=new dummyGLWidget(VuMeter);
    ADM_setGlWidget(topGlWidgetRoot);
    topGlWidgetRoot->resize(4,4);
    topGlWidgetRoot->show();
    
    void  *func;

    #define PROBE_GL_EXT(funcName, meth)     func=topGlWidgetRoot->context()->getProcAddress(QLatin1String(#funcName));   \
             ADM_glExt::meth(func); \
             if(!func) ADM_warning("Extension "#funcName" missing\n"); \
             else ADM_info("Extension "#funcName" found\n");

    PROBE_GL_EXT(glActiveTexture,setActivateTexture)
    PROBE_GL_EXT(glBindBufferARB,setBindBuffer)
    PROBE_GL_EXT(glGenBuffers,setGenBuffers)
    PROBE_GL_EXT(glDeleteBuffers,setDeleteBuffers)
    PROBE_GL_EXT(glMapBufferARB,setMapBuffer)
    PROBE_GL_EXT(glUnmapBufferARB,setUnmapBuffer)
    PROBE_GL_EXT(glBufferDataARB,setBufferData)


    if(ADM_glHasARB())
    {
        ADM_info("openGL ARB activated\n");
    }else
        ADM_warning("OpenGL: Not enough ARB extension found\n");

	printf("[GL Render] OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
	printf("[GL Render] OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
	printf("[GL Render] OpenGL Version: %s\n", glGetString(GL_VERSION));
	printf("[GL Render] OpenGL Extensions: %s\n", glGetString(GL_EXTENSIONS));

}
/**
    \fn UI_Qt4CleanGl
*/
void UI_Qt4CleanGl(void)
{
    if(topGlWidgetRoot) delete topGlWidgetRoot;
    topGlWidgetRoot=NULL;
}
#endif
//********************************************
//EOF
