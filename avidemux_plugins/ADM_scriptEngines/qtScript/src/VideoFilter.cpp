#include "VideoFilter.h"
#include "VideoOutput.h"
#include "ADM_coreVideoFilterFunc.h"
#include "ADM_videoFilterBridge.h"
#include "MyQScriptEngine.h"

namespace ADM_qtScript
{
    VideoFilterShim::VideoFilterShim() : ADM_coreVideoFilter(NULL, NULL)
	{
		this->_dummyInfo.width = 1;
		this->_dummyInfo.height = 1;
	}

    bool VideoFilterShim::getNextFrame(uint32_t *frameNumber, ADMImage *image)
    {
        return false;
    }

    FilterInfo* VideoFilterShim::getInfo(void)
    {
        return &this->_dummyInfo;
    }

    bool VideoFilterShim::getCoupledConf(CONFcouple **couples)
    {
        *couples = NULL;

        return true;
    }

    void VideoFilterShim::setCoupledConf(CONFcouple *couples) {} 

    VideoFilter::VideoFilter(
        QScriptEngine *engine, IEditor *editor, ADM_vf_plugin *plugin) : QtScriptConfigObject(editor)
    {
		this->_videoFilterShim = new VideoFilterShim();
        this->filterPlugin = plugin;
        this->_filter = filterPlugin->create(_videoFilterShim, NULL);
        this->_filter->getCoupledConf(&this->_defaultConf);
        this->_attachedToFilterChain = false;

        this->_configObject = this->createConfigContainer(
                                  engine, QtScriptConfigObject::defaultConfigGetterSetter);
    }

    VideoFilter::VideoFilter(
        QScriptEngine *engine, IEditor *editor, ADM_VideoFilterElement *element) : QtScriptConfigObject(editor)
    {
		this->_videoFilterShim = NULL;
        this->filterPlugin = ADM_vf_getPluginFromTag(element->tag);

		VideoFilterShim* videoFilterShim = new VideoFilterShim();
        ADM_coreVideoFilter *filter = filterPlugin->create(videoFilterShim, NULL);

        filter->getCoupledConf(&this->_defaultConf);
        delete filter;
		delete videoFilterShim;

        this->_filter = element->instance;
        this->_attachedToFilterChain = true;
        this->_configObject = this->createConfigContainer(
                                  engine, QtScriptConfigObject::defaultConfigGetterSetter);
    }

    VideoFilter::~VideoFilter()
    {
        delete this->_defaultConf;

        if (!this->_attachedToFilterChain)
        {
            delete this->_filter;
			delete this->_videoFilterShim;
        }
    }

    QScriptValue VideoFilter::constructor(QScriptContext *context, QScriptEngine *engine)
    {
        if (context->isCalledAsConstructor())
        {
            VideoFilter *videoFilterProto = qobject_cast<VideoFilter*>(
                                                context->thisObject().prototype().toQObject());
            VideoFilter *videoFilter = new VideoFilter(
                engine, static_cast<MyQScriptEngine*>(engine)->wrapperEngine->editor(),
                videoFilterProto->filterPlugin);

            return engine->newQObject(videoFilter, QScriptEngine::ScriptOwnership);
        }

        return engine->undefinedValue();
    }

    QScriptValue VideoFilter::getConfiguration()
    {
        return this->_configObject;
    }

    void VideoFilter::resetConfiguration(void)
    {
        if (this->verifyFilter())
        {
            this->_filter->setCoupledConf(this->_defaultConf);
        }
    }

    QString VideoFilter::getName()
    {
        return this->filterPlugin->getDisplayName();
    }

    QScriptValue VideoFilter::getVideoOutput()
    {
        if (this->isFilterUsed())
        {
            return this->engine()->newQObject(new VideoOutput(this->_editor, this->_filter->getInfo()));
        }
        else
        {
            return this->engine()->undefinedValue();
        }
    }

    void VideoFilter::getConfCouple(CONFcouple** conf, const QString& containerName)
    {
        if (this->verifyFilter())
        {
            this->_filter->getCoupledConf(conf);
        }
        else
        {
            *conf = NULL;
        }
    }

    void VideoFilter::setConfCouple(CONFcouple* conf, const QString& containerName)
    {
        if (this->verifyFilter())
        {
            this->_filter->setCoupledConf(conf);
        }
    }

    bool VideoFilter::isFilterUsed()
    {
        return this->_attachedToFilterChain;
    }

    void VideoFilter::setFilterAsUsed(ADM_VideoFilterElement* element)
    {
		delete this->_videoFilterShim;
		delete this->_filter;

        this->_trackObjectId = element->objectId;
		this->_filter = element->instance;
        this->_attachedToFilterChain = true;
    }

    bool VideoFilter::verifyFilter()
    {
        if (!this->_attachedToFilterChain)
        {
            return true;
        }

        bool found = false;

        for (unsigned int filterIndex = 0; filterIndex < ADM_VideoFilters.size(); filterIndex++)
        {
            ADM_VideoFilterElement *element = &ADM_VideoFilters[filterIndex];

            if (this->_filter == element->instance && this->_trackObjectId == element->objectId)
            {
                found = true;
                break;
            }
        }

        return found;
    }
}
