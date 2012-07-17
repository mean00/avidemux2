#ifndef ADM_qtScript_ComboBoxControl
#define ADM_qtScript_ComboBoxControl

#include <vector>
#include <QtScript/QScriptEngine>

#include "Control.h"
#include "ComboBoxItem.h"
#include "DIA_factory.h"

namespace ADM_qtScript
{
	/** \brief The ComboBoxControl %class provides a combobox widget with a text label.
	 */
	class ComboBoxControl : public Control
	{
		Q_OBJECT

	private:
		std::vector<ComboBoxItem*> _items;
		diaMenuEntryDynamic** _menuEntries;
		uint32_t _selectedIndex;
		QString _title;

		QScriptValue getItems();
		QScriptValue getSelectedItem();
		uint getSelectedIndex();
		const QString& getTitle();

		void setSelectedIndex(uint index);
		void setTitle(const QString &title);

	public:
		/** \cond */
		static QScriptValue constructor(QScriptContext *context, QScriptEngine *engine);

		~ComboBoxControl();
		diaElem* createControl();
		/** \endcond */

		/** \brief Constructs a combobox widget with the given title.
		 */
		ComboBoxControl(const QString& /*% String %*/ title);

		/** \brief Provides access to the items displayed in the combobox widget.
		 */
		Q_PROPERTY(QScriptValue /*% ComboBoxItemCollection %*/ items READ getItems);

		/** \brief Gets the ComboBoxItem that is currently selected in the combobox widget.
		 */
		Q_PROPERTY(QScriptValue /*% ComboBoxItem %*/ selectedItem READ getSelectedItem);

		/** \brief Gets the zero-based index of the item that is currently selected in the combobox widget.
		 */
		Q_PROPERTY(uint /*% Number %*/ selectedItemIndex READ getSelectedIndex WRITE setSelectedIndex);

		/** \brief Gets or sets the title of the combobox widget.
		 */
		Q_PROPERTY(const QString& /*% String %*/ title READ getTitle WRITE setTitle);
	};
}

#endif