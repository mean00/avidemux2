#include "SliderControl.h"

namespace ADM_qtScript
{
	SliderControl::SliderControl(const QString& title, int minValue, int maxValue, int value, int increment)
	{
		this->_title = title;
		this->_minValue = minValue;
		this->_maxValue = maxValue;
		this->_value = value;
		this->_increment = increment;
	}

	QScriptValue SliderControl::constructor(QScriptContext *context, QScriptEngine *engine)
	{
		if (context->isCalledAsConstructor())
		{
			if (context->argumentCount() == 3 && context->argument(0).isString() && context->argument(1).isNumber() && context->argument(2).isNumber())
			{
				return engine->newQObject(new SliderControl(
					context->argument(0).toString(), context->argument(1).toNumber(), context->argument(2).toNumber()), QScriptEngine::ScriptOwnership);
			}
			else if (context->argumentCount() == 4 && context->argument(0).isString() && context->argument(1).isNumber() && context->argument(2).isNumber() &&
				context->argument(3).isNumber())
			{
				return engine->newQObject(new SliderControl(
					context->argument(0).toString(), context->argument(1).toNumber(), context->argument(2).toNumber(), context->argument(3).toNumber()),
					QScriptEngine::ScriptOwnership);
			}
			else if (context->argumentCount() == 5 && context->argument(0).isString() && context->argument(1).isNumber() && context->argument(2).isNumber() &&
				context->argument(3).isNumber() && context->argument(3).isNumber())
			{
				return engine->newQObject(new SliderControl(
					context->argument(0).toString(), context->argument(1).toNumber(), context->argument(2).toNumber(), context->argument(3).toNumber(),
					context->argument(4).toNumber()), QScriptEngine::ScriptOwnership);
			}
			else
			{
				return context->throwError("Invalid arguments passed to constructor");
			}
		}

		return engine->undefinedValue();
	}

	diaElem* SliderControl::createControl(void)
	{
		return new diaElemSlider(&this->_value, this->_title.toUtf8().constData(), this->_minValue, this->_maxValue, this->_increment);
	}

	int SliderControl::getMinimumValue()
	{
		return this->_minValue;
	}

	int SliderControl::getMaximumValue()
	{
		return this->_maxValue;
	}

	const QString& SliderControl::getTitle()
	{
		return this->_title;
	}

	int SliderControl::getValue()
	{
		return this->_value;
	}
	
	void SliderControl::setMaximumValue(int value)
	{
		this->_maxValue = value;
	}

	void SliderControl::setMinimumValue(int value)
	{
		this->_minValue = value;
	}

	void SliderControl::setTitle(const QString& title)
	{
		this->_title = title;
	}

	void SliderControl::setValue(int value)
	{
		this->_value = value;
	}
}