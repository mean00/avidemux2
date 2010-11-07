#ifndef Q_job_h
#define Q_job_h

#include <QtGui/QWidget>
#include "ui_uiJobs.h"

/**
    \class jobWindow
*/
class jobWindow   : public QDialog 
{
	Q_OBJECT

public:
                jobWindow(void);
	virtual     ~jobWindow();	    
	
protected:
    Ui_jobs     ui;
//public slots:
};
#endif	// Q_gui2_h
