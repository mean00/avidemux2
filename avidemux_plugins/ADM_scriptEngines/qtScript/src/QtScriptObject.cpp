#include <QtScript/QScriptContext>

#include "QtScriptObject.h"

namespace ADM_qtScript
{
	QtScriptObject::QtScriptObject(IEditor *editor)
	{
		ADM_assert(editor);

		this->_editor = editor;
	}

	QScriptValue QtScriptObject::throwError(const QString& error)
	{
		return this->context()->throwError(error);
	}

	QScriptValue QtScriptObject::validateNumber(const QString& parameterName, QScriptValue value)
	{
		if (!value.isNumber())
		{
			return this->throwError(QString(QT_TR_NOOP("Parameter %1 must be a number")).arg(parameterName));
		}

		return QScriptValue(QScriptValue::UndefinedValue);
	}

	QScriptValue QtScriptObject::validateNumber(const QString& parameterName, QScriptValue value, double minValue, double maxValue)
	{
		QScriptValue result = this->validateNumber(parameterName, value);

		if (result.isUndefined())
		{
			qsreal number = value.toNumber();

			if (number < minValue)
			{
				result = this->throwError(QString(QT_TR_NOOP("Parameter %1 must be greater than %2")).arg(parameterName).arg(minValue));
			}
			else if (number > maxValue)
			{
				result = this->throwError(QString(QT_TR_NOOP("Parameter %1 must be less than %2")).arg(parameterName).arg(maxValue));
			}
			else
			{
				result = QScriptValue(QScriptValue::UndefinedValue);
			}
		}

		return result;
	}
}
