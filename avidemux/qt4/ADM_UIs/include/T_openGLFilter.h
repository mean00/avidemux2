/***************************************************************************
  \file T_openGL.h
  \brief OpenGL related filters
  \author (C) 2011 Mean Fixounet@free.fr 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef T_OPENGL_FILTER_H
#define T_OPENGL_FILTER_H
#include "T_openGL.h"

#include "ADM_assert.h"
#include "ADM_coreVideoFilter.h"

/**
 *  \class ADM_coreVideoFilterQtGl
 *  \brief base class for all QtGl video filters
 */
class ADM_coreVideoFilterQtGl:  public ADM_coreVideoFilter
{
protected:
                            QGLWidget            *widget;
                    const   QGLContext           *context;
                            QGLFramebufferObject *fboY;
                            QGLFramebufferObject *fboUV;
                            QGLShaderProgram     *glProgramY;
                            QGLShaderProgram     *glProgramUV;
                            int                   firstRun;
                            GlActiveTexture_Type *myGlActiveTexture;
protected:
                            bool downloadTexture(ADMImage *target, ADM_PLANE plane,QGLFramebufferObject *fbo);

public:
                            ADM_coreVideoFilterQtGl(ADM_coreVideoFilter *previous,CONFcouple *conf=NULL);
       virtual             ~ADM_coreVideoFilterQtGl();

                                                                                        
protected:
};
// Hooks
// Get our top widget
bool ADM_setGlWidget(QGLWidget *w);
QGLWidget *ADM_getGlWidget(void);
#endif

