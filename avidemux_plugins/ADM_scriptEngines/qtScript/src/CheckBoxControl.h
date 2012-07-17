#ifndef ADM_qtScript_CheckBoxControl
#define ADM_qtScript_CheckBoxControl

#include <QtScript/QScriptEngine>

#include "Control.h"

namespace ADM_qtScript
{
	/** \brief The CheckBoxControl %class provides a checkbox widget with a text label.
	 */
	class CheckBoxControl : public Control
	{
		Q_OBJECT

	private:
		QString _title;
		bool _value;

		const QString& getTitle();
		bool getValue();

		void setTitle(const QString &title);
		void setValue(bool value);

	public:
		/** \cond */
		static QScriptValue constructor(QScriptContext *context, QScriptEngine *engine);

		diaElem* createControl();
		/** \endcond */

		/** \brief Constructs a checkbox widget with the given title and check state.
		 */
		CheckBoxControl(const QString& /*% String %*/ title, bool /*% Boolean %*/ checked = false);

		/** \brief Gets or sets the title of the checkbox widget.
		 */
		Q_PROPERTY(const QString& /*% String %*/ title READ getTitle WRITE setTitle);

		/** \brief Gets or sets whether the checkbox widget is checked or unchecked.
		 */
		Q_PROPERTY(bool /*% Number %*/ checked READ getValue WRITE setValue);
	};
}

#endif