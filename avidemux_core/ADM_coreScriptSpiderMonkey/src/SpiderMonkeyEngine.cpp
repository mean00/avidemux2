#include "jsapi.h"

#include "ADM_assert.h"
#include "SpiderMonkeyEngine.h"
#include "SpiderMonkeyScriptWriter.h"

#include "src_dialogFactory/ADM_JSDFMenu.h"
#include "src_dialogFactory/ADM_JSDFToggle.h"
#include "src_dialogFactory/ADM_JSDFInteger.h"
#include "src_dialogFactory/ADM_JSDialogFactory.h"

#include "ADM_scriptDFMenu.h"
#include "ADM_scriptDFToggle.h"
#include "ADM_scriptDFInteger.h"
#include "ADM_scriptDialogFactory.h"

using namespace std;

extern "C"
{
	JSFunctionSpec *jsGetIfFunctions(void);
	JSFunctionSpec *jsGetTestFunctions(void);
	JSFunctionSpec *jsGetAdmFunctions(void);
	JSFunctionSpec *jsGetEditFunctions(void);
	JSFunctionSpec *jsGetDialogFactoryFunctions(void);
	JSObject *jsEditorInit(JSContext *cx, JSObject *obj);
	JSObject *jsAvidemuxInit(JSContext *cx, JSObject *obj);
}

JSClass SpiderMonkeyEngine::_globalClass = {
	"global", JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

SpiderMonkeyEngine::~SpiderMonkeyEngine()
{
	this->callEventHandlers(IScriptEngine::Information, NULL, -1, "Closing Spidermonkey");

	JS_DestroyContext(_jsContext);
	JS_DestroyRuntime(_jsRuntime);
}

string SpiderMonkeyEngine::name()
{
    return string("SpiderMonkey");
}

void SpiderMonkeyEngine::initialise(IEditor *editor)
{
	ADM_assert(editor);

	_editor = editor;

	_jsRuntime = JS_NewRuntime(1000000L);
	ADM_assert(_jsRuntime);

	_jsContext = JS_NewContext(_jsRuntime, 8192);
	ADM_assert(_jsContext);

	JS_SetContextPrivate(_jsContext, this);

	_jsObject = JS_NewObject(_jsContext, &_globalClass, 0, 0);
	ADM_assert(_jsObject);

	ADM_assert(JS_InitStandardClasses(_jsContext, _jsObject) == JS_TRUE);

	JS_SetErrorReporter(_jsContext, SpiderMonkeyEngine::printError);

	this->registerFunctions(_jsContext, _jsObject);
	this->registerDialogFunctions(_jsContext, _jsObject);
	this->callEventHandlers(IScriptEngine::Information, NULL, -1, "Spidermonkey initialised");
}

void SpiderMonkeyEngine::registerFunctions(JSContext *cx, JSObject *obj)
{
	this->registerFunctionGroup("Debug", "", jsGetIfFunctions(), cx, obj);
	this->registerFunctionGroup("Test", "", jsGetTestFunctions(), cx, obj);

	// Register also our class (for  help() )
	JsHook h;
	h.name = "adm";
	h.text = "Please prefix this with adm.";
	h.jsFunctions = jsGetAdmFunctions();

	jsHooks.push_back(h);
	jsAvidemuxInit(cx, obj);

	// Register also edit
	h.name = "editor";
	h.text = "Please prefix this with editor.";
	h.jsFunctions = jsGetEditFunctions();
	jsEditorInit(cx, obj);
	jsHooks.push_back(h);
}

void SpiderMonkeyEngine::registerDialogFunctions(JSContext *cx, JSObject *obj)
{
    ADM_assert(ADM_JSDialogFactory::JSInit(cx, obj) != NULL);
    ADM_assert(ADM_JSDFMenu::JSInit(cx, obj) != NULL);
    ADM_assert(ADM_JSDFToggle::JSInit(cx, obj) != NULL);
	ADM_assert(ADM_JSDFInteger::JSInit(cx, obj) != NULL);

	this->callEventHandlers(IScriptEngine::Information, NULL, -1, "Registered DialogFactory classes");
}

void SpiderMonkeyEngine::registerFunctionGroup(const char *name, const char *text, JSFunctionSpec *s, JSContext *cx, JSObject *obj)
{
	assert(JS_DefineFunctions(cx, obj, s) == JS_TRUE);

	this->callEventHandlers(IScriptEngine::Information, NULL, -1,
		(string("Registered ") + string(name) + string(" functions")).c_str());

	JsHook h;

	h.name = name;
	h.text = text;
	h.jsFunctions = s;

	jsHooks.push_back(h);
}

void SpiderMonkeyEngine::printError(JSContext *cx, const char *message, JSErrorReport *report)
{
	SpiderMonkeyEngine *engine = (SpiderMonkeyEngine*)JS_GetContextPrivate(cx);

	engine->callEventHandlers(IScriptEngine::Error, report->filename, report->lineno, message);
}

void SpiderMonkeyEngine::registerEventHandler(eventHandlerFunc *func)
{
	_eventHandlerSet.insert(func);
}

void SpiderMonkeyEngine::unregisterEventHandler(eventHandlerFunc *func)
{
	_eventHandlerSet.erase(func);
}

void SpiderMonkeyEngine::callEventHandlers(EventType eventType, const char *fileName, int lineNo, const char *message)
{
	EngineEvent event = { this, eventType, fileName, lineNo, message };
	set<eventHandlerFunc*>::iterator it;

	for (it = _eventHandlerSet.begin(); it != _eventHandlerSet.end(); ++it)
	{
		(*it)(&event);
	}
}

bool SpiderMonkeyEngine::runScript(string script, RunMode mode)
{
	jsval rval;

	return JS_EvaluateScript(_jsContext, _jsObject, script.c_str(), script.length(), "dummy", 1, &rval) == JSVAL_TRUE;
}

bool SpiderMonkeyEngine::runScriptFile(string name, RunMode mode)
{
	jsval rval;
	uintN lineno = 0;
	bool success = false;

	this->callEventHandlers(IScriptEngine::Information, NULL, -1,
		(string("Compiling \"" + string(name) + string("\"...")).c_str()));
	JSScript *pJSScript = JS_CompileFile(_jsContext, _jsObject, name.c_str());
	this->callEventHandlers(IScriptEngine::Information, NULL, -1, "Done.");

	if (pJSScript != NULL)
	{
		this->callEventHandlers(IScriptEngine::Information, NULL, -1,
			(string("Executing ") + string(name) + string("...")).c_str());

		JSBool ok = JS_ExecuteScript(_jsContext, _jsObject, pJSScript, &rval);
		JS_DestroyScript(_jsContext, pJSScript);

		this->callEventHandlers(IScriptEngine::Information, NULL, -1, "Done");
	}

	JS_GC(_jsContext);

	return success;
}

IEditor* SpiderMonkeyEngine::editor()
{
	return _editor;
}

IScriptEngine::Capabilities SpiderMonkeyEngine::capabilities()
{
	return IScriptEngine::None;
}

IScriptWriter* SpiderMonkeyEngine::createScriptWriter()
{
    return new SpiderMonkeyScriptWriter();
}

void SpiderMonkeyEngine::openDebuggerShell() {}

string SpiderMonkeyEngine::defaultFileExtension()
{
	return string("js");
}