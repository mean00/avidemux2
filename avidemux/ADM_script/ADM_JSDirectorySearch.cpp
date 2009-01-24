// C++ Interface: Spider Monkey interface
//
// Description: 
//
//
// Author: Anish Mistry
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"
#include "ADM_JSDirectorySearch.h"
#include "DirectorySearch.h"

JSPropertySpec ADM_JSDirectorySearch::directorysearch_properties[] = 
{ 
	{ "isDoubleDot", is_double_dot_prop, JSPROP_ENUMERATE },	// .
	{ "isSingleDot", is_single_dot_prop, JSPROP_ENUMERATE },	// ..
	{ "isHidden", is_hidden_prop, JSPROP_ENUMERATE },	// hidden file
	{ "isArchive", is_archive_prop, JSPROP_ENUMERATE },	// archived file
	{ "isDirectory", is_directory_prop, JSPROP_ENUMERATE },	// directory
	{ "isNotFile", is_not_file_prop, JSPROP_ENUMERATE },	// not a file
	{ 0 }
};

JSFunctionSpec ADM_JSDirectorySearch::directorysearch_methods[] =
{
	{ "Init", Init, 1, 0, 0 },	// Initialize search
	{ "Close", Close, 0, 0, 0 },	// Close the search
	{ "GetFileName", GetFileName, 0, 0, 0 },	// return the current file name
	{ "GetFileDirectory", GetFileDirectory, 0, 0, 0 },	// returns the directory of the file
	{ "GetFilePath", GetFilePath, 0, 0, 0 },	// returns the full path of the file
	{ "NextFile", NextFile, 0, 0, 0 },	// Advance search to the next file
	{ "GetFileSize", GetFileSize, 0, 0, 0 },	// Returns the size of the file in bytes
	{ "GetExtension", GetExtension, 0, 0, 0 },	// Returns the extension of the current file
	{ "IsExtension", IsExtension, 1, 0, 0 },	// Compares the first argument string with the current extension and returns true on a match, false otherwise
	{ 0 }
};

JSClass ADM_JSDirectorySearch::m_classDirectorySearch =
{
	"DirectorySearch", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub,
	ADM_JSDirectorySearch::JSGetProperty, ADM_JSDirectorySearch::JSSetProperty,
	JS_EnumerateStub, JS_ResolveStub,
	JS_ConvertStub, ADM_JSDirectorySearch::JSDestructor
};

ADM_JSDirectorySearch::~ADM_JSDirectorySearch(void)
{
	if(m_pObject != NULL)
		delete m_pObject;
	m_pObject = NULL;
}

void ADM_JSDirectorySearch::setObject(CDirectorySearch *pObject)
{
	m_pObject = pObject; 
}
	
CDirectorySearch *ADM_JSDirectorySearch::getObject()
{
	return m_pObject; 
}

JSObject *ADM_JSDirectorySearch::JSInit(JSContext *cx, JSObject *obj, JSObject *proto)
{
	JSObject *newObj = JS_InitClass(cx, obj, proto, &m_classDirectorySearch, 
									ADM_JSDirectorySearch::JSConstructor, 0,
									ADM_JSDirectorySearch::directorysearch_properties, ADM_JSDirectorySearch::directorysearch_methods,
									NULL, NULL);
	return newObj;
}

JSBool ADM_JSDirectorySearch::JSConstructor(JSContext *cx, JSObject *obj, uintN argc, 
								 jsval *argv, jsval *rval)
{
	if(argc != 0)
		return JS_FALSE;
	ADM_JSDirectorySearch *p = new ADM_JSDirectorySearch();
	CDirectorySearch *pObject = new CDirectorySearch();
	p->setObject(pObject);
	if ( ! JS_SetPrivate(cx, obj, p) )
		return JS_FALSE;
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}

void ADM_JSDirectorySearch::JSDestructor(JSContext *cx, JSObject *obj)
{
	ADM_JSDirectorySearch *p = (ADM_JSDirectorySearch *)JS_GetPrivate(cx, obj);
	if(p != NULL)
		delete p;
	p = NULL;
}

JSBool ADM_JSDirectorySearch::JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	if (JSVAL_IS_INT(id)) 
	{
		ADM_JSDirectorySearch *priv = (ADM_JSDirectorySearch *) JS_GetPrivate(cx, obj);
		switch(JSVAL_TO_INT(id))
		{
			case is_double_dot_prop:
				*vp = INT_TO_JSVAL(priv->getObject()->IsDoubleDot());
				break;
			case is_single_dot_prop:
				*vp = INT_TO_JSVAL(priv->getObject()->IsSingleDot());
				break;
			case is_hidden_prop:
				*vp = INT_TO_JSVAL(priv->getObject()->IsHidden());
				break;
			case is_archive_prop:
				*vp = INT_TO_JSVAL(priv->getObject()->IsArchive());
				break;
			case is_directory_prop:
				*vp = INT_TO_JSVAL(priv->getObject()->IsDirectory());
				break;
			case is_not_file_prop:
				*vp = INT_TO_JSVAL(priv->getObject()->IsNotFile());
				break;
		}
	}
	return JS_TRUE;
}

