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

#include "ADM_UIQT46_export.h"
#include "T_openGL.h"

#include "ADM_assert.h"
#include "ADM_coreVideoFilter.h"


/**
 *  \class ADM_coreQtGl
 *  \brief base class for all QtGl video filters
 */
class ADM_UIQT46_EXPORT ADM_coreQtGl
{
protected:
                            
                            QGLWidget            *widget;
                    const   QGLContext           *context;
                            int                   firstRun;
                            GLuint                texName[3];
protected:
                            // image <--> texture
                            void uploadAllPlanes(ADMImage *image);
                            void uploadOnePlane(ADMImage *image, ADM_PLANE plane, GLuint tex,int texNum );

public:
                            ADM_coreQtGl(QGLWidget *parentWidget);
       virtual             ~ADM_coreQtGl();

        
                            static bool checkGlError(const char *op);
protected:
};

/**
 *  \class ADM_coreVideoFilterQtGl
 *  \brief base class for all QtGl video filters
 */
class ADM_UIQT46_EXPORT ADM_coreVideoFilterQtGl:  public ADM_coreVideoFilter,public ADM_coreQtGl
{
protected:
                            GLuint                bufferARB   ;
                            QGLFramebufferObject *fboY;
                            QGLFramebufferObject *fboUV;
                            QGLShaderProgram     *glProgramY;
                            QGLShaderProgram     *glProgramUV;

                            bool                  resizeFBO(uint32_t w,uint32_t h);

protected:
                            // image <--> texture
                            bool downloadTexture(ADMImage *target, ADM_PLANE plane,QGLFramebufferObject *fbo);
                            bool downloadTextures(ADMImage *target, QGLFramebufferObject *fbo);
                            bool downloadTexturesDma(ADMImage *target, QGLFramebufferObject *fbo);
                            bool downloadTexturesQt(ADMImage *target, QGLFramebufferObject *fbo);

public:
                            ADM_coreVideoFilterQtGl(ADM_coreVideoFilter *previous,CONFcouple *conf=NULL);
       virtual             ~ADM_coreVideoFilterQtGl();

};
// Hooks
// Get our top widget
ADM_UIQT46_EXPORT bool ADM_setGlWidget(QGLWidget *w);
QGLWidget *ADM_getGlWidget(void);
#endif

