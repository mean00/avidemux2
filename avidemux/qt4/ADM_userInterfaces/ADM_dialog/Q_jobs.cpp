/**
    Jobs dialog
    (c) Mean 2007
*/
#include <stdio.h>
#include <stdlib.h>
#include "Q_jobs.h"
#include "DIA_coreToolkit.h"
#include "ADM_script.h"

static void updateStatus(void);
extern bool parseECMAScript(const char *name);
static const char *StringStatus[]={QT_TR_NOOP("Ready"),QT_TR_NOOP("Succeeded"),QT_TR_NOOP("Failed"),QT_TR_NOOP("Deleted"),QT_TR_NOOP("Running")};

ADM_Job_Descriptor::ADM_Job_Descriptor(void)
{
	status = STATUS_READY;
	memset(&startDate, 0, sizeof(startDate));
	memset(&endDate, 0, sizeof(startDate));
}

 /**
          \fn jobsWindow
 */
jobsWindow::jobsWindow(uint32_t n,char **j)     : QDialog()
 {
     ui.setupUi(this);
     _nbJobs=n;
     _jobsName=j;
     desc=new ADM_Job_Descriptor[n];
     // Setup display
	 ui.tableWidget->setRowCount(_nbJobs);
     ui.tableWidget->setColumnCount(4);

     // Set headers
      QStringList headers;
     headers << QT_TR_NOOP("Job Name") << QT_TR_NOOP("Status") << QT_TR_NOOP("Start Time") << QT_TR_NOOP("End Time");

     ui.tableWidget->setVerticalHeaderLabels(headers);
     updateRows();

#define CNX(x) connect( ui.pushButton##x,SIGNAL(clicked(bool)),this,SLOT(x(bool)))
           //connect( ui.pushButtonRunOne,SIGNAL(buttonPressed(const char *)),this,SLOT(runOne(const char *)));
      CNX(RunOne);
      CNX(RunAll);
      CNX(DeleteAll);
      CNX(DeleteOne);
 }
 /**
    \fn ~jobsWindow
 */
jobsWindow::~jobsWindow()
{
	delete [] desc;
}
 /*
    There is maybe a huge mem leak here
 */
static void ADM_setText(const char *txt,uint32_t col, uint32_t row,QTableWidget *w)
{
        QString str(txt);
        QTableWidgetItem *newItem = new QTableWidgetItem(str);//GetFileName(_jobsName[i]));
        w->setItem(row, col, newItem);

}
 /**
      \fn updateRaw
      \brief update display for raw x
 */
void jobsWindow::updateRows(void)
{
   ui.tableWidget->clear();
   ADM_Job_Descriptor *j;
   char str[20];
   for(int i=0;i<_nbJobs;i++)
   {
      j=&(desc[i]);
      ADM_setText(ADM_GetFileName(_jobsName[i]),0,i,ui.tableWidget);
      ADM_setText(StringStatus[j->status],1,i,ui.tableWidget);

      sprintf(str,"%02u:%02u:%02u",j->startDate.hours,j->startDate.minutes,j->startDate.seconds);
      ADM_setText(str,2,i,ui.tableWidget);

      sprintf(str,"%02u:%02u:%02u",j->endDate.hours,j->endDate.minutes,j->endDate.seconds);
      ADM_setText(str,3,i,ui.tableWidget);
   }
}



/**
      \fn deleteOne
      \brief delete one job
*/
void jobsWindow::DeleteOne(bool b)
{
	int sel = ui.tableWidget->currentRow();

	if (sel >= 0 && sel < _nbJobs)
	{
		if (GUI_Confirmation_HIG(QT_TR_NOOP("Sure!"), QT_TR_NOOP("Delete job"), QT_TR_NOOP("Are you sure you want to delete %s job?"), ADM_GetFileName(_jobsName[sel])))
		{
			desc[sel].status = STATUS_DELETED;
			unlink(_jobsName[sel]);
			updateRows();
		}
	}
}
/**
      \fn deleteAll
      \brief delete all job
*/
void jobsWindow::DeleteAll(bool b)
{
	if (GUI_Confirmation_HIG(QT_TR_NOOP("Sure!"), QT_TR_NOOP("Delete *all* job"), QT_TR_NOOP("Are you sure you want to delete ALL jobs?")))
	{
		for(int sel = 0; sel < _nbJobs; sel++)
		{
			desc[sel].status = STATUS_DELETED;
			unlink(_jobsName[sel]);
		}

		updateRows();
	}
}

/**
      \fn runOne
      \brief Run one job
*/
void jobsWindow::RunOne(bool b)
{
	int sel = ui.tableWidget->currentRow();
	printf("Selected %d\n", sel);

	if(sel >= 0 && sel < _nbJobs)
	{
		if(desc[sel].status == STATUS_SUCCEED)
			GUI_Info_HIG(ADM_LOG_INFO,QT_TR_NOOP("Already done"),QT_TR_NOOP("This script has already been successfully executed."));
		else
		{
			desc[sel].status=STATUS_RUNNING;
			updateRows();
			GUI_Quiet();
			TLK_getDate(&(desc[sel].startDate));

			if (getSpiderMonkeyEngine()->runScriptFile(_jobsName[sel]))
				desc[sel].status=STATUS_SUCCEED;
			else
				desc[sel].status=STATUS_FAILED;

			TLK_getDate(&(desc[sel].endDate));
			updateRows();
			GUI_Verbose();
		}
	}
}
/**
      \fn RunAll
      \brief Run all jobs
*/
void jobsWindow::RunAll(bool b)
{
	for(int sel=0;sel<_nbJobs;sel++)
	{
		if(desc[sel].status == STATUS_SUCCEED || desc[sel].status == STATUS_DELETED)
			continue;

		desc[sel].status=STATUS_RUNNING;
		updateRows();
		GUI_Quiet();
		TLK_getDate(&(desc[sel].startDate));

		if (getSpiderMonkeyEngine()->runScriptFile(_jobsName[sel]))
			desc[sel].status=STATUS_SUCCEED;
		else
			desc[sel].status=STATUS_FAILED;

		TLK_getDate(&(desc[sel].endDate));
		updateRows();
		GUI_Verbose();
	}
}

/**
    \fn     DIA_job
    \brief
*/
uint8_t  DIA_job(uint32_t nb, char **name)
{
  uint8_t r=0;
  jobsWindow jobswindow(nb,name) ;

     if(jobswindow.exec()==QDialog::Accepted)
     {
       r=1;
     }
     return r;
}

//EOF
