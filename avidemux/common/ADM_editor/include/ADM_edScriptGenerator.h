#ifndef ADM_edScriptGenerator_h
#define ADM_edScriptGenerator_h

#include <iostream>
#include "IEditor.h"
#include "IScriptWriter.h"

class ADM_ScriptGenerator
{
private:
	IEditor* _editor;
	IScriptWriter* _scriptWriter;

public:
	ADM_ScriptGenerator(IEditor *editor, IScriptWriter* scriptWriter);
	void generateScript(std::iostream& stream);
};

#endif
