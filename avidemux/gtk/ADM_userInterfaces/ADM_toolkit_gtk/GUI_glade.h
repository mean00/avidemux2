/**
 *      \file GUI_glade.h
 *      \brief simple utility class to deal with glade class
 */

#ifndef ADM_GLADE_H
#define ADM_GLADE_H
#include <gtk/gtk.h>
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
                admGlade() ;
        void    init(void);
                ~admGlade();
        bool    loadFile(const char *file);   
        GtkWidget *getWidget(const char *widgetName);



};



#endif

