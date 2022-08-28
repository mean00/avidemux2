/***************************************************************************
  \file T_openGL.h
  \brief OpenGL related filters
  \author (C) 2011 Mean Fixounet@free.fr 
***************************************************************************/
#include "ADM_openGl.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"

static bool initedOnced=false;
static bool initedValue=false;

static bool initOnce(QOpenGLWidget *widget);

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
ADM_coreQtGl::ADM_coreQtGl(QOpenGLWidget *parent, bool delayedInit, ADM_pixelFormat fmt)
{
    _parentQGL=parent;
    firstRun=0;
    switch(fmt)
    {
        case ADM_PIXFRMT_YV12:
            glPixFrmt = GL_LUMINANCE;
            nbTex = 3;
            nbComponents = 1;
            break;
        case ADM_PIXFRMT_RGB32A:
            glPixFrmt = GL_BGRA;
            nbTex = 1;
            nbComponents = 4;
            break;
        default:
            ADM_error("Fatal error: unsupported pixel format %d\n",(int)fmt);
            ADM_assert(0);
            break;
    }
    if(!delayedInit)
    {
        _parentQGL->makeCurrent();
        ADM_assert(initTextures());
        _parentQGL->doneCurrent();
    }
}
ADM_coreQtGl::~ADM_coreQtGl()
{
    glDeleteTextures(nbTex,texName);
    _parentQGL=NULL;
    // MEMLEAK : CAUSE A CRASH
    // Will be deleted when top level widget is cleared out by Qt
    //if(widget) delete widget;       
}

/**
    \fn initTextures
*/
bool ADM_coreQtGl::initTextures(void)
{
    ADM_info("Gl : Allocating context and frameBufferObjects\n");
    _context=QOpenGLContext::currentContext();
    if(!_context)
        return false;
    glGenTextures(nbTex,texName);
    checkGlError("GenTex");
    checkGlError("GenBuffer");
    return true;
}
/**
    \fn uploadOnePlane
*/
void ADM_coreQtGl::uploadOnePlane(ADMImage *image, ADM_PLANE plane, GLuint tex,int texNum )
{
    ADM_assert(texNum < nbTex);
    ADM_glExt::activeTexture(tex);  // Activate texture unit "tex"
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, texNum); // Use texture "texNum"

    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

#define ALIGNX(x,y) ((x+y-1)&~(y-1))
    int pitch = (nbComponents == 1)? image->GetPitch(plane) : ALIGNX(image->GetWidth(plane),16); // ???

    if(!firstRun)
    {
        glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, nbComponents,
            pitch,
            image->GetHeight(plane), 0, glPixFrmt, GL_UNSIGNED_BYTE,
            image->GetReadPtr(plane));
    }else
    {
        glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0,
            pitch,
            image->GetHeight(plane),
            glPixFrmt, GL_UNSIGNED_BYTE,
            image->GetReadPtr(plane));
    }
}
/**
    \fn uploadAllPlanes
*/
void ADM_coreQtGl::uploadAllPlanes(ADMImage *image)
{
    for(int xplane=nbTex-1;xplane>=0;xplane--)
    {
        ADM_glExt::activeTexture(GL_TEXTURE0+xplane);
        ADM_PLANE plane=(ADM_PLANE)xplane;
        glBindTexture(GL_TEXTURE_RECTANGLE_NV, texName[xplane]); // Use tex engine "texNum"
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        int pitch = (nbComponents == 1)? image->GetPitch(plane) : ALIGNX(image->GetWidth(plane),16); // ???

        if(!firstRun)
        {
            glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, nbComponents,
                pitch,
                image->GetHeight(plane), 0, glPixFrmt, GL_UNSIGNED_BYTE,
                image->GetReadPtr(plane));
        }else
        {
            glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0,
                pitch,
                image->GetHeight(plane),
                glPixFrmt, GL_UNSIGNED_BYTE,
                image->GetReadPtr(plane));
        }
    }
}
/****************************************************************************/

static const char *yuvToRgb =
    "#extension GL_ARB_texture_rectangle: enable\n"

    "uniform sampler2DRect texY, texU, texV;\n"

    "uniform float height;\n"

    "const mat4 mytrix=mat4( 1.164383561643836,  0,                  1.596026785714286,  0,"
                            "1.164383561643836, -0.391762290094914, -0.812967647237771,  0,"
                            "1.164383561643836,  2.017232142857142,  0,                  0,"
                            "0,                  0,                  0,                  1);\n"
    "const vec2 divby2=vec2( 0.5  ,0.5);\n"
    "const vec4 offsetx=vec4(-0.0627450980392157, -0.5, -0.5, 0);\n"

    "void main(void) {\n"
    "  float nx = gl_TexCoord[0].x;\n"
    "  float ny = height - gl_TexCoord[0].y;\n"
    "\n"
    "  vec2 coord=vec2(nx,ny);"
    "  vec2 coord2=coord*divby2;"
    "  float y = texture2DRect(texY, coord).r;\n"
    "  float u = texture2DRect(texU, coord2).r;\n"
    "  float v = texture2DRect(texV, coord2).r;\n"

    "  vec4 inx=vec4(y,u,v,1.0);\n"
    "  vec4 outx=(inx+offsetx)*mytrix;\n"
    "  gl_FragColor = outx;\n"
    "}\n";