JSBool ADM_JSDirectorySearch::JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	if (JSVAL_IS_INT(id)) 
	{
		ADM_JSDirectorySearch *priv = (ADM_JSDirectorySearch *) JS_GetPrivate(cx, obj);
		switch(JSVAL_TO_INT(id))
		{
			case is_double_dot_prop:
			case is_single_dot_prop:
			case is_hidden_prop:
			case is_archive_prop:
			case is_directory_prop:
			case is_not_file_prop:
				return JS_FALSE;
				break;
		}
	}
	return JS_TRUE;
}

JSBool ADM_JSDirectorySearch::GetExtension(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin GetExtension
	ADM_JSDirectorySearch *p = (ADM_JSDirectorySearch *)JS_GetPrivate(cx, obj);
	// default return value
	*rval = BOOLEAN_TO_JSVAL(false);
	if(argc != 0)
		return JS_FALSE;
	*rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,p->getObject()->GetExtension()));
	return JS_TRUE;
}// end GetExtension

JSBool ADM_JSDirectorySearch::IsExtension(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin IsExtension
	ADM_JSDirectorySearch *p = (ADM_JSDirectorySearch *)JS_GetPrivate(cx, obj);
	// default return value
	*rval = BOOLEAN_TO_JSVAL(false);
	if(argc != 1)
		return JS_FALSE;
	if(JSVAL_IS_STRING(argv[0]) == false)
		return JS_FALSE;
	*rval = BOOLEAN_TO_JSVAL(p->getObject()->IsExtension(JS_GetStringBytes(JSVAL_TO_STRING(argv[0]))));
	return JS_TRUE;
}// end IsExtension

JSBool ADM_JSDirectorySearch::NextFile(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin NextFile
	ADM_JSDirectorySearch *p = (ADM_JSDirectorySearch *)JS_GetPrivate(cx, obj);
	// default return value
	*rval = BOOLEAN_TO_JSVAL(false);
	if(argc != 0)
		return JS_FALSE;
	*rval = BOOLEAN_TO_JSVAL(p->getObject()->NextFile());
	return JS_TRUE;
}// end NextFile

JSBool ADM_JSDirectorySearch::Init(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin Init
	ADM_JSDirectorySearch *p = (ADM_JSDirectorySearch *)JS_GetPrivate(cx, obj);
	// default return value
	*rval = BOOLEAN_TO_JSVAL(false);
	if(argc != 1)
		return JS_FALSE;
	if(JSVAL_IS_STRING(argv[0]) == false)
		return JS_FALSE;
	*rval = BOOLEAN_TO_JSVAL(p->getObject()->Init(JS_GetStringBytes(JSVAL_TO_STRING(argv[0]))));
	return JS_TRUE;
}// end Init


JSBool ADM_JSDirectorySearch::GetFileSize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin GetFileSize
	ADM_JSDirectorySearch *p = (ADM_JSDirectorySearch *)JS_GetPrivate(cx, obj);
	// default return value
	*rval = INT_TO_JSVAL(0);
	if(argc != 0)
		return JS_FALSE;
	*rval = INT_TO_JSVAL(p->getObject()->GetFileSize());
	return JS_TRUE;
}// end GetFileSize

JSBool ADM_JSDirectorySearch::GetFileDirectory(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin GetFileDirectory
	ADM_JSDirectorySearch *p = (ADM_JSDirectorySearch *)JS_GetPrivate(cx, obj);
	// default return value
	*rval = BOOLEAN_TO_JSVAL(false);
	if(argc != 0)
		return JS_FALSE;
	*rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,p->getObject()->GetFileDirectory().c_str()));
	return JS_TRUE;
}// end GetFileDirectory

JSBool ADM_JSDirectorySearch::GetFilePath(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin GetFilePath
	ADM_JSDirectorySearch *p = (ADM_JSDirectorySearch *)JS_GetPrivate(cx, obj);
	// default return value
	*rval = BOOLEAN_TO_JSVAL(false);
	if(argc != 0)
		return JS_FALSE;
	*rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,p->getObject()->GetFilePath().c_str()));
	return JS_TRUE;
}// end GetFilePath

JSBool ADM_JSDirectorySearch::GetFileName(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin GetFileName
	ADM_JSDirectorySearch *p = (ADM_JSDirectorySearch *)JS_GetPrivate(cx, obj);
	// default return value
	*rval = BOOLEAN_TO_JSVAL(false);
	if(argc != 0)
		return JS_FALSE;
	*rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,p->getObject()->GetFileName()));
	return JS_TRUE;
}// end GetFileName

JSBool ADM_JSDirectorySearch::Close(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin Close
	ADM_JSDirectorySearch *p = (ADM_JSDirectorySearch *)JS_GetPrivate(cx, obj);
	// default return value
	*rval = BOOLEAN_TO_JSVAL(false);
	if(argc != 0)
		return JS_FALSE;
	*rval = BOOLEAN_TO_JSVAL(p->getObject()->Close());
	return JS_TRUE;
}// end Close
