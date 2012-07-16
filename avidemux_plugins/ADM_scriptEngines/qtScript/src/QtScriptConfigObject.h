#ifndef ADM_qtScript_QtScriptConfigObject
#define ADM_qtScript_QtScriptConfigObject

#include <map>
#include "QtScriptObject.h"

namespace ADM_qtScript
{
    class QtScriptConfigObject : public QtScriptObject
    {
        Q_OBJECT

    private:
        QScriptValue createConfigContainer(
            QScriptEngine* engine, const QString& parentContainerName, QScriptEngine::FunctionSignature getSetFunction,
            std::map<const QString, QScriptEngine::FunctionSignature>* configSubGroups);

    protected:
        std::map<const QString, std::map<const QString, const QString> > propertyNameMappings;
        QScriptValue createConfigContainer(
            QScriptEngine* engine, QScriptEngine::FunctionSignature getSetFunction = defaultConfigGetterSetter,
            std::map<const QString, QScriptEngine::FunctionSignature>* configSubGroups = NULL);
        QString createPropertyNameMapping(const QString containerName, QString propertyName);
        static QScriptValue defaultConfigGetterSetter(QScriptContext * context, QScriptEngine * engine);
        virtual void getConfCouple(CONFcouple** conf, const QString& containerName) = 0;
        virtual void setConfCouple(CONFcouple* conf, const QString& containerName) = 0;

    public:
        QtScriptConfigObject(IEditor *_editor);
        const QString& getOriginalPropertyName(const QString containerName, const QString propertyName);
    };
}
#endif