static const char *rgbToRgb =
    "#extension GL_ARB_texture_rectangle: enable\n"

    "uniform sampler2DRect texRgb;\n"
    "uniform float height;\n"

    "void main(void) {\n"
    "    float nx = gl_TexCoord[0].x;\n"
    "    float ny = height - gl_TexCoord[0].y;\n"
    "    vec2 coord = vec2(nx,ny);"
    "    gl_FragColor = texture2DRect(texRgb, coord);\n"
    "    gl_FragColor.a = 1.0;\n"
    "}\n";

/**
    \fn initOnce
*/
bool initOnce(QOpenGLWidget *widget)
{
    if(initedOnced) return initedValue;
    initedOnced=initedValue=true;
    ADM_info("[GL Render] OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
    ADM_info("[GL Render] OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    ADM_info("[GL Render] OpenGL Version: %s\n", glGetString(GL_VERSION));
    ADM_info("[GL Render] OpenGL Extensions:\n");
    printf("%s\n",(const char *)glGetString(GL_EXTENSIONS)); // too long for ADM_info
    return true;
}
/**
    \fn ctor
*/
QtGlAccelWidget::QtGlAccelWidget(QWidget *parent, int w, int h, ADM_pixelFormat fmt) : QOpenGLWidget(parent), ADM_coreQtGl(this,true,fmt)
{
    ADM_info("[QTGL]\t Creating glWidget\n");
    switch(fmt)
    {
        case ADM_PIXFRMT_YV12:
        case ADM_PIXFRMT_RGB32A:
            break;
        default:
            ADM_error("Fatal error: unsupported pixel format %d\n",(int)fmt);
            ADM_assert(0);
            break;
    }

    imageWidth = w;
    imageHeight = h;
    pixelFormat = fmt;
    glProgram = NULL;
    operational = false;
}
/**
        \fn dtor
*/
QtGlAccelWidget::~QtGlAccelWidget()
{
    ADM_info("[QTGL]\t Deleting glWidget\n");
    if(glProgram)
    {
        glProgram->release();
        delete glProgram;
        glProgram = NULL;
    }
}
/**
    \fn setDisplaySize
*/
bool QtGlAccelWidget::setDisplaySize(int width,int height)
{
    resize(width,height);
    return true;
}

/**
    \fn setImage
*/
bool QtGlAccelWidget::setImage(ADMImage *pic)
{
    if (!operational)
        return false;
    imageWidth = pic->_width;
    imageHeight = pic->_height;
    updateTexture(pic);
    return true;
}
/**
    \fn initializeGL
*/
void QtGlAccelWidget::initializeGL()
{
    if(!initTextures() || !initOnce(this))
    {
        ADM_warning("No QtGl support\n");
        return;
    }

    glProgram = new QOpenGLShaderProgram(this);

    const char *shader = (pixelFormat == ADM_PIXFRMT_RGB32A)? rgbToRgb : yuvToRgb;

    if (!glProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, shader))
    {
        ADM_info("[GL Render] Fragment log: %s\n", glProgram->log().toUtf8().constData());
        return;
    }

    if (!glProgram->link())
    {
        ADM_info("[GL Render] Link log: %s\n", glProgram->log().toUtf8().constData());
        return;
    }

    if (!glProgram->bind())
    {
        ADM_info("[GL Render] Binding FAILED\n");
        return;
    }

    if (pixelFormat == ADM_PIXFRMT_RGB32A)
    {
        glProgram->setUniformValue("texRgb", 0);
    } else
    {
        glProgram->setUniformValue("texY", 0);
        glProgram->setUniformValue("texU", 2);
        glProgram->setUniformValue("texV", 1);
    }
    glProgram->setUniformValue("height", (float)imageHeight);

    ADM_info("[GL Render] Init successful\n");
    operational = true;
}
/**
    \fn updateTexture
*/
void QtGlAccelWidget::updateTexture(ADMImage *pic)
{
    if (!operational)
        return;

    uploadAllPlanes(pic);

    if (pixelFormat == ADM_PIXFRMT_RGB32A)
    {
        glProgram->setUniformValue("texRgb", 0);
    } else
    {
        glProgram->setUniformValue("texY", 0);
        glProgram->setUniformValue("texU", 2);
        glProgram->setUniformValue("texV", 1);
    }
    glProgram->setUniformValue("height", (float)imageHeight);

    checkGlError("setUniformValue");
}
/**
    \fn paintGL
*/
void QtGlAccelWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBegin(GL_QUADS);
    glTexCoord2i(0, 0);
    glVertex2i(0, 0);
    glTexCoord2i(imageWidth, 0);
    glVertex2i(width(), 0);
    glTexCoord2i(imageWidth, imageHeight);
    glVertex2i(width(), height());
    glTexCoord2i(0, imageHeight);
    glVertex2i(0, height());
    glEnd();
    checkGlError("draw");
}
/**
    \fn resizeGL
*/
void QtGlAccelWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
}
// EOF
