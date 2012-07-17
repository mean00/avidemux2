#ifndef ADM_qtScript_DoubleSpinBoxControl
#define ADM_qtScript_DoubleSpinBoxControl

#include <QtScript/QScriptEngine>

#include "Control.h"

namespace ADM_qtScript
{
	/** \brief The DoubleSpinBoxControl %class provides a spin box widget that takes doubles.
	 */
	class DoubleSpinBoxControl : public Control
	{
		Q_OBJECT

	private:
		QString _title;
		double _minValue, _maxValue;
		float _value;
		int _decimals;

		int getDecimals();
		double getMaximumValue();
		double getMinimumValue();
		const QString& getTitle();
		double getValue();

		void setDecimals(int decimals);
		void setMaximumValue(double value);
		void setMinimumValue(double value);
		void setTitle(const QString &title);
		void setValue(double value);

	public:
		/** \cond */
		static QScriptValue constructor(QScriptContext *context, QScriptEngine *engine);

		diaElem* createControl();
		/** \endcond */

		/** \brief Constructs a spin box with the given title, minimum value, maximum value and optional initial value and precision.
		 */
		DoubleSpinBoxControl(const QString& /*% String %*/ title, double /*% Number %*/ minValue, double /*% Number %*/ maxValue, double /*% Number %*/ value = 0, int /*% Number %*/ decimals = 2);

		/** \brief Gets or sets the precision of the spin box, in decimals.
		 */
		Q_PROPERTY(int /*% Number %*/ decimals READ getDecimals WRITE setDecimals);

		/** \brief Gets or sets the minimum value of the spin box.
		 */
		Q_PROPERTY(double /*% Number %*/ minimumValue READ getMinimumValue WRITE setMinimumValue);

		/** \brief Gets or sets the maximum value of the spin box.
		 */
		Q_PROPERTY(double /*% Number %*/ maximumValue READ getMaximumValue WRITE setMaximumValue);

		/** \brief Gets or sets the title of the spin box.
		 */
		Q_PROPERTY(const QString& /*% String %*/ title READ getTitle WRITE setTitle);

		/** \brief Gets or sets the value of the spin box.
		 */
		Q_PROPERTY(double /*% Number %*/ value READ getValue WRITE setValue);
	};
}

#endif