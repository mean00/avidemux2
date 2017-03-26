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
#pragma once
#include "ADM_UIQT46_export.h"
#include "ADM_assert.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_openGl.h"



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
        bool downloadTextures(ADMImage *image, QGLFramebufferObject *fbo)
        {
            return ADM_coreQtGl::downloadTextures(image, fbo,bufferARB);                                
        }
        bool downloadTexturesDma(ADMImage *image,  QGLFramebufferObject *fbo)
        {
             return ADM_coreQtGl::downloadTexturesDma(image, fbo,bufferARB);           
        }
        QGLShaderProgram *createShaderFromSource(QGLShader::ShaderType type,const char *proggy);

public:
          ADM_coreVideoFilterQtGl(ADM_coreVideoFilter *previous,CONFcouple *conf=NULL);
  virtual ~ADM_coreVideoFilterQtGl();

};
// Hooks
// Get our top widget
ADM_UIQT46_EXPORT bool ADM_setGlWidget(QGLWidget *w);
ADM_UIQT46_EXPORT QGLWidget *ADM_getGlWidget(void);

