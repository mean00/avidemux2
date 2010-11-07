#include "ADM_default.h"
#include "ADM_coreJobs.h"

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
