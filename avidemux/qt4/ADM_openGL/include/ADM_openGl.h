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
#define GL_GLEXT_PROTOTYPES
#include "ADM_openGL_export.h"

#ifdef __APPLE__
#   define GL_SILENCE_DEPRECATION
#	include <OpenGL/gl.h>
#   include <OpenGL/glu.h>
#	include <OpenGL/glext.h>
#	define GL_TEXTURE_RECTANGLE_NV GL_TEXTURE_RECTANGLE_EXT


typedef void (* PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (* PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (* PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (* PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef GLvoid* (* PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
typedef GLboolean (* PFNGLUNMAPBUFFERPROC) (GLenum target);
typedef void (* PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
#else
#	ifdef _WIN32
#		include <windows.h>
#	endif
#	include <GL/gl.h>
#   include <GL/glu.h>
#	include <GL/glext.h>
#endif

#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <math.h>
#include "ADM_image.h"
/**
    \class ADM_glExt
*/
class ADM_OPENGL6_EXPORT ADM_glExt
{
public:
 static       void setActivateTexture(void *func);
 static       void setBindBuffer(void *func);
 static       void setGenBuffers(void *func);
 static       void setDeleteBuffers(void *func);
 static       void setMapBuffer(void *func);
 static       void setUnmapBuffer(void *func);
 static       void setBufferData(void *func);
public:
 static       void activeTexture  (GLenum texture);
 static       void bindBuffer     (GLenum target, GLuint buffer);
 static       void genBuffers     (GLsizei n, GLuint *buffers);
 static       void deleteBuffers  (GLsizei n, const GLuint *buffers);
 static       void *mapBuffer     (GLenum target, GLenum access);
 static       void unmapBuffer    (GLenum target);
 static       void bufferData     (GLenum target,GLsizeiptr size, const GLvoid *data,GLenum usage);
};


 bool ADM_OPENGL6_EXPORT ADM_glHasActiveTexture(void);
 bool ADM_OPENGL6_EXPORT ADM_glHasARB(void);



/**
 *  \class ADM_coreQtGl
 *  \brief base class for all QtGl video filters
 */
class  ADM_OPENGL6_EXPORT ADM_coreQtGl
{
protected:
                            GLuint                glPixFrmt;
                            QOpenGLWidget         *_parentQGL;
                    const   QOpenGLContext        *_context;
                            int                   firstRun;
                            int                   nbTex;
                            int                   nbComponents;
                            GLuint                texName[3];
protected:
                            // image <--> texture
                            void uploadAllPlanes(ADMImage *image);
                            void uploadOnePlane(ADMImage *image, ADM_PLANE plane, GLuint tex,int texNum );
                            
                            bool downloadTexture(ADMImage *image, ADM_PLANE plane, QOpenGLFramebufferObject *fbo);
                            bool downloadTextures(ADMImage *image, QOpenGLFramebufferObject *fbo,GLuint bufferArb);
                            bool downloadTexturesDma(ADMImage *image, QOpenGLFramebufferObject *fbo,GLuint bufferArb);
                            bool downloadTexturesQt(ADMImage *image, QOpenGLFramebufferObject *fbo);

public:
                            ADM_coreQtGl(QOpenGLWidget *parentWidget, bool delayedInit=false, ADM_pixelFormat fmt = ADM_PIXFRMT_YV12);
       virtual             ~ADM_coreQtGl();

        
                            static bool checkGlError(const char *op);
                            bool initTextures(void);
};

/**
    \class QtGlAccelWidget
*/
class ADM_OPENGL6_EXPORT QtGlAccelWidget : public QOpenGLWidget, public ADM_coreQtGl
{
private:
        int             imageWidth, imageHeight;
        ADM_pixelFormat pixelFormat;

        QOpenGLShaderProgram *glProgram;
        bool            operational;
        bool            draw;

protected:
        void initializeGL();
        void paintGL() attribute_align_arg;
        void resizeGL(int width, int height);
        void updateTexture(ADMImage *pic);

public:
             QtGlAccelWidget(QWidget *parent, int imagew, int imageh, ADM_pixelFormat fmt = ADM_PIXFRMT_YV12);
             ~QtGlAccelWidget();
        bool isOperational(void) { return operational; }
        bool setImage(ADMImage *pic);
        bool setDisplaySize(int width, int height);
};
