    /***************************************************************************
  \file T_openGL.h
  \brief OpenGL related filters
  \author (C) 2011 Mean Fixounet@free.fr 
***************************************************************************/
#include "ADM_openGl.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"

/**

*/
static PFNGLACTIVETEXTUREPROC myActiveTexture=NULL;
static PFNGLBINDBUFFERPROC    myBindBuffer=NULL;
static PFNGLDELETEBUFFERSPROC myDeleteBuffers=NULL;
static PFNGLGENBUFFERSPROC    myGenBuffers=NULL;
static PFNGLMAPBUFFERPROC     myMapBuffer=NULL;
static PFNGLUNMAPBUFFERPROC   myUnmapBuffer=NULL;
static PFNGLBUFFERDATAARBPROC myBufferData=NULL;

 void ADM_glExt::setBufferData(void *func)
{
    myBufferData=(PFNGLBUFFERDATAARBPROC )func;
}

 void ADM_glExt::setActivateTexture(void *func)
{
    myActiveTexture=(PFNGLACTIVETEXTUREPROC )func;
}
 void ADM_glExt::setBindBuffer(void *func)
{
    myBindBuffer=(PFNGLBINDBUFFERPROC)func;
}
 void ADM_glExt::setGenBuffers(void *func)
{
    myGenBuffers=(PFNGLGENBUFFERSPROC)func;
}
 void ADM_glExt::setDeleteBuffers(void *func)
{
    myDeleteBuffers=(PFNGLDELETEBUFFERSPROC)func;
}
 void ADM_glExt::setMapBuffer(void *func)
{
    myMapBuffer=(PFNGLMAPBUFFERPROC)func;
}
 void ADM_glExt::setUnmapBuffer(void *func)
{
    myUnmapBuffer=(PFNGLUNMAPBUFFERPROC)func;
}


                            
//-------------                            
/**
     \fn checkGlError
     \brief pop an error if an operation failed
*/
bool ADM_coreQtGl::checkGlError(const char *op)
{
    GLenum er=glGetError();
    if(!er) return true;
    ADM_error("[GLERROR]%s: %d => %s\n",op,er,gluErrorString(er));
    return false;
}          

#define CHECK(x) if(!x) {GUI_Error_HIG("Missing extension "#x,#x" not defined");ADM_assert(0);}
/**
    \class ADM_glExt
*/
void ADM_glExt::activeTexture  (GLenum texture)
{
      CHECK(myActiveTexture);
      myActiveTexture(texture);
}
void ADM_glExt::bindBuffer     (GLenum target, GLuint buffer)
{
    CHECK(myBindBuffer);
      myBindBuffer(target,buffer);

}
void ADM_glExt::genBuffers     (GLsizei n, GLuint *buffers)
{
    CHECK(myGenBuffers);
      myGenBuffers(n,buffers);

}
void ADM_glExt::deleteBuffers  (GLsizei n, const GLuint *buffers)
{
    CHECK(myDeleteBuffers);
      myDeleteBuffers(n,buffers);

}

void *ADM_glExt::mapBuffer    (GLenum target, GLenum access)
{
    CHECK(myMapBuffer);
    return myMapBuffer(target,access);

}
void ADM_glExt::unmapBuffer    (GLenum target)
{
    CHECK(myUnmapBuffer);
      myUnmapBuffer(target);

}
void ADM_glExt::bufferData(GLenum target,GLsizeiptr size, const GLvoid *data,GLenum usage)
{
    CHECK(myBufferData);
      myBufferData(target,size,data,usage);

}

/**
    \fn ADM_hasActiveTexture
*/
bool ADM_glHasActiveTexture(void)
{
    if(!myActiveTexture) return false;
    return true;
}

/**
    \fn ADM_glHasARB
*/
bool ADM_glHasARB(void)
{
    if(!myBindBuffer) return false;
    if(!myDeleteBuffers) return false;
    if(!myGenBuffers) return false;
    if(!myMapBuffer) return false;
    if(!myUnmapBuffer) return false;
    return true;
}
/**
 * 
 * @param parent
 */
ADM_coreQtGl::ADM_coreQtGl(QGLWidget *parent)
{
        _parentQGL=parent;
        _parentQGL->makeCurrent();
        firstRun=0;
        ADM_info("Gl : Allocating context and frameBufferObjects\n");
        _context=QGLContext::currentContext();
        ADM_assert(_context);
        glGenTextures(3,texName);
        checkGlError("GenTex");
        checkGlError("GenBuffer");
        _parentQGL->doneCurrent();

    
}
ADM_coreQtGl::~ADM_coreQtGl()
{
    glDeleteTextures(3,texName);
    _parentQGL=NULL;
    // MEMLEAK : CAUSE A CRASH
    // Will be deleted when top level widget is cleared out by Qt
    //if(widget) delete widget;       
}


/**
    \fn uploadTexture
*/
void ADM_coreQtGl::uploadOnePlane(ADMImage *image, ADM_PLANE plane, GLuint tex,int texNum )
{
        ADM_glExt::activeTexture(tex);  // Activate texture unit "tex"
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
void ADM_coreQtGl::uploadAllPlanes(ADMImage *image)
{
          // Activate texture unit "tex"
        for(int xplane=2;xplane>=0;xplane--)
        {
            ADM_glExt::activeTexture(GL_TEXTURE0+xplane);
            ADM_PLANE plane=(ADM_PLANE)xplane;
            glBindTexture(GL_TEXTURE_RECTANGLE_NV, texName[xplane]); // Use tex engine "texNum"
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
}
// EOF
