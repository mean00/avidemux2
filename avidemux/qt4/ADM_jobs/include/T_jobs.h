#ifndef Q_job_h
#define Q_job_h

#include <QtGui/QWidget>
#include "ui_uiJobs.h"
#include <vector>
#include <string>
using std::string;
using std::vector;
class jobWindow;
#include "ADM_default.h"
#include "ADM_coreSocket/include/ADM_coreSocket.h"

typedef enum
{
    JobAction_setReady,
    JobAction_setOk,
    JobAction_runNow,
    JobAction_delete
}JobAction;

typedef struct
{
    jobWindow *me;
    const char *exeName;
    string script;
    string outputFile;
}spawnData;

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
    bool        runProcess(spawnData *data);
protected:
    ADM_socket  mySocket;
    uint32_t    localPort;
protected:
    int         getActiveIndex(void)	;
    bool        runOneJob(ADMJob &job)   ;
    bool        spawnChild(const char *exeName, const string &script, const string &outputFile);
    bool        popup(const char *errorMessage);
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

