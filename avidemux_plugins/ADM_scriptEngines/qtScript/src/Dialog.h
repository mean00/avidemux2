#ifndef ADM_qtScript_Dialog
#define ADM_qtScript_Dialog

#include <vector>

#include <QtScript/QScriptable>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

#include "Control.h"

namespace ADM_qtScript
{
	/** \brief The Dialog %class represents a dialog window with an OK and Cancel button.
	 */
	class Dialog : public QObject, protected QScriptable
	{
		Q_OBJECT

	private:
		QString _title;
		std::vector<Control*> _controls;

		QScriptValue getControls(void);

	public:
		/** \cond */
		static QScriptValue constructor(QScriptContext *context, QScriptEngine *engine);
		/** \endcond */

		/** \brief Constructs a dialog window with the given title.
		 */
		Dialog(const QString& /*% String %*/ title);

		/** \brief Provides access to the dialog window's controls.
		 */
		Q_PROPERTY(QScriptValue /*% ControlCollection %*/ controls READ getControls);

		/** \brief Shows the dialog window and its controls.
		 *
		 * \returns Returns true if OK was clicked; otherwise false.
		 */
		Q_INVOKABLE QScriptValue /*% Boolean %*/ show(void);
	};
}

#endif