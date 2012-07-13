#ifndef ADM_qtScript_QtScriptObject
#define ADM_qtScript_QtScriptObject

#include <QtCore/QObject>
#include <QtScript/QScriptable>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

#include "ADM_editor/include/IEditor.h"

namespace ADM_qtScript
{
	class QtScriptObject : public QObject, protected QScriptable
	{
		Q_OBJECT

	protected:
		IEditor *_editor;

		QtScriptObject(IEditor *editor);
		QScriptValue throwError(const QString& error);
		QScriptValue validateNumber(const QString& parameterName, QScriptValue value);
		QScriptValue validateNumber(const QString& parameterName, QScriptValue value, double minValue, double maxValue);
	};
}
#endif
