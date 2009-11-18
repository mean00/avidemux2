#ifndef Q_working_h
#define Q_working_h

#include "ui_working.h"

class workWindow : public QDialog
{
	Q_OBJECT
    
public:
    bool active;
	workWindow();
	Ui_workingDialog ui;
public slots:
    void stop(bool a);
};
#endif	// Q_working_h
