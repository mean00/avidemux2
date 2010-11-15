#ifndef Q_job_h
#define Q_job_h

#include <QtGui/QWidget>
#include "ui_uiJobs.h"
#include <vector>
using std::vector;

typedef enum
{
    JobAction_setReady,
    JobAction_setOk,
    JobAction_runNow,
    JobAction_delete
}JobAction;


class ADMJob;
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
    int         getActiveIndex(void)	;
    bool        runOneJob(ADMJob &job)   ;

protected:
    Ui_jobs     ui;
    void        refreshList(void);
    vector      <ADMJob> listOfJob;
    void        runAction(JobAction action);
public slots:
    // Actions
    
    void        del(void); 
    void        setOk(void); 
    void        setReady(void); 
    void        runNow(void); 
    void        quit(void);
    void        runAllJob(void);
};
#endif	// Q_gui2_h

