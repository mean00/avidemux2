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
#include "sampleGl.h"
#include "ADM_clock.h"
/**

*/

//#define BENCH 1
//#define BENCH_READTEXTURE


/**
    \class openGlSample
*/
class openGlSample : public  ADM_coreVideoFilter
{
protected:
                            bool                 firstRun;
                     const  QGLContext           *context;
                            QGLFramebufferObject *fbo;
                            QGLShaderProgram     *glProgram;
protected:
                        bool uploadTexture(ADMImage *image);
                        bool render(ADMImage *image);
                        bool downloadTexture(ADMImage *image);
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
openGlSample::openGlSample(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
UNUSED_ARG(setup);
        context=QGLContext::currentContext();
        ADM_assert(context);
        fbo = new QGLFramebufferObject(info.width,info.height);
        ADM_assert(fbo);
        fbo->bind();
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
        fbo->release();
        firstRun=true;

}
/**
    \fn openGlSample
    \brief destructor
*/
openGlSample::~openGlSample()
{
		if(glProgram) delete glProgram;
        if(fbo) delete fbo;
        fbo=NULL;
        glProgram=NULL;

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

#ifdef BENCH
    ADMBenchmark bench;
    for(int i=0;i<100;i++)
    {
        bench.start();
#endif

        render(image);
        
        
        downloadTexture(image);
        
#ifdef BENCH
        bench.end();
    }
    ADM_info("GL result: ");
    bench.printResult();
#endif    

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
bool openGlSample::uploadTexture(ADMImage *image)
{
	if (true==firstRun)
		glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_LUMINANCE, 
                        image->GetPitch(PLANAR_Y),
                        image->GetHeight(PLANAR_Y), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                        image->GetReadPtr(PLANAR_Y));
	else
		glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 
                image->GetPitch(PLANAR_Y),
                image->GetHeight(PLANAR_Y),
                GL_LUMINANCE, GL_UNSIGNED_BYTE, 
                image->GetReadPtr(PLANAR_Y));
    firstRun=false;
    return true;
}
/**
    \fn render
*/
bool openGlSample::render(ADMImage *image)
{
    fbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, info.width, info.height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, info.width, 0, info.height, -1, 1);

    glProgram->setUniformValue("width", info.width);
    glProgram->setUniformValue("height",info.height);


    // load input texture in fbo
    uploadTexture(image);
    //
    glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex2i(0, 0);
	glTexCoord2i(info.width, 0);
	glVertex2i(info.width, 0);
	glTexCoord2i(info.width, info.height);
	glVertex2i(info.width ,info.height);
	glTexCoord2i(0, info.height);
	glVertex2i(0, info.height);
	glEnd();	// draw cube background
    fbo->release();
    return true;
}
/**
    \fn downloadTexture
*/
bool openGlSample::downloadTexture(ADMImage *image)
{
#ifdef BENCH_READTEXTURE
    {
    ADMBenchmark bench;
    for(int i=0;i<100;i++)
    {
        bench.start();
        QImage qimg(fbo->toImage());
        bench.end();
     }
    ADM_warning("convert to Qimage\n");
    bench.printResult();
    }
#endif

    QImage qimg(fbo->toImage());



    // Assume RGB32, read R or A
#ifdef BENCH_READTEXTURE
    ADMBenchmark bench;
    for(int i=0;i<100;i++)
    {
        bench.start();
#endif
    int stride=image->GetPitch(PLANAR_Y);
    uint8_t *to=image->GetWritePtr(PLANAR_Y);
    for(int y=0;y<info.height;y++)
    {
        const uchar *src=qimg.constScanLine(info.height-y);
        if(!src)
        {
            ADM_error("Can t get pointer to openGl texture\n");
            return false;
        }
        for(int x=0;x<info.width;x++)
            to[x]=src[x*4];
        to+=stride;
    }
#ifdef BENCH_READTEXTURE
        bench.end();
    }
    bench.printResult();

#endif

    return true;
}
//EOF
