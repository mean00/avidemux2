#ifndef QtScriptEngine_h
#define QtScriptEngine_h

#include <map>
#include <set>
#include <vector>

#include "IScriptEngine.h"
#include "ADM_muxerInternal.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "audioencoderInternal.h"

namespace ADM_qtScript
{
	class AudioEncoder;
	class Muxer;
	class VideoEncoder;
}

class QMetaObject;
class QScriptContext;
class QScriptEngine;
class QScriptValue;
class QString;

class QtScriptEngine : public IScriptEngine
{
private:
	IEditor *_editor;
	std::set<eventHandlerFunc*> _eventHandlerSet;

	void copyEnumsToScriptObject(QScriptEngine *engine, const QMetaObject *metaObject, QScriptValue *object);
	static QScriptValue executeFunction(QScriptContext *context, QScriptEngine *engine);
	static QScriptValue includeFunction(QScriptContext *context, QScriptEngine *engine);
	static QScriptValue printFunction(QScriptContext *context, QScriptEngine *engine);
	void registerAudioEncoderPlugins(QScriptEngine *engine);
	void registerMuxerPlugins(QScriptEngine *engine, std::map<ADM_dynMuxer*, ADM_qtScript::Muxer*>* muxers);
	void registerScriptClasses(
		QScriptEngine *engine, std::map<ADM_dynMuxer*, ADM_qtScript::Muxer*>* muxers,
		std::map<ADM_videoEncoder6*, ADM_qtScript::VideoEncoder*>* videoEncoders);
	void registerScriptEnums(
		QScriptEngine *engine, const QString& parentPropertyName, const QMetaObject* metaObject);
	void registerVideoEncoderPlugins(
		QScriptEngine *engine, std::map<ADM_videoEncoder6*, ADM_qtScript::VideoEncoder*>* encoders);
	void registerVideoFilterPlugins(QScriptEngine *engine);
	bool runScript(const QString& script, const QString& name, RunMode mode);

public:
	~QtScriptEngine();
	void callEventHandlers(EventType eventType, const char *fileName, int lineNo, const char *message);
	Capabilities capabilities();
	IEditor* editor();
	void initialise(IEditor *videoBody);
	std::string name();
	void registerEventHandler(eventHandlerFunc *func);
	bool runScript(std::string script, RunMode mode);
	bool runScriptFile(std::string name, RunMode mode);
	void unregisterEventHandler(eventHandlerFunc *func);
};
#endif
