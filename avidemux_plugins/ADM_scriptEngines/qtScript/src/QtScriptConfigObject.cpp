#include "QtScriptConfigObject.h"

using std::map;

namespace ADM_qtScript
{
	QtScriptConfigObject::QtScriptConfigObject(IEditor *editor) : QtScriptObject(editor) {}

	QScriptValue QtScriptConfigObject::createConfigContainer(
		QScriptEngine* engine, QScriptEngine::FunctionSignature getSetFunction,
		map<const QString, QScriptEngine::FunctionSignature>* configSubGroups)
	{
		return this->createConfigContainer(engine, QString(), getSetFunction, configSubGroups);
	}

	QScriptValue QtScriptConfigObject::createConfigContainer(
		QScriptEngine* engine, const QString& parentContainerName, QScriptEngine::FunctionSignature getSetFunction,
		map<const QString, QScriptEngine::FunctionSignature>* configSubGroups)
	{
		CONFcouple* conf;

		this->getConfCouple(&conf, parentContainerName);

		if (conf == NULL)
		{
			return engine->undefinedValue();
		}
		else
		{
			QScriptValue configContainer = engine->newObject();

			for (uint32_t coupleIndex = 0; coupleIndex < conf->getSize(); coupleIndex++)
			{
				char *name, *value;

				conf->getInternalName(coupleIndex, &name, &value);

				map<const QString, QScriptEngine::FunctionSignature>::iterator it;

				if (configSubGroups != NULL)
				{
					it = configSubGroups->find(name);
				}

				if (configSubGroups == NULL || it == configSubGroups->end())
				{
					QScriptValue func = engine->newFunction(getSetFunction);
					func.setProperty("parentContainerName", parentContainerName);
					func.setProperty("functionName", name);
					func.setData(engine->newQObject(this, QScriptEngine::ScriptOwnership));

					configContainer.setProperty(
						name, func, QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
				}
				else
				{
					configContainer.setProperty(
						name, this->createConfigContainer(engine, name, (*it).second, configSubGroups));
				}
			}

				delete conf;

			return configContainer;
		}
	}

	QScriptValue QtScriptConfigObject::defaultConfigGetterSetter(QScriptContext * context, QScriptEngine * engine)
	{
		QtScriptConfigObject *configObject = qobject_cast<QtScriptConfigObject*>(context->callee().data().toQObject());
		QString parentContainerName = context->callee().property("parentContainerName").toString();
		QString propertyName = context->callee().property("functionName").toString();
		CONFcouple *containerConf;
		QString value;

		configObject->getConfCouple(&containerConf, parentContainerName);

		int confIndex = containerConf->lookupName(propertyName.toUtf8().constData());

		if (context->argumentCount() == 1)
		{
			value = context->argument(0).toString();

			containerConf->updateValue(confIndex, value.toUtf8().constData());
			configObject->setConfCouple(containerConf, parentContainerName);
		}
		else
		{
			char *coupleName, *coupleValue;

			containerConf->getInternalName(confIndex, &coupleName, &coupleValue);

			value = coupleValue;
		}

		delete containerConf;

		return value;
	}
}
