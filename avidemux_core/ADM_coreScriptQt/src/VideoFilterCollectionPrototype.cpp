#include "ADM_inttype.h"
#include "VideoFilter.h"
#include "ADM_coreVideoFilterFunc.h"
#include "VideoFilterCollectionPrototype.h"

extern BVector <ADM_VideoFilterElement> ADM_VideoFilters;

namespace ADM_qtScript
{
    VideoFilterCollectionPrototype::VideoFilterCollectionPrototype(QObject* parent, IEditor* editor) :
        QtScriptObject(editor)
    {
        this->setParent(parent);
    }

    QScriptValue VideoFilterCollectionPrototype::getLength()
    {
        return ADM_VideoFilters.size();
    }

    QScriptValue VideoFilterCollectionPrototype::add(QScriptValue filter)
    {
        VideoFilter *filterObject = qobject_cast<VideoFilter*>(filter.toQObject());

        if (filterObject == NULL)
        {
            return this->throwError("Invalid video filter object.");
        }

		if (filterObject != NULL && filterObject->isFilterUsed())
		{
			return this->throwError("Video filter is already attached to the filter chain.");
		}

        int index = ADM_VideoFilters.size();
        CONFcouple *couple;

        filterObject->getConfCouple(&couple);
        filterObject->setFilterAsUsed(ADM_vf_addFilterFromTag(this->_editor, filterObject->filterPlugin->tag, couple, false));

        delete couple;

        return index;
    }

    void VideoFilterCollectionPrototype::clear()
    {
        ADM_vf_clearFilters();
    }

    QScriptValue VideoFilterCollectionPrototype::insert(uint index, QScriptValue filter)
    {
        if (index > ADM_VideoFilters.size())
		{
			return this->throwError("Index is out of range");
		}

        VideoFilter *filterObject = qobject_cast<VideoFilter*>(filter.toQObject());

        if (filterObject == NULL)
        {
            return this->throwError("Invalid video filter object.");
        }

		if (filterObject != NULL && filterObject->isFilterUsed())
		{
			return this->throwError("Video filter is already attached to the filter chain.");
		}

		CONFcouple *couple;

        filterObject->getConfCouple(&couple);
        filterObject->setFilterAsUsed(ADM_vf_insertFilterFromTag(this->_editor, filterObject->filterPlugin->tag, couple, false));

        delete couple;

        return QScriptValue();
    }

    void VideoFilterCollectionPrototype::removeAt(uint index)
    {
        if (index >= ADM_VideoFilters.size())
        {
            this->throwError("Index is out of range");
            return;
        }

        ADM_vf_removeFilterAtIndex(index);
    }
}
