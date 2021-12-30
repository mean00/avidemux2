#ifndef Q_log_h
#define Q_log_h

#include "ui_log.h"

class Ui_logWindow : public QDialog
{
	Q_OBJECT

private:
	Ui_logDialog ui;

public:
	Ui_logWindow(QWidget *parent);
};

#endif	// Q_log_h
