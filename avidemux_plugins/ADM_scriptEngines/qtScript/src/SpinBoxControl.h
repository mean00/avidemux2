#ifndef ADM_qtScript_SpinBoxControl
#define ADM_qtScript_SpinBoxControl

#include <QtScript/QScriptEngine>

#include "Control.h"

namespace ADM_qtScript
{
	/** \brief The SpinBoxControl %class provides a spin box widget that takes integers.
	 */
	class SpinBoxControl : public Control
	{
		Q_OBJECT

	private:
		QString _title;
		int _minValue, _maxValue, _value;

		const QString& getTitle();
		int getMaximumValue();
		int getMinimumValue();
		int getValue();

		void setTitle(const QString &title);
		void setMaximumValue(int value);
		void setMinimumValue(int value);
		void setValue(int value);

	public:
		/** \cond */
		static QScriptValue constructor(QScriptContext *context, QScriptEngine *engine);

		diaElem* createControl();
		/** \endcond */

		/** \brief Constructs a spin box with the given title, minimum value, maximum value and optional initial value and precision.
		 */
		SpinBoxControl(const QString& /*% String %*/ title, int /*% Number %*/ minValue, int /*% Number %*/ maxValue, int /*% Number %*/ value = 0);

		/** \brief Gets or sets the minimum value of the spin box.
		 */
		Q_PROPERTY(int /*% Number %*/ minimumValue READ getMinimumValue WRITE setMinimumValue);

		/** \brief Gets or sets the maximum value of the spin box.
		 */
		Q_PROPERTY(int /*% Number %*/ maximumValue READ getMaximumValue WRITE setMaximumValue);

		/** \brief Gets or sets the title of the spin box.
		 */
		Q_PROPERTY(const QString& /*% String %*/ title READ getTitle WRITE setTitle);

		/** \brief Gets or sets the value of the spin box.
		 */
		Q_PROPERTY(int /*% Number %*/ value READ getValue WRITE setValue);
	};
}

#endif