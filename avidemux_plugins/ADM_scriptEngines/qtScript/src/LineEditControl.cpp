#include "LineEditControl.h"

namespace ADM_qtScript
{
	LineEditControl::LineEditControl(const QString& title, const QString& value)
	{
		this->_title = title;
		this->_value = NULL;
		this->setValue(value);
	}

	LineEditControl::~LineEditControl()
	{
		if (this->_value)
		{
			delete this->_value;
		}
	}

	QScriptValue LineEditControl::constructor(QScriptContext *context, QScriptEngine *engine)
	{
		if (context->isCalledAsConstructor())
		{
			if (context->argumentCount() == 1 && context->argument(0).isString())
			{
				return engine->newQObject(new LineEditControl(context->argument(0).toString()), QScriptEngine::ScriptOwnership);
			}
			else if (context->argumentCount() == 2 && context->argument(0).isString() && context->argument(1).isString())
			{
				return engine->newQObject(
					new LineEditControl(context->argument(0).toString(), context->argument(1).toString()), QScriptEngine::ScriptOwnership);
			}
			else
			{
				return context->throwError("Invalid arguments passed to constructor");
			}
		}

		return engine->undefinedValue();
	}

	diaElem* LineEditControl::createControl(void)
	{
		return new diaElemText(&this->_value, this->_title.toUtf8().constData(), NULL);
	}

	const QString& LineEditControl::getTitle()
	{
		return this->_title;
	}

	QString LineEditControl::getValue()
	{
		return this->_value;
	}
	
	void LineEditControl::setTitle(const QString& title)
	{
		this->_title = title;
	}

	void LineEditControl::setValue(QString value)
	{
		if (this->_value)
		{
			delete this->_value;
		}

		this->_value = ADM_strdup(value.toUtf8().constData());
	}
}