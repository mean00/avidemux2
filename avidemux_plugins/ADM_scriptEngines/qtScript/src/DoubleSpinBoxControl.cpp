#include "DoubleSpinBoxControl.h"

namespace ADM_qtScript
{
	DoubleSpinBoxControl::DoubleSpinBoxControl(const QString& title, double minValue, double maxValue, double value, int decimals)
	{
		this->_title = title;
		this->_minValue = minValue;
		this->_maxValue = maxValue;
		this->_value = value;
		this->_decimals = decimals;
	}

	QScriptValue DoubleSpinBoxControl::constructor(QScriptContext *context, QScriptEngine *engine)
	{
		if (context->isCalledAsConstructor())
		{
			if (context->argumentCount() == 3 && context->argument(0).isString() && context->argument(1).isNumber() && context->argument(2).isNumber())
			{
				return engine->newQObject(new DoubleSpinBoxControl(
					context->argument(0).toString(), context->argument(1).toNumber(), context->argument(2).toNumber()), QScriptEngine::ScriptOwnership);
			}
			else if (context->argumentCount() == 4 && context->argument(0).isString() && context->argument(1).isNumber() && context->argument(2).isNumber() &&
				context->argument(3).isNumber())
			{
				return engine->newQObject(new DoubleSpinBoxControl(
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

	diaElem* DoubleSpinBoxControl::createControl(void)
	{
		return new diaElemFloat(&this->_value, this->_title.toUtf8().constData(), this->_minValue, this->_maxValue, NULL, this->_decimals);
	}

	int DoubleSpinBoxControl::getDecimals()
	{
		return this->_decimals;
	}

	double DoubleSpinBoxControl::getMinimumValue()
	{
		return this->_minValue;
	}

	double DoubleSpinBoxControl::getMaximumValue()
	{
		return this->_maxValue;
	}

	const QString& DoubleSpinBoxControl::getTitle()
	{
		return this->_title;
	}

	double DoubleSpinBoxControl::getValue()
	{
		return this->_value;
	}

	void DoubleSpinBoxControl::setDecimals(int decimals)
	{
		this->_decimals = decimals;
	}
	
	void DoubleSpinBoxControl::setMaximumValue(double value)
	{
		this->_maxValue = value;
	}

	void DoubleSpinBoxControl::setMinimumValue(double value)
	{
		this->_minValue = value;
	}

	void DoubleSpinBoxControl::setTitle(const QString& title)
	{
		this->_title = title;
	}

	void DoubleSpinBoxControl::setValue(double value)
	{
		this->_value = value;
	}
}