#ifndef ADM_edScriptGenerator_h
#define ADM_edScriptGenerator_h

#include <iostream>
#include "IEditor.h"
#include "IScriptWriter.h"
/**
 * \class ADM_ScriptGenerator
 */
class ADM_ScriptGenerator
{
public:    
    enum GeneratorType
    {
        GENERATE_ALL=0,
        GENERATE_SETTINGS=1
    };
private:
	IEditor* _editor;
	IScriptWriter* _scriptWriter;

public:
             ADM_ScriptGenerator(IEditor *editor, IScriptWriter* scriptWriter);
	void generateScript(std::iostream& stream,const GeneratorType &type=GENERATE_ALL);
        bool init(std::iostream& stream);
        bool end();
        bool saveVideoFilters();
};

#endif
