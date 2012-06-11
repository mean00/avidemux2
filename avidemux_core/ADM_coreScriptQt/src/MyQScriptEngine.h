#ifndef ADM_qtScript_MyQScriptEngine
#define ADM_qtScript_MyQScriptEngine

#include <QtScript/QScriptEngine>
#include "QtScriptEngine.h"

namespace ADM_qtScript
{
	class MyQScriptEngine : public QScriptEngine
	{
	public:
		QtScriptEngine *wrapperEngine;

		MyQScriptEngine(QtScriptEngine *wrapperEngine);
	};
}

#endif
