/** *************************************************************************
                    \fn       openGlFragmentSample.cpp  
                    \brief    simple fragment shader

    Simple vertex shader
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


#define ADM_LEGACY_PROGGY
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "T_openGL.h"
#include "T_openGLFilter.h"
#include "sampleGl.h"
#include "sampleGlvertex.h"
#include "ADM_clock.h"
/**

*/

//#define BENCH 1
//#define BENCH_READTEXTURE


/**
    \class openGlVertex
*/
class openGlVertex : public  ADM_coreVideoFilterQtGl
{
protected:

protected:
                        bool render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo);
public:
                             openGlVertex(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~openGlVertex();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   openGlVertex,   // Class
                        1,0,0,              // Version
                        ADM_UI_QT4+ADM_UI_GL,         // UI
                        VF_OPENGL,            // Category
                        "glSampleVertex",            // internal name (must be uniq!)
                        "OpenGl Vertex Shader",            // Display name
                        "Run a simple vertex shader." // Description
                    );

// Now implements the interesting parts
/**
    \fn openGlVertex
    \brief constructor
*/
openGlVertex::openGlVertex(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilterQtGl(in,setup)
{
UNUSED_ARG(setup);
        widget->makeCurrent();
        fboY->bind();
        printf("Compiling shader \n");
        // vertex shader 
       
        // frag shader
        glProgramY = new QGLShaderProgram(context);
        ADM_assert(glProgramY);
#if 1
        if ( !glProgramY->addShaderFromSourceCode(QGLShader::Vertex, myVertex))
        {
                ADM_error("[GL Render] Vertex log: %s\n", glProgramY->log().toUtf8().constData());
                ADM_assert(0);
        }

#endif
        if ( !glProgramY->addShaderFromSourceCode(QGLShader::Fragment, myShaderY))
        {
                ADM_error("[GL Render] Fragment log: %s\n", glProgramY->log().toUtf8().constData());
                ADM_assert(0);
        }
        if ( !glProgramY->link())
        {
            ADM_error("[GL Render] Link log: %s\n", glProgramY->log().toUtf8().constData());
            ADM_assert(0);
        }

        if ( !glProgramY->bind())
        {
                ADM_error("[GL Render] Binding FAILED\n");
                ADM_assert(0);
        }

        fboY->release();
        widget->doneCurrent();

}
/**
    \fn openGlVertex
    \brief destructor
*/
openGlVertex::~openGlVertex()
{

}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool openGlVertex::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,image))
    {
        ADM_warning("FlipFilter : Cannot get frame\n");
        return false;
    }
    widget->makeCurrent();
    glPushMatrix();
    // size is the last one...
    fboY->bind();
    int pulse=(*fn)*4;;
    pulse&=63;
    if(pulse<32) pulse=64-pulse;
    float angle=pulse;
    angle=1+(angle-32)/160;
    glProgramY->setUniformValue("skew", (GLfloat)angle);     
    glProgramY->setUniformValue("myTextureU", 1); 
    glProgramY->setUniformValue("myTextureV", 2); 
    glProgramY->setUniformValue("myTextureY", 0); 
    glProgramY->setUniformValue("myWidth", (GLfloat)image->GetWidth(PLANAR_Y)); 
    glProgramY->setUniformValue("myHeight",(GLfloat)image->GetHeight(PLANAR_Y)); 

    uploadAllPlanes(image);

    render(image,PLANAR_Y,fboY);

    downloadTextures(image,fboY);

    fboY->release();
    firstRun=false;
    glPopMatrix();
    widget->doneCurrent();
    
    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         openGlVertex::getCoupledConf(CONFcouple **couples)
{
    *couples=new CONFcouple(0); // Even if we dont have configuration we must allocate one 
    return true;
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *openGlVertex::getConfiguration(void)
{
    
    return "openGl Sample.";
}


/**
    \fn render
*/
bool openGlVertex::render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo)
{
    int width=image->GetWidth(plane);
    int height=image->GetHeight(plane);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    //
    // Split our image into 16 pixels quad
    int roundW=width>>4;
    int roundH=height>>4;
    for(int y=0;y<roundH;y++)
        for(int x=0;x<roundW;x++)
        {
            int startx=x*16;
            int starty=y*16;
            float z=x-roundW/2;    
            z=z/roundW;
            z/=5;
            if(z<0) z=-z;
            int offset=0;
            glBegin(GL_QUADS);
            glVertex3i(startx+offset, starty+offset,z);
            glVertex3i(startx+offset+16, starty+offset,z);
            glVertex3i(startx+offset+16 ,starty+16+offset,z);
            glVertex3i(startx+offset, starty+16+offset,z);
            glEnd();	
        }
    return true;
}
//EOF
