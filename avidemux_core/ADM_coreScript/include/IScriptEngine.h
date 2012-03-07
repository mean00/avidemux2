#ifndef IScriptEngine_h
#define IScriptEngine_h

#include <string>
#include "ADM_editor/IEditor.h"

class IScriptEngine
{
public:
	typedef enum
	{
		EVENT_TYPE_INFORMATION,
		EVENT_TYPE_ERROR
	} EVENT_TYPE;

	struct EngineEvent
	{
		IScriptEngine *engine;
		EVENT_TYPE eventType;
		const char *fileName;
		int lineNo;
		const char *message;
	};

	typedef void (eventHandlerFunc)(EngineEvent *event);

	virtual ~IScriptEngine() {}
	virtual IEditor* getEditor() = 0;
	virtual std::string getName() = 0;
	virtual void initialise(IEditor *videoBody) = 0;
	virtual void registerEventHandler(eventHandlerFunc *func) = 0;
	virtual bool runScript(std::string script) = 0;
	virtual bool runScriptFile(std::string name) = 0;
	virtual void unregisterEventHandler(eventHandlerFunc *func) = 0;
};

#endif
