#include "Muxer.h"

namespace ADM_qtScript
{
	Muxer::Muxer(QScriptEngine *engine, IEditor *editor, ADM_dynMuxer *muxer) : QtScriptConfigObject(editor)
	{
		this->muxerPlugin = muxer;
		this->_configObject = this->createConfigContainer(engine);
	}

	QScriptValue Muxer::getName(void)
	{
		return this->muxerPlugin->displayName;
	}

	QScriptValue Muxer::getDefaultExtension(void)
	{
		return this->muxerPlugin->defaultExtension;
	}

	QScriptValue Muxer::getConfiguration(void)
	{
		return this->_configObject;
	}

	void Muxer::resetConfiguration(void)
	{
		this->muxerPlugin->resetConfiguration();
	}

	void Muxer::getConfCouple(CONFcouple** conf, const QString& containerName)
	{
		this->muxerPlugin->getConfiguration(conf);
	}

	void Muxer::setConfCouple(CONFcouple* conf, const QString& containerName)
	{
		this->muxerPlugin->setConfiguration(conf);
	}
}
