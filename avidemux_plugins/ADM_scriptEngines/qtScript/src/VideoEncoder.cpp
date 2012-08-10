#include "VideoEncoder.h"
#include "FFcodecContext_param.h"

namespace ADM_qtScript
{
	VideoEncoder::VideoEncoder(
		QScriptEngine *engine, IEditor *editor, ADM_videoEncoder6* encoder) : QtScriptConfigObject(editor)
	{
		std::map<const QString, QScriptEngine::FunctionSignature> configSubGroups;

		configSubGroups.insert(
			std::pair<const QString, QScriptEngine::FunctionSignature>(
				"lavcSettings", QtScriptConfigObject::defaultConfigGetterSetter));

		this->encoderPlugin = encoder;
		this->_configObject = this->createConfigContainer(
								  engine, QtScriptConfigObject::defaultConfigGetterSetter, &configSubGroups);
	}

	QScriptValue VideoEncoder::getName(void)
	{
		return this->encoderPlugin->desc->menuName;
	}

	QScriptValue VideoEncoder::getConfiguration(void)
	{
		return this->_configObject;
	}

	void VideoEncoder::resetConfiguration(void)
	{
		this->encoderPlugin->desc->resetConfigurationData();
	}

	void VideoEncoder::getConfCouple(CONFcouple** conf, const QString& containerName)
	{
		if (this->encoderPlugin->desc->getConfigurationData && containerName == "")
		{
			this->encoderPlugin->desc->getConfigurationData(conf);
		}
		else if (containerName == "lavcSettings")
		{
			char *configData;

			this->encoderPlugin->desc->getConfigurationData(conf);
			(*conf)->readAsString("lavcSettings", &configData);
			delete *conf;

			getCoupleFromString(conf, configData, FFcodecContext_param);
			delete configData;
		}
		else
		{
			*conf = NULL;
		}
	}

	void VideoEncoder::setConfCouple(CONFcouple* conf, const QString& containerName)
	{
		if (this->encoderPlugin->desc->setConfigurationData && containerName == "")
		{
			this->encoderPlugin->desc->setConfigurationData(conf, true);
		}
		else if (containerName == "lavcSettings")
		{
			char *confString;
			CONFcouple *mainConf;

			lavCoupleToString(conf, &confString);
			this->encoderPlugin->desc->getConfigurationData(&mainConf);

			mainConf->updateValue(mainConf->lookupName("lavcSettings"), confString);
			this->encoderPlugin->desc->setConfigurationData(mainConf, true);

			delete [] confString;
			delete mainConf;
		}
	}
}
