#ifndef IScriptEngine_h
#define IScriptEngine_h

#include <string>
#include "ADM_assert.h"
#include "IScriptWriter.h"
#include "ADM_editor/include/IEditor.h"

class IScriptEngine
{
public:
    enum Capabilities
    {
        None = 0,
        Debugger = 1 << 0,
        DebuggerShell = 1 << 1
    };

    enum EventType
    {
        Information,
        Warning,
        Error
    };

    enum RunMode
    {
        Normal = 0,
        Debug = 1 << 0,
        DebugOnError = 1 << 1
    };

    struct EngineEvent
    {
        IScriptEngine *engine;
        EventType eventType;
        const char *fileName;
        int lineNo;
        const char *message;
    };

    typedef void (eventHandlerFunc)(EngineEvent *event);

    virtual ~IScriptEngine() {}
    virtual Capabilities capabilities() = 0;
    virtual IScriptWriter* createScriptWriter() = 0;
    virtual std::string defaultFileExtension() = 0;
    virtual IEditor* editor() = 0;
    virtual void initialise(IEditor *videoBody) = 0;
    virtual int maturityRanking() = 0;
    virtual std::string name() = 0;
    virtual void openDebuggerShell() = 0;
    virtual std::string referenceUrl() = 0;
    virtual void registerEventHandler(eventHandlerFunc *func) = 0;
    virtual bool runScript(std::string script, RunMode mode) = 0;
    virtual bool runScriptFile(std::string name, RunMode mode) = 0;
    virtual void unregisterEventHandler(eventHandlerFunc *func) = 0;
    virtual void raise(const char *exception) {printf("Exception : %s\n",exception);}
};

#endif
