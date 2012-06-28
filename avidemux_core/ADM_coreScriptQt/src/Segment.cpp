#include "Segment.h"

namespace ADM_qtScript
{
    Segment::Segment(IEditor *editor, _SEGMENT* segment) : QtScriptObject(editor)
    {
        this->_videoIndex = segment->_reference;
        this->_duration = segment->_durationUs;
        this->_absoluteStartTime = segment->_startTimeUs;
        this->_relativeStartTime = segment->_refStartTimeUs;
    }

    QScriptValue Segment::getAbsoluteStartTime(void)
    {
        return (double)this->_absoluteStartTime;
    }

    QScriptValue Segment::getDuration(void)
    {
        return (double)this->_duration;
    }

    QScriptValue Segment::getVideoIndex(void)
    {
        return this->_videoIndex;
    }

    QScriptValue Segment::getRelativeStartTime(void)
    {
        return (double)this->_relativeStartTime;
    }
}
