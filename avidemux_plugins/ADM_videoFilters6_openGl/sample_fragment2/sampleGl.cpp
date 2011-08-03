/** *************************************************************************
                    \fn       openGlFragmentSample.cpp  
                    \brief    simple fragment shader

    This one is combining the 3 textures and apply the shader once


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
                        uint8_t *uBuffer;
                        uint8_t *vBuffer;
                        GLuint  texName[3];
protected:
                        //bool uploadTexture(ADMImage *image, ADM_PLANE plane);
                        bool render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo);
                        void tinyUploadTex(ADMImage *img, ADM_PLANE plane, GLuint tex,int texNum );
                        void multiUploadTex(ADMImage *image);
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
                        "glsampleFragment2",            // internal name (must be uniq!)
                        "OpenGl Fragment Shader Sample2",            // Display name
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
        uBuffer=new uint8_t[info.width*info.height];
        vBuffer=new uint8_t[info.width*info.height];
        glGenTextures(3,texName);
        fboY->release();
        widget->doneCurrent();

}
/**
    \fn openGlSample
    \brief destructor
*/
openGlSample::~openGlSample()
{
    delete [] uBuffer;
    delete [] vBuffer;
    glDeleteTextures(3,texName);
    uBuffer=NULL;
    vBuffer=NULL;

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
    // size is the last one...
    fboY->bind();

    float angle=*fn;
    angle=angle/40;
    glProgramY->setUniformValue("teta", angle);     
    glProgramY->setUniformValue("myTextureU", 1); 
    glProgramY->setUniformValue("myTextureV", 2); 
    glProgramY->setUniformValue("myTextureY", 0); 
    glProgramY->setUniformValue("myWidth", image->GetWidth(PLANAR_Y)); 
    glProgramY->setUniformValue("myHeight", image->GetHeight(PLANAR_Y)); 

#if 0
    tinyUploadTex(image,PLANAR_V,GL_TEXTURE2,texName[2]);
    tinyUploadTex(image,PLANAR_U,GL_TEXTURE1,texName[1]);
    tinyUploadTex(image,PLANAR_Y,GL_TEXTURE0,texName[0]);
#else
    multiUploadTex(image);
#endif
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
void openGlSample::tinyUploadTex(ADMImage *image, ADM_PLANE plane, GLuint tex,int texNum )
{
        myGlActiveTexture(tex);  // Activate texture unit "tex"
        glBindTexture(GL_TEXTURE_RECTANGLE_NV, texNum); // Use texture "texNum"

        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
    \fn uploadTexture
*/
void openGlSample::multiUploadTex(ADMImage *image)
{
          // Activate texture unit "tex"
        for(int xplane=2;xplane>=0;xplane--)
        {
            myGlActiveTexture(GL_TEXTURE0+xplane);
            ADM_PLANE plane=(ADM_PLANE)xplane;
            glBindTexture(GL_TEXTURE_RECTANGLE_NV, texName[xplane]); // Use tex engine "texNum"
            glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            if(plane==PLANAR_Y) // take Y as it is..
            {
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
            }else
            {
                    uint8_t *buffer,*src,*tmp,*tgt;
                    int stride;
                    // expand into U or V buffer... 
                    if(plane==PLANAR_U) 
                            buffer=uBuffer;
                        else buffer=vBuffer;
                    
                    src=image->GetReadPtr(plane);
                    stride=image->GetPitch(plane);

                    tmp=src;
                    tgt=buffer;
                    for(int y=0;y<info.height;y+=2)
                    {
                        for(int x=0;x<info.width;x+=2)
                        {
                                tgt[x]=tgt[x+stride]=tmp[x/2];
                                tgt[x+1]=tgt[x+1+stride]=tmp[x/2];
                        }
                        tmp+=stride;
                        tgt+=2*info.width;
                    }
                    // upload
                    if(!firstRun)
                    {
                        glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, 
                                    info.width,
                                    info.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                                    buffer);
                    }else
                    {
                        glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 
                        info.width,
                        info.height,
                        GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                        buffer);
                    } 
            }
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
