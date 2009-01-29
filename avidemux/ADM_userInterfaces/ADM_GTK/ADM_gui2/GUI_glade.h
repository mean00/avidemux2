/**
 *      \file GUI_glade.h
 *      \brief simple utility class to deal with glade class
 */

#ifndef ADM_GLADE_H
#define ADM_GLADE_H
#include <glade/glade.h>
/**
    \class admGlade
    \brief simple glade wrapping class
*/
class admGlade
{
protected:
        void *gxml;
        bool tryLoad(const char *prefix, const char *file);
public:
                admGlade() {gxml=NULL;}
                ~admGlade();
        bool    loadFile(const char *file);   
        GtkWidget *getWidget(const char *widgetName);



};



#endif

