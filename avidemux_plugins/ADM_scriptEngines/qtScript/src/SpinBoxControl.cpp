#include "SpinBoxControl.h"

namespace ADM_qtScript
{
	SpinBoxControl::SpinBoxControl(const QString& title, int minValue, int maxValue, int value)
	{
		this->_title = title;
		this->_minValue = minValue;
		this->_maxValue = maxValue;
		this->_value = value;
	}

	QScriptValue SpinBoxControl::constructor(QScriptContext *context, QScriptEngine *engine)
	{
		if (context->isCalledAsConstructor())
		{
			if (context->argumentCount() == 3 && context->argument(0).isString() && context->argument(1).isNumber() && context->argument(2).isNumber())
			{
				return engine->newQObject(new SpinBoxControl(
					context->argument(0).toString(), context->argument(1).toNumber(), context->argument(2).toNumber()), QScriptEngine::ScriptOwnership);
			}
			else if (context->argumentCount() == 4 && context->argument(0).isString() && context->argument(1).isNumber() && context->argument(2).isNumber() &&
				context->argument(3).isNumber())
			{
				return engine->newQObject(new SpinBoxControl(
					context->argument(0).toString(), context->argument(1).toNumber(), context->argument(2).toNumber(), context->argument(3).toNumber()),
					QScriptEngine::ScriptOwnership);
			}
			else
			{
				return context->throwError("Invalid arguments passed to constructor");
			}
		}

		return engine->undefinedValue();
	}

	diaElem* SpinBoxControl::createControl(void)
	{
		return new diaElemInteger(&this->_value, this->_title.toUtf8().constData(), this->_minValue, this->_maxValue);
	}

	int SpinBoxControl::getMinimumValue()
	{
		return this->_minValue;
	}

	int SpinBoxControl::getMaximumValue()
	{
		return this->_maxValue;
	}

	const QString& SpinBoxControl::getTitle()
	{
		return this->_title;
	}

	int SpinBoxControl::getValue()
	{
		return this->_value;
	}
	
	void SpinBoxControl::setMaximumValue(int value)
	{
		this->_maxValue = value;
	}

	void SpinBoxControl::setMinimumValue(int value)
	{
		this->_minValue = value;
	}

	void SpinBoxControl::setTitle(const QString& title)
	{
		this->_title = title;
	}

	void SpinBoxControl::setValue(int value)
	{
		this->_value = value;
	}
}