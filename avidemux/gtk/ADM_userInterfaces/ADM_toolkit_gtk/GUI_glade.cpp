/**
 *      \file GUI_glade.cpp
 *      \brief simple utility class to deal with glade class
 */
#include <stdlib.h>
#include "GUI_glade.h"
#define GXML (GtkBuilder *)gxml

admGlade::admGlade()
{
    gxml=NULL;
}
void admGlade::init(void)
{
GtkBuilder *x=NULL;
    x =gtk_builder_new ();
    if(!x)
    {
        printf("[GtkBuilder] Cannot create a builder\n");
        exit(-1);
    }
    gxml=(void *)x; // Memleak!

}

/**
 *
 */
admGlade::~admGlade()
{

}
/**
        \fn loadFile
*/
bool    admGlade::loadFile(const char *file)   
{
    if(tryLoad(TARGET_DIR,file)) return true;
    if(tryLoad(SOURCE_DIR"/../glade",file)) return true;
    if(tryLoad("glade",file)) return true;
    return false;

}
/**
        \fn tryLoad
        \brief try loading the file in the dir prefix
*/
bool admGlade::tryLoad(const char *prefix, const char *file)
{
#define ADM_GLADE_PATH 1024
char path[ADM_GLADE_PATH];
GError *er=NULL;
            GtkBuilder *x;
            snprintf(path,ADM_GLADE_PATH,"%s/%s",prefix,file);
            printf("Trying :<%s>\n",path);
           
            gtk_builder_add_from_file(GXML,path,&er);
            if(er)
            {
                printf("[GtkBuilder] %s\n",er->message);
                return false;
            }
            return true;


}
/**
 *  \fn getWidget
 *
 */
GtkWidget *admGlade::getWidget(const char *widgetName)
{
        if(!gxml) return NULL;
        GtkWidget *w= (GtkWidget *)gtk_builder_get_object (GXML, widgetName);
        if(!w) printf("[admGlade] Cannot locate widget %s\n",widgetName);
        return w;
}   
//EOF






