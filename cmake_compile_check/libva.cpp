#include <string.h>
#include <stdio.h>
#include <va/va.h>

int main(int argc, char **argv)
{
        unsigned int       num_attribs;
        VADisplay dpy;
        int w,h,format,num_surface;
        VASurfaceID surface;
        VASurfaceAttrib attr;
        vaCreateSurfaces(dpy,format,
                                w,h,
                                &surface, 1,
                                &attr,1);
        return 0;
}
