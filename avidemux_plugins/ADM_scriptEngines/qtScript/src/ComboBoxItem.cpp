#include "ComboBoxItem.h"

namespace ADM_qtScript
{
	ComboBoxItem::ComboBoxItem(const QString& title, const QString& value) : _title(title), _value(value)
	{
	}

	QScriptValue ComboBoxItem::constructor(QScriptContext *context, QScriptEngine *engine)
	{
		if (context->isCalledAsConstructor())
		{
			if (context->argumentCount() == 1 && context->argument(0).isString())
			{
				return engine->newQObject(new ComboBoxItem(context->argument(0).toString()), QScriptEngine::ScriptOwnership);
			}
			else if (context->argumentCount() == 2 && context->argument(0).isString() && context->argument(1).isString())
			{
				return engine->newQObject(
					new ComboBoxItem(context->argument(0).toString(), context->argument(1).toString()), QScriptEngine::ScriptOwnership);
			}
			else
			{
				return context->throwError("Invalid arguments passed to constructor");
			}
		}

		return engine->undefinedValue();
	}

	const QString& ComboBoxItem::getTitle()
	{
		return this->_title;
	}

	const QString& ComboBoxItem::getValue()
	{
		return this->_value;
	}
	
	void ComboBoxItem::setTitle(const QString& title)
	{
		this->_title = title;
	}

	void ComboBoxItem::setValue(const QString& value)
	{
		this->_value = value;
	}
}