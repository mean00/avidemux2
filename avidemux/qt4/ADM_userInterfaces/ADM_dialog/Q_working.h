#ifndef Q_working_h
#define Q_working_h

#include "ui_working.h"

class workWindow : public QDialog
{
	Q_OBJECT
    
public:
    bool active;
	workWindow(QWidget *parent);
	Ui_workingDialog *ui;
public slots:
    void stop(bool a);
protected:
  void closeEvent(QCloseEvent *event)
    {
        stop(true);
    }
      
};
#endif	// Q_working_h
