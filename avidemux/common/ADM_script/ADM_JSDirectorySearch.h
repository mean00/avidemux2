#ifndef _ADM_JSDIRECTORYSEARCH_H
#define _ADM_JSDIRECTORYSEARCH_H

#pragma once



// Spidermonkey
#include "ADM_smjs/jsapi.h"
#include <string.h>
#include "DirectorySearch.h"

class ADM_JSDirectorySearch
{
public:
	ADM_JSDirectorySearch(void) : m_pObject(NULL) {}
	virtual ~ADM_JSDirectorySearch(void);

	static JSBool JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc, 
								jsval *argv, jsval *rval);
	static void JSDestructor(JSContext *cx, JSObject *obj);
	static JSObject *JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL);
	static JSBool GetExtension(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool IsExtension(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool NextFile(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool Init(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool GetFileSize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool GetFileDirectory(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool GetFilePath(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool GetFileName(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool Close(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);


	static JSPropertySpec directorysearch_properties[];
	static JSFunctionSpec directorysearch_methods[];
	enum
	{
		is_double_dot_prop,
		is_single_dot_prop,
		is_hidden_prop,
		is_archive_prop,
		is_directory_prop,
		is_not_file_prop
	};
	static JSClass m_classDirectorySearch;

	void setObject(CDirectorySearch *pObject);
	CDirectorySearch *getObject();

private:
	CDirectorySearch *m_pObject;

};

#endif
