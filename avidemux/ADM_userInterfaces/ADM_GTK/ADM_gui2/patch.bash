cat GUI_main2.cpp | sed 's/, 0,$/, (GdkModifierType) 0,/g' | sed 's/-2/-1/g' | sed 's/GtkAccelGroup \*/extern GtkAccelGroup */' | sed 's/(GdkModifierType)/(GdkModifierType)(/g' | sed 's/MASK,$/MASK\),/g' |sed 's/( 0,$/( 0 ),/g' | \
sed 's/sliderNavigate = gtk_hscale_new/sliderNavigate = gtk_markscale_new/' | \
sed 's/jog_shuttle_new ("jogg", "", "", 0, 0);/jog_shuttle_new ();gtk_widget_set_size_request (jogg, -1, 16); /' | \
sed 's/gtk_widget_show (sliderNavigate);/gtk_widget_show (sliderNavigate);gtk_scale_set_draw_value (GTK_SCALE (sliderNavigate), FALSE);/' |\
sed 's/#include "interface.h"/#include "..\/ADM_toolkit_gtk\/jogshuttle.h"/' |\
sed 's/#include "support.h"/#include "..\/ADM_toolkit_gtk\/gtkmarkscale.h"/' |\
sed 's/#include "callbacks.h"/#include "..\/ADM_toolkit_gtk\/ADM_gladeSupport.h"/' |\
sed 's/_(/QT_TR_NOOP(/g'  \
> GUI_main2.p
cp GUI_main2.cpp GUI_main2.tmp
cp GUI_main2.p GUI_main2.cpp
#sed 's/#include "interface.h"//' |\
#include "../ADM_toolkit_gtk/ADM_gladeSupport.h"/' |\
#sed 's/#include "support.h"//'\
#sed 's/#include "callbacks.h"/#include "../ADM_toolkit_gtk/ADM_gladeSupport.h"/' |\

