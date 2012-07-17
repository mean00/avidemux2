#include "ComboBoxControl.h"
#include "ComboBoxItemCollection.h"

namespace ADM_qtScript
{
	ComboBoxControl::ComboBoxControl(const QString& title)
	{
		this->_title = title;
		this->_menuEntries = NULL;
		this->_selectedIndex = 0;
	}

	ComboBoxControl::~ComboBoxControl()
	{
		if (this->_menuEntries)
		{
			for (unsigned int index = 0; index < this->_items.size(); index++)
			{
				delete this->_menuEntries[index];
			}

			delete [] this->_menuEntries;
		}
	}

	QScriptValue ComboBoxControl::constructor(QScriptContext *context, QScriptEngine *engine)
	{
		if (context->isCalledAsConstructor())
		{
			if (context->argumentCount() == 1 && context->argument(0).isString())
			{
				return engine->newQObject(new ComboBoxControl(context->argument(0).toString()), QScriptEngine::ScriptOwnership);
			}
			else
			{
				return context->throwError("Invalid arguments passed to constructor");
			}
		}

		return engine->undefinedValue();
	}

	diaElem* ComboBoxControl::createControl(void)
	{
		if (this->_menuEntries)
		{
			for (unsigned int index = 0; index < this->_items.size(); index++)
			{
				delete this->_menuEntries[index];
			}

			delete [] this->_menuEntries;
		}

		this->_menuEntries = new diaMenuEntryDynamic*[this->_items.size()];	

		for (unsigned int index = 0; index < this->_items.size(); index++)
		{
			ComboBoxItem *item = this->_items.at(index);
			diaMenuEntryDynamic *menuEntry = new diaMenuEntryDynamic(
				index, item->getTitle().toUtf8().constData(), item->getValue().toUtf8().constData());

			this->_menuEntries[index] = menuEntry;
		}

		return new diaElemMenuDynamic(
			&this->_selectedIndex, this->_title.toUtf8().constData(), this->_items.size(), this->_menuEntries, NULL);
	}

	QScriptValue ComboBoxControl::getItems()
	{
		return this->engine()->newObject(new ComboBoxItemCollection(this->engine(), this->_items), QScriptEngine::ScriptOwnership);
	}

	uint ComboBoxControl::getSelectedIndex()
	{
		return this->_selectedIndex;
	}

	QScriptValue ComboBoxControl::getSelectedItem()
	{
		if (this->_selectedIndex < this->_items.size())
		{
			return this->engine()->newQObject(this->_items[this->_selectedIndex], QScriptEngine::ScriptOwnership);
		}
		else
		{
			return this->context()->throwError("An item hasn't been selected");
		}
	}

	const QString& ComboBoxControl::getTitle()
	{
		return this->_title;
	}

	void ComboBoxControl::setSelectedIndex(uint index)
	{
		if (index < this->_items.size())
		{
			this->_selectedIndex = index;
		}
	}

	void ComboBoxControl::setTitle(const QString& title)
	{
		this->_title = title;
	}
}