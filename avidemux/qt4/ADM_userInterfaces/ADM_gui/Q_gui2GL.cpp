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
    
#ifndef QT_OPENGL_ES
    GlActiveTexture_Type *tex= (GlActiveTexture_Type *)topGlWidgetRoot->context()->getProcAddress(QLatin1String("glActiveTexture"));

	if (!tex)
	{
		ADM_error("[GL Render] Active Texture function not found!\n");
	}else
    {
        ADM_warning("[GL Render] Active Texture function found (Not openGL_ES)\n");
        ADM_setActiveTexture(tex);
    }
#else
    ADM_setActiveTexture(glActiveTexture);
#endif

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
