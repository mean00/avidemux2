#ifndef T_dialogFactory_h
#define T_dialogFactory_h

#include "ADM_UIQT46_export.h"
#include <QtGui/QDialog>

ADM_UIQT46_EXPORT void InitFactory(void);

class factoryWindow : public QDialog
{
	Q_OBJECT

public:
	factoryWindow();
};
#endif	// T_dialogFactory_h
