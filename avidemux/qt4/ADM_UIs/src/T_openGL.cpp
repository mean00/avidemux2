#include <QtOpenGL/QtOpenGL>
#include "ADM_assert.h"
#include "T_openGL.h"
static QGLWidget *thisWidget=NULL;

bool ADM_setGlWidget(QGLWidget *w)
{
        thisWidget=w;
        return true;
}
QGLWidget *ADM_getGlWidget(void)
{
        return thisWidget;
        
}
