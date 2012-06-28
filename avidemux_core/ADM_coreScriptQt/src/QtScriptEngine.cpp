#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QMetaEnum>
#include <QtCore/QProcess>
#include <QtCore/QTextStream>

#ifdef QT_SCRIPTTOOLS
#include <QtGui/QAction>
#include <QtGui/QMainWindow>
#include <QtScriptTools/QScriptEngineDebugger>
#endif

#include "MyQScriptEngine.h"

#include "AudioEncoder.h"
#include "AudioOutput.h"
#include "Directory.h"
#include "Editor.h"
#include "File.h"
#include "FileInformation.h"
#include "FrameProperties.h"
#include "Muxer.h"
#include "VideoEncoder.h"
#include "VideoFilter.h"

#include "BVector.h"
#include "ADM_muxerInternal.h"
#include "ADM_coreVideoFilterInternal.h"

extern BVector <ADM_dynMuxer *> ListOfMuxers;
extern BVector <ADM_videoEncoder6 *> ListOfEncoders;
extern BVector <ADM_audioEncoder *> ListOfAudioEncoder;
extern BVector <ADM_vf_plugin *> ADM_videoFilterPluginsList[VF_MAX];

using namespace std;

namespace ADM_qtScript
{
    MyQScriptEngine::MyQScriptEngine(QtScriptEngine *wrapperEngine) : QScriptEngine()
    {
        this->wrapperEngine = wrapperEngine;
    }

    QtScriptEngine::~QtScriptEngine()
    {
        this->callEventHandlers(IScriptEngine::Information, NULL, -1, "Closing QtScript");
    }

    IScriptEngine::Capabilities QtScriptEngine::capabilities()
    {
#ifdef QT_SCRIPTTOOLS
        return IScriptEngine::Debugger;
#else
        return IScriptEngine::None;
#endif
    }

    IEditor* QtScriptEngine::editor()
    {
        return _editor;
    }

    void QtScriptEngine::initialise(IEditor *editor)
    {
        ADM_assert(editor);

        this->_editor = editor;
        this->callEventHandlers(IScriptEngine::Information, NULL, -1, "Initialised");
    }

    string QtScriptEngine::name()
    {
        return string("QtScript");
    }

    void QtScriptEngine::registerEventHandler(eventHandlerFunc *func)
    {
        _eventHandlerSet.insert(func);
    }

    void QtScriptEngine::unregisterEventHandler(eventHandlerFunc *func)
    {
        _eventHandlerSet.erase(func);
    }

    void QtScriptEngine::callEventHandlers(EventType eventType, const char *fileName, int lineNo, const char *message)
    {
        EngineEvent event = { this, eventType, fileName, lineNo, message };
        set<eventHandlerFunc*>::iterator it;

        for (it = _eventHandlerSet.begin(); it != _eventHandlerSet.end(); ++it)
        {
            (*it)(&event);
        }
    }

    bool QtScriptEngine::runScript(const QString& script, const QString& name, RunMode mode)
    {
        MyQScriptEngine engine(this);

        map<ADM_dynMuxer*, Muxer*> muxers;
        map<ADM_videoEncoder6*, VideoEncoder*> videoEncoders;

        this->registerAudioEncoderPlugins(&engine);
        this->registerMuxerPlugins(&engine, &muxers);
        this->registerVideoEncoderPlugins(&engine, &videoEncoders);
        this->registerVideoFilterPlugins(&engine);
        this->registerScriptClasses(&engine, &muxers, &videoEncoders);

#ifdef QT_SCRIPTTOOLS
        QScriptEngineDebugger debugger;

        if (mode == IScriptEngine::Debug || mode == IScriptEngine::DebugOnError)
        {
            debugger.attachTo(&engine);
            debugger.standardWindow()->setWindowTitle(QT_TR_NOOP("Avidemux Script Debugger"));
            debugger.standardWindow()->setWindowModality(Qt::ApplicationModal);

            if (mode == IScriptEngine::Debug)
            {
                debugger.action(QScriptEngineDebugger::InterruptAction)->trigger();
            }
        }

#endif

        QScriptValue result = engine.evaluate(script, name);
        bool success = false;

        if (engine.hasUncaughtException())
        {
            QString errorDetails = (QString("Unable to process script.\n\nLine number: %1\n").arg(
                                        engine.uncaughtExceptionLineNumber()) + result.toString());

            this->callEventHandlers(IScriptEngine::Error, NULL, -1, (QString("Script error ") + errorDetails).toUtf8().constData());
            success = false;
        }
        else
        {
            this->callEventHandlers(IScriptEngine::Information, NULL, -1, (QString("Result: ") + result.toString()).toUtf8().constData());
            success = true;
        }

        return success;
    }

