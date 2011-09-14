#ifndef Q_about_h
#define Q_about_h
#include "ui_about.h"

class Ui_aboutWindow : public QDialog
{
	Q_OBJECT

private:
	Ui_aboutDialog ui;

public:
	Ui_aboutWindow(QWidget* parent);

private slots:
	void licenseButton_clicked(bool);
};

#endif	// Q_about_h
