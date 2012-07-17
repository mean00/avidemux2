#include "DIA_factory.h"

#include "Dialog.h"
#include "ControlCollection.h"

namespace ADM_qtScript
{
	Dialog::Dialog(const QString& title)
	{
		this->_title = title;
	}

	QScriptValue Dialog::constructor(QScriptContext *context, QScriptEngine *engine)
	{
		if (context->isCalledAsConstructor())
		{
			if (context->argumentCount() == 1 && context->argument(0).isString())
			{
				return engine->newQObject(new Dialog(context->argument(0).toString()), QScriptEngine::ScriptOwnership);
			}
			else
			{
				return context->throwError("Invalid arguments passed to constructor");
			}
		}

		return engine->undefinedValue();
	}

	QScriptValue Dialog::getControls(void)
    {
        return this->engine()->newObject(new ControlCollection(
        	this->engine(), this->_controls), QScriptEngine::ScriptOwnership);
    }

	QScriptValue Dialog::show(void)
	{
		if (this->_controls.size() == 0)
		{
			return this->context()->throwError("No controls have been added to the dialog");
		}

		diaElem **controls = new diaElem*[this->_controls.size()];
		std::vector<Control*>::iterator it;
		unsigned int index = 0;

		for (it = _controls.begin(); it != _controls.end(); it++)
		{
			controls[index++] = (*it)->createControl();
		}

		bool result = diaFactoryRun(this->_title.toUtf8().constData(), this->_controls.size(), controls) != 0;

		for (index = 0; index < this->_controls.size(); index++)
		{
			delete controls[index];
		}

		delete [] controls;

		return result;
	}
}