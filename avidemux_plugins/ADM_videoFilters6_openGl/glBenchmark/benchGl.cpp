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
#include "ADM_openGl.h"
#define ADM_LEGACY_PROGGY
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "T_openGLFilter.h"
#include "benchGl.h"
#include "ADM_clock.h"


/**
    \class openGlBenchmark
*/
class openGlBenchmark : public  ADM_coreVideoFilterQtGl
{
protected:

protected:
                        bool render(ADMImage *image,ADM_PLANE plane,QOpenGLFramebufferObject *fbo);

public:
                             openGlBenchmark(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~openGlBenchmark();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   openGlBenchmark,   // Class
                        1,0,0,              // Version
                        ADM_UI_QT4+ADM_FEATURE_OPENGL,         // UI
                        VF_OPENGL,            // Category
                        "glBenchmark",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("glBenchmark","OpenGl ReadBack benchmark"),            // Display name
                        QT_TRANSLATE_NOOP("glBenchmark","Check how fast readback is.") // Description
                    );

// Now implements the interesting parts
/**
    \fn openGlBenchmark
    \brief constructor
*/
openGlBenchmark::openGlBenchmark(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilterQtGl(in,setup)
{
UNUSED_ARG(setup);
    _parentQGL->makeCurrent();
    fboY->bind();
    ADM_info("Compiling shader \n");
    glProgramY =    createShaderFromSource(QOpenGLShader::Fragment,myShaderY);
    if(!glProgramY)
    {
        ADM_error("Cannot setup shader\n");
    }
    fboY->release();
    _parentQGL->doneCurrent();

}
/**
    \fn openGlBenchmark
    \brief destructor
*/
openGlBenchmark::~openGlBenchmark()
{
    
}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool openGlBenchmark::getNextFrame(uint32_t *fn,ADMImage *image)
{
    char str1[81];
    char str2[81];

    if(!glProgramY)
    {
        snprintf(str1,80,"Shader was not compiled succesfully");
        image->printString(2,4,str1); 
        return true;
    }
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,image))
    {
        ADM_warning("BenchMark : Cannot get frame\n");
        return false;
    }
    _parentQGL->makeCurrent();
    glPushMatrix();
    // size is the last one...
    fboY->bind();
    
    float angle=*fn;
    angle=angle/40;
    glProgramY->setUniformValue("teta", angle);     
    glProgramY->setUniformValue("myTextureU", 2);
    glProgramY->setUniformValue("myTextureV", 1);
    glProgramY->setUniformValue("myTextureY", 0); 
    glProgramY->setUniformValue("myWidth", (GLfloat)image->GetWidth(PLANAR_Y)); 
    glProgramY->setUniformValue("myHeight", (GLfloat)image->GetHeight(PLANAR_Y)); 

    uploadAllPlanes(image);

    render(image,PLANAR_Y,fboY);

    ADMBenchmark bench;

    for(int i=0;i<10;i++)
    {
        bench.start();
        downloadTexturesQt(image,fboY);
        bench.end();
    }
    ADMBenchmark bench2;
    for(int i=0;i<10;i++)
    {
        bench2.start();
        downloadTexturesDma(image,fboY);
        bench2.end();
    }
    printf("Qt4 Benchmark\n");
    bench.printResult();
    printf("PBO/FBO Benchmark\n");
    bench2.printResult();

    float avg1,avg2;
    int min1,min2,max1,max2;
    bench.getResultUs(avg1,min1,max1);
    bench2.getResultUs(avg2,min2,max2);


    snprintf(str1,80,"Qt  avg=%03.2f us, min=%d max=%d us",avg1,(int)min1,(int)max1);
    snprintf(str2,80,"DMA avg=%03.2f us, min=%d max=%d us",avg2,(int)min2,(int)max2);
    image->printString(2,4,str1);
    image->printString(2,8,str2);

    
    fboY->release();
    firstRun=false;
    glPopMatrix();
    _parentQGL->doneCurrent();
    
    return true;
}

/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         openGlBenchmark::getCoupledConf(CONFcouple **couples)
{
    *couples=new CONFcouple(0); // Even if we dont have configuration we must allocate one 
    return true;
}

void openGlBenchmark::setCoupledConf(CONFcouple *couples)
{
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *openGlBenchmark::getConfiguration(void)
{
    
    return "openGl benchmark.";
}


/**
    \fn render
*/
bool openGlBenchmark::render(ADMImage *image,ADM_PLANE plane,QOpenGLFramebufferObject *fbo)
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
