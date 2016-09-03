#ifndef Q_props_h
#define Q_props_h

#include "ui_props.h"

class propWindow : public QDialog
{
	Q_OBJECT

public:
	propWindow(QWidget *parent);
	Ui_propsDialog ui;
};
#endif	// Q_props_h
