/**
    \file   ADM_coreJobs
    \brief  Handle low level access to jobs
    \author (C) 2010 by mean fixounet@free.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_coreJobs.h"
#include "ADM_default.h"
#include "sqlJobs.h"

static char *dbFile=NULL;
Database    *mydb=NULL;

#define ADM_DB_SCHEMA 3

static const char *createString1="\
CREATE TABLE version(\
value integer not null\
);";
static const char *createString2="\
CREATE TABLE jobs(\
id integer primary key autoincrement not null,\
jscript varchar(100) default '' not null,\
jobname varchar(100) default '' not null,\
outputFile varchar(256) default '' not null,\
status integer,\
startTime date,\
endTime date\
);\
";

/**
    \fn ADM_jobInitializeDb
*/
static bool ADM_jobInitializeDb(void)
{
    bool r=true;
    Database *m=new Database(dbFile);
    if(!m->Connected())
    {
        ADM_warning("Cannot create database  %s \n",dbFile);
        return false;
    }
    ADM_info("Creating database schema...\n");
    Query q(*m);
    r=q.execute(createString1);
    r=q.execute(createString2);
    q.execute("COMMIT;");

    if(r){
        // update version
        char s[256];
        sprintf(s,"INSERT INTO version (value) VALUES (%d);",ADM_DB_SCHEMA);
        r=q.execute(s);
        delete m;
    }
    return r;
}
/**
        \fn dbInit
*/
static bool dbInit(void)
{
    mydb=new Database(dbFile);
    if(!mydb->Connected())
    {
        delete mydb;
        mydb=NULL;
        return false;
    }
    return true;
}
/**
        \fn dbCleanup
*/

static bool dbCleanup(void)
{
    if(mydb)
    {
        delete mydb;
        mydb=NULL;
    }
    return true;
}
/**
    \fn ADM_jobCheckVersion
    \brief returns true if the db has the right version
*/
static bool ADM_jobCheckVersion(void)
{
    if(!mydb) return false;
    Query q(*mydb);
	q.get_result("select * from version");
	if(!q.fetch_row())
    {
        ADM_warning("Cannot get version\n");
        return false;
    }
    int dbVersion=q.getval();
    q.free_result();
    ADM_info("Db version %d, our version %d\n",dbVersion,ADM_DB_SCHEMA);
    if(dbVersion==ADM_DB_SCHEMA)
    {
        ADM_info("Same version, continuing..\n");
        return true;
    }
    ADM_info("Version mismatch, recreating db..\n");
    return false;
}
/**
    \fn ADM_jobInit
    \brief init sql and friends
*/
bool    ADMJob::jobInit(void)
{
    dbFile=new char[1024];
    strcpy(dbFile,ADM_getBaseDir());
    strcat(dbFile,"jobs.sql");

    ADM_info("Initializing database (%s)\n",dbFile);
    if(!ADM_fileExist(dbFile))
    {
        ADM_warning("[Jobs] jobs.sql does not exist, creating from default...\n");
        if(!ADM_jobInitializeDb())
        {
            ADM_warning("[Jobs] Db Init failed\n");
            return false;
        }
        ADM_info("Database created\n");
    }
    if(false==dbInit())
    {
        ADM_warning("Cannot initialize database \n");
        dbCleanup();
        return false;
    }
    // Check DB version...
    if(false==ADM_jobCheckVersion())
    {
        ADM_info("Bad database version...\n");
        dbCleanup();
        unlink(dbFile);
        if(true==ADM_jobInitializeDb())
        {
            if(false==dbInit())
            {
                dbCleanup();
                ADM_warning("Cannot recreate database\n");
                return false;
            }
        }
    }
    //
    ADM_info("Successfully connected to jobs database..\n");
    return true;
}
/**
        \fn ADM_jobShutDown
*/
bool    ADMJob::jobShutDown(void)
{
	if (dbFile)
	{
		delete [] dbFile;
	}

    dbCleanup();
    ADM_info("Shutting down jobs database\n");
    return true;
}
/**
    \fn ADM_dumpJobs
*/
bool ADMJob::jobDump(const ADMJob &job)
{

    printf("Id       :%d\n",job.id);
    printf("Name     :%s\n",job.jobName.c_str());
    printf("Script   :%s\n",job.scriptName.c_str());
    printf("Output   :%s\n",job.outputFileName.c_str());
    printf("Status   :%d\n",job.status);
    printf("Start    :%"PRId64"\n",job.startTime);
    printf("End      :%"PRId64"\n",job.endTime);
    return true;
}

/**
    \fn ADM_jobAdd
    \brief add a job
*/
bool    ADMJob::jobAdd(const ADMJob& job)
{
        if(!mydb)
        {   
            ADM_warning("No database to save jobs\n");
            return false;
        }
        db::Jobs myJob(mydb);
#define OP(x,y) myJob.Set##x(job.y);

        OP(Jscript,scriptName)
        OP(Jobname,jobName)
        OP(Outputfile,outputFileName)
#undef OP
#define OP(x,y) myJob.Set##x(y);

        OP(Status,ADM_JOB_IDLE)
        OP(Starttime,0)
        OP(Endtime,0)
        myJob.save();
        return true;
}
/**
    \fn ADM_jobGet
    \brief Get all jobs as a vector
*/
bool    ADMJob::jobGet(vector <ADMJob> &jobs)
{
    jobs.clear();
    if(!mydb) return false;
    Query q(*mydb);
	q.get_result("select * from jobs");
	while (q.fetch_row())
	{
        printf("*\n");
		db::Jobs oneJob(mydb,&q); // spawns an object from Query object
        ADMJob newJob;
        newJob.id=oneJob.GetId();
        newJob.jobName=oneJob.GetJobname();
        newJob.scriptName=oneJob.GetJscript();
        newJob.outputFileName=oneJob.GetOutputfile();
        newJob.startTime=oneJob.GetStarttime();
        newJob.endTime=oneJob.GetEndtime();
        newJob.status=(ADM_JOB_STATUS)oneJob.GetStatus();
        jobs.push_back(newJob);
	}
	q.free_result();
    return true;

}
/**
    \fn ADM_jobUpdate
    \brief update an existing job, only date & status are updated
*/
bool    ADMJob::jobUpdate(const ADMJob & job)
{
    if(!mydb) return false;
    int id=job.id;
    db::Jobs myJob(*mydb,id);
//#warning detect invalid one ?
    myJob.SetStarttime(job.startTime);
    myJob.SetEndtime(job.endTime);
    myJob.SetStatus(job.status);
    myJob.save();
    return true;
}
/**
    \fn ADM_jobDropAllJobs
    \brief Empty the database
*/
bool    ADMJob::jobDropAllJobs(void)
{
    if(!mydb) return false;
    Query q(*mydb);
    q.get_result("delete from jobs");
    q.free_result();
    return true;
}
/**
    \fn     ADM_jobDelete
    \brief  Delete the job given as arg
*/
bool    ADMJob::jobDelete(const ADMJob& job)
{
    if(!mydb) return false;
    Query q(*mydb);
    char cmd[256];
    sprintf(cmd,"delete from jobs where id=%d",job.id);
    ADM_info("%s\n",cmd);
    q.get_result(cmd);
    return true;
}
//EOF
