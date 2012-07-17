#ifndef ADM_qtScript_LineEditControl
#define ADM_qtScript_LineEditControl

#include <QtScript/QScriptEngine>

#include "Control.h"

namespace ADM_qtScript
{
	/** \brief The LineEditControl %class provides a one-line edit widget.
	 */
	class LineEditControl : public Control
	{
		Q_OBJECT

	private:
		QString _title;
		char *_value;

		const QString& getTitle();
		QString getValue();

		void setLastValue(const QString& value);
		void setTitle(const QString &title);
		void setValue(QString value);

	public:
		/** \cond */
		static QScriptValue constructor(QScriptContext *context, QScriptEngine *engine);

		~LineEditControl();
		diaElem* createControl();
		/** \endcond */

		/** \brief Constructs an edit widget with the given title and optional initial value.
		 */
		LineEditControl(const QString& /*% String %*/ title, const QString& /*% String %*/ value = "");

		/** \brief Gets or sets the title of the edit widget.
		 */
		Q_PROPERTY(const QString& /*% String %*/ title READ getTitle WRITE setTitle);

		/** \brief Gets or sets the value of the edit widget.
		 */
		Q_PROPERTY(QString /*% String %*/ value READ getValue WRITE setValue);
	};
}

#endif