#ifndef Q_jobs_h
#define Q_jobs_h

#include "ui_jobs.h"
#include "ADM_misc.h"

typedef enum
{
	STATUS_READY = 0,
	STATUS_SUCCEED,
	STATUS_FAILED,
	STATUS_DELETED,
	STATUS_RUNNING
} JOB_STATUS;

class ADM_Job_Descriptor
{
public:
	JOB_STATUS status;
	ADM_date startDate;
	ADM_date endDate;

	ADM_Job_Descriptor(void);
};

class jobsWindow : public QDialog
{
	Q_OBJECT

protected:
	uint32_t _nbJobs;
	char **_jobsName;
	ADM_Job_Descriptor *desc;

	void updateRows(void);

public:
	jobsWindow(uint32_t n, char **j);
	~jobsWindow();
	Ui_jobsDialog ui;

public slots:
      void RunOne(bool b);
      void RunAll(bool b);
      void DeleteOne(bool b);
      void DeleteAll(bool b);
};
#endif	// Q_jobs_h
