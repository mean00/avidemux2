#include "SegmentCollectionPrototype.h"

namespace ADM_qtScript
{
	SegmentCollectionPrototype::SegmentCollectionPrototype(QObject* parent, IEditor* editor) :
		QtScriptObject(editor)
	{
		this->setParent(parent);
		this->_editor = editor;
	}

	QScriptValue SegmentCollectionPrototype::getLength()
	{
		return this->_editor->getNbSegment();
	}

	QScriptValue SegmentCollectionPrototype::getTotalDuration(void)
	{
		return (double)this->_editor->getVideoDuration();
	}

	QScriptValue SegmentCollectionPrototype::add(QScriptValue startTime, QScriptValue duration, QScriptValue videoIndex)
	{
		if (!this->_editor->getVideoCount())
        {
            return this->throwError(QString(QT_TR_NOOP("A video must be open to perform this operation.")));
        }

        QScriptValue result;

        while (true)
        {
            result = this->validateNumber("videoIndex", videoIndex, 0, this->_editor->getVideoCount());

            if (!result.isUndefined())
            {
                break;
            }

            this->_editor->addSegment(videoIndex.toNumber(), startTime.toNumber(), duration.toNumber());

            result = this->_editor->getNbSegment() - 1;

            break;
        }

        return result;
	}

	void SegmentCollectionPrototype::clear()
	{
		if (this->_editor->getVideoCount())
        {
            this->_editor->clearSegment();
        }
	}
}
