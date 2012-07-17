#ifndef ADM_qtScript_ComboBoxItem
#define ADM_qtScript_ComboBoxItem

#include <QtScript/QScriptable>
#include <QtScript/QScriptEngine>

namespace ADM_qtScript
{
	/** \brief The ComboBoxItem %class represents an item to be displayed in a combobox widget.
	 */
	class ComboBoxItem : public QObject, protected QScriptable
	{
		Q_OBJECT

	private:
		QString _title;
		QString _value;

		void setTitle(const QString& title);
		void setValue(const QString& value);

	public:
		/** \cond */
		static QScriptValue constructor(QScriptContext *context, QScriptEngine *engine);

		const QString& getTitle();
		const QString& getValue();
		/** \endcond */

		/** \brief Constructs a combobox item with the given title and optional user data associated with the item.
		 */
		ComboBoxItem(const QString& /*% String %*/ title, const QString& /*% String %*/ value = "");

		/** \brief Gets or sets the title of the combobox item.
		 */
		Q_PROPERTY(const QString& /*% String %*/ title READ getTitle WRITE setTitle);

		/** \brief Gets or sets the user data associated with the combobox item.
		 */
		Q_PROPERTY(const QString& /*% String %*/ value READ getValue WRITE setValue);
	};
}

#endif