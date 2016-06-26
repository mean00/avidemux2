/** *************************************************************************
                    \file     shaderLoader
                    \brief    Rotate picture

   
                    copyright            : (C) 2011 by mean

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
#include "ADM_clock.h"
#include "DIA_factory.h"
#include "shaderLoader.h"
#include "shaderLoader_desc.cpp"
/**
    \class shaderLoader
*/
class shaderLoader : public  ADM_coreVideoFilterQtGl
{
protected:
                shaderLoaderConf params;
                GLuint       glList;
                ADMImage    *original;
                bool         ready;
                std::string  erString;
                bool         genQuad(void);

protected:
                bool        render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo);
public:
                             shaderLoader(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~shaderLoader();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void         setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;             /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   shaderLoader,   // Class
                        1,0,0,              // Version
                        ADM_UI_QT4+ADM_FEATURE_OPENGL,         // UI
                        VF_OPENGL,            // Category
                        "shaderLoader",            // internal name (must be uniq!)
                        "Shader Loader",            // Display name
                        "Run an external shader program." // Description
                    );

// Now implements the interesting parts
/**
    \fn shaderLoader
    \brief constructor
*/
shaderLoader::shaderLoader(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilterQtGl(in,setup)
{
        if(!setup || !ADM_paramLoad(setup,shaderLoaderConf_param,&params))
        {
            params.shaderFile=strdup("");
        }
        original=new ADMImageDefault(in->getInfo()->width,in->getInfo()->height);
        _parentQGL->makeCurrent();
        fboY->bind();
        ready=true;
       
        printf("Compiling shader \n");
        glProgramY = new QGLShaderProgram(_context);
        ADM_assert(glProgramY);
        
        // Load the file info memory
        int sourceSize=-1;
        if(ADM_fileExist(params.shaderFile))
        {
            sourceSize=ADM_fileSize(params.shaderFile);
        }else
        {
            ADM_warning("Shader file does not exist (%s)\n",params.shaderFile);
            ready=false;
        }
        if(sourceSize<5)
        {
            ADM_warning("File too short, needs to be at least 5 chars\n");
            ready=false;
        }
        if(ready)
        {
            uint8_t *buffer=(uint8_t *)alloca(sourceSize+1);
            FILE *f=fopen(params.shaderFile,"rt");
            if(f)
            {
                fread(buffer,sourceSize,1,f);
                buffer[sourceSize]=0;
                fclose(f);
                if ( !glProgramY->addShaderFromSourceCode(QGLShader::Fragment,(char *) buffer))
                {
                    ready=false;
                    erString="Compiling shader failed"+std::string(glProgramY->log().toUtf8().constData());
                    ADM_warning("Compilation failed (size=%d)\n",sourceSize);
                }
            }else
            {
                ADM_warning("Cannot open file %s\n",params.shaderFile);
                erString=std::string("Cannot open file");
                ready=false;
            }
        }
        if ( ready && !glProgramY->link())
        {
            ready=false;
            erString=std::string( glProgramY->log().toUtf8().constData());
            ADM_warning("Link failed\n");
        }

        if ( ready && ! glProgramY->bind())
        {
                ready=false;
                erString=std::string("OpenGl bind failed");
                ADM_warning("Bind failed\n");
        }
        glList=glGenLists(1);
        genQuad();
        fboY->release();
        _parentQGL->doneCurrent();
}
/**
    \fn shaderLoader
    \brief destructor
*/
shaderLoader::~shaderLoader()
{
    if(original) delete original;
    original=NULL;
    glDeleteLists(glList,1);

}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool shaderLoader::getNextFrame(uint32_t *fn,ADMImage *image)
{
    // since we do nothing, just get the output of previous filter
    if(false==previousFilter->getNextFrame(fn,original))
    {
        ADM_warning("glRotate : Cannot get frame\n");
        return false;
    }
    if(!ready)
    {
        ADM_info("OpenGl shader not loaded (%s)\n",erString.c_str());
        image->duplicateFull(original);
        image->printString(2,2,"Shader not loaded");
        image->printString(2,2,erString.c_str());
        return true;
    }
    _parentQGL->makeCurrent();
    glPushMatrix();
    // size is the last one...
    fboY->bind();

    glProgramY->setUniformValue("myTextureU", 1); 
    glProgramY->setUniformValue("myTextureV", 2); 
    glProgramY->setUniformValue("myTextureY", 0); 
    glProgramY->setUniformValue("pts", (GLfloat)original->Pts); 

    uploadAllPlanes(original);

    render(image,PLANAR_Y,fboY);

    downloadTextures(image,fboY);

    fboY->release();
    firstRun=false;
    glPopMatrix();
    _parentQGL->doneCurrent();
    image->copyInfo(original);
    return true;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         shaderLoader::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, shaderLoaderConf_param,&params);
}

void shaderLoader::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, shaderLoaderConf_param, &params);
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *shaderLoader::getConfiguration(void)
{
    static char st[200];
    snprintf(st,199,"Shader Loader %s",params.shaderFile);
    return st;
}

/**
    \fn configure
*/
bool shaderLoader::configure( void) 
{
    std::string shaderFile=std::string(params.shaderFile);
    diaElemFile shader(0,shaderFile,"ShaderFile to load");
     
    diaElem *elems[]={&shader};
     
     if(diaFactoryRun(QT_TR_NOOP("ShaderLoader"),sizeof(elems)/sizeof(diaElem *),elems))
     {
                // MEMLEAK !ADM_dealloc(params.shaderFile);
                params.shaderFile=ADM_strdup(shaderFile.c_str()); // memleak
                return true;
     }
     return false;
}
/**
 * 
 * @return 
 */
bool shaderLoader::genQuad(void)
{
  int width=info.width;
  int height=info.height;

#define POINT(a,b)  glTexCoord2i(a, b);glVertex2i(a, b);  
  
  glDeleteLists(glList,1);
  glNewList(glList,GL_COMPILE);
  glBegin(GL_QUADS);
    int x=0,y=0;

    POINT(0,0);
    POINT(width,0);
    POINT(width,height);
    POINT(0,height);

  glEnd();
  glEndList();
  return true;
}


/**
    \fn render
*/
bool shaderLoader::render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo)
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
