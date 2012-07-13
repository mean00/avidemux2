#include "VideoOutput.h"

namespace ADM_qtScript
{
    VideoOutput::VideoOutput(IEditor *editor, FilterInfo *info) : QtScriptObject(editor)
    {
        this->_width = info->width;
        this->_height = info->height;
        this->_duration = info->totalDuration;
    }

    QScriptValue VideoOutput::getHeight()
    {
        return this->_height;
    }

    QScriptValue VideoOutput::getDuration()
    {
        return (double)this->_duration;
    }

    QScriptValue VideoOutput::getWidth()
    {
        return this->_width;
    }
}
