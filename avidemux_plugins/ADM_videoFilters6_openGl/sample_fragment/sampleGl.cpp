/** *************************************************************************
                    \fn       openGlFragmentSample.cpp  
                    \brief    simple fragment shader

    That one is performing the same shader 3 times, one time per plane.


    copyright            : (C) 2011 by mean

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
#include "ADM_openGL.h"
#define ADM_LEGACY_PROGGY
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
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
                        //bool uploadTexture(ADMImage *image, ADM_PLANE plane);
                        bool render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo);
                        void tinyUploadTex(ADMImage *img, ADM_PLANE plane, GLuint tex,int texNum );
public:
                             openGlSample(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~openGlSample();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   openGlSample,   // Class
                        1,0,0,              // Version
                        ADM_UI_QT4+ADM_FEATURE_OPENGL,         // UI
                        VF_OPENGL,            // Category
                        "glsampleFragment",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("glFragment","OpenGl Fragment Shader Sample"),            // Display name
                        QT_TRANSLATE_NOOP("glFragment","Run a fragment shader.") // Description
                    );

// Now implements the interesting parts
/**
    \fn openGlSample
    \brief constructor
*/
openGlSample::openGlSample(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilterQtGl(in,setup)
{
UNUSED_ARG(setup);
        widget->makeCurrent();
        fboY->bind();
        printf("Compiling shader \n");
        glProgramY = new QGLShaderProgram(context);
        ADM_assert(glProgramY);
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
//
        fboUV->bind();
        printf("Compiling shader \n");
        glProgramUV = new QGLShaderProgram(context);
        ADM_assert(glProgramUV);
        if ( !glProgramUV->addShaderFromSourceCode(QGLShader::Fragment, myShaderY))
        {
                ADM_error("[GL Render] Fragment log: %s\n", glProgramUV->log().toUtf8().constData());
                ADM_assert(0);
        }
        if ( !glProgramUV->link())
        {
            ADM_error("[GL Render] Link log: %s\n", glProgramUV->log().toUtf8().constData());
            ADM_assert(0);
        }

        if ( !glProgramUV->bind())
        {
                ADM_error("[GL Render] Binding FAILED\n");
                ADM_assert(0);
        }
        fboUV->release();
        widget->doneCurrent();

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
    widget->makeCurrent();
    glPushMatrix();
    float angle=*fn;
    angle=angle/40;
    
    glProgramUV->setUniformValue("kernelSize", 1);  // Do a convolution kernelSize*2+1 pixels
    glProgramUV->setUniformValue("normalization", 4); 
    // size is the last one...
    fboY->bind();
    // upload kernel...
    
    // here we go
    tinyUploadTex(image,PLANAR_Y,GL_TEXTURE0,0);
    render(image,PLANAR_Y,fboY);
    downloadTexture(image,PLANAR_Y,fboY);
    fboY->release();

    fboUV->bind();
    tinyUploadTex(image,PLANAR_U,GL_TEXTURE1,1);
    glProgramUV->setUniformValue("myTexture", 1); 
    render(image,PLANAR_U,fboUV);
    downloadTexture(image,PLANAR_U,fboUV);
    
    tinyUploadTex(image,PLANAR_V,GL_TEXTURE2,2);
    glProgramUV->setUniformValue("myTexture", 2); 
    render(image,PLANAR_V,fboUV);
    downloadTexture(image,PLANAR_V,fboUV);
    fboUV->release();
    firstRun=false;
    glPopMatrix();
    widget->doneCurrent();
    
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

void openGlSample::setCoupledConf(CONFcouple *couples)
{
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
void openGlSample::tinyUploadTex(ADMImage *image, ADM_PLANE plane, GLuint tex,int texNum )
{
        myGlActiveTexture(tex);
        glBindTexture(GL_TEXTURE_RECTANGLE_NV, texNum);
        glProgramY->setUniformValue("myTexture", texNum); 
        glProgramY->setUniformValue("myWidth", image->GetWidth(plane)); 
        glProgramY->setUniformValue("myHeight", image->GetHeight(plane)); 

        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        if(!firstRun)
        {
            glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, 
                            image->GetPitch(plane),
                            image->GetHeight(plane), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                            image->GetReadPtr(plane));
        }else
        {
            glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 
                image->GetPitch(plane),
                image->GetHeight(plane),
                GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                image->GetReadPtr(plane));
        }
}


/**
    \fn render
*/
bool openGlSample::render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo)
{
    int width=image->GetWidth(plane);
    int height=image->GetHeight(plane);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

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
    return true;
}
//EOF
