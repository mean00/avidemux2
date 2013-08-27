#include <string.h>
#include <stdio.h>
#include <va/va.h>

int main(int argc, char **argv)
{
        unsigned int       num_attribs;
        VADisplay dpy;
        int w,h,format,num_surface;
        VASurfaceID id;
        vaCreateSurfaces(dpy,w,h,format,1,&id);
        return 0;
}
