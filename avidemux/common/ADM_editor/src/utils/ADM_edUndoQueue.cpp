/** *************************************************************************
     \file       ADM_edUndoQueue.cpp
     \brief      Handle Undo

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_segment.h"
#include "ADM_edit.hxx"

static const uint8_t maxUndoSteps=50;
uint32_t _cnt; // track the nb of performed undo steps for redo

/**
    \fn addToUndoQueue
    \brief stores the segment layout and markers in the undo queue
*/

bool ADM_Composer::addToUndoQueue(void)
{
    // truncate the dead branch first if we add a new element to the undo queue after _cnt-1 undo steps
    if(_cnt>1 && undoQueue.size()>1)
    {
        for(uint32_t i=0;i<_cnt;i++)
        {
            undoQueue.pop_back();
        }
        ADM_info("Deleted last %d elements from the undo queue\n",_cnt);
    }
    // now populate the new element...
    undoQueueElem rec;
    uint64_t a=getMarkerAPts();
    uint64_t b=getMarkerBPts();
    rec.segm=_segments.getSegments();
    rec.markerA=a;
    rec.markerB=b;
    // ...do some housekeeping...
    uint32_t m=maxUndoSteps;
    if(m<10) m=10; // less than 10 undo steps is a bad idea, ignore the limit then
    uint32_t nb=undoQueue.size();
    if(nb>m)
    {
        // erase the oldest records if the limit is exceeded and create space for a new one
        undoQueue.erase(undoQueue.begin(),undoQueue.begin()+nb-m+1);
    }
    // ...and store the new element in the queue
    undoQueue.push_back(rec);
    ADM_info("The undo queue has now %d element(s)\n",undoQueue.size());
    _cnt=0; // redo should not be available after a new undo-able action has been performed
    return true;
}

/**
    \fn undo
    \brief restores the last recorded state of segments and markers
*/

bool ADM_Composer::undo(void)
{
    if(undoQueue.empty() || undoQueue.size()<_cnt+1)
    {
        ADM_info("The undo queue is empty, nothing to do\n");
        return false;
    }
    if(!_cnt) // no undo steps performed yet, thus we don't have the current state in the undo queue
    {
        // store the current state in the queue and bump the tracker
        // to account for the queue becoming one element longer
        addToUndoQueue();
        _cnt=1;
    }
    undoQueueElem rec=undoQueue.at(undoQueue.size()-_cnt-1);
    ADM_info("Restoring the state recorded in the element %d starting with 1 of %d\n",undoQueue.size()-_cnt,undoQueue.size());
    _segments.setSegments(rec.segm);
    setMarkerAPts(rec.markerA);
    setMarkerBPts(rec.markerB);
    _cnt++;
    return true;
}

/**
    \fn redo
    \brief reinstates the last undone state of segments and markers
*/

bool ADM_Composer::redo(void)
{
    if(_cnt<2 || undoQueue.size()<_cnt) // _cnt=2 once the first undo operation has been performed
    {
        ADM_info("The redo queue is empty, cannot perform redo\n");
        return false;
    }
    undoQueueElem rec=undoQueue.at(undoQueue.size()-_cnt+1);
    ADM_info("Restoring the state recorded in the element %d starting with 1 of %d\n",undoQueue.size()-_cnt+2,undoQueue.size());
    _segments.setSegments(rec.segm);
    setMarkerAPts(rec.markerA);
    setMarkerBPts(rec.markerB);
    _cnt--;
    return true;
}

/**
    \fn clearUndoQueue
*/

bool ADM_Composer::clearUndoQueue(void)
{
    undoQueue.clear();
    return true;
}
//EOF
