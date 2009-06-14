/**
 *      \file GUI_glade.cpp
 *      \brief simple utility class to deal with glade class
 */

#include "GUI_glade.h"
#define GXML (GladeXML *)gxml
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
            GladeXML *x;
            snprintf(path,ADM_GLADE_PATH,"%s/%s",prefix,file);
            printf("Trying :<%s>\n",path);
            x = glade_xml_new (path, NULL, NULL);
            if(!x) return false;
            gxml=(void *)x;
           // glade_xml_signal_autoconnect (x);
            return true;


}
/**
 *  \fn getWidget
 *
 */
GtkWidget *admGlade::getWidget(const char *widgetName)
{
        if(!gxml) return NULL;
        GtkWidget *w= glade_xml_get_widget (GXML, widgetName);
        if(!w) printf("[admGlade] Cannot locate widget %s\n",widgetName);
        return w;
}   
//EOF






