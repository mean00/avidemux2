#ifndef ADM_qtScript_VideoFilterCollection
#define ADM_qtScript_VideoFilterCollection

#include <QtScript/QScriptClass>
#include <QtScript/QScriptEngine>

#include "ADM_editor/include/IEditor.h"

namespace ADM_qtScript
{
	class VideoFilterCollection : public QObject, public QScriptClass
	{
    private:
        IEditor* _editor;
        QScriptValue _prototype;

	public:
		VideoFilterCollection(QScriptEngine *engine, IEditor *editor);

		QScriptClassPropertyIterator *newIterator(const QScriptValue &object);
		QScriptValue property(
			const QScriptValue &object,	const QScriptString &name, uint id);
		QScriptValue::PropertyFlags propertyFlags(
			const QScriptValue &object, const QScriptString &name, uint id);
		QScriptValue prototype() const;
		QueryFlags queryProperty(
			const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id);

		QString name() const;
	};
}

#endif
