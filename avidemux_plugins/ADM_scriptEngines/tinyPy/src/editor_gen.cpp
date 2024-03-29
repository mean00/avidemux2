// Generated by admPyClass.pl from binding/editor.admPyClass, DO NOT edit !
// dumpAllSegments -> void editor->dumpSegments(void)
static tp_obj zzpy_dumpAllSegments(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  editor->dumpSegments();
  return tp_None;
}
// dumpRefVideo -> int editor->dumpRefVideos(void)
static tp_obj zzpy_dumpRefVideo(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  int r = editor->dumpRefVideos();
  return tp_number(r);
}
// dumpSegment -> void editor->dumpSegment(int)
static tp_obj zzpy_dumpSegment(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  int p0 = pm.asInt();
  editor->dumpSegment(p0);
  return tp_None;
}
// getCurrentFlags -> int pyGetCurrentFrameFlags(IEditor)
static tp_obj zzpy_getCurrentFlags(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  int r = pyGetCurrentFrameFlags(p0);
  return tp_number(r);
}
// getCurrentPts -> double editor->getCurrentFramePts(void)
static tp_obj zzpy_getCurrentPts(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  double r = editor->getCurrentFramePts();
  return tp_number(r);
}
// getDts -> double pyGetDts(IEditor int)
static tp_obj zzpy_getDts(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  int p1 = pm.asInt();
  double r = pyGetDts(p0, p1);
  return tp_number(r);
}
// getDurationForSegment -> double pySegmentGetDuration(IEditor int)
static tp_obj zzpy_getDurationForSegment(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  int p1 = pm.asInt();
  double r = pySegmentGetDuration(p0, p1);
  return tp_number(r);
}
// getFrameSize -> int editor->getFrameSize(int)
static tp_obj zzpy_getFrameSize(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  int p0 = pm.asInt();
  int r = editor->getFrameSize(p0);
  return tp_number(r);
}
// getNextKFramePts -> double pyGetNextKFramePts(IEditor double)
static tp_obj zzpy_getNextKFramePts(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  double p1 = pm.asDouble();
  double r = pyGetNextKFramePts(p0, p1);
  return tp_number(r);
}
// getPrevKFramePts -> double pyGetPrevKFramePts(IEditor double)
static tp_obj zzpy_getPrevKFramePts(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  double p1 = pm.asDouble();
  double r = pyGetPrevKFramePts(p0, p1);
  return tp_number(r);
}
// getPts -> double pyGetPts(IEditor int)
static tp_obj zzpy_getPts(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  int p1 = pm.asInt();
  double r = pyGetPts(p0, p1);
  return tp_number(r);
}
// getRefIdxForSegment -> int pySegmentGetRefIdx(IEditor int)
static tp_obj zzpy_getRefIdxForSegment(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  int p1 = pm.asInt();
  int r = pySegmentGetRefIdx(p0, p1);
  return tp_number(r);
}
// getRefVideoDuration -> double pyGetRefVideoDuration(IEditor int)
static tp_obj zzpy_getRefVideoDuration(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  int p1 = pm.asInt();
  double r = pyGetRefVideoDuration(p0, p1);
  return tp_number(r);
}
// getRefVideoName -> str pyGetRefVideoName(IEditor int)
static tp_obj zzpy_getRefVideoName(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  int p1 = pm.asInt();
  char *r = pyGetRefVideoName(p0, p1);
  if(!r) return tp_None;

  tp_obj o = tp_string_copy(tp, r, strlen(r));
  ADM_dealloc(r);
  return o;
}
// getTimeOffsetForSegment -> double pySegmentGetTimeOffset(IEditor int)
static tp_obj zzpy_getTimeOffsetForSegment(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  int p1 = pm.asInt();
  double r = pySegmentGetTimeOffset(p0, p1);
  return tp_number(r);
}
// getVideoDuration -> double editor->getVideoDuration(void)
static tp_obj zzpy_getVideoDuration(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  double r = editor->getVideoDuration();
  return tp_number(r);
}
// hexDumpFrame -> int pyHexDumpFrame(IEditor int)
static tp_obj zzpy_hexDumpFrame(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  int p1 = pm.asInt();
  int r = pyHexDumpFrame(p0, p1);
  return tp_number(r);
}
// nbSegments -> int editor->getNbSegment(void)
static tp_obj zzpy_nbSegments(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  int r = editor->getNbSegment();
  return tp_number(r);
}
// nbVideos -> int editor->getVideoCount(void)
static tp_obj zzpy_nbVideos(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  int r = editor->getVideoCount();
  return tp_number(r);
}
// nextFrame -> int pyNextFrame(IEditor)
static tp_obj zzpy_nextFrame(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  int r = pyNextFrame(p0);
  return tp_number(r);
}
// printFrameInfo -> int pyPrintFrameInfo(IEditor int)
static tp_obj zzpy_printFrameInfo(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  int p1 = pm.asInt();
  int r = pyPrintFrameInfo(p0, p1);
  return tp_number(r);
}
// printTiming -> int pyPrintTiming(IEditor int)
static tp_obj zzpy_printTiming(TP)
{
  tp_obj self = tp_getraw(tp);
  IScriptEngine *engine = (IScriptEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(tp);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);

  IEditor *p0 = editor;
  int p1 = pm.asInt();
  int r = pyPrintTiming(p0, p1);
  return tp_number(r);
}
tp_obj zzpy__pyEditor_get(tp_vm *vm)
{
  tp_obj self = tp_getraw(vm);
  IScriptEngine *engine = (IScriptEngine*)tp_get(vm, vm->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(vm);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);
  char const *key = pm.asString();
  if (!strcmp(key, "dumpAllSegments"))
  {
    return tp_method(vm, self, zzpy_dumpAllSegments);
  }
  if (!strcmp(key, "dumpRefVideo"))
  {
    return tp_method(vm, self, zzpy_dumpRefVideo);
  }
  if (!strcmp(key, "dumpSegment"))
  {
    return tp_method(vm, self, zzpy_dumpSegment);
  }
  if (!strcmp(key, "getCurrentFlags"))
  {
    return tp_method(vm, self, zzpy_getCurrentFlags);
  }
  if (!strcmp(key, "getCurrentPts"))
  {
    return tp_method(vm, self, zzpy_getCurrentPts);
  }
  if (!strcmp(key, "getDts"))
  {
    return tp_method(vm, self, zzpy_getDts);
  }
  if (!strcmp(key, "getDurationForSegment"))
  {
    return tp_method(vm, self, zzpy_getDurationForSegment);
  }
  if (!strcmp(key, "getFrameSize"))
  {
    return tp_method(vm, self, zzpy_getFrameSize);
  }
  if (!strcmp(key, "getNextKFramePts"))
  {
    return tp_method(vm, self, zzpy_getNextKFramePts);
  }
  if (!strcmp(key, "getPrevKFramePts"))
  {
    return tp_method(vm, self, zzpy_getPrevKFramePts);
  }
  if (!strcmp(key, "getPts"))
  {
    return tp_method(vm, self, zzpy_getPts);
  }
  if (!strcmp(key, "getRefIdxForSegment"))
  {
    return tp_method(vm, self, zzpy_getRefIdxForSegment);
  }
  if (!strcmp(key, "getRefVideoDuration"))
  {
    return tp_method(vm, self, zzpy_getRefVideoDuration);
  }
  if (!strcmp(key, "getRefVideoName"))
  {
    return tp_method(vm, self, zzpy_getRefVideoName);
  }
  if (!strcmp(key, "getTimeOffsetForSegment"))
  {
    return tp_method(vm, self, zzpy_getTimeOffsetForSegment);
  }
  if (!strcmp(key, "getVideoDuration"))
  {
    return tp_method(vm, self, zzpy_getVideoDuration);
  }
  if (!strcmp(key, "hexDumpFrame"))
  {
    return tp_method(vm, self, zzpy_hexDumpFrame);
  }
  if (!strcmp(key, "nbSegments"))
  {
    return tp_method(vm, self, zzpy_nbSegments);
  }
  if (!strcmp(key, "nbVideos"))
  {
    return tp_method(vm, self, zzpy_nbVideos);
  }
  if (!strcmp(key, "nextFrame"))
  {
    return tp_method(vm, self, zzpy_nextFrame);
  }
  if (!strcmp(key, "printFrameInfo"))
  {
    return tp_method(vm, self, zzpy_printFrameInfo);
  }
  if (!strcmp(key, "printTiming"))
  {
    return tp_method(vm, self, zzpy_printTiming);
  }
  return tp_get(vm, self, tp_string(key));
}
tp_obj zzpy__pyEditor_set(tp_vm *vm)
{
  tp_obj self = tp_getraw(vm);
  IScriptEngine *engine = (IScriptEngine*)tp_get(vm, vm->builtins, tp_string("userdata")).data.val;
  IEditor *editor = engine->editor();
  TinyParams pm(vm);
  void *me = (void *)pm.asThis(&self, ADM_PYID_EDITOR);
  char const *key = pm.asString();
  return tp_None;
}
// Dctor
static void myDtorpyEditor(tp_vm *vm,tp_obj self)
{
}
// Ctor ()
static tp_obj myCtorpyEditor(tp_vm *vm)
{
  tp_obj self = tp_getraw(vm);
  TinyParams pm(vm);
  void *me = NULL;
  tp_obj cdata = tp_data(vm, ADM_PYID_EDITOR, me);
  cdata.data.info->xfree = myDtorpyEditor;
  tp_set(vm, self, tp_string("cdata"), cdata);
  return tp_None;
}
static tp_obj zzpy__pyEditor_help(TP)
{
  PythonEngine *engine = (PythonEngine*)tp_get(tp, tp->builtins, tp_string("userdata")).data.val;

  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "constructor:\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "obj	Editor()\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "methods:\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "void\t dumpAllSegments()\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "int\t dumpRefVideo()\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "void\t dumpSegment(int segment)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "int\t getCurrentFlags()\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "double\t getCurrentPts()\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "double\t getDts(int frameNum)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "double\t getDurationForSegment(int segment)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "int\t getFrameSize(int frameNum)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "double\t getNextKFramePts(double time)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "double\t getPrevKFramePts(double time)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "double\t getPts(int frameNum)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "int\t getRefIdxForSegment(int segment)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "double\t getRefVideoDuration(int videoIndex)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "str\t getRefVideoName(int videoIndex)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "double\t getTimeOffsetForSegment(int segment)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "double\t getVideoDuration()\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "int\t hexDumpFrame(int frameNum)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "int\t nbSegments()\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "int\t nbVideos()\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "int\t nextFrame()\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "int\t printFrameInfo(int frameNum)\n");
  engine->callEventHandlers(IScriptEngine::Information, NULL, -1, "int\t printTiming(int frameNum)\n");

  return tp_None;
}
tp_obj initClasspyEditor(tp_vm *vm)
{
  tp_obj myClass = tp_class(vm);
  tp_set(vm,myClass, tp_string("__init__"), tp_fnc(vm,myCtorpyEditor));
  tp_set(vm,myClass, tp_string("__set__"), tp_fnc(vm,zzpy__pyEditor_set));
  tp_set(vm,myClass, tp_string("__get__"), tp_fnc(vm,zzpy__pyEditor_get));
  tp_set(vm,myClass, tp_string("help"), tp_fnc(vm,zzpy__pyEditor_help));
  return myClass;
}
