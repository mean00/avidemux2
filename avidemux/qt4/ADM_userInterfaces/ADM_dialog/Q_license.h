#ifndef Q_license_h
#define Q_license_h

#include "ui_license.h"

class Ui_licenseWindow : public QDialog
{
	Q_OBJECT

private:
	Ui_licenseDialog ui;

public:
	Ui_licenseWindow(QWidget *parent);
};

#endif	// Q_license_h
