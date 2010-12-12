/**
    \file   ADM_jobProgress
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
#include "T_progress.h"
#include "ADM_default.h"

/**
    \fn ctor
*/
jobProgress::jobProgress(uint32_t nbJobs)
{
    numberOfJobs=nbJobs;
    currentOutputFile=string("");
    currentJob=0;
    percent=0;
    ui.setupUi(this);
    ui.progressBar->setMinimum(0);
    ui.progressBar->setMaximum(100);
    ui.progressBar->setValue(0);
    ADM_info("Starting progress dialog\n");
    open();
    QApplication::processEvents();
}
/**
    \fn dtor
*/
jobProgress::~jobProgress()
{
    ADM_info("Deleting progress..\n");
}
/**

*/
void  jobProgress::setCurrentJob(uint32_t job)
{
    currentJob=job;
    updateUi();
}
/**

*/
void  jobProgress::setCurrentOutputName(const string &name)
{
    currentOutputFile=name;
    updateUi();

}
/**

*/
void  jobProgress::setPercent(uint32_t percent)
{
    this->percent=percent;
    updatePercent();
}
/**

*/
void     jobProgress::updateUi(void)
{
    char buffer[128];
    sprintf(buffer,"%d/%d",currentJob+1,numberOfJobs);
    ui.labelJobCount->setText(buffer);
    ui.labelOutputFile_2->setText(currentOutputFile.c_str());
    updatePercent();
}
/**

*/
void     jobProgress::updatePercent(void)
{
    ui.progressBar->setValue(percent);
    QApplication::processEvents();
}
// EOF