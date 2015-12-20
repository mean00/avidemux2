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
#include "ADM_openGL.h"
#define ADM_LEGACY_PROGGY
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "T_openGL.h"
#include "T_openGLFilter.h"
#include "sampleGl.h"
#include "sampleGlDistort.h"
#include "ADM_clock.h"
/**

*/
#define DEPTH 300
//#define BENCH 1
//#define BENCH_READTEXTURE


/**
    \class openGlDistort
*/
class openGlDistort : public  ADM_coreVideoFilterQtGl
{
protected:
                        GLuint  glList;
                        bool    buildVertex(int phase);
protected:
                        bool render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo);
public:
                             openGlDistort(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~openGlDistort();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) {return true;}             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   openGlDistort,   // Class
                        1,0,0,              // Version
                        ADM_UI_QT4+ADM_FEATURE_OPENGL,         // UI
                        VF_OPENGL,            // Category
                        "glSampleDistort",            // internal name (must be uniq!)
                        "OpenGl wave ",            // Display name
                        "Simple wave filter." // Description
                    );

// Now implements the interesting parts
/**
    \fn openGlDistort
    \brief constructor
*/
openGlDistort::openGlDistort(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilterQtGl(in,setup)
{
UNUSED_ARG(setup);
        widget->makeCurrent();
        fboY->bind();
        printf("Compiling shader \n");
        // vertex shader 
       
        // frag shader
        glProgramY = new QGLShaderProgram(_context);
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
        glList=glGenLists(1);
        
        fboY->release();
        widget->doneCurrent();

}
/**
    \fn openGlDistort
    \brief destructor
*/
openGlDistort::~openGlDistort()
{
    glDeleteLists(glList,1);
}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool openGlDistort::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,image))
    {
        ADM_warning("Distort : Cannot get frame\n");
        return false;
    }
    widget->makeCurrent();
    glPushMatrix();
    
    // size is the last one...
    fboY->bind();

    int pulse=(*fn)*8;
    pulse=pulse%info.width;
    float angle=pulse;

    buildVertex(angle);

    glProgramY->setUniformValue("myTextureU", 1); 
    glProgramY->setUniformValue("myTextureV", 2); 
    glProgramY->setUniformValue("myTextureY", 0); 
    glProgramY->setUniformValue("myWidth", (GLfloat)info.width); 
    glProgramY->setUniformValue("myHeight",(GLfloat)info.height); 

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
bool         openGlDistort::getCoupledConf(CONFcouple **couples)
{
    *couples=new CONFcouple(0); // Even if we dont have configuration we must allocate one 
    return true;
}

void openGlDistort::setCoupledConf(CONFcouple *couples)
{
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *openGlDistort::getConfiguration(void)
{
    
    return "openGl Distort.";
}
/**
    \fn buildVertex
    \brief Build a grid mapping of the image using 16x16 quads
*/
static int getY(int phase, int x,int y,int width,int height,int rippleHeight)
{
    double angle=(x+phase);
    angle=angle/width;
    double angle2=y;
    angle2/=height;
    angle=angle*4*3.1415;
    angle2=angle2*6*3.1415;
    double shift=sin(angle+angle2);
    //printf("%d:angle=%f:%f\n",x,angle/3.14,shift);
    return rippleHeight*shift;
}
static void myGlVertex(int x,int y,int phase, int width, int height,int rippleHeight)
{
  glTexCoord2i(x, y);
  glVertex2i(x, y+getY(phase,x,y,width,height,rippleHeight));
}
bool openGlDistort::buildVertex(int phase)
{
  int width=info.width;
  int height=info.height;
  glDeleteLists(glList,1);
  glNewList(glList,GL_COMPILE);
  glBegin(GL_QUADS);

  int rippleHeight=height/20;


#define STEP 3
  int roundW=width>>STEP;
  int roundH=height>>STEP;

  
    for(int y=0;y<roundH;y++)
        for(int x=0;x<roundW;x++)
        {
            int startx=x*(1<<STEP);
            int starty=y*(1<<STEP);

            myGlVertex(startx+ 00, starty+00,phase,width,height,rippleHeight);
            myGlVertex(startx+ (1<<STEP), starty+00,phase,width,height,rippleHeight);
            myGlVertex(startx+ (1<<STEP) ,starty+(1<<STEP),phase,width,height,rippleHeight);
            myGlVertex(startx+ 00, starty+(1<<STEP),phase,width,height,rippleHeight);
            	
        }

    glEnd();
    glEndList();
    return true;
}

/**
    \fn render
*/
bool openGlDistort::render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo)
{
    int width=image->GetWidth(plane);
    int height=image->GetHeight(plane);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    glCallList(glList);
    return true;
}
//EOF
