#include "CheckBoxControl.h"

namespace ADM_qtScript
{
	CheckBoxControl::CheckBoxControl(const QString& title, bool value)
	{
		this->_title = title;
		this->_value = value;
	}

	QScriptValue CheckBoxControl::constructor(QScriptContext *context, QScriptEngine *engine)
	{
		if (context->isCalledAsConstructor())
		{
			if (context->argumentCount() == 1 && context->argument(0).isString())
			{
				return engine->newQObject(new CheckBoxControl(context->argument(0).toString()), QScriptEngine::ScriptOwnership);
			}
			else if (context->argumentCount() == 2 && context->argument(0).isString() && context->argument(1).isBool())
			{
				return engine->newQObject(
					new CheckBoxControl(context->argument(0).toString(), context->argument(1).toBool()), QScriptEngine::ScriptOwnership);
			}
			else
			{
				return context->throwError("Invalid arguments passed to constructor");
			}
		}

		return engine->undefinedValue();
	}

	diaElem* CheckBoxControl::createControl(void)
	{
		return new diaElemToggle(&this->_value, this->_title.toUtf8().constData(), NULL);
	}

	const QString& CheckBoxControl::getTitle()
	{
		return this->_title;
	}

	bool CheckBoxControl::getValue()
	{
		return this->_value;
	}
	
	void CheckBoxControl::setTitle(const QString& title)
	{
		this->_title = title;
	}

	void CheckBoxControl::setValue(bool value)
	{
		this->_value = value;
	}
}