    bool QtScriptEngine::runScript(string script, RunMode mode)
    {
        return this->runScript(QString(script.c_str()), "", mode);
    }

    bool QtScriptEngine::runScriptFile(string name, RunMode mode)
    {
        QFile scriptFile(name.c_str());

        if (!scriptFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            this->callEventHandlers(IScriptEngine::Error, NULL, -1, "Unable to open script file.");

            return false;
        }

        QTextStream stream(&scriptFile);
        QString contents = stream.readAll();
        scriptFile.close();

        return this->runScript(contents, QString(name.c_str()), mode);
    }

    void QtScriptEngine::copyEnumsToScriptObject(
        QScriptEngine *engine, const QMetaObject *metaObject, QScriptValue *object)
    {
        for (int enumIndex = 0; enumIndex < metaObject->enumeratorCount(); enumIndex++)
        {
            QMetaEnum metaEnum = metaObject->enumerator(enumIndex);
            QScriptValue enumClass = engine->newObject();

            for (int keyIndex = 0; keyIndex < metaEnum.keyCount(); keyIndex++)
            {
                enumClass.setProperty(metaEnum.key(keyIndex), metaEnum.value(keyIndex));
            }

            object->setProperty(metaEnum.name(), enumClass);
        }
    }

    void QtScriptEngine::registerScriptEnums(
        QScriptEngine *engine, const QString& parentPropertyName, const QMetaObject* metaObject)
    {
        QScriptValue scriptObject = engine->newObject();

        this->copyEnumsToScriptObject(engine, metaObject, &scriptObject);
        engine->globalObject().setProperty(parentPropertyName, scriptObject);
    }

    void QtScriptEngine::registerScriptClasses(
        QScriptEngine *engine, map<ADM_dynMuxer*, Muxer*>* muxers, map<ADM_videoEncoder6*, VideoEncoder*>* videoEncoders)
    {
        // Register various enums
        this->registerScriptEnums(engine, "AudioOutput", &AudioOutput::staticMetaObject);
        this->registerScriptEnums(engine, "FrameProperties", &FrameProperties::staticMetaObject);

        // Register Directory class
        QScriptValue dirObject = engine->newFunction(Directory::constructor);

        this->copyEnumsToScriptObject(engine, &Directory::staticMetaObject, &dirObject);
        engine->globalObject().setProperty("Directory", dirObject);

        // Register static Editor object
        Editor* editor = new Editor(engine, this->_editor, muxers, videoEncoders);
        QScriptValue mainObject = engine->newQObject(
                                      editor, QScriptEngine::ScriptOwnership, QScriptEngine::ExcludeSlots);

        this->copyEnumsToScriptObject(engine, &Editor::staticMetaObject, &mainObject);
        engine->globalObject().setProperty("Editor", mainObject);

        // Register File class
        QScriptValue fileObject = engine->newFunction(File::constructor);

        this->copyEnumsToScriptObject(engine, &File::staticMetaObject, &fileObject);
        engine->globalObject().setProperty("File", fileObject);

        // Register FileInfo class
        QScriptValue fileInformationObject = engine->newFunction(FileInformation::constructor);

        this->copyEnumsToScriptObject(engine, &FileInformation::staticMetaObject, &fileInformationObject);
        engine->globalObject().setProperty("FileInformation", fileInformationObject);

        // Register custom functions
        QScriptValue executeFunc = engine->newFunction(executeFunction);
        engine->globalObject().setProperty("execute", executeFunc);

        QScriptValue includeFunc = engine->newFunction(includeFunction);
        engine->globalObject().setProperty("include", includeFunc);

        QScriptValue printFunc = engine->newFunction(printFunction);
        engine->globalObject().setProperty("print", printFunc);
    }

    void QtScriptEngine::registerAudioEncoderPlugins(QScriptEngine *engine)
    {
        for (int encoderIndex = 0; encoderIndex < ListOfAudioEncoder.size(); encoderIndex++)
        {
            ADM_audioEncoder* encoderPlugin = ListOfAudioEncoder[encoderIndex];
            AudioEncoder *encoder = new AudioEncoder(engine, this->_editor, encoderPlugin, encoderIndex);
        QString encoderName = QString(encoderPlugin->codecName).toLower() + "AudioEncoder";

        encoderName = encoderName[0].toUpper() + encoderName.mid(1);

            engine->globalObject().setProperty(
            encoderName, engine->newFunction(
                    AudioEncoder::constructor, engine->newQObject(encoder, QScriptEngine::ScriptOwnership)));
        }
    }

