#include "FrameProperties.h"
#include "ADM_imageFlags.h"

namespace ADM_qtScript
{
	FrameProperties::FrameProperties(IEditor * editor, uint64_t time) : QtScriptObject(editor)
	{
		this->_time = time;

		uint32_t flags, quantiser;

		if (time == editor->getCurrentFramePts())
		{
			editor->getCurrentFrameFlags(&flags, &quantiser);

			this->_fieldType = this->getFieldType(flags);
			this->_frameType = this->getFrameType(flags);
			this->_quantiser = quantiser;
		}
		else
		{
			// currently too awkward to retrieve properties of a frame
			// without affecting the editor since everything is tightly coupled
			this->_fieldType = FrameProperties::UnknownField;
			this->_frameType = FrameProperties::UnknownFrame;
			this->_quantiser = 0;
		}
	}

    QScriptValue FrameProperties::getFieldType(void)
    {
        return (int)this->_fieldType;
    }

    QScriptValue FrameProperties::getFrameType(void)
    {
        return (int)this->_frameType;
    }

    QScriptValue FrameProperties::getQuantiser(void)
    {
        return this->_quantiser;
    }

    QScriptValue FrameProperties::getTime(void)
    {
        return (double)this->_time;
    }

    FrameProperties::CodingType FrameProperties::getFrameType(int frameType)
    {
        switch (frameType & AVI_FRAME_TYPE_MASK)
        {
            case AVI_KEY_FRAME:
                return FrameProperties::I_Frame;

            case AVI_B_FRAME:
                return FrameProperties::B_Frame;

            case 0:
                return FrameProperties::P_Frame;

            default:
                return FrameProperties::UnknownFrame;
        }
    }

    FrameProperties::FieldType FrameProperties::getFieldType(int frameType)
    {
        switch (frameType & AVI_STRUCTURE_TYPE_MASK)
        {
            case AVI_TOP_FIELD + AVI_FIELD_STRUCTURE:
                return FrameProperties::TopField;

            case AVI_BOTTOM_FIELD + AVI_FIELD_STRUCTURE:
                return FrameProperties::BottomField;

            case AVI_FRAME_STRUCTURE:
                return FrameProperties::NoField;

            default:
                return FrameProperties::UnknownField;
        }
    }
}
