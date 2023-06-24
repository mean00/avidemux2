#include <map>
static  std::map<std::string, tp_obj> persistentStorage;

tp_obj pyMemory_get(tp_vm *vm)
{
    tp_obj self = tp_getraw(vm);
    TinyParams pm(vm);
    const char *prefix = (const char *)pm.asThis(&self, ADM_PYID_MEMORY);
    char const *keyc = pm.asString();
    std::string key = std::string(prefix) + std::string(keyc);

    std::map<std::string,tp_obj>::iterator it;
    it=persistentStorage.find(key);
    if(persistentStorage.end() !=it)
    {
        return persistentStorage[key];
    }
    return tp_None;
}

tp_obj pyMemory_set(tp_vm *vm)
{
    tp_obj self = tp_getraw(vm);
    TinyParams pm(vm);
    const char *prefix = (const char *)pm.asThis(&self, ADM_PYID_MEMORY);
    char const *keyc = pm.asString();
    std::string key = std::string(prefix) + std::string(keyc);

    std::map<std::string,tp_obj>::iterator it;
    it=persistentStorage.find(key);
    if(persistentStorage.end() !=it)
    {
        if (persistentStorage[key].type > TP_NUMBER)
            tp_delete(vm, persistentStorage[key]);
        persistentStorage.erase(it);
    }
    tp_obj new_elem = {TP_NONE};
    tp_obj r = tp_get(vm,vm->params,tp_None);
    int type = r.type;
    if (type == TP_LIST) {
        new_elem = _tp_list_copy(vm,r);
    } else if (type == TP_DICT) {
        new_elem = _tp_dict_copy(vm,r);
    } else if (type == TP_NUMBER) {
        new_elem = r;
    }  else if (type == TP_STRING) {
        new_elem = tp_string_copy(vm, r.string.val, r.string.len);
    } else {
        pm.raise("pyMemory:Invalid type!");
    }
    persistentStorage.insert(std::pair<std::string, tp_obj>(key,new_elem));
    return tp_None;
}

// Dctor
static void myDtorpyMemory(tp_vm *vm,tp_obj self)
{
    char * prefix = (char *)self.data.val;
    ADM_dezalloc(prefix);
    self.data.val = NULL;
}

// Ctor (str)
static tp_obj myCtorpyMemory(tp_vm *vm)
{
    tp_obj self = tp_getraw(vm);
    TinyParams pm(vm);
    const char *p0 = pm.asString();
    char *prefix = ADM_strdup(p0);
    tp_obj cdata = tp_data(vm, ADM_PYID_MEMORY, prefix);
    cdata.data.info->xfree = myDtorpyMemory;
    tp_set(vm, self, tp_string("cdata"), cdata);
    return tp_None;
}

static tp_obj pyMemory_help(TP)
{
    PythonEngine *engine = (PythonEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;

    engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "constructor:\n");
    engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "obj	Memory(str prefix)\n");
    engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "usage example:\n");
    engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "wp = Memory(\"myScript\")\n");
    engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "no = Memory(\"\")\n");
    engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "wp.MyVariable = 2\n");
    engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "print(wp.MyVariable) --> 2\n");
    engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "print(no.myScriptMyVariable) --> 2\n");
    engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "no.myScriptMyVariable = \"example\"\n");
    engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "print(wp.MyVariable) --> \"example\"\n");
    engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "the user defined variables are available during runtime in any script\n");

    return tp_None;
}

tp_obj initClasspyMemory(tp_vm *vm)
{
    tp_obj myClass = tp_class(vm);
    tp_set(vm,myClass, tp_string("__init__"), tp_fnc(vm,myCtorpyMemory));
    tp_set(vm,myClass, tp_string("__set__"), tp_fnc(vm,pyMemory_set));
    tp_set(vm,myClass, tp_string("__get__"), tp_fnc(vm,pyMemory_get));
    tp_set(vm,myClass, tp_string("help"), tp_fnc(vm,pyMemory_help));
    return myClass;
}