    void QtScriptEngine::registerMuxerPlugins(QScriptEngine *engine, map<ADM_dynMuxer*, Muxer*>* muxers)
    {
        muxers->clear();

        for (int muxerIndex = 0; muxerIndex < ListOfMuxers.size(); muxerIndex++)
        {
            ADM_dynMuxer* muxerPlugin = ListOfMuxers[muxerIndex];
            Muxer *muxer = new Muxer(engine, this->_editor, muxerPlugin);
        QString muxerName = QString(muxerPlugin->name).toLower() + "Muxer";

        muxerName = muxerName[0].toUpper() + muxerName.mid(1);

        engine->globalObject().setProperty(muxerName, engine->newQObject(muxer, QScriptEngine::ScriptOwnership));
            muxers->insert(pair<ADM_dynMuxer*, Muxer*>(muxerPlugin, muxer));
        }
    }

    void QtScriptEngine::registerVideoEncoderPlugins(
        QScriptEngine *engine, map<ADM_videoEncoder6*, VideoEncoder*>* encoders)
    {
        encoders->clear();

        for (int encoderIndex = 0; encoderIndex < ListOfEncoders.size(); encoderIndex++)
        {
            ADM_videoEncoder6* encoderPlugin = ListOfEncoders[encoderIndex];
            VideoEncoder *encoder = new VideoEncoder(engine, this->_editor, encoderPlugin);
        QString encoderName = QString(encoderPlugin->desc->encoderName).toLower() + "VideoEncoder";

        encoderName = encoderName[0].toUpper() + encoderName.mid(1);

            engine->globalObject().setProperty(
            encoderName, engine->newQObject(encoder, QScriptEngine::ScriptOwnership));
            encoders->insert(pair<ADM_videoEncoder6*, VideoEncoder*>(encoderPlugin, encoder));
        }
    }

    void QtScriptEngine::registerVideoFilterPlugins(QScriptEngine *engine)
    {
        for (int filterGroupIndex = 0; filterGroupIndex < VF_MAX; filterGroupIndex++)
        {
            for (int filterIndex = 0; filterIndex < ADM_videoFilterPluginsList[filterGroupIndex].size();
                    filterIndex++)
            {
                ADM_vf_plugin* filterPlugin = ADM_videoFilterPluginsList[filterGroupIndex][filterIndex];
                VideoFilter *filter = new VideoFilter(engine, this->_editor, filterPlugin);
            QString filterName = QString(filterPlugin->getInternalName()).toLower() + "VideoFilter";

            filterName = filterName[0].toUpper() + filterName.mid(1);

                engine->globalObject().setProperty(
                filterName, engine->newFunction(
                        VideoFilter::constructor, engine->newQObject(filter, QScriptEngine::ScriptOwnership)));
            }
        }
    }

    QScriptValue QtScriptEngine::printFunction(QScriptContext *context, QScriptEngine *engine)
    {
        QString output;

        for (int i = 0; i < context->argumentCount(); ++i)
        {
            if (i > 0)
            {
                output += " ";
            }

            output += context->argument(i).toString();
        }

        static_cast<MyQScriptEngine*>(engine)->wrapperEngine->callEventHandlers(
            IScriptEngine::Information, NULL, -1, output.toUtf8().constData());

        return engine->undefinedValue();
    }

    QScriptValue QtScriptEngine::includeFunction(QScriptContext *context, QScriptEngine *engine)
    {
        while (context->argumentCount())
        {
            QString filename = context->argument(0).toString();
            QFile scriptFile(filename);

            if (!scriptFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                context->throwError(QString(QT_TR_NOOP("Unable to open script file %1")).arg(filename));
                break;
            }

            QTextStream stream(&scriptFile);
            QString contents = stream.readAll();
            scriptFile.close();

            context->setActivationObject(context->parentContext()->activationObject());
            engine->evaluate(contents, filename);

            break;
        }

        return engine->undefinedValue();
    }

    QScriptValue QtScriptEngine::executeFunction(QScriptContext *context, QScriptEngine *engine)
    {
        if (context->argumentCount() < 1)
        {
            return engine->undefinedValue();
        }

        QString command = context->argument(0).toString();
        QStringList arguments = QStringList();

        for (int i = 1; i < context->argumentCount(); i++)
        {
            arguments.push_back(context->argument(i).toString());
        }

        return QProcess::execute(command, arguments);
    }
}
