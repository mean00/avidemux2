/**
    \file   ADM_jobControl
    \author mean fixounet@free.Fr (c) 2010

*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "T_jobs.h"
#include "ADM_default.h"
#include "ADM_coreJobs.h"


static QTableWidgetItem *fromText(const string &t)
{
    QString s(QString::fromUtf8(t.c_str()));
    QTableWidgetItem *w=new QTableWidgetItem(s,0);
    return w;
}

/**
    \fn refreshList
*/
void jobWindow::refreshList(void)
{
      QTableWidget *list=ui.tableWidget;
      list->clear();
      listOfJob.clear();
      if(false==ADM_jobGet(listOfJob)) return ;

      int n=listOfJob.size();
      ADM_info("Found %d jobs\n",(int)n);
      if(!n) return;
      list->setRowCount(n);
      for(int i=0;i<n;i++)
      {
           QTableWidgetItem *nm=fromText(listOfJob[i].jobName);
           QTableWidgetItem *out=fromText(listOfJob[i].outputFileName);
           string s;
            switch(listOfJob[i].status)
            {
                case ADM_JOB_IDLE:
                            s=string("Ready");
                            break;
                case ADM_JOB_RUNNING:
                            s=string("Running....");
                            break;
                case ADM_JOB_OK:
                            s=string("Success");
                            break;
                case ADM_JOB_KO:
                            s=string("Failed");
                            break;
                default:
                            s=string("???");
                            break;
            }
        QTableWidgetItem *status=fromText (s);
        list->setItem(i,0,nm);
        list->setItem(i,1,out);
        list->setItem(i,2,status);
      }
}


/**
    \fn ctor
*/
jobWindow::jobWindow(void) : QDialog()
{
    ui.setupUi(this);
    ui.tableWidget->setColumnCount(3); // Job name, fileName, Status
    refreshList();
}
/**
    \fn dtor
*/
jobWindow::~jobWindow()
{

}

/**
    \fn jobRun
*/
bool jobRun(int ac,char **av)
{
    QApplication *app=new QApplication(ac,av,0);
    jobWindow *jWindow=new jobWindow();
    jWindow->exec();
    delete jWindow;
    delete app;
    return true;
}


#if 0
bool jobRun(void)
{

        ADM_jobDropAllJobs();

        vector <ADMJob> jobs;
        ADM_jobGet(jobs);

        int n=jobs.size();
        printf("Found %d jobs...\n",n);

        ADMJob job;
        job.endTime=2;
        job.startTime=3;
        job.scriptName="myscript1";
        job.jobName="myjob1";
        job.outputFileName="output1";
        
        ADM_jobAdd(job);


        
        ADM_jobGet(jobs);

        n=jobs.size();
        printf("Found %d jobs...\n",n);
        for(int i=0;i<n;i++)
        {
            printf("%d/%d ",i,n);
            ADM_jobDump(jobs[i]);
        }
        printf("\n***************************\n");
        jobs[0].status=ADM_JOB_KO;
        ADM_jobUpdate(jobs[0]);
        n=jobs.size();
        printf("Found %d jobs...\n",n);
        for(int i=0;i<n;i++)
        {
            printf("%d/%d ",i,n);
            ADM_jobDump(jobs[i]);
        }
        printf("\n***************************\n");
        // Delete
        ADM_jobDelete(jobs[0]);
        ADM_jobGet(jobs);

        n=jobs.size();
        printf("Found %d jobs...\n",n);
        for(int i=0;i<n;i++)
        {
            printf("%d/%d ",i,n);
            ADM_jobDump(jobs[i]);
        }
        return true;
}
#endif