/** *************************************************************************
                    \fn       openGlSample.cpp  
                    \brief simplest of all video filters, it does nothing

    copyright            : (C) 2009 by mean

bench : 1280*720, null shader, 20 ms, 95% of it in download texture.
            Download Texture
                RGB2Y=5ms               (MMX it)
                toQimage=14 ms  <<==    TOO SLOW

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#define GL_GLEXT_PROTOTYPES

#include <QtGui/QPainter>

#ifdef __APPLE__
#       include <OpenGL/gl.h>
#       include <OpenGL/glext.h>
#       define GL_TEXTURE_RECTANGLE_NV GL_TEXTURE_RECTANGLE_EXT
#else
#       include <GL/gl.h>
#       include <GL/glext.h>
#endif

#include <QtGui/QImage>
#include <QtOpenGL/QtOpenGL>
#include <QtOpenGL/QGLShader>

#ifdef __MINGW32__
        #define glActiveTexture(...) {} // FIXME!
#endif

#define ADM_LEGACY_PROGGY
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "T_openGL.h"
#include "T_openGLFilter.h"
#include "sampleGl.h"
#include "ADM_clock.h"
/**

*/

//#define BENCH 1
//#define BENCH_READTEXTURE


/**
    \class openGlSample
*/
class openGlSample : public  ADM_coreVideoFilterQtGl
{
protected:
protected:
                        bool uploadTexture(ADMImage *image, ADM_PLANE plane);
                        bool render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo);
public:
                             openGlSample(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~openGlSample();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   openGlSample,   // Class
                        1,0,0,              // Version
                        ADM_UI_QT4+ADM_UI_GL,         // UI
                        VF_OPENGL,            // Category
                        "glsample",            // internal name (must be uniq!)
                        "OpenGl Sample",            // Display name
                        "Run a fragment shader." // Description
                    );

// Now implements the interesting parts
/**
    \fn openGlSample
    \brief constructor
*/
openGlSample::openGlSample(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilterQtGl(in,setup)
{
UNUSED_ARG(setup);
        fboY->bind();
        printf("Compiling shader \n");
        glProgram = new QGLShaderProgram(context);
        ADM_assert(glProgram);
        if ( !glProgram->addShaderFromSourceCode(QGLShader::Fragment, myShader))
        {
                ADM_error("[GL Render] Fragment log: %s\n", glProgram->log().toUtf8().constData());
                ADM_assert(0);
        }
        if ( !glProgram->link())
        {
            ADM_error("[GL Render] Link log: %s\n", glProgram->log().toUtf8().constData());
            ADM_assert(0);
        }

        if ( !glProgram->bind())
        {
                ADM_error("[GL Render] Binding FAILED\n");
                ADM_assert(0);
        }

        glProgram->setUniformValue("myTex", 0); 
        printf("Setuping texture\n");
        glProgram->setUniformValue("texY", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_RECTANGLE_NV, 0);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        //glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, WIDTH, HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, myTexture);
        fboY->release();
}
/**
    \fn openGlSample
    \brief destructor
*/
openGlSample::~openGlSample()
{

}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool openGlSample::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,image))
    {
        ADM_warning("FlipFilter : Cannot get frame\n");
        return false;
    }
    render(image,PLANAR_Y,fboY);
    downloadTexture(image,PLANAR_Y,fboY);
    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         openGlSample::getCoupledConf(CONFcouple **couples)
{
    *couples=new CONFcouple(0); // Even if we dont have configuration we must allocate one 
    return true;
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *openGlSample::getConfiguration(void)
{
    
    return "openGl Sample.";
}
/**
    \fn uploadTexture
*/
bool openGlSample::uploadTexture(ADMImage *image, ADM_PLANE plane)
{
	if (firstRun<3)
		glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, 
                        image->GetPitch(plane),
                        image->GetHeight(plane), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                        image->GetReadPtr(plane));
	else
		glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 
                image->GetPitch(plane),
                image->GetHeight(plane),
                GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                image->GetReadPtr(plane));
    firstRun++;
    return true;
}
/**
    \fn render
*/
bool openGlSample::render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo)
{
    int width=image->GetWidth(plane);
    int height=image->GetHeight(plane);

    fbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    // load input texture in fbo
    uploadTexture(image,plane);
    //
    glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex2i(0, 0);
	glTexCoord2i(width, 0);
	glVertex2i(width, 0);
	glTexCoord2i(width, height);
	glVertex2i(width ,height);
	glTexCoord2i(0, height);
	glVertex2i(0, height);
	glEnd();	// draw cube background
    fbo->release();
    return true;
}
//EOF
