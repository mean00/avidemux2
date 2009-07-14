// C++ Interface: Spider Monkey interface
//
// Description: 
//
//
// Author: Anish Mistry
//      Some modification by mean
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ADM_default.h"
#include <sys/types.h>
#include <sys/stat.h>
#if defined( ADM_WIN32 ) || defined(__MINGW32__)
#define ADM_JS_WIN32
#endif

#ifndef ADM_JS_WIN32
#include <sys/wait.h>
#include <sys/param.h>
#endif
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <math.h>
#include <vector>
#include <string>
#include "ADM_JSAvidemux.h"
#include "ADM_JSGlobal.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "ADM_editor/ADM_outputfmt.h"
#include "adm_scanner.h" 
#include "avi_vars.h"
#include "gui_action.hxx"
#include "ADM_videoFilter.h"
#include "ADM_editor/ADM_outputfmt.h"

#include "ADM_script/ADM_container.h"

#include "ADM_JSGlobal.h"
#include "DIA_fileSel.h"

#include "DIA_factory.h"

std::vector <std::string> g_vIncludes;
extern char **environ;
extern char *script_getVar(char *in, int *r);

JSBool displayError(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool displayInfo(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool fileWriteSelect(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool dirSelect(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool fileReadSelect(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool print(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool allFilesFrom(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool nextFile(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool setSuccess(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval);
JSBool getVar(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval);
JSBool systemExecute(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool systemInclude(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool pathOnly(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JSBool facInt(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facFloat(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facToggle(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facMenu(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facFile(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facBitrate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facBar(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facRoText(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facText(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facTab(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facFrame(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facHex(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facDirSel(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facButton(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facMatrix(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facNotch(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facThreadCount(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool facSlider(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool crashTest(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool assertTest(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);


static JSFunctionSpec adm_functions[] = {
  /*    name          native          nargs    */
  {"displayError",      displayError,         1},
  {"displayInfo",       displayInfo,        1},
  {"fileReadSelect",    fileReadSelect,        0},
  {"fileWriteSelect",   fileWriteSelect,        0},
  {"dirSelect",         dirSelect,        0},
  {"print",             print,        1},
  {"allFilesFrom",      allFilesFrom,        0},
  {"nextFile",          nextFile,        0},
  {"setSuccess",          setSuccess,        1},
  {"getVar",          getVar,        1},
  {"exec",          systemExecute,        3},
  {"include",          systemInclude,        1},
  {"pathOnly",          pathOnly,        1},
  {"dialogFactoryInt",          facInt,           0},
  {"dialogFactoryFloat",        facFloat,         0},
  {"dialogFactoryToggle",       facToggle,        0},
  {"dialogFactoryMenu",         facMenu,          0},
  {"dialogFactoryFileSel",      facFile,          0},
  {"dialogFactoryBitrate",      facBitrate,       0},
  {"dialogFactoryBar",        facBar,             0},
  {"dialogFactoryRoText",     facRoText,          0},
  {"dialogFactoryText",       facText,            0},
  {"dialogFactoryTabs",       facTab,             0},
  {"dialogFactoryDirSel",       facDirSel,        0},
  {"dialogFactoryFrame",       facFrame,          0},
  {"dialogFactoryHex",       facHex,              0},
  {"dialogFactoryButton",       facButton,        0},
  {"dialogFactorySlider",       facSlider,        0},
  {"dialogFactoryMatrix",       facMatrix,        0},
  {"dialogFactoryNotch",       facNotch,		  0},
  {"dialogFactoryThreadCount", facThreadCount,		  0},
  {"crashTest",               crashTest,          0},
  {"assertTest",               assertTest,        0},
  {0}
};

uint8_t JS_AvidemuxFunction(JSContext *cx,JSObject *global)
{
	if(JS_DefineFunctions(cx, global, adm_functions) == true)
		return 1;

	printf("JSAvidemuxFunction: Unable to define functions\n");
	return 0;
}

JSBool getVar(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin getVar
        int out=0;
        char *dupe=NULL;

        // default return value
        if(argc != 1)
                return JS_FALSE;
	if(JSVAL_IS_STRING(argv[0]) == false)
		return JS_FALSE;
	char  *stringa = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	dupe=script_getVar(stringa ,&out);

	if(!dupe)
		return JS_FALSE;
	// if out=1 it is a string else a number
	if(out)
		*rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,dupe));
	else
		*rval = INT_TO_JSVAL(atoi(dupe));
	return JS_TRUE;
}// end getVar

JSBool setSuccess(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin setSuccess
	bool Jscript_succeed = false;
	// default return value
	if(argc != 1)
		return JS_FALSE;
	if(JSVAL_IS_BOOLEAN(argv[0]) == false)
		return JS_FALSE;
	Jscript_succeed = JSVAL_TO_BOOLEAN(argv[0]);
	JS_setSuccess(Jscript_succeed);

	return JS_TRUE;
}// end setSuccess

JSBool displayError(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin displayError
	// default return value
	if(argc != 1)
		return JS_FALSE;
	if(JSVAL_IS_STRING(argv[0]) == false)
		return JS_FALSE;
	char  *stringa = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	GUI_Verbose();
	GUI_Error_HIG("Error",stringa);
	GUI_Quiet();

	return JS_TRUE;
}// end displayError
JSBool displayInfo(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin displayInfo
	// default return value
	if(argc != 1)
		return JS_FALSE;
	if(JSVAL_IS_STRING(argv[0]) == false)
		return JS_FALSE;
	char  *stringa = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	GUI_Verbose();
	GUI_Info_HIG(ADM_LOG_IMPORTANT,"Info",stringa);
	GUI_Quiet();
	return JS_TRUE;
}// end displayInfo

JSBool fileReadSelect(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin fileReadSelect
        char *name;
        // default return value
        if(argc != 0)
                return JS_FALSE;
        if(!name) return JS_FALSE;
        GUI_FileSelRead("Open file (Read mode)", &name);
        *rval=STRING_TO_JSVAL(JS_NewStringCopyZ(cx,name));
        ADM_dealloc(name);
        return JS_TRUE;
}// end fileReadSelect

JSBool fileWriteSelect(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin fileWriteSelect
        char *name;
        // default return value
        if(argc != 0)
                return JS_FALSE;
        GUI_FileSelWrite("Open file (Write mode)", &name);
        if(!name) return JS_FALSE;
        *rval=STRING_TO_JSVAL(JS_NewStringCopyZ(cx,name));
        ADM_dealloc(name);
        return JS_TRUE;
}// end fileWriteSelect
JSBool dirSelect(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{
        char name[1024];
        // default return value
        if(argc != 0)
                return JS_FALSE;
        if(!FileSel_SelectDir(QT_TR_NOOP("Select a directory"),name,1023, NULL))
         return JS_FALSE;
        *rval=STRING_TO_JSVAL(JS_NewStringCopyZ(cx,name));
        return JS_TRUE;
}
JSBool print(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin print
        if(argc != 1)
                return JS_FALSE;
	fprintf(stderr,"JSConsole: %s\n", JS_GetStringBytes(JS_ValueToString(cx, argv[0])));
        return JS_TRUE;
}// end print
/*****************************************************
        To process a whole directiry at a time
*******************************************************/
#define ADM_MAX_DIR 1024
static char *dirs[ADM_MAX_DIR];
static int dirmax=0;
static int curdir=0;
static void cleanup( void );
void cleanup( void )
{
        for(int i=0;i<dirmax;i++)
                ADM_dealloc(dirs[i]);
        dirmax=0;
        curdir=0;
}
JSBool allFilesFrom(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
char *str;
DIR *dir;
struct dirent *direntry;
       // ADM_JSAvidemux *p = (ADM_JSAvidemux *)JS_GetPrivate(cx, obj);
        cleanup();
        // default return value
        if(argc != 1)
                return JS_FALSE;
	if(JSVAL_IS_STRING(argv[0]) == false)
		return JS_FALSE;

        str=JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        dir=opendir(str);
        if(!dir)
        {
                *rval=INT_TO_JSVAL(0); // No files
                return JS_TRUE;
        }
        while((direntry=readdir(dir)) && dirmax<ADM_MAX_DIR-1)
        {
                dirs[dirmax]=(char *)ADM_alloc(strlen(str)+strlen(direntry->d_name)+2);
                strcpy(dirs[dirmax],str);
                strcat(dirs[dirmax],direntry->d_name);
                //printf("File:<%s>\n",dirs[dirmax]);
                dirmax++;
        }
        closedir(dir);
        *rval=INT_TO_JSVAL(dirmax);
        return JS_TRUE;
}

JSBool nextFile(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
char *n;
        if(argc != 0)
                return JS_FALSE;

        if(curdir==dirmax)
        {
                n="";
        }
        else
        {
                n=dirs[curdir++];
        }
        *rval=STRING_TO_JSVAL(JS_NewStringCopyZ(cx,n));
        return JS_TRUE;
}

JSBool systemExecute(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin systemExecute
	// default return value
	*rval = INT_TO_JSVAL(-1);

	if(argc != 3)
		return JS_FALSE;
	if(JSVAL_IS_STRING(argv[0]) == false || JSVAL_IS_OBJECT(argv[1]) == false || JSVAL_IS_BOOLEAN(argv[2]) == false)
		return JS_FALSE;

	char *pExecutable = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	JSObject *pArgs = JSVAL_TO_OBJECT(argv[1]);
	bool bWait = JSVAL_TO_BOOLEAN(argv[2]);
	int status = 0;
	jsuint nArgsLength = 0;
	jsval jsValue;
	struct stat sbFileInfo;

	if(JS_IsArrayObject(cx, pArgs) == false)
		return JS_FALSE;

	JS_GetArrayLength(cx,pArgs,&nArgsLength);
	char **args = new char *[nArgsLength + 2];
	args[0] = pExecutable;
	args[nArgsLength + 1] = NULL;

	for(int i = 0; i < nArgsLength; i++)
	{
		if(JS_GetElement(cx, pArgs, i, &jsValue) == JS_FALSE)
		{// begin failure to get item
			JS_ReportError(cx, "exec() JS_GetElement failed to get an array item.");
			return JS_FALSE;
		}// end failure to get item
		args[i + 1] = JS_GetStringBytes(JSVAL_TO_STRING(jsValue));
	}

#ifndef ADM_JS_WIN32
	if(getuid() == 0)
	{// begin running as root
		JS_ReportError(cx, "exec() disallowed while running as root.");
		return JS_FALSE;
	}// end running as root
	if(stat(pExecutable , &sbFileInfo) != 0)
	{// begin can't stat file
		JS_ReportError(cx, "exec() Can't stat \"%s\" errno(%i).", pExecutable, errno);
		return JS_FALSE;
	}// end can't stat file
	if((sbFileInfo.st_mode & S_ISUID) == S_ISUID || (sbFileInfo.st_mode & S_ISGID) == S_ISGID)
	{// begin setuid/setgid files disallowed
		JS_ReportError(cx, "exec() disallowed execution of \"%s\" since it is a setuid/setgid file.", pExecutable);
		return JS_FALSE;
	}// end setuid/setgid files disallowed
#endif

        enterLock();
	// clear file descriptor table of forked process and fork
#if defined( __linux__) || defined(__macosx__) || defined(__APPLE__) || defined(__CYGWIN__)
	pid_t pidRtn = fork();
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	pid_t pidRtn = rfork(RFPROC|RFCFDG);
#endif

#ifdef ADM_JS_WIN32
	intptr_t pidRtn = spawnvpe(bWait ? P_WAIT : P_NOWAIT, pExecutable, args, environ);
	#define WEXITSTATUS
#else
	if(pidRtn == 0)
	{// begin child process
#if defined( __linux__) || defined(__macosx__) || defined(__APPLE__) || defined(__CYGWIN__)
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
#endif
		char **pEnv = environ;
		//char *pEnv[] = {NULL};
		execve(pExecutable,args,pEnv);
		printf("Error: execve failure errno(%d)\n",errno);
		_exit(errno);
	}// end child process
	else if(bWait && pidRtn != -1)
	{// begin wait for execution to finish
		printf("Waiting on pid %d...",pidRtn);
		do
		{// begin wait for child
			waitpid(pidRtn,&status,WUNTRACED);
		}// end wait for child
		while(WIFEXITED(status) == false && WIFSIGNALED(status) == false);
		printf("Done...\n");
	}// end wait for execution to finish
	else if(pidRtn == -1)
	{// begin rfork failure
		printf("Error: execve failure errno(%d)\n",errno);
	}// end rfork failure
#endif

        leaveLock();

	// cleanup
	delete []args;
	if(pidRtn != -1)
		*rval = INT_TO_JSVAL(WEXITSTATUS(status));	// success return child's exit status
	else
		*rval = INT_TO_JSVAL(-1);	// failure
	return JS_TRUE;
}// end systemExecute
JSBool systemInclude(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin systemInclude
	// default return value
	*rval = BOOLEAN_TO_JSVAL(false);
	if(argc != 1)
		return JS_FALSE;
	if(JSVAL_IS_STRING(argv[0]) == false)
		return JS_FALSE;

	struct stat sbFileInfo;
	const char *pIncludeFile = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	// make sure we haven't included this already to avoid a recursive
	// dependency loop
	char *pTempStr = new char[PATH_MAX+1];
 #ifdef __MINGW32__
 	if(_fullpath(pTempStr,pIncludeFile,PATH_MAX) == NULL)
 #else
	if(realpath(pIncludeFile,pTempStr) == NULL)
#endif
	{// begin can't resolve path
		JS_ReportError(cx, "include() can't resolve the path of \"%s\".", pIncludeFile);
		return JS_FALSE;
	}// end can't resolve path

	std::string sRealPath = pTempStr;
	if(stat(sRealPath.c_str() , &sbFileInfo) != 0)
	{// begin can't stat file
		JS_ReportError(cx, "include() Can't stat \"%s\" errno(%i).", sRealPath.c_str(), errno);
		return JS_FALSE;
	}// end can't stat file

	for(int i = 0;i < g_vIncludes.size();i++)
	{// begin check previous includes
		if(g_vIncludes[i] == sRealPath)
		{// begin found
			printf("include() Warning: Duplicated include of \"%s\"...ignoring.\n",sRealPath.c_str());
			return JS_TRUE;
		}// end found
	}// end check previous includes
	g_vIncludes.push_back(sRealPath);

	JSScript *pJSScript = JS_CompileFile(cx, obj, sRealPath.c_str());
	jsval lastRval;
	if(pJSScript != NULL)
	{// begin execute external file
		JSBool ok = JS_ExecuteScript(cx, obj, pJSScript, &lastRval);
		JS_DestroyScript(cx,pJSScript);
		*rval = BOOLEAN_TO_JSVAL(ok);
	}// end execute external file
	else
	{// begin error including
		JS_ReportError(cx, "include() Cannot compile file \"%s\"", pIncludeFile);
		return JS_FALSE;
	}// end error including
	return JS_TRUE;
}// end systemInclude

/********************** Extract Path from a filename **************/
JSBool pathOnly(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin systemExecute
	// default return value
  *rval = INT_TO_JSVAL(-1);

  if(argc != 1)
    return JS_FALSE;
  if(JSVAL_IS_STRING(argv[0]) == false )
    return JS_FALSE;
  char *name=NULL;
  char *orgName = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
  ADM_PathStripName(orgName);
  /* Remove last / or last \ */
  int l=strlen(orgName);
  if(l) orgName[l-1]=0;
  *rval=STRING_TO_JSVAL(JS_NewStringCopyZ(cx,orgName));
  return JS_TRUE;
}// end systemExecute

JSBool crashTest(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  
  int *foobar=NULL;
  *foobar=0; // CRASH!
  return JS_TRUE;
}
JSBool assertTest(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  
  ADM_assert(0);
  return JS_TRUE;
}

//EOF 
