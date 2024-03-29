// Generated by admPyClass.pl from binding/pyDFText.admPyClass, DO NOT edit !
tp_obj zzpy__pyDFText_get(tp_vm *vm)
{
  tp_obj self = tp_getraw(vm);
  IScriptEngine *engine = (IScriptEngine*)tp_get(vm, vm->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(vm);
  ADM_scriptDFTextHelper *me = (ADM_scriptDFTextHelper *)pm.asThis(&self, ADM_PYID_DF_TEXT);
  char const *key = pm.asString();
  if (!strcmp(key, "value"))
  {
     if(!me) pm.raise("pyDFText:No this!");
     return tp_string(me->value());
  }
  return tp_get(vm, self, tp_string(key));
}
tp_obj zzpy__pyDFText_set(tp_vm *vm)
{
  tp_obj self = tp_getraw(vm);
  IScriptEngine *engine = (IScriptEngine*)tp_get(vm, vm->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(vm);
  ADM_scriptDFTextHelper *me = (ADM_scriptDFTextHelper *)pm.asThis(&self, ADM_PYID_DF_TEXT);
  char const *key = pm.asString();
  if (!strcmp(key, "value"))
  {
     if(!me) pm.raise("pyDFText:No this!");
     const char * val = pm.asString();
     me->setValue(val);
     return tp_None;
  }
  return tp_None;
}
// Dctor
static void myDtorpyDFText(tp_vm *vm,tp_obj self)
{
  ADM_scriptDFTextHelper *cookie = (ADM_scriptDFTextHelper *)self.data.val;
  if (cookie) delete cookie;
  self.data.val = NULL;
}
// Ctor (str)
static tp_obj myCtorpyDFText(tp_vm *vm)
{
  tp_obj self = tp_getraw(vm);
  TinyParams pm(vm);
  const char *p0 = pm.asString();
  ADM_scriptDFTextHelper *me = new ADM_scriptDFTextHelper(p0);
  tp_obj cdata = tp_data(vm, ADM_PYID_DF_TEXT, me);
  cdata.data.info->xfree = myDtorpyDFText;
  tp_set(vm, self, tp_string("cdata"), cdata);
  return tp_None;
}
static tp_obj zzpy__pyDFText_help(TP)
{
  PythonEngine *engine = (PythonEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;

  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "constructor:\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "obj	DFText(str title)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "variables:\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "str\t value\n");

  return tp_None;
}
tp_obj initClasspyDFText(tp_vm *vm)
{
  tp_obj myClass = tp_class(vm);
  tp_set(vm,myClass, tp_string("__init__"), tp_fnc(vm,myCtorpyDFText));
  tp_set(vm,myClass, tp_string("__set__"), tp_fnc(vm,zzpy__pyDFText_set));
  tp_set(vm,myClass, tp_string("__get__"), tp_fnc(vm,zzpy__pyDFText_get));
  tp_set(vm,myClass, tp_string("help"), tp_fnc(vm,zzpy__pyDFText_help));
  return myClass;
}
