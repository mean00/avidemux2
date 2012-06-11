#include "SegmentProperties.h"

namespace ADM_qtScript
{
    SegmentProperties::SegmentProperties(IEditor *editor, _SEGMENT* segment) : QtScriptObject(editor)
    {
        this->_videoIndex = segment->_reference;
        this->_duration = segment->_durationUs;
        this->_absoluteStartTime = segment->_startTimeUs;
        this->_relativeStartTime = segment->_refStartTimeUs;
    }

    QScriptValue SegmentProperties::getAbsoluteStartTime(void)
    {
        return (double)this->_absoluteStartTime;
    }

    QScriptValue SegmentProperties::getDuration(void)
    {
        return (double)this->_duration;
    }

    QScriptValue SegmentProperties::getVideoIndex(void)
    {
        return this->_videoIndex;
    }

    QScriptValue SegmentProperties::getRelativeStartTime(void)
    {
        return (double)this->_relativeStartTime;
    }
}
