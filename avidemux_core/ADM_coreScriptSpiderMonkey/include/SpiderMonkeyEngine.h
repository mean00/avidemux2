#ifndef SpiderMonkeyEngine_h
#define SpiderMonkeyEngine_h

#include <set>
#include <vector>
#include "IScriptEngine.h"
#include "jsapi.h"

class SpiderMonkeyEngine : public IScriptEngine
{
private:
	typedef struct
	{
		const char *name;
		const char *text;
		JSFunctionSpec *jsFunctions;
	} JsHook;

	static JSClass _globalClass;

	JSContext *_jsContext;
	JSObject  *_jsObject;
	JSRuntime *_jsRuntime;
	IEditor *_editor;

	std::set<eventHandlerFunc*> _eventHandlerSet;

	static void printError(JSContext *cx, const char *message, JSErrorReport *report);
	void registerFunctions(JSContext *cx, JSObject *obj);
	void registerDialogFunctions(JSContext *cx, JSObject *obj);
	void registerFunctionGroup(const char *name, const char *text, JSFunctionSpec *s, JSContext *cx, JSObject *obj);

public:
	std::vector<JsHook> jsHooks;

	~SpiderMonkeyEngine();
	void callEventHandlers(EVENT_TYPE eventType, const char *fileName, int lineNo, const char *message);
	IEditor* getEditor();
	std::string getName();
	void initialise(IEditor *videoBody);
	void registerEventHandler(eventHandlerFunc *func);
	bool runScript(std::string script);
	bool runScriptFile(std::string name);
	void unregisterEventHandler(eventHandlerFunc *func);
};

#endif